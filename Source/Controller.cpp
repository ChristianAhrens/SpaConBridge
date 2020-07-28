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

#include "Overview/OverviewManager.h"
#include "SoundsourceProcessor/SoundsourceProcessor.h"


namespace SoundscapeBridgeApp
{


static constexpr int PROTOCOL_INTERVAL_MIN = 20;		//< Minimum supported OSC messaging rate in milliseconds
static constexpr int PROTOCOL_INTERVAL_MAX = 5000;	//< Maximum supported OSC messaging rate in milliseconds
static constexpr int PROTOCOL_INTERVAL_DEF = 50;		//< Default OSC messaging rate in milliseconds

static const String PROTOCOL_DEFAULT_IP("127.0.0.1");	//< Default IP Address

static constexpr int RX_PORT_DS100 = 50010;		//< UDP port which the DS100 is listening to for OSC
static constexpr int RX_PORT_HOST = 50011;		//< UDP port to which the DS100 will send OSC replies

static constexpr int KEEPALIVE_TIMEOUT = 5000;	//< Milliseconds without response after which we consider plugin "Offline"
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

/**
 * Pre-define processing bridge config values
 */
static constexpr int DEFAULT_PROCNODE_ID = 1;
static constexpr int DEFAULT_PROCPROT_A_ID = 2;
static constexpr int DEFAULT_PROCPROT_B_ID = 2;



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
	m_processingNode.AddListener(this);

	CreateNodeConfiguration();

	// Default OSC server settings. These might become overwritten 
	// by setStateInformation()
	SetRate(DCS_Protocol, PROTOCOL_INTERVAL_DEF);
	SetIpAddress(DCS_Protocol, PROTOCOL_DEFAULT_IP);
}

/**
 * Destroys the CController.
 */
