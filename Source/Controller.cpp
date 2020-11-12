/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file is part of the Soundscape VST, AU, and AAX Plug-in.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY d&b audiotechnik GmbH & Co. KG "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================
*/


#include "Controller.h"

#include "PagedUI/PageComponentManager.h"
#include "PagedUI/PageContainerComponent.h"
#include "SoundsourceProcessor/SoundsourceProcessor.h"


namespace SoundscapeBridgeApp
{


static constexpr int PROTOCOL_INTERVAL_MIN = 20;		//< Minimum supported OSC messaging rate in milliseconds
static constexpr int PROTOCOL_INTERVAL_MAX = 5000;	//< Maximum supported OSC messaging rate in milliseconds
static constexpr int PROTOCOL_INTERVAL_DEF = 100;		//< Default OSC messaging rate in milliseconds

static constexpr int KEEPALIVE_TIMEOUT = 5000;	//< Milliseconds without response after which we consider processor "Offline"
static constexpr int KEEPALIVE_INTERVAL = 1500;	//< Interval at which keepalive (ping) messages are sent, in milliseconds
static constexpr int MAX_HEARTBEAT_COUNT = 0xFFFF;	//< No point counting beyond this number.


/**
 * Pre-defined OSC command and response strings
 */
static const String kOscDelimiterString("/");
static const String kOscCommandString_ping("/ping");
static const String kOscCommandString_source_position_xy("/dbaudio1/coordinatemapping/source_position_xy/%d/%d");
static const String kOscCommandString_reverbsendgain("/dbaudio1/matrixinput/reverbsendgain/%d");
static const String kOscCommandString_source_spread("/dbaudio1/positioning/source_spread/%d");
static const String kOscCommandString_source_delaymode("/dbaudio1/positioning/source_delaymode/%d");
static const String kOscResponseString_pong("/pong");
static const String kOscResponseString_source_position_xy("/dbaudio1/coordinatemapping/source_position_xy");
static const String kOscResponseString_reverbsendgain("/dbaudio1/matrixinput/reverbsendgain");
static const String kOscResponseString_source_spread("/dbaudio1/positioning/source_spread");
static const String kOscResponseString_source_delaymode("/dbaudio1/positioning/source_delaymode");


/*
===============================================================================
 Class CController
===============================================================================
*/

/**
 * The one and only instance of CController.
 */
CController* CController::m_singleton = nullptr;

/**
 * Constructs an CController object.
 * There can be only one instance of this class, see m_singleton. This is so that network traffic
 * is managed from a central point and only one UDP port is opened for all OSC communication.
 */
CController::CController()
{
	jassert(!m_singleton);	// only one instnce allowed!!
	m_singleton = this;

	// Clear all changed flags initially
	for (int cs = 0; cs < DCS_Max; cs++)
		m_parametersChanged[cs] = DCT_None;

	// CController derives from ProcessingEngineNode::Listener
	AddProtocolBridgingWrapperListener(this);

	// Default OSC server settings. These might become overwritten 
	// by setStateInformation()
	SetRate(DCS_Init, PROTOCOL_INTERVAL_DEF, true);
	SetIpAddress(DCS_Init, PROTOCOL_DEFAULT_IP, true);
}

/**
 * Destroys the CController.
 */
CController::~CController()
{
	stopTimer();
	Disconnect();

	// Destroy overView window and overView Manager
	PageComponentManager* pageMgr = PageComponentManager::GetInstance();
	if (pageMgr)
		pageMgr->ClosePageContainer(true);

	const ScopedLock lock(m_mutex);
	m_processors.clearQuick();

	m_singleton = nullptr;
}

/**
 * Returns the one and only instance of CController. If it doesn't exist yet, it is created.
 * @return The CController singleton object.
 * @sa m_singleton, CController
 */
CController* CController::GetInstance()
{
	if (m_singleton == nullptr)
	{
		m_singleton = new CController();
	}
	return m_singleton;
}

/**
 * Triggers destruction of CController singleton object
 */
void CController::DestroyInstance()
{
	delete m_singleton;
}

/**
 * Method which will be called every time a parameter or property has been changed.
 * @param changeSource	The application module which is causing the property change.
 * @param changeTypes	Defines which parameter or property has been changed.
 */
void CController::SetParameterChanged(DataChangeSource changeSource, DataChangeType changeTypes)
{
	const ScopedLock lock(m_mutex);

	// Set the specified change flag for all DataChangeSources, except for the one causing the change.
	for (int cs = 0; cs < DCS_Max; cs++)
	{
		m_parametersChanged[cs] |= changeTypes;
	}

	// Forward the change to all processor instances. This is needed, for example, so that all processor's
	// GUIs update an IP address change.
	for (int i = 0; i < m_processors.size(); ++i)
	{
		m_processors[i]->SetParameterChanged(changeSource, changeTypes);
	}

	switch (changeTypes)
	{
	case DCT_NumProcessors:
	case DCT_None:
	case DCT_IPAddress:
	case DCT_MessageRate:
	case DCT_OscConfig:
	case DCT_SourceID:
	case DCT_MappingID:
	case DCT_ComsMode:
	case DCT_PluginInstanceConfig:
		if (changeSource != DCS_Init)
			triggerConfigurationUpdate(true);
		break;
	case DCT_Online:
	case DCT_SourcePosition:
	case DCT_ReverbSendGain:
	case DCT_SourceSpread:
	case DCT_DelayMode:
	case DCT_AutomationParameters:
	case DCT_DebugMessage:
	default:
		break;
	}
}

/**
 * Get the state of the desired flag (or flags) for the desired change source.
 * @param changeSource	The application module querying the change flag.
 * @param change		The desired parameter (or parameters).
 * @return	True if any of the given parameters has changed it's value 
 *			since the last time PopParameterChanged() was called.
 */
bool CController::GetParameterChanged(DataChangeSource changeSource, DataChangeType change)
{
	const ScopedLock lock(m_mutex);
	return ((m_parametersChanged[changeSource] & change) != 0);
}

/**
 * Reset the state of the desired flag (or flags) for the desired change source.
 * Will return the state of the flag before the resetting.
 * @param changeSource	The application module querying the change flag.
 * @param change		The desired parameter (or parameters).
 * @return	The state of the flag before the resetting.
 */
bool CController::PopParameterChanged(DataChangeSource changeSource, DataChangeType change)
{
	const ScopedLock lock(m_mutex);
	bool ret((m_parametersChanged[changeSource] & change) != 0);
	m_parametersChanged[changeSource] &= ~change; // Reset flag.
	return ret;
}

/**
 * Helper method to create a new processor incl. implicit triggering of
 * inserting it into xml config (by setting constructor bool flag to insertToConfig=true)
 */
void CController::createNewProcessor()
{
	auto processor = std::make_unique<SoundscapeBridgeApp::SoundsourceProcessor>(true);
	processor.release(); // let go of the instance here, we do not want to destroy it, since it lives as member of CCOntroller when constructed
}

/**
 * Register a processor instance to the local list of processors. 
 * @param changeSource	The application module which is causing the property change.
 * @param p				Pointer to newly crated processor processor object.
 * @return				The ProcessorId of the newly added processor.
 */
ProcessorId CController::AddProcessor(DataChangeSource changeSource, SoundsourceProcessor* p)
{
	const ScopedLock lock(m_mutex);

	// Get the highest Input number of all current processors.
	SourceId currentMaxSourceId = 0;
	for (int i = 0; i < m_processors.size(); ++i)
	{
		if (m_processors[i]->GetSourceId() > currentMaxSourceId)
			currentMaxSourceId = m_processors[i]->GetSourceId();
	}

	m_processors.add(p);
	ProcessorId newProcessorId = static_cast<ProcessorId>(m_processors.size() - 1);

	// Set the new Processor's id
	p->SetProcessorId(changeSource, newProcessorId);
	
#ifdef JUCE_DEBUG
	p->PushDebugMessage("CController::AddProcessor: #" + String(newProcessorId));
#endif
	SetParameterChanged(changeSource, DCT_NumProcessors);

	// Set the new Processor's InputID to the next in sequence.
	p->SetSourceId(changeSource, currentMaxSourceId + 1);

	return newProcessorId;
}

/**
 * Remove a Processor instance from the local list of processors.
 * @param p		Pointer to Processor object which should be removed.
 */
void CController::RemoveProcessor(SoundsourceProcessor* p)
{
	DeactivateSoundSourceId(p->GetSourceId(), p->GetMappingId());

	int idx = m_processors.indexOf(p);
	jassert(idx >= 0); // Tried to remove inexistent Processor object.
	if (idx >= 0)
	{
		const ScopedLock lock(m_mutex);
		m_processors.remove(idx);

		SetParameterChanged(DCS_Protocol, DCT_NumProcessors);
	}
}

/**
 * Number of registered Processor instances.
 * @return	Number of registered Processor instances.
 */
int CController::GetProcessorCount() const
{
	const ScopedLock lock(m_mutex);
	return m_processors.size();
}

/**
 * Get a pointer to a specified processor.
 * @param idx	The index of the desired processor.
 * @return	The pointer to the desired processor.
 */
SoundsourceProcessor* CController::GetProcessor(ProcessorId idx) const
{
	const ScopedLock lock(m_mutex);
	if ((idx >= 0) && (idx < m_processors.size()))
		return m_processors[idx];

	jassertfalse; // Index out of range!
	return nullptr;
}

/**
 * Getter function for the IP address to which m_oscSender and m_oscReceiver are connected.
 * @return	Current IP address.
 */
String CController::GetIpAddress() const
{
	return m_ipAddress;
}

/**
 * Static methiod which returns the default IP address.
 * @return	IP address to use as default.
 */
String CController::GetDefaultIpAddress()
{
	return PROTOCOL_DEFAULT_IP;
}

/**
 * Setter function for the IP address to which m_oscSender and m_oscReceiver are connected.
 * NOTE: changing ip address will disconnect m_oscSender and m_oscReceiver.
 * @param changeSource	The application module which is causing the property change.
 * @param ipAddress		New IP address.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 */
void CController::SetIpAddress(DataChangeSource changeSource, String ipAddress, bool dontSendNotification)
{
	if (m_ipAddress != ipAddress)
	{
		const ScopedLock lock(m_mutex);

		m_ipAddress = ipAddress;

		m_protocolBridge.SetDS100IpAddress(ipAddress, dontSendNotification);

		// Start "offline" after changing IP address
		m_heartBeatsRx = MAX_HEARTBEAT_COUNT;
		m_heartBeatsTx = 0;

		// Signal the change to all Processors. 
		SetParameterChanged(changeSource, (DCT_IPAddress | DCT_Online));

		Reconnect();
	}
}

/**
 * Getter function for the OSC communication state.
 * @return		True if a valid OSC message was received and successfully processed recently.
 *				False if no response was received for longer than the timeout threshold.
 */
bool CController::GetOnline() const
{
	return ((m_heartBeatsRx * m_oscMsgRate) < KEEPALIVE_TIMEOUT);
}

/**
 * Getter for the rate at which OSC messages are being sent out.
 * @return	Messaging rate, in milliseconds.
 */
int CController::GetRate() const
{
	return m_oscMsgRate;
}

/**
 * Setter for the rate at which OSC messages are being sent out.
 * @param changeSource	The application module which is causing the property change.
 * @param rate	New messaging rate, in milliseconds.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 */
void CController::SetRate(DataChangeSource changeSource, int rate, bool dontSendNotification)
{
	if (rate != m_oscMsgRate)
	{
		const ScopedLock lock(m_mutex);

		// Clip rate to the allowed range.
		rate = jmin(PROTOCOL_INTERVAL_MAX, jmax(PROTOCOL_INTERVAL_MIN, rate));

		m_oscMsgRate = rate;

		m_protocolBridge.SetDS100MsgRate(rate, dontSendNotification);

		// Signal the change to all Processors.
		SetParameterChanged(changeSource, DCT_MessageRate);

		// Reset timer to the new interval.
		startTimer(m_oscMsgRate);
	}
}

/**
 * Static methiod which returns the allowed minimum and maximum PROTOCOL message rates.
 * @return	Returns a std::pair<int, int> where the first number is the minimum supported rate, 
 *			and the second number is the maximum.
 */
std::pair<int, int> CController::GetSupportedRateRange()
{
	return std::pair<int, int>(PROTOCOL_INTERVAL_MIN, PROTOCOL_INTERVAL_MAX);
}

/**
 * Method to initialize IP address and polling rate.
 * @param changeSource	The application module which is causing the property change.
 * @param ipAddress		New IP address.
 * @param rate			New messaging rate, in milliseconds.
 */
void CController::InitGlobalSettings(DataChangeSource changeSource, String ipAddress, int rate)
{
	SetIpAddress(changeSource, ipAddress);
	SetRate(changeSource, rate);
}

/**
 * Reimplemented callback for bridging wrapper callback to process incoming protocol data.
 * It forwards the message to all registered Processor objects.
 * @param nodeId	The bridging node that the message data was received on (only a single default id node supported currently).
 * @param senderProtocolId	The protocol that the message data was received on and was sent to controller from.
 * @param objectId	The remote object id of the object that was received
 * @param msgData	The actual message data that was received
 */
void CController::HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier objectId, RemoteObjectMessageData& msgData)
{
	jassert(nodeId == DEFAULT_PROCNODE_ID);
	if (nodeId != DEFAULT_PROCNODE_ID)
		return;

	// Do not handle any protocol data except what is received
	// from DS100 - any data that was sent by 3rd party devices 
	// is bridged to DS100 and returned by it 
	// so we can handle the data in the end as well
	if (senderProtocolId != DS100_PROCESSINGPROTOCOL_ID)
		return;

	const ScopedLock lock(m_mutex);

	if (m_processors.size() > 0)
	{
		int i;
		bool resetHeartbeat = false;

		// Check if the incoming message is a response to a sent "ping".
		if (objectId == RemoteObjectIdentifier::ROI_HeartbeatPong)
			resetHeartbeat = true;

		// Check if the incoming message contains parameters.
		else
		{
			// Parse the Source ID
			SourceId sourceId = msgData.addrVal.first;
			jassert(sourceId > 0);
			if (sourceId > 0)
			{
				AutomationParameterIndex pIdx = ParamIdx_MaxIndex;
				DataChangeType change = DCT_None;
				int mappingId = 0;

				// Determine which parameter was changed depending on the incoming message's address pattern.
				if (objectId == RemoteObjectIdentifier::ROI_CoordinateMapping_SourcePosition_XY)
				{
					// The Mapping ID
					mappingId = msgData.addrVal.second;
					jassert(mappingId > 0);

					pIdx = ParamIdx_X;
					change = DCT_SourcePosition;
				}
				else if (objectId == RemoteObjectIdentifier::ROI_MatrixInput_ReverbSendGain)
				{
					pIdx = ParamIdx_ReverbSendGain;
					change = DCT_ReverbSendGain;
				}
				else if (objectId == RemoteObjectIdentifier::ROI_Positioning_SourceSpread)
				{
					pIdx = ParamIdx_SourceSpread;
					change = DCT_SourceSpread;
				}
				else if (objectId == RemoteObjectIdentifier::ROI_Positioning_SourceDelayMode)
				{
					pIdx = ParamIdx_DelayMode;
					change = DCT_DelayMode;
				}

				// Continue if the message's address pattern was recognized 
				if (change != DCT_None)
				{
					// Check all Processor instances to see if any of them want the new coordinates.
					for (i = 0; i < m_processors.size(); ++i)
					{
						// Check for matching Input number.
						SoundsourceProcessor* processor = m_processors[i];
						if (sourceId == processor->GetSourceId())
						{
							// Check if a SET command was recently sent out and might currently be on transit to the device.
							// If so, ignore the incoming message so that our local data does not jump back to a now outdated value.
							bool ignoreResponse = processor->IsParamInTransit(change);
							ComsMode mode = processor->GetComsMode();

							// Only pass on new positions to processors that are in RX mode.
							// Also, ignore all incoming messages for properties which this processor wants to send a set command.
							if (!ignoreResponse && ((mode & (CM_Rx | CM_PollOnce)) != 0) && (processor->GetParameterChanged(DCS_Protocol, change) == false))
							{
								// Special handling for X/Y position, since message contains two parameters and MappingID needs to match too.
								if (pIdx == ParamIdx_X)
								{
									if (mappingId == processor->GetMappingId())
									{
										jassert(msgData.valCount == 2 && msgData.valType == RemoteObjectValueType::ROVT_FLOAT);
										// Set the processor's new position.
										processor->SetParameterValue(DCS_Protocol, ParamIdx_X, static_cast<float*>(msgData.payload)[0]);
										processor->SetParameterValue(DCS_Protocol, ParamIdx_Y, static_cast<float*>(msgData.payload)[1]);

										// A request was sent to the DS100 by the CController because this processor was in CM_PollOnce mode.
										// Since the response was now processed, set the processor back into it's original mode.
										if ((mode & CM_PollOnce) == CM_PollOnce)
										{
											mode &= ~CM_PollOnce;
											processor->SetComsMode(DCS_Protocol, mode);
										}
									}
								}

								// All other automation parameters.
								else
								{
									float newValue;
									switch (msgData.valType)
									{
									case RemoteObjectValueType::ROVT_INT:
										newValue = static_cast<float>(static_cast<int*>(msgData.payload)[0]);
										break;
									case RemoteObjectValueType::ROVT_FLOAT:
										newValue = static_cast<float*>(msgData.payload)[0];
										break;
									case RemoteObjectValueType::ROVT_STRING:
										newValue = std::stof(std::string(static_cast<char*>(msgData.payload)));
										break;
									case RemoteObjectValueType::ROVT_NONE:
									default:
										newValue = 0.0f;
										break;
									}

									processor->SetParameterValue(DCS_Protocol, pIdx, newValue);
								}
							}
						}
					}

					// Since pIdx was set, we know the received OSC message has valid format.
					// -> Signal to reset the number of heartbeats since last response.
					resetHeartbeat = true;
				}
			}
		}

		// A valid OSC message was received and successfully processed
		// -> reset the number of heartbeats since last response.
		if (resetHeartbeat)
		{
			bool wasOnline = GetOnline();
			m_heartBeatsRx = 0;

			// If previous state was "Offline", force all processors to
			// update their GUI, since we are now Online.
			if (!wasOnline)
			{
				SetParameterChanged(DCS_Protocol, DCT_Online);
			}
		}
	}
}

/**
 * Disconnect the active bridging nodes' protocols.
 */
void CController::Disconnect()
{
	m_protocolBridge.Disconnect();
}

/**
 * Disconnect and re-connect the OSCSender to a host specified by the current ip settings.
 */
void CController::Reconnect()
{
	m_protocolBridge.Reconnect();
}

/**
 * Timer callback function, which will be called at regular intervals to
 * send out OSC messages.
 * Reimplemented from base class Timer.
 */
void CController::timerCallback()
{
	const ScopedLock lock(m_mutex);
	if (m_processors.size() > 0)
	{
		// Check that we don't flood the line with pings, only send them in small intervals.
		bool sendKeepAlive = (((m_heartBeatsRx * m_oscMsgRate) > KEEPALIVE_INTERVAL) ||
								((m_heartBeatsTx * m_oscMsgRate) > KEEPALIVE_INTERVAL));

		int i;
		SoundsourceProcessor* pro = nullptr;
		ComsMode mode;
		RemoteObjectMessageData newMsgData;

		for (i = 0; i < m_processors.size(); ++i)
		{
			pro = m_processors[i];

			mode = pro->GetComsMode();

			// Signal every timer tick to each processor instance. 
			// This is used to trigger gestures for touch automation.
			pro->Tick();

			bool msgSent;
			DataChangeType paramSetsInTransit = DCT_None;

			newMsgData.addrVal.first = static_cast<juce::uint16>(pro->GetSourceId());
			newMsgData.addrVal.second = static_cast<juce::uint16>(pro->GetMappingId());

			// Iterate through all automation parameters.
			for (int pIdx = ParamIdx_X; pIdx < ParamIdx_MaxIndex; ++pIdx)
			{
				msgSent = false;

				switch (pIdx)
				{
					case ParamIdx_X:
					{
						float newDualFloatValue[2];

						newDualFloatValue[0] = pro->GetParameterValue(ParamIdx_X);
						newDualFloatValue[1] = pro->GetParameterValue(ParamIdx_Y);

						newMsgData.valCount = 2;
						newMsgData.valType = ROVT_FLOAT;
						newMsgData.payload = &newDualFloatValue;
						newMsgData.payloadSize = 2 * sizeof(float);

						// SET command is only sent out while in CM_Tx mode, provided that
						// this parameter has been changed since the last timer tick.
						if (((mode & CM_Tx) == CM_Tx) && pro->GetParameterChanged(DCS_Protocol, DCT_SourcePosition))
						{
							msgSent = m_protocolBridge.SendMessage(ROI_CoordinateMapping_SourcePosition_XY, newMsgData);
							paramSetsInTransit |= DCT_SourcePosition;
						}
					}
					break;

					case ParamIdx_Y:
						// Changes to ParamIdx_Y are handled together with ParamIdx_X, so skip it.
						continue;
						break;

					case ParamIdx_ReverbSendGain:
					{
						float newFloatValue = pro->GetParameterValue(ParamIdx_ReverbSendGain);

						newMsgData.valCount = 1;
						newMsgData.valType = ROVT_FLOAT;
						newMsgData.payload = &newFloatValue;
						newMsgData.payloadSize = sizeof(float);

						// SET command is only sent out while in CM_Tx mode, provided that
						// this parameter has been changed since the last timer tick.
						if (((mode & CM_Tx) == CM_Tx) && pro->GetParameterChanged(DCS_Protocol, DCT_ReverbSendGain))
						{
							msgSent = m_protocolBridge.SendMessage(ROI_MatrixInput_ReverbSendGain, newMsgData);
							paramSetsInTransit |= DCT_ReverbSendGain;
						}
					}
					break;

					case ParamIdx_SourceSpread:
					{
						float newFloatValue = pro->GetParameterValue(ParamIdx_SourceSpread);

						newMsgData.valCount = 1;
						newMsgData.valType = ROVT_FLOAT;
						newMsgData.payload = &newFloatValue;
						newMsgData.payloadSize = sizeof(float);

						// SET command is only sent out while in CM_Tx mode, provided that
						// this parameter has been changed since the last timer tick.
						if (((mode & CM_Tx) == CM_Tx) && pro->GetParameterChanged(DCS_Protocol, DCT_SourceSpread))
						{
							msgSent = m_protocolBridge.SendMessage(ROI_Positioning_SourceSpread, newMsgData);
							paramSetsInTransit |= DCT_SourceSpread;
						}
					}
					break;

					case ParamIdx_DelayMode:
					{
						float newFloatValue = pro->GetParameterValue(ParamIdx_DelayMode);

						newMsgData.valCount = 1;
						newMsgData.valType = ROVT_FLOAT;
						newMsgData.payload = &newFloatValue;
						newMsgData.payloadSize = sizeof(float);

						// SET command is only sent out while in CM_Tx mode, provided that
						// this parameter has been changed since the last timer tick.
						if (((mode & CM_Tx) == CM_Tx) && pro->GetParameterChanged(DCS_Protocol, DCT_DelayMode))
						{
							msgSent = m_protocolBridge.SendMessage(ROI_Positioning_SourceDelayMode, newMsgData);
							paramSetsInTransit |= DCT_DelayMode;
						}
					}
					break;

					default:
						jassertfalse;
						break;
				}

				if (msgSent)
				{
					// Since we are expecting at least one response from the DS100, 
					// we can use that as heartbeat, no need to send an extra ping.
					sendKeepAlive = false;
				}
			}

			// Flag the parameters for which we just sent a SET command out.
			pro->SetParamInTransit(paramSetsInTransit);

			// All changed parameters were sent out, so we can reset their flags now.
			pro->PopParameterChanged(DCS_Protocol, DCT_AutomationParameters);
		}
		
		if (sendKeepAlive)
		{
			// If we aren't expecting any responses from the DS100, we need to at least send a "ping"
			// so that we can use the "pong" to check our connection status.
			newMsgData.valCount = 0;
			newMsgData.valType = ROVT_NONE;
			newMsgData.payload = 0;
			newMsgData.payloadSize = 0;
			m_protocolBridge.SendMessage(ROI_HeartbeatPing, newMsgData);
		}

		bool wasOnline = GetOnline();
		if (m_heartBeatsRx < MAX_HEARTBEAT_COUNT)
			m_heartBeatsRx++;
		if (m_heartBeatsTx < MAX_HEARTBEAT_COUNT)
			m_heartBeatsTx++;

		// If we have just crossed the treshold, force all processors to update their
		// GUI, since we are now Offline.
		if (wasOnline && (GetOnline() == false))
			SetParameterChanged(DCS_Protocol, DCT_Online);
	}
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to set this objects' settings
 * from a XML element structure that passed as argument.
 * @param stateXml	The XML element containing this objects' configuration data
 * @return	True if the data was read and handled successfuly, false if not.
 */
bool CController::setStateXml(XmlElement* stateXml)
{
	if (!stateXml || (stateXml->getTagName() != AppConfiguration::getTagName(AppConfiguration::TagID::CONTROLLER)))
		return false;

	bool retVal = true;

	auto processorsXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::SOUNDSOURCEPROCESSORS));
	if (processorsXmlElement)
	{
		forEachXmlChildElement(*processorsXmlElement, processorXmlElement)
		{
			jassert(processorXmlElement->getTagName().contains(AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORINSTANCE)));
			int elementProcessorId = processorXmlElement->getTagName().getTrailingIntValue();
			bool alreadyExists = false;
			for (auto processor : m_processors)
				if (processor->GetProcessorId() == elementProcessorId)
				{
					processor->setStateXml(processorXmlElement);
					alreadyExists = true;
				}

			if (!alreadyExists)
			{
				auto newProcessor =	std::make_unique<SoundsourceProcessor>(false);
				newProcessor->SetProcessorId(DCS_Init, elementProcessorId);
				auto p = newProcessor.release();
				jassert(m_processors.contains(p));
				p->setStateXml(processorXmlElement);
			}
		}

		auto pageMgr = PageComponentManager::GetInstance();
		if (pageMgr)
		{
			auto pageContainer = pageMgr->GetPageContainer();
			if (pageContainer)
				pageContainer->UpdateGui(false);
		}
	}
	else
		retVal = false;

	auto bridgingXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::BRIDGING));
	if (bridgingXmlElement)
	{
		if (m_protocolBridge.setStateXml(bridgingXmlElement))
		{
			SetIpAddress(DataChangeSource::DCS_Init, m_protocolBridge.GetDS100IpAddress(), true);
			SetRate(DataChangeSource::DCS_Init, m_protocolBridge.GetDS100MsgRate(), true);
		}
	}

	return retVal;
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to dump this objects' settings
 * to a XML element structure that is returned and written to config file by the
 * singleton AppConfiguration class implementation.
 * @return	The XML element data that was created.
 */