CController::~CController()
{
	stopTimer();
	Disconnect();

	// Destroy overView window and overView Manager
	COverviewManager* ovrMgr = COverviewManager::GetInstance();
	if (ovrMgr)
		ovrMgr->CloseOverview(true);

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
 * Method which will be called every time a parameter or property has been changed.
 * @param changeSource	The application module which is causing the property change.
 * @param changeTypes	Defines which parameter or property has been changed.
 */
void CController::SetParameterChanged(DataChangeSource changeSource, DataChangeTypes changeTypes)
{
	const ScopedLock lock(m_mutex);

	// Set the specified change flag for all DataChangeSources, except for the one causing the change.
	for (int cs = 0; cs < DCS_Max; cs++)
	{
		m_parametersChanged[cs] |= changeTypes;
	}

	// Forward the change to all plugin instances. This is needed, for example, so that all plugin's
	// GUIs update an IP address change.
	for (int i = 0; i < m_processors.size(); ++i)
	{
		m_processors[i]->SetParameterChanged(changeSource, changeTypes);
	}
}

/**
 * Get the state of the desired flag (or flags) for the desired change source.
 * @param changeSource	The application module querying the change flag.
 * @param change		The desired parameter (or parameters).
 * @return	True if any of the given parameters has changed it's value 
 *			since the last time PopParameterChanged() was called.
 */
bool CController::GetParameterChanged(DataChangeSource changeSource, DataChangeTypes change)
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
bool CController::PopParameterChanged(DataChangeSource changeSource, DataChangeTypes change)
{
	const ScopedLock lock(m_mutex);
	bool ret((m_parametersChanged[changeSource] & change) != 0);
	m_parametersChanged[changeSource] &= ~change; // Reset flag.
	return ret;
}

/**
 * Register a plugin instance to the local list of processors. 
 * @param p		Pointer to newly crated plugin processor object.
 * @return		The PluginId of the newly added Plug-in.
 */
ProcessorId CController::AddProcessor(SoundsourceProcessor* p)
{
	const ScopedLock lock(m_mutex);

	// Get the highest Input number of all current Plugins.
	SourceId currentMaxSourceId = 0;
	for (int i = 0; i < m_processors.size(); ++i)
	{
		if (m_processors[i]->GetSourceId() > currentMaxSourceId)
			currentMaxSourceId = m_processors[i]->GetSourceId();
	}

	m_processors.add(p);
	SetParameterChanged(DCS_Protocol, DCT_NumPlugins);

	// Set the new Plugin's InputID to the next in sequence.
	p->SetSourceId(DCS_Protocol, currentMaxSourceId + 1);

	ProcessorId newProcessorId = static_cast<ProcessorId>(m_processors.size() - 1);
#ifdef JUCE_DEBUG
	p->PushDebugMessage("CController::AddProcessor: #" + String(newProcessorId));
#endif

	auto protocolData = m_processingConfig.GetProtocolData(DEFAULT_PROCNODE_ID, DEFAULT_PROCPROT_A_ID);
	for (int roi = ROI_SoundObject_Position_XY; roi < ROI_UserMAX; roi++)
	{
		RemoteObject newSourceObject;
		newSourceObject.Id = static_cast<RemoteObjectIdentifier>(roi);
		newSourceObject.Addr.first = static_cast<juce::int16>(p->GetSourceId());
		newSourceObject.Addr.second = static_cast<juce::int16>(p->GetMappingId());

		if (!protocolData.RemoteObjects.contains(newSourceObject))
			protocolData.RemoteObjects.add(newSourceObject);
	}
	m_processingConfig.SetProtocolData(DEFAULT_PROCNODE_ID, DEFAULT_PROCPROT_A_ID, protocolData);
	m_processingNode.SetNodeConfiguration(m_processingConfig, DEFAULT_PROCNODE_ID);

	return newProcessorId;
}

/**
 * Remove a plugin instance from the local list of processors.
 * @param p		Pointer to plugin processor object which should be removed.
 */
void CController::RemoveProcessor(SoundsourceProcessor* p)
{
	auto protocolData = m_processingConfig.GetProtocolData(DEFAULT_PROCNODE_ID, DEFAULT_PROCPROT_A_ID);
	for (int roi = ROI_SoundObject_Position_XY; roi < ROI_UserMAX; roi++)
	{
		RemoteObject newSourceObject;
		newSourceObject.Id = static_cast<RemoteObjectIdentifier>(roi);
		newSourceObject.Addr.first = static_cast<juce::int16>(p->GetSourceId());
		newSourceObject.Addr.second = static_cast<juce::int16>(p->GetMappingId());

		if (protocolData.RemoteObjects.contains(newSourceObject))
			protocolData.RemoteObjects.remove(protocolData.RemoteObjects.removeAllInstancesOf(newSourceObject));
	}
	m_processingConfig.SetProtocolData(DEFAULT_PROCNODE_ID, DEFAULT_PROCPROT_A_ID, protocolData);
	m_processingNode.SetNodeConfiguration(m_processingConfig, DEFAULT_PROCNODE_ID);

	if (m_processors.size() > 1)
	{
		int idx = m_processors.indexOf(p);
		jassert(idx >= 0); // Tried to remove inexistent plugin object.
		if (idx >= 0)
		{
			const ScopedLock lock(m_mutex);
			m_processors.remove(idx);

			SetParameterChanged(DCS_Protocol, DCT_NumPlugins);
		}
	}

	// If last plugin instance is being removed, delete CController singleton.
	else if (m_processors.size() == 1)
	{
		{ // Scope for lock.
			const ScopedLock lock(m_mutex);
			m_processors.clearQuick();
		}
	
		delete this;
	}
}

/**
 * Number of registered plugin instances.
 * @return	Number of registered plugin instances.
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
 */
void CController::SetIpAddress(DataChangeSource changeSource, String ipAddress)
{
	if (m_ipAddress != ipAddress)
	{
		const ScopedLock lock(m_mutex);

		m_ipAddress = ipAddress;

		m_processingConfig.GetProtocolAIds(DEFAULT_PROCNODE_ID).getFirst();
		auto protocolData = m_processingConfig.GetProtocolData(DEFAULT_PROCNODE_ID, m_processingConfig.GetProtocolAIds(DEFAULT_PROCNODE_ID).getFirst());
		protocolData.IpAddress = ipAddress;
		m_processingConfig.SetProtocolData(DEFAULT_PROCNODE_ID, m_processingConfig.GetProtocolAIds(DEFAULT_PROCNODE_ID).getFirst(), protocolData);
		m_processingNode.SetNodeConfiguration(m_processingConfig, DEFAULT_PROCNODE_ID);

		// Start "offline" after changing IP address
		m_heartBeatsRx = MAX_HEARTBEAT_COUNT;
		m_heartBeatsTx = 0;

		// Signal the change to all plugins. 
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
 */
void CController::SetRate(DataChangeSource changeSource, int rate)
{
	if (rate != m_oscMsgRate)
	{
		const ScopedLock lock(m_mutex);

		// Clip rate to the allowed range.
		rate = jmin(PROTOCOL_INTERVAL_MAX, jmax(PROTOCOL_INTERVAL_MIN, rate));

		m_oscMsgRate = rate;

		for (auto protocolAId : m_processingConfig.GetProtocolAIds(DEFAULT_PROCNODE_ID))
			m_processingConfig.SetPollingInterval(DEFAULT_PROCNODE_ID, protocolAId, rate);
		for (auto protocolBId : m_processingConfig.GetProtocolBIds(DEFAULT_PROCNODE_ID))
			m_processingConfig.SetPollingInterval(DEFAULT_PROCNODE_ID, protocolBId, rate);

		m_processingNode.SetNodeConfiguration(m_processingConfig, DEFAULT_PROCNODE_ID);

		// Signal the change to all plugins.
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
 * Method to create a basic configuration to use to setup the single supported
 * bridging node.
 */
void CController::CreateNodeConfiguration()
{
	ProcessingEngineConfig::ObjectHandlingData newObjectHandling;
	newObjectHandling.Mode = ObjectHandlingMode::OHM_Bypass;
	newObjectHandling.ACnt = 1;
	newObjectHandling.BCnt = 0;
	newObjectHandling.Prec = 1.0f;
	ProcessingEngineConfig::NodeData newNode;
	newNode.Id = DEFAULT_PROCNODE_ID;
	newNode.ObjectHandling = newObjectHandling;
	newNode.RoleAProtocols.add(DEFAULT_PROCPROT_A_ID);
	m_processingConfig.SetNode(newNode.Id, newNode);

	auto protocolData = m_processingConfig.GetProtocolData(DEFAULT_PROCNODE_ID, DEFAULT_PROCPROT_A_ID);
	protocolData.IpAddress = PROTOCOL_DEFAULT_IP;
	protocolData.ClientPort = RX_PORT_DS100;
	protocolData.HostPort = RX_PORT_HOST;
	protocolData.UsesActiveRemoteObjects = true;
	protocolData.Type = ProtocolType::PT_OSCProtocol;
	m_processingConfig.SetProtocolData(DEFAULT_PROCNODE_ID, DEFAULT_PROCPROT_A_ID, protocolData);
}

/**
 * Called when the OSCReceiver receives a new OSC message, since CController inherits from OSCReceiver::Listener.
 * It forwards the message to all registered plugin objects.
 * @param nodeId	The bridging node that the message data was received on (only a single default id node supported currently).
 * @param senderProtocolId	The protocol that the message data was received on and was sent to controller from.
 * @param objectId	The remote object id of the object that was received
 * @param msgData	The actual message data that was received
 */
void CController::HandleNodeData(NodeId nodeId, ProtocolId senderProtocolId, ProtocolType senderProtocolType, RemoteObjectIdentifier objectId, RemoteObjectMessageData& msgData)
{
	jassert(nodeId == DEFAULT_PROCNODE_ID);
	if (nodeId != DEFAULT_PROCNODE_ID)
		return;

	jassert(senderProtocolId == DEFAULT_PROCPROT_A_ID);
	if (senderProtocolId != DEFAULT_PROCPROT_A_ID)
		return;

	ignoreUnused(senderProtocolType);

	const ScopedLock lock(m_mutex);
	jassert(m_processors.size() > 0);

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
				DataChangeTypes change = DCT_None;
				int mappingId = 0;

				// Determine which parameter was changed depending on the incoming message's address pattern.
				if (objectId == RemoteObjectIdentifier::ROI_SoundObject_Position_XY)
				{
					// The Mapping ID
					mappingId = msgData.addrVal.second;
					jassert(mappingId > 0);

					pIdx = ParamIdx_X;
					change = DCT_SourcePosition;
				}
				else if (objectId == RemoteObjectIdentifier::ROI_ReverbSendGain)
				{
					pIdx = ParamIdx_ReverbSendGain;
					change = DCT_ReverbSendGain;
				}
				else if (objectId == RemoteObjectIdentifier::ROI_SoundObject_Spread)
				{
					pIdx = ParamIdx_SourceSpread;
					change = DCT_SourceSpread;
				}
				else if (objectId == RemoteObjectIdentifier::ROI_SoundObject_DelayMode)
				{
					pIdx = ParamIdx_DelayMode;
					change = DCT_DelayMode;
				}

				// Continue if the message's address pattern was recognized 
				if (change != DCT_None)
				{
					// Check all plugin instances to see if any of them want the new coordinates.
					for (i = 0; i < m_processors.size(); ++i)
					{
						// Check for matching Input number.
						SoundsourceProcessor* plugin = m_processors[i];
						if (sourceId == plugin->GetSourceId())
						{
							// Check if a SET command was recently sent out and might currently be on transit to the device.
							// If so, ignore the incoming message so that our local data does not jump back to a now outdated value.
							bool ignoreResponse = plugin->IsParamInTransit(change);
							ComsMode mode = plugin->GetComsMode();

							// Only pass on new positions to plugins that are in RX mode.
							// Also, ignore all incoming messages for properties which this plugin wants to send a set command.
							if (!ignoreResponse && ((mode & (CM_Rx | CM_PollOnce)) != 0) && (plugin->GetParameterChanged(DCS_Protocol, change) == false))
							{
								// Special handling for X/Y position, since message contains two parameters and MappingID needs to match too.
								if (pIdx == ParamIdx_X)
								{
									if (mappingId == plugin->GetMappingId())
									{
										jassert(msgData.valCount == 2 && msgData.valType == RemoteObjectValueType::ROVT_FLOAT);
										// Set the plugin's new position.
										plugin->SetParameterValue(DCS_Protocol, ParamIdx_X, static_cast<float*>(msgData.payload)[0]);
										plugin->SetParameterValue(DCS_Protocol, ParamIdx_Y, static_cast<float*>(msgData.payload)[1]);

										// A request was sent to the DS100 by the CController because this plugin was in CM_PollOnce mode.
										// Since the response was now processed, set the plugin back into it's original mode.
										if ((mode & CM_PollOnce) == CM_PollOnce)
										{
											mode &= ~CM_PollOnce;
											plugin->SetComsMode(DCS_Protocol, mode);
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

									plugin->SetParameterValue(DCS_Protocol, pIdx, newValue);
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

			// If previous state was "Offline", force all plugins to
			// update their GUI, since we are now Online.
			if (!wasOnline)
			{
				SetParameterChanged(DCS_Protocol, DCT_Online);
			}
		}
	}
}

/**
 * Send a Message out via the active bridging node.
 * @param Id	The id of the remote object to be sent.
 * @param msgData	The message data to be sent.
 * @return True on success, false on failure
 */
bool CController::SendMessage(RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData)
{
	auto sendSuccess{ true };

	auto activeBridgeNodeData = m_processingConfig.GetNodeData(m_processingConfig.GetNodeIds().getFirst());
	for (auto protocolA : activeBridgeNodeData.RoleAProtocols)
		sendSuccess &= m_processingNode.SendMessageTo(protocolA, Id, msgData);
	for (auto protocolB : activeBridgeNodeData.RoleAProtocols)
		sendSuccess &= m_processingNode.SendMessageTo(protocolB, Id, msgData);

	return sendSuccess;
}

/**
 * Disconnect the active bridging nodes' protocols.
 */
void CController::Disconnect()
{
	m_processingNode.Stop();
}

/**
 * Disconnect and re-connect the OSCSender to a host specified by the current ip settings.
 */
void CController::Reconnect()
{
	Disconnect();

	m_processingNode.SetNodeConfiguration(m_processingConfig, m_processingConfig.GetNodeIds().getFirst());
	m_processingNode.Start();
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

			// If the OscBypass parameter has changed since the last interval, 
			// update the OSC Rx/Tx mode of each Plugin accordingly.
			bool oscBypassed = pro->GetBypass();
			if (pro->PopParameterChanged(DCS_Protocol, DCT_Bypass))
			{
				if (oscBypassed)
					pro->SetComsMode(DCS_Protocol, CM_Off);
				else
					pro->RestoreComsMode(DCS_Protocol);

				// Changing ComsMode also sets the changed flag for Bypass. 
				// Clear it so we don't come in here again unnecessarily.
				pro->PopParameterChanged(DCS_Protocol, DCT_Bypass);
			}
			mode = pro->GetComsMode();

			// Signal every timer tick to each plugin instance. 
			// This is used to trigger gestures for touch automation.
			pro->Tick();

			bool msgSent;
			DataChangeTypes paramSetsInTransit = DCT_None;

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
						newMsgData.payload = &newDualFloatValue;
						newMsgData.payloadSize = 2 * sizeof(float);

						// SET command is only sent out while in CM_Tx mode, provided that
						// this parameter has been changed since the last timer tick.
						if (((mode & CM_Tx) == CM_Tx) && pro->GetParameterChanged(DCS_Protocol, DCT_SourcePosition))
						{
							msgSent = SendMessage(ROI_SoundObject_Position_XY, newMsgData);
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
						newMsgData.payload = &newFloatValue;
						newMsgData.payloadSize = sizeof(float);

						// SET command is only sent out while in CM_Tx mode, provided that
						// this parameter has been changed since the last timer tick.
						if (((mode & CM_Tx) == CM_Tx) && pro->GetParameterChanged(DCS_Protocol, DCT_ReverbSendGain))
						{
							msgSent = SendMessage(ROI_ReverbSendGain, newMsgData);
							paramSetsInTransit |= DCT_ReverbSendGain;
						}
					}
					break;

					case ParamIdx_SourceSpread:
					{
						float newFloatValue = pro->GetParameterValue(ParamIdx_SourceSpread);

						newMsgData.valCount = 1;
						newMsgData.payload = &newFloatValue;
						newMsgData.payloadSize = sizeof(float);

						// SET command is only sent out while in CM_Tx mode, provided that
						// this parameter has been changed since the last timer tick.
						if (((mode & CM_Tx) == CM_Tx) && pro->GetParameterChanged(DCS_Protocol, DCT_SourceSpread))
						{
							msgSent = SendMessage(ROI_SoundObject_Spread, newMsgData);
							paramSetsInTransit |= DCT_SourceSpread;
						}
					}
					break;

					case ParamIdx_DelayMode:
					{
						float newFloatValue = pro->GetParameterValue(ParamIdx_DelayMode);

						newMsgData.valCount = 1;
						newMsgData.payload = &newFloatValue;
						newMsgData.payloadSize = sizeof(float);

						// SET command is only sent out while in CM_Tx mode, provided that
						// this parameter has been changed since the last timer tick.
						if (((mode & CM_Tx) == CM_Tx) && pro->GetParameterChanged(DCS_Protocol, DCT_DelayMode))
						{
							msgSent = SendMessage(ROI_SoundObject_DelayMode, newMsgData);
							paramSetsInTransit |= DCT_DelayMode;
						}
					}
					break;

					case ParamIdx_Bypass:
						// Nothing to do, this is not a parameter which will arrive per remote protocol.
						continue;
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
			newMsgData.payload = 0;
			newMsgData.payloadSize = 0;
			SendMessage(ROI_HeartbeatPing, newMsgData);
		}

		bool wasOnline = GetOnline();
		if (m_heartBeatsRx < MAX_HEARTBEAT_COUNT)
			m_heartBeatsRx++;
		if (m_heartBeatsTx < MAX_HEARTBEAT_COUNT)
			m_heartBeatsTx++;

		// If we have just crossed the treshold, force all plugins to update their
		// GUI, since we are now Offline.
		if (wasOnline && (GetOnline() == false))
			SetParameterChanged(DCS_Protocol, DCT_Online);
	}
}


} // namespace SoundscapeBridgeApp