std::unique_ptr<XmlElement> CController::createStateXml()
{
	auto controllerXmlElement = std::make_unique<XmlElement>(AppConfiguration::getTagName(AppConfiguration::TagID::CONTROLLER));

	auto processorsXmlElement = controllerXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::SOUNDSOURCEPROCESSORS));
	if (processorsXmlElement)
	{
		for (auto processor : m_processors)
		{
			jassert(processor->GetProcessorId() != -1);
			processorsXmlElement->addChildElement(processor->createStateXml().release());
		}
	}

	auto bridgingXmlElement = m_protocolBridge.createStateXml();
	if (bridgingXmlElement)
		controllerXmlElement->addChildElement(bridgingXmlElement.release());

    return controllerXmlElement;
}

/**
 * Activates the remote objects in protocol bridge proxy corresponding to given source/mapping via proxy bridge object
 * @param sourceId	The soundsource object id to activate
 * @param mappingId	The soundsource mapping id to activate
 */
void CController::ActivateSoundSourceId(SourceId sourceId, MappingId mappingId)
{
	m_protocolBridge.ActivateDS100SourceId(static_cast<juce::int16>(sourceId), static_cast<juce::int16>(mappingId));
}

/**
 * Deactivates the remote objects in protocol bridge proxy corresponding to given source/mapping via proxy bridge object
 * @param sourceId	The soundsource object id to activate
 * @param mappingId	The soundsource mapping id to activate
 */
void CController::DeactivateSoundSourceId(SourceId sourceId, MappingId mappingId)
{
	m_protocolBridge.DeactivateDS100SourceId(static_cast<juce::int16>(sourceId), static_cast<juce::int16>(mappingId));
}

/**
 * Adds a given listener object to this controller instance's bridging wrapper object.
 * @param listener	The listener object to add to bridging wrapper.
 */
void CController::AddProtocolBridgingWrapperListener(ProtocolBridgingWrapper::Listener* listener)
{
	m_protocolBridge.AddListener(listener);
}

/**
 * Getter for the active protocol bridging types (active protocols RoleB - those are used for bridging to DS100 running as RoleA, see RemoteProtocolBridge for details)
 * @return The bitfield containing all active bridging types
 */
ProtocolBridgingType CController::GetActiveProtocolBridging()
{
	return m_protocolBridge.GetActiveBridgingProtocols();
}

/**
 * Getter for currently active bridging protocols count.
 * @return The number of currently active bridging protocols.
 */
int CController::GetActiveProtocolBridgingCount()
{
	auto activeProtocolBridgingCount = 0;
	auto activeBridging = GetActiveProtocolBridging();

	if ((activeBridging & PBT_DiGiCo) == PBT_DiGiCo)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_GenericOSC) == PBT_GenericOSC)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_BlacktraxRTTrPM) == PBT_BlacktraxRTTrPM)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_GenericMIDI) == PBT_GenericMIDI)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_YamahaSQ) == PBT_YamahaSQ)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_HUI) == PBT_HUI)
		activeProtocolBridgingCount++;
	if ((activeBridging & PBT_DS100) == PBT_DS100)
		activeProtocolBridgingCount++;

	return activeProtocolBridgingCount;
}

/**
 * Setter for protocol bridging types that shall be active.
 * @param	bridgingTypes	Bitfield containing all types that are to be active.
 */
void CController::SetActiveProtocolBridging(ProtocolBridgingType bridgingTypes)
{
	m_protocolBridge.SetActiveBridgingProtocols(bridgingTypes);
}

/**
 * Gets the mute state of the given source via proxy bridge object
 * @param sourceId The id of the source for which the mute state shall be returned
 * @return The mute state
 */
bool CController::GetMuteBridgingSourceId(ProtocolBridgingType bridgingType, SourceId sourceId)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.GetMuteDiGiCoSourceId(sourceId);
	case PBT_GenericOSC:
		return m_protocolBridge.GetMuteGenericOSCSourceId(sourceId);
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.GetMuteRTTrPMSourceId(sourceId);
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

/**
 * Sets the given source to be (un-)muted in DiGiCo protocol via proxy bridge object
 * @param sourceId The id of the source that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool CController::SetMuteBridgingSourceId(ProtocolBridgingType bridgingType, SourceId sourceId, bool mute)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.SetMuteDiGiCoSourceId(sourceId, mute);
	case PBT_GenericOSC:
		return m_protocolBridge.SetMuteGenericOSCSourceId(sourceId, mute);
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.SetMuteRTTrPMSourceId(sourceId, mute);
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

String CController::GetBridgingIpAddress(ProtocolBridgingType bridgingType)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.GetDiGiCoIpAddress();
	case PBT_GenericOSC:
		return m_protocolBridge.GetGenericOSCIpAddress();
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.GetRTTrPMIpAddress();
	case PBT_DS100:
		return m_protocolBridge.GetDS100IpAddress();
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	default:
		jassertfalse;
		return String();
	}
}

bool CController::SetBridgingIpAddress(ProtocolBridgingType bridgingType, String ipAddress, bool dontSendNotification)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.SetDiGiCoIpAddress(ipAddress, dontSendNotification);
	case PBT_GenericOSC:
		return m_protocolBridge.SetGenericOSCIpAddress(ipAddress, dontSendNotification);
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.SetRTTrPMIpAddress(ipAddress, dontSendNotification);
	case PBT_DS100:
		return m_protocolBridge.SetDS100IpAddress(ipAddress, dontSendNotification);
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	default:
		jassertfalse;
		return false;
	}
}

int CController::GetBridgingListeningPort(ProtocolBridgingType bridgingType)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.GetDiGiCoListeningPort();
	case PBT_GenericOSC:
		return m_protocolBridge.GetGenericOSCListeningPort();
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.GetRTTrPMListeningPort();
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

bool CController::SetBridgingListeningPort(ProtocolBridgingType bridgingType, int listeningPort, bool dontSendNotification)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.SetDiGiCoListeningPort(listeningPort, dontSendNotification);
	case PBT_GenericOSC:
		return m_protocolBridge.SetGenericOSCListeningPort(listeningPort, dontSendNotification);
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.SetRTTrPMListeningPort(listeningPort, dontSendNotification);
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

int CController::GetBridgingRemotePort(ProtocolBridgingType bridgingType)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.GetDiGiCoRemotePort();
	case PBT_GenericOSC:
		return m_protocolBridge.GetGenericOSCRemotePort();
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.GetRTTrPMRemotePort();
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

bool CController::SetBridgingRemotePort(ProtocolBridgingType bridgingType, int remotePort, bool dontSendNotification)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.SetDiGiCoRemotePort(remotePort, dontSendNotification);
	case PBT_GenericOSC:
		return m_protocolBridge.SetGenericOSCRemotePort(remotePort, dontSendNotification);
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.SetRTTrPMRemotePort(remotePort, dontSendNotification);
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

int CController::GetBridgingMappingArea(ProtocolBridgingType bridgingType)
{
	switch (bridgingType)
	{
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.GetRTTrPMMappingArea();
	case PBT_DiGiCo:
	case PBT_GenericOSC:
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

bool CController::SetBridgingMappingArea(ProtocolBridgingType bridgingType, int mappingAreaId, bool dontSendNotification)
{
	switch (bridgingType)
	{
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.SetRTTrPMMappingArea(mappingAreaId, dontSendNotification);
	case PBT_DiGiCo:
	case PBT_GenericOSC:
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

} // namespace SoundscapeBridgeApp
