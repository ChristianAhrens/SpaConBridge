/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file was originally part of the Soundscape VST, AU, and AAX Plug-in
and now in a derived version is part of SoundscapeBridgeApp.

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
#include "CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"
#include "CustomAudioProcessors/MatrixChannelProcessor/MatrixChannelProcessor.h"


namespace SoundscapeBridgeApp
{


static constexpr int PROTOCOL_INTERVAL_MIN = 20;		//< Minimum supported OSC messaging rate in milliseconds
static constexpr int PROTOCOL_INTERVAL_MAX = 5000;	//< Maximum supported OSC messaging rate in milliseconds
static constexpr int PROTOCOL_INTERVAL_DEF = 100;		//< Default OSC messaging rate in milliseconds

static constexpr int KEEPALIVE_TIMEOUT = 5000;	//< Milliseconds without response after which we consider processor "Offline"
static constexpr int KEEPALIVE_INTERVAL = 1500;	//< Interval at which keepalive (ping) messages are sent, in milliseconds
static constexpr int MAX_HEARTBEAT_COUNT = 0xFFFF;	//< No point counting beyond this number.


/*
===============================================================================
 Class Controller
===============================================================================
*/

/**
 * The one and only instance of Controller.
 */
Controller* Controller::m_singleton = nullptr;

/**
 * Constructs an Controller object.
 * There can be only one instance of this class, see m_singleton. This is so that network traffic
 * is managed from a central point and only one UDP port is opened for all OSC communication.
 */
Controller::Controller()
{
	jassert(!m_singleton);	// only one instnce allowed!!
	m_singleton = this;

	// Clear all changed flags initially
	for (int cs = 0; cs < DCS_Max; cs++)
		m_parametersChanged[cs] = DCT_None;

	// Controller derives from ProcessingEngineNode::Listener
	AddProtocolBridgingWrapperListener(this);

	// Default OSC server settings. These might become overwritten 
	// by setStateInformation()
	SetRate(DCS_Init, PROTOCOL_INTERVAL_DEF, true);
	SetDS100IpAddress(DCS_Init, PROTOCOL_DEFAULT_IP, true);
	SetExtensionMode(DCS_Init, EM_Off, true);
}

/**
 * Destroys the Controller.
 */
Controller::~Controller()
{
	stopTimer();
	Disconnect();

	// Destroy overView window and overView Manager
	auto pageMgr = PageComponentManager::GetInstance();
	if (pageMgr)
		pageMgr->ClosePageContainer(true);

	const ScopedLock lock(m_mutex);
	m_soundobjectProcessors.clearQuick();

	m_singleton = nullptr;
}

/**
 * Returns the one and only instance of Controller. If it doesn't exist yet, it is created.
 * @return The Controller singleton object.
 * @sa m_singleton, Controller
 */
Controller* Controller::GetInstance()
{
	if (m_singleton == nullptr)
	{
		m_singleton = new Controller();
	}
	return m_singleton;
}

/**
 * Triggers destruction of Controller singleton object
 */
void Controller::DestroyInstance()
{
	delete m_singleton;
}

/**
 * Method which will be called every time a parameter or property has been changed.
 * @param changeSource	The application module which is causing the property change.
 * @param changeTypes	Defines which parameter or property has been changed.
 */
void Controller::SetParameterChanged(DataChangeSource changeSource, DataChangeType changeTypes)
{
	const ScopedLock lock(m_mutex);

	// Set the specified change flag for all DataChangeSources, except for the one causing the change.
	for (int cs = 0; cs < DCS_Max; cs++)
	{
		m_parametersChanged[cs] |= changeTypes;
	}

	// Forward the change to all processor instances. This is needed, for example, so that all processor's
	// GUIs update an IP address change.
	for (auto const& processor : m_soundobjectProcessors)
	{
		processor->SetParameterChanged(changeSource, changeTypes);
	}

	switch (changeTypes)
	{
	case DCT_NumProcessors:
	case DCT_None:
	case DCT_IPAddress:
	case DCT_MessageRate:
	case DCT_CommunicationConfig:
	case DCT_SoundobjectID:
	case DCT_MatrixChannelID:
	case DCT_MappingID:
	case DCT_ComsMode:
	case DCT_ProcessorInstanceConfig:
		if (changeSource != DCS_Init)
			triggerConfigurationUpdate(true);
		break;
	case DCT_BridgingConfig:
	case DCT_MuteState:
		if (changeSource != DCS_Init)
			triggerConfigurationUpdate(false);
		break;
	case DCT_NumBridgingModules:
		if (changeSource != DCS_Init)
			triggerConfigurationUpdate(true);
		break;
	case DCT_Online:
	case DCT_SoundobjectPosition:
	case DCT_ReverbSendGain:
	case DCT_SoundobjectSpread:
	case DCT_DelayMode:
	case DCT_SoundobjectParameters:
	case DCT_MatrixChannelParameters:
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
bool Controller::GetParameterChanged(DataChangeSource changeSource, DataChangeType change)
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
bool Controller::PopParameterChanged(DataChangeSource changeSource, DataChangeType change)
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
void Controller::createNewSoundobjectProcessor()
{
	auto processor = std::make_unique<SoundscapeBridgeApp::SoundobjectProcessor>(true);
	processor.release(); // let go of the instance here, we do not want to destroy it, since it lives as member of CCOntroller when constructed
}

/**
 * Register a processor instance to the local list of processors. 
 * @param changeSource	The application module which is causing the property change.
 * @param p				Pointer to newly crated processor processor object.
 * @return				The ProcessorId of the newly added processor.
 */
SoundobjectProcessorId Controller::AddSoundobjectProcessor(DataChangeSource changeSource, SoundobjectProcessor* p)
{
	const ScopedLock lock(m_mutex);

	// Get the highest Input number of all current processors.
	SoundobjectId currentMaxSoundobjectId = 0;
	for (auto const& processor : m_soundobjectProcessors)
	{
		if (processor->GetSoundobjectId() > currentMaxSoundobjectId)
			currentMaxSoundobjectId = processor->GetSoundobjectId();
	}

	// Get the next free processor id to use (can be one inbetween or the next after the last)
	auto newProcessorId = SoundobjectProcessorId(0);
	auto processorIds = GetSoundobjectProcessorIds();
	std::sort(processorIds.begin(), processorIds.end());
	for (auto const& processorId : processorIds)
	{
		if (processorId > newProcessorId) // we have found a gap in the list that we can use
			break;
		else
			newProcessorId++;
	}

	// add the processor to list now, since we have taken all info we require from the so far untouched list
	m_soundobjectProcessors.add(p);

	// Set the new Processor's id
	p->SetProcessorId(changeSource, newProcessorId);
	
	SetParameterChanged(changeSource, DCT_NumProcessors);

	// Set the new Processor's InputID to the next in sequence.
	p->SetSoundobjectId(changeSource, currentMaxSoundobjectId + 1);

	return newProcessorId;
}

/**
 * Remove a Processor instance from the local list of processors.
 * @param p		Pointer to Processor object which should be removed.
 */
void Controller::RemoveSoundobjectProcessor(SoundobjectProcessor* p)
{
	DeactivateSoundobjectId(p->GetSoundobjectId(), p->GetMappingId());

	int idx = m_soundobjectProcessors.indexOf(p);
	jassert(idx >= 0); // Tried to remove inexistent Processor object.
	if (idx >= 0)
	{
		const ScopedLock lock(m_mutex);
		m_soundobjectProcessors.remove(idx);

		SetParameterChanged(DCS_Protocol, DCT_NumProcessors);
	}
}

/**
 * Number of registered Processor instances.
 * @return	Number of registered Processor instances.
 */
int Controller::GetSoundobjectProcessorCount() const
{
	const ScopedLock lock(m_mutex);
	return m_soundobjectProcessors.size();
}

/**
 * Get a pointer to a specified processor.
 * @param processorId	The id of the desired processor.
 * @return	The pointer to the desired processor.
 */
SoundobjectProcessor* Controller::GetSoundobjectProcessor(SoundobjectProcessorId processorId) const
{
	const ScopedLock lock(m_mutex);
	for (auto processor : m_soundobjectProcessors)
		if (processor->GetProcessorId() == processorId)
			return processor;

	jassertfalse; // id not existing!
	return nullptr;
}

/**
 * Getter for all currently active processor's processorIds
 * @return	The vector of active processorids
 */
std::vector<SoundobjectProcessorId> Controller::GetSoundobjectProcessorIds() const
{
	std::vector<SoundobjectProcessorId> processorIds;
	processorIds.reserve(m_soundobjectProcessors.size());
	for (auto const& p : m_soundobjectProcessors)
		processorIds.push_back(p->GetProcessorId());
	return processorIds;
}


/**
 * Helper method to create a new processor incl. implicit triggering of
 * inserting it into xml config (by setting constructor bool flag to insertToConfig=true)
 */
void Controller::createNewMatrixChannelProcessor()
{
	auto processor = std::make_unique<SoundscapeBridgeApp::MatrixChannelProcessor>(true);
	processor.release(); // let go of the instance here, we do not want to destroy it, since it lives as member of controller when constructed
}

/**
 * Register a processor instance to the local list of processors.
 * @param changeSource	The application module which is causing the property change.
 * @param p				Pointer to newly crated processor processor object.
 * @return				The ProcessorId of the newly added processor.
 */
MatrixChannelProcessorId Controller::AddMatrixChannelProcessor(DataChangeSource changeSource, MatrixChannelProcessor* p)
{
	const ScopedLock lock(m_mutex);

	// Get the highest Input number of all current processors.
	MatrixChannelId currentMaxMatrixChannelId = 0;
	for (auto const& processor : m_matrixChannelProcessors)
	{
		if (processor->GetMatrixChannelId() > currentMaxMatrixChannelId)
			currentMaxMatrixChannelId = processor->GetMatrixChannelId();
	}

	// Get the next free processor id to use (can be one inbetween or the next after the last)
	auto newProcessorId = MatrixChannelProcessorId(0);
	auto processorIds = GetMatrixChannelProcessorIds();
	std::sort(processorIds.begin(), processorIds.end());
	for (auto const& processorId : processorIds)
	{
		if (processorId > newProcessorId) // we have found a gap in the list that we can use
			break;
		else
			newProcessorId++;
	}

	// add the processor to list now, since we have taken all info we require from the so far untouched list
	m_matrixChannelProcessors.add(p);

	// Set the new Processor's id
	p->SetProcessorId(changeSource, newProcessorId);

	SetParameterChanged(changeSource, DCT_NumProcessors);

	// Set the new Processor's InputID to the next in sequence.
	p->SetMatrixChannelId(changeSource, currentMaxMatrixChannelId + 1);

	return newProcessorId;
}

/**
 * Remove a Processor instance from the local list of processors.
 * @param p		Pointer to Processor object which should be removed.
 */
void Controller::RemoveMatrixChannelProcessor(MatrixChannelProcessor* p)
{
	DeactivateMatrixChannelId(p->GetMatrixChannelId());

	int idx = m_matrixChannelProcessors.indexOf(p);
	jassert(idx >= 0); // Tried to remove inexistent Processor object.
	if (idx >= 0)
	{
		const ScopedLock lock(m_mutex);
		m_matrixChannelProcessors.remove(idx);

		SetParameterChanged(DCS_Protocol, DCT_NumProcessors);
	}
}

/**
 * Number of registered Processor instances.
 * @return	Number of registered Processor instances.
 */
int Controller::GetMatrixChannelProcessorCount() const
{
	const ScopedLock lock(m_mutex);
	return m_matrixChannelProcessors.size();
}

/**
 * Get a pointer to a specified processor.
 * @param processorId	The id of the desired processor.
 * @return	The pointer to the desired processor.
 */
MatrixChannelProcessor* Controller::GetMatrixChannelProcessor(MatrixChannelProcessorId processorId) const
{
	const ScopedLock lock(m_mutex);
	for (auto processor : m_matrixChannelProcessors)
		if (processor->GetProcessorId() == processorId)
			return processor;

	jassertfalse; // id not existing!
	return nullptr;
}

/**
 * Getter for all currently active processor's processorIds
 * @return	The vector of active processorids
 */
std::vector<MatrixChannelProcessorId> Controller::GetMatrixChannelProcessorIds() const
{
	std::vector<MatrixChannelProcessorId> processorIds;
	processorIds.reserve(m_matrixChannelProcessors.size());
	for (auto const& p : m_matrixChannelProcessors)
		processorIds.push_back(p->GetProcessorId());
	return processorIds;
}



/**
 * Getter function for the IP address to which m_oscSender and m_oscReceiver are connected.
 * @return	Current IP address.
 */
String Controller::GetDS100IpAddress() const
{
	return m_DS100IpAddress;
}

/**
 * Static methiod which returns the default IP address.
 * @return	IP address to use as default.
 */
String Controller::GetDefaultDS100IpAddress()
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
void Controller::SetDS100IpAddress(DataChangeSource changeSource, String ipAddress, bool dontSendNotification)
{
	if (m_DS100IpAddress != ipAddress)
	{
		const ScopedLock lock(m_mutex);

		m_DS100IpAddress = ipAddress;

		m_protocolBridge.SetDS100IpAddress(ipAddress, dontSendNotification);

		// Signal the change to all Processors. 
		SetParameterChanged(changeSource, (DCT_IPAddress | DCT_Online));

		Reconnect();
	}
}

/**
 * Getter function for the IP address to which m_oscSender and m_oscReceiver are connected.
 * @return	Current IP address.
 */
String Controller::GetSecondDS100IpAddress() const
{
	return m_SecondDS100IpAddress;
}

/**
 * Setter function for the IP address to which m_oscSender and m_oscReceiver are connected.
 * NOTE: changing ip address will disconnect m_oscSender and m_oscReceiver.
 * @param changeSource	The application module which is causing the property change.
 * @param ipAddress		New IP address.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 */
void Controller::SetSecondDS100IpAddress(DataChangeSource changeSource, String ipAddress, bool dontSendNotification)
{
	if (m_SecondDS100IpAddress != ipAddress)
	{
		const ScopedLock lock(m_mutex);

		m_SecondDS100IpAddress = ipAddress;

		m_protocolBridge.SetSecondDS100IpAddress(ipAddress, dontSendNotification);

		// Signal the change to all Processors. 
		SetParameterChanged(changeSource, (DCT_IPAddress | DCT_Online));

		Reconnect();
	}
}

/**
 * Getter function for the DS100 bridging communication state.
 * @return		True if all communication channels are online.
 */
bool Controller::IsOnline() const
{
	switch (GetExtensionMode())
	{
	case ExtensionMode::EM_Off:
		return IsFirstDS100Online();
	case ExtensionMode::EM_Extend:
	case ExtensionMode::EM_Mirror:
	case ExtensionMode::EM_Parallel:
		return (IsFirstDS100Online() && IsSecondDS100Online());
	default:
		return false;
	}
}

/**
 * Getter function for the DS100 bridging communication state.
 * @return		True if communication channel with first DS100 is online.
 */
bool Controller::IsFirstDS100Online() const
{
	return ((m_protocolBridge.GetDS100State() & OHS_Protocol_Up) == OHS_Protocol_Up);
}

/**
 * Getter function for the DS100 bridging mirror state.
 * @return		True if first DS100 is currently master in mirror extension mode.
 */
bool Controller::IsFirstDS100MirrorMaster() const
{
	if (GetExtensionMode() != EM_Mirror)
		return false;

	return ((m_protocolBridge.GetDS100State() & OHS_Protocol_Master) == OHS_Protocol_Master);
}

/**
 * Getter function for the DS100 bridging communication state.
 * @return		True if communication channel with second DS100 is online.
 */
bool Controller::IsSecondDS100Online() const
{
	return ((m_protocolBridge.GetSecondDS100State() & OHS_Protocol_Up) == OHS_Protocol_Up);
}

/**
 * Getter function for the DS100 bridging mirror state.
 * @return		True if second DS100 is currently master in mirror extension mode.
 */
bool Controller::IsSecondDS100MirrorMaster() const
{
	if (GetExtensionMode() != EM_Mirror)
		return false;

	return ((m_protocolBridge.GetSecondDS100State() & OHS_Protocol_Master) == OHS_Protocol_Master);
}

/**
 * Getter for the rate at which OSC messages are being sent out.
 * @return	Messaging rate, in milliseconds.
 */
int Controller::GetRate() const
{
	return m_oscMsgRate;
}

/**
 * Setter for the rate at which OSC messages are being sent out.
 * @param changeSource	The application module which is causing the property change.
 * @param rate	New messaging rate, in milliseconds.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 */
void Controller::SetRate(DataChangeSource changeSource, int rate, bool dontSendNotification)
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
std::pair<int, int> Controller::GetSupportedRateRange()
{
	return std::pair<int, int>(PROTOCOL_INTERVAL_MIN, PROTOCOL_INTERVAL_MAX);
}

/**
 * Getter function for the IP address to which m_oscSender and m_oscReceiver are connected.
 * @return	Current IP address.
 */
ExtensionMode Controller::GetExtensionMode() const
{
	return m_DS100ExtensionMode;
}

/**
 * Setter function for the IP address to which m_oscSender and m_oscReceiver are connected.
 * NOTE: changing ip address will disconnect m_oscSender and m_oscReceiver.
 * @param changeSource	The application module which is causing the property change.
 * @param ipAddress		New IP address.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 */
void Controller::SetExtensionMode(DataChangeSource changeSource, ExtensionMode mode, bool dontSendNotification)
{
	if (m_DS100ExtensionMode != mode)
	{
		const ScopedLock lock(m_mutex);

		m_DS100ExtensionMode = mode;

		m_protocolBridge.SetDS100ExtensionMode(mode, dontSendNotification);

		// Signal the change to all Processors. 
		SetParameterChanged(changeSource, (DCT_ExtensionMode | DCT_Online));

		Reconnect();
	}
}

/**
 * Method to initialize IP address and polling rate.
 * @param changeSource	The application module which is causing the property change.
 * @param ipAddress		New IP address.
 * @param rate			New messaging rate, in milliseconds.
 */
void Controller::InitGlobalSettings(DataChangeSource changeSource, String ipAddress, int rate)
{
	SetDS100IpAddress(changeSource, ipAddress);
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
void Controller::HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData)
{
	jassert(nodeId == DEFAULT_PROCNODE_ID);
	if (nodeId != DEFAULT_PROCNODE_ID)
		return;

	if (ProtocolBridgingWrapper::IsBridgingObjectOnly(objectId))
	{
		// Do not handle any protocol data except what is received
		// from DS100 - any data that was sent by 3rd party devices 
		// is bridged to DS100 and returned by it 
		// so we can handle the data in the end as well
		if (senderProtocolId != DS100_1_PROCESSINGPROTOCOL_ID && senderProtocolId != DS100_2_PROCESSINGPROTOCOL_ID)
			return;
	}

	const ScopedLock lock(m_mutex);

	if (m_soundobjectProcessors.size() > 0)
	{
		int i;
		bool resetHeartbeat = false;

		// Check if the incoming message is a response to a sent "ping".
		if (objectId == RemoteObjectIdentifier::ROI_HeartbeatPong)
			resetHeartbeat = true;

		// Check if the incoming message contains parameters.
		else
		{
			SoundobjectParameterIndex pIdx = SPI_ParamIdx_MaxIndex;
			DataChangeType change = DCT_None;
			SoundobjectId soundobjectId = INVALID_ADDRESS_VALUE;
			MappingId mappingId = INVALID_ADDRESS_VALUE;

			// Determine which parameter was changed depending on the incoming message's address pattern.
			switch (objectId)
			{
			case RemoteObjectIdentifier::ROI_CoordinateMapping_SourcePosition_XY:
				{
					// The Source ID
					soundobjectId = msgData._addrVal._first;
					jassert(soundobjectId > 0);
					// The Mapping ID
					mappingId = msgData._addrVal._second;
					jassert(mappingId > 0);

					pIdx = SPI_ParamIdx_X;
					change = DCT_SoundobjectPosition;
				}
				break;
			case RemoteObjectIdentifier::ROI_MatrixInput_ReverbSendGain:
				{
					// The Source ID
					soundobjectId = msgData._addrVal._first;
					jassert(soundobjectId > 0);

					pIdx = SPI_ParamIdx_ReverbSendGain;
					change = DCT_ReverbSendGain;
				}
				break;
			case RemoteObjectIdentifier::ROI_Positioning_SourceSpread:
				{
					// The Source ID
					soundobjectId = msgData._addrVal._first;
					jassert(soundobjectId > 0);

					pIdx = SPI_ParamIdx_ObjectSpread;
					change = DCT_SoundobjectSpread;
				}
				break;
			case RemoteObjectIdentifier::ROI_Positioning_SourceDelayMode:
				{
					// The Source ID
					soundobjectId = msgData._addrVal._first;
					jassert(soundobjectId > 0);

					pIdx = SPI_ParamIdx_DelayMode;
					change = DCT_DelayMode;
				}
				break;
			case RemoteObjectIdentifier::ROI_RemoteProtocolBridge_SoundObjectSelect:
			case RemoteObjectIdentifier::ROI_MatrixInput_Select:
				{
					// The Source ID
					soundobjectId = msgData._addrVal._first;
					jassert(soundobjectId > 0);

					jassert(msgData._valCount == 1 && msgData._valType == RemoteObjectValueType::ROVT_INT);

					change = DCT_ProcessorSelection;
				}
				break;
			case RemoteObjectIdentifier::ROI_RemoteProtocolBridge_UIElementIndexSelect:
				{
					jassert(msgData._valCount == 1 && msgData._valType == RemoteObjectValueType::ROVT_INT);

					change = DCT_TabPageSelection;
				}
				break;
			default:
				break;
			}

			// If source id is present, it needs to be checked regarding special DS100 extension mode
			if (soundobjectId > 0)
			{
				if (senderProtocolId == DS100_2_PROCESSINGPROTOCOL_ID && GetExtensionMode() == EM_Extend)
					soundobjectId += DS100_CHANNELCOUNT;
			}

			// now process what changes were detected to be neccessary to perform
			if (change == DCT_ProcessorSelection)
			{
				if (msgData._valCount == 1 && msgData._valType == RemoteObjectValueType::ROVT_INT)
				{
					auto newSelectState = (static_cast<int*>(msgData._payload)[0] == 1);
					if (IsSoundobjectIdSelected(soundobjectId) != newSelectState)
					{
						SetSoundobjectIdSelectState(soundobjectId, newSelectState);
						SetParameterChanged(DCS_Protocol, DCT_ProcessorSelection);
					}
				}
			}
			else if (change == DCT_TabPageSelection)
			{
				if (msgData._valCount == 1 && msgData._valType == RemoteObjectValueType::ROVT_INT)
				{
					auto pageMgr = PageComponentManager::GetInstance();
					if (pageMgr)
					{
						auto tabIndex = static_cast<int*>(msgData._payload)[0];
						pageMgr->SetActiveTab(tabIndex, dontSendNotification);
					}
				}
			}
			// Continue if the message's address pattern was recognized 
			else if (change != DCT_None)
			{
				// Check all Processor instances to see if any of them want the new coordinates.
				for (i = 0; i < m_soundobjectProcessors.size(); ++i)
				{
					// Check for matching Input number.
					SoundobjectProcessor* processor = m_soundobjectProcessors[i];
					if (soundobjectId == processor->GetSoundobjectId())
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
							if (pIdx == SPI_ParamIdx_X)
							{
								if (mappingId == processor->GetMappingId())
								{
									jassert(msgData._valCount == 2 && msgData._valType == RemoteObjectValueType::ROVT_FLOAT);
									// Set the processor's new position.
									processor->SetParameterValue(DCS_Protocol, SPI_ParamIdx_X, static_cast<float*>(msgData._payload)[0]);
									processor->SetParameterValue(DCS_Protocol, SPI_ParamIdx_Y, static_cast<float*>(msgData._payload)[1]);

									// A request was sent to the DS100 by the Controller because this processor was in CM_PollOnce mode.
									// Since the response was now processed, set the processor back into it's original mode.
									if ((mode & CM_PollOnce) == CM_PollOnce)
									{
										mode &= ~CM_PollOnce;
										processor->SetComsMode(DCS_Host, mode);
									}
								}
							}

							// All other automation parameters.
							else
							{
								float newValue;
								switch (msgData._valType)
								{
								case RemoteObjectValueType::ROVT_INT:
									newValue = static_cast<float>(static_cast<int*>(msgData._payload)[0]);
									break;
								case RemoteObjectValueType::ROVT_FLOAT:
									newValue = static_cast<float*>(msgData._payload)[0];
									break;
								case RemoteObjectValueType::ROVT_STRING:
									newValue = std::stof(std::string(static_cast<char*>(msgData._payload)));
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
}

/**
 * Disconnect the active bridging nodes' protocols.
 */
void Controller::Disconnect()
{
	m_protocolBridge.Disconnect();
}

/**
 * Disconnect and re-connect the OSCSender to a host specified by the current ip settings.
 */
void Controller::Reconnect()
{
	m_protocolBridge.Reconnect();
}

/**
 * Timer callback function, which will be called at regular intervals to
 * send out OSC messages for the parameters that have been changed on UI.
 * 
 * Reimplemented from base class Timer.
 */
void Controller::timerCallback()
{
	const ScopedLock lock(m_mutex);
	if (m_soundobjectProcessors.size() > 0)
	{
		float newDualFloatValue[2];
		RemoteObjectMessageData newMsgData;

		for (auto const& soProcessor : m_soundobjectProcessors)
		{
			auto comsMode = soProcessor->GetComsMode();

			// Check if the processor configuration has changed
			// and need to be updated in the bridging configuration
			if (soProcessor->GetParameterChanged(DCS_SoundobjectTable, DCT_ProcessorInstanceConfig))
			{
				auto activateSSId = false;
				auto deactivateSSId = false;
				if (soProcessor->GetParameterChanged(DCS_SoundobjectTable, DCT_SoundobjectID))
				{
					// SoundsourceID change means update is only required when
					// remote object is currently activated. 
					activateSSId = ((comsMode & CM_Rx) == CM_Rx);
				}
				soProcessor->PopParameterChanged(DCS_SoundobjectTable, DCT_SoundobjectID);

				if (soProcessor->GetParameterChanged(DCS_SoundobjectTable, DCT_MappingID))
				{
					// MappingID change means update is only required when
					// remote object is currently activated. 
					activateSSId = ((comsMode & CM_Rx) == CM_Rx);
				}
				soProcessor->PopParameterChanged(DCS_SoundobjectTable, DCT_MappingID);

				if (soProcessor->GetParameterChanged(DCS_SoundobjectTable, DCT_ComsMode))
				{
					// ComsMode change means toggling polling for the remote object,
					// so one of the two activate/deactivate actions is required
					activateSSId = ((comsMode & CM_Rx) == CM_Rx);
					deactivateSSId = !activateSSId;
				}
				soProcessor->PopParameterChanged(DCS_SoundobjectTable, DCT_ComsMode);

				if (activateSSId)
					ActivateSoundobjectId(soProcessor->GetSoundobjectId(), soProcessor->GetMappingId());
				else if (deactivateSSId)
					DeactivateSoundobjectId(soProcessor->GetSoundobjectId(), soProcessor->GetMappingId());
			}

			// Signal every timer tick to each processor instance.
			soProcessor->Tick();

			bool msgSent;
			DataChangeType paramSetsInTransit = DCT_None;

			newMsgData._addrVal._first = static_cast<juce::uint16>(soProcessor->GetSoundobjectId());
			newMsgData._addrVal._second = static_cast<juce::uint16>(soProcessor->GetMappingId());

			// Iterate through all automation parameters.
			for (int pIdx = SPI_ParamIdx_X; pIdx < SPI_ParamIdx_MaxIndex; ++pIdx)
			{
				msgSent = false;

				switch (pIdx)
				{
					case SPI_ParamIdx_X:
					{
						// SET command is only sent out while in CM_Tx mode, provided that
						// this parameter has been changed since the last timer tick.
						if (((comsMode & CM_Tx) == CM_Tx) && soProcessor->GetParameterChanged(DCS_Protocol, DCT_SoundobjectPosition))
						{
							newDualFloatValue[0] = soProcessor->GetParameterValue(SPI_ParamIdx_X);
							newDualFloatValue[1] = soProcessor->GetParameterValue(SPI_ParamIdx_Y);

							newMsgData._valCount = 2;
							newMsgData._valType = ROVT_FLOAT;
							newMsgData._payload = &newDualFloatValue;
							newMsgData._payloadSize = 2 * sizeof(float);

							msgSent = m_protocolBridge.SendMessage(ROI_CoordinateMapping_SourcePosition_XY, newMsgData);
							paramSetsInTransit |= DCT_SoundobjectPosition;
						}
					}
					break;

					case SPI_ParamIdx_Y:
						// Changes to ParamIdx_Y are handled together with ParamIdx_X, so skip it.
						continue;
						break;

					case SPI_ParamIdx_ReverbSendGain:
					{
						// SET command is only sent out while in CM_Tx mode, provided that
						// this parameter has been changed since the last timer tick.
						if (((comsMode & CM_Tx) == CM_Tx) && soProcessor->GetParameterChanged(DCS_Protocol, DCT_ReverbSendGain))
						{
							newDualFloatValue[0] = soProcessor->GetParameterValue(SPI_ParamIdx_ReverbSendGain);

							newMsgData._valCount = 1;
							newMsgData._valType = ROVT_FLOAT;
							newMsgData._payload = &newDualFloatValue;
							newMsgData._payloadSize = sizeof(float);

							msgSent = m_protocolBridge.SendMessage(ROI_MatrixInput_ReverbSendGain, newMsgData);
							paramSetsInTransit |= DCT_ReverbSendGain;
						}
					}
					break;

					case SPI_ParamIdx_ObjectSpread:
					{
						// SET command is only sent out while in CM_Tx mode, provided that
						// this parameter has been changed since the last timer tick.
						if (((comsMode & CM_Tx) == CM_Tx) && soProcessor->GetParameterChanged(DCS_Protocol, DCT_SoundobjectSpread))
						{
							newDualFloatValue[0] = soProcessor->GetParameterValue(SPI_ParamIdx_ObjectSpread);

							newMsgData._valCount = 1;
							newMsgData._valType = ROVT_FLOAT;
							newMsgData._payload = &newDualFloatValue;
							newMsgData._payloadSize = sizeof(float);

							msgSent = m_protocolBridge.SendMessage(ROI_Positioning_SourceSpread, newMsgData);
							paramSetsInTransit |= DCT_SoundobjectSpread;
						}
					}
					break;

					case SPI_ParamIdx_DelayMode:
					{
						// SET command is only sent out while in CM_Tx mode, provided that
						// this parameter has been changed since the last timer tick.
						if (((comsMode & CM_Tx) == CM_Tx) && soProcessor->GetParameterChanged(DCS_Protocol, DCT_DelayMode))
						{
							newDualFloatValue[0] = soProcessor->GetParameterValue(SPI_ParamIdx_DelayMode);

							newMsgData._valCount = 1;
							newMsgData._valType = ROVT_FLOAT;
							newMsgData._payload = &newDualFloatValue;
							newMsgData._payloadSize = sizeof(float);

							msgSent = m_protocolBridge.SendMessage(ROI_Positioning_SourceDelayMode, newMsgData);
							paramSetsInTransit |= DCT_DelayMode;
						}
					}
					break;

					default:
						jassertfalse;
						break;
				}
			}

			// Flag the parameters for which we just sent a SET command out.
			soProcessor->SetParamInTransit(paramSetsInTransit);

			// All changed parameters were sent out, so we can reset their flags now.
			soProcessor->PopParameterChanged(DCS_Protocol, DCT_SoundobjectParameters);
		}

		for (auto const& mcProcessor : m_matrixChannelProcessors)
		{
			auto comsMode = mcProcessor->GetComsMode();

			// Check if the processor configuration has changed
			// and need to be updated in the bridging configuration
			if (mcProcessor->GetParameterChanged(DCS_SoundobjectTable, DCT_ProcessorInstanceConfig))
			{
				auto activateSSId = false;
				auto deactivateSSId = false;
				if (mcProcessor->GetParameterChanged(DCS_SoundobjectTable, DCT_SoundobjectID))
				{
					// SoundsourceID change means update is only required when
					// remote object is currently activated. 
					activateSSId = ((comsMode & CM_Rx) == CM_Rx);
				}
				mcProcessor->PopParameterChanged(DCS_SoundobjectTable, DCT_SoundobjectID);

				if (mcProcessor->GetParameterChanged(DCS_SoundobjectTable, DCT_MappingID))
				{
					// MappingID change means update is only required when
					// remote object is currently activated. 
					activateSSId = ((comsMode & CM_Rx) == CM_Rx);
				}
				mcProcessor->PopParameterChanged(DCS_SoundobjectTable, DCT_MappingID);

				if (mcProcessor->GetParameterChanged(DCS_SoundobjectTable, DCT_ComsMode))
				{
					// ComsMode change means toggling polling for the remote object,
					// so one of the two activate/deactivate actions is required
					activateSSId = ((comsMode & CM_Rx) == CM_Rx);
					deactivateSSId = !activateSSId;
				}
				mcProcessor->PopParameterChanged(DCS_SoundobjectTable, DCT_ComsMode);

				if (activateSSId)
					ActivateMatrixChannelId(mcProcessor->GetMatrixChannelId());
				else if (deactivateSSId)
					DeactivateMatrixChannelId(mcProcessor->GetMatrixChannelId());
			}

			// Signal every timer tick to each processor instance.
			mcProcessor->Tick();

			bool msgSent;
			DataChangeType paramSetsInTransit = DCT_None;

			newMsgData._addrVal._first = static_cast<juce::uint16>(mcProcessor->GetMatrixChannelId());

			// Iterate through all automation parameters.
			for (int pIdx = SPI_ParamIdx_X; pIdx < SPI_ParamIdx_MaxIndex; ++pIdx)
			{
				msgSent = false;

				switch (pIdx)
				{
				case SPI_ParamIdx_X:
				{
					// SET command is only sent out while in CM_Tx mode, provided that
					// this parameter has been changed since the last timer tick.
					if (((comsMode & CM_Tx) == CM_Tx) && mcProcessor->GetParameterChanged(DCS_Protocol, DCT_SoundobjectPosition))
					{
						newDualFloatValue[0] = mcProcessor->GetParameterValue(SPI_ParamIdx_X);
						newDualFloatValue[1] = mcProcessor->GetParameterValue(SPI_ParamIdx_Y);

						newMsgData._valCount = 2;
						newMsgData._valType = ROVT_FLOAT;
						newMsgData._payload = &newDualFloatValue;
						newMsgData._payloadSize = 2 * sizeof(float);

						msgSent = m_protocolBridge.SendMessage(ROI_CoordinateMapping_SourcePosition_XY, newMsgData);
						paramSetsInTransit |= DCT_SoundobjectPosition;
					}
				}
				break;

				case SPI_ParamIdx_Y:
					// Changes to ParamIdx_Y are handled together with ParamIdx_X, so skip it.
					continue;
					break;

				case SPI_ParamIdx_ReverbSendGain:
				{
					// SET command is only sent out while in CM_Tx mode, provided that
					// this parameter has been changed since the last timer tick.
					if (((comsMode & CM_Tx) == CM_Tx) && mcProcessor->GetParameterChanged(DCS_Protocol, DCT_ReverbSendGain))
					{
						newDualFloatValue[0] = mcProcessor->GetParameterValue(SPI_ParamIdx_ReverbSendGain);

						newMsgData._valCount = 1;
						newMsgData._valType = ROVT_FLOAT;
						newMsgData._payload = &newDualFloatValue;
						newMsgData._payloadSize = sizeof(float);

						msgSent = m_protocolBridge.SendMessage(ROI_MatrixInput_ReverbSendGain, newMsgData);
						paramSetsInTransit |= DCT_ReverbSendGain;
					}
				}
				break;

				case SPI_ParamIdx_ObjectSpread:
				{
					// SET command is only sent out while in CM_Tx mode, provided that
					// this parameter has been changed since the last timer tick.
					if (((comsMode & CM_Tx) == CM_Tx) && mcProcessor->GetParameterChanged(DCS_Protocol, DCT_SoundobjectSpread))
					{
						newDualFloatValue[0] = mcProcessor->GetParameterValue(SPI_ParamIdx_ObjectSpread);

						newMsgData._valCount = 1;
						newMsgData._valType = ROVT_FLOAT;
						newMsgData._payload = &newDualFloatValue;
						newMsgData._payloadSize = sizeof(float);

						msgSent = m_protocolBridge.SendMessage(ROI_Positioning_SourceSpread, newMsgData);
						paramSetsInTransit |= DCT_SoundobjectSpread;
					}
				}
				break;

				case SPI_ParamIdx_DelayMode:
				{
					// SET command is only sent out while in CM_Tx mode, provided that
					// this parameter has been changed since the last timer tick.
					if (((comsMode & CM_Tx) == CM_Tx) && mcProcessor->GetParameterChanged(DCS_Protocol, DCT_DelayMode))
					{
						newDualFloatValue[0] = mcProcessor->GetParameterValue(SPI_ParamIdx_DelayMode);

						newMsgData._valCount = 1;
						newMsgData._valType = ROVT_FLOAT;
						newMsgData._payload = &newDualFloatValue;
						newMsgData._payloadSize = sizeof(float);

						msgSent = m_protocolBridge.SendMessage(ROI_Positioning_SourceDelayMode, newMsgData);
						paramSetsInTransit |= DCT_DelayMode;
					}
				}
				break;

				default:
					jassertfalse;
					break;
				}
			}

			// Flag the parameters for which we just sent a SET command out.
			mcProcessor->SetParamInTransit(paramSetsInTransit);

			// All changed parameters were sent out, so we can reset their flags now.
			mcProcessor->PopParameterChanged(DCS_Protocol, DCT_MatrixChannelParameters);
		}
	}
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to set this objects' settings
 * from a XML element structure that passed as argument.
 * @param stateXml	The XML element containing this objects' configuration data
 * @return	True if the data was read and handled successfuly, false if not.
 */
bool Controller::setStateXml(XmlElement* stateXml)
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
			for (auto processor : m_soundobjectProcessors)
				if (processor->GetProcessorId() == elementProcessorId)
				{
					processor->setStateXml(processorXmlElement);
					alreadyExists = true;
				}

			if (!alreadyExists)
			{
				auto newProcessor =	std::make_unique<SoundobjectProcessor>(false);
				newProcessor->SetProcessorId(DCS_Init, elementProcessorId);
				auto p = newProcessor.release();
				jassert(m_soundobjectProcessors.contains(p));
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
			SetExtensionMode(DataChangeSource::DCS_Init, m_protocolBridge.GetDS100ExtensionMode(), true);
			SetDS100IpAddress(DataChangeSource::DCS_Init, m_protocolBridge.GetDS100IpAddress(), true);
			SetSecondDS100IpAddress(DataChangeSource::DCS_Init, m_protocolBridge.GetSecondDS100IpAddress(), true);
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
std::unique_ptr<XmlElement> Controller::createStateXml()
{
	auto controllerXmlElement = std::make_unique<XmlElement>(AppConfiguration::getTagName(AppConfiguration::TagID::CONTROLLER));

	auto processorsXmlElement = controllerXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::SOUNDSOURCEPROCESSORS));
	if (processorsXmlElement)
	{
		for (auto processor : m_soundobjectProcessors)
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
 * Helper method to get a list of currently active remote objects.
 * This is generated by dumping all active processor properties and their objects to a list.
 * @return	The list of currently active remote objects.
 */
const std::vector<RemoteObject> Controller::GetActivatedSoundObjectRemoteObjects()
{
	std::vector<RemoteObject> activeRemoteObjects;
	for (auto const& processor : m_soundobjectProcessors)
	{
		if ((processor->GetComsMode() & CM_Rx) == CM_Rx)
		{
			for (auto const& roi : SoundobjectProcessor::GetUsedRemoteObjects())
			{
				auto sourceId = processor->GetSoundobjectId();
				auto mappingId = processor->GetMappingId();
				if (sourceId != INVALID_ADDRESS_VALUE)
				{
					if (ProcessingEngineConfig::IsRecordAddressingObject(roi) && mappingId != INVALID_ADDRESS_VALUE)
						activeRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(sourceId, mappingId)));
					else if (!ProcessingEngineConfig::IsRecordAddressingObject(roi))
						activeRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(sourceId, INVALID_ADDRESS_VALUE)));
				}
			}
		}
	}
	return activeRemoteObjects;
}

/**
 * Helper method to get a list of currently active remote objects.
 * This is generated by dumping all active processor properties and their objects to a list.
 * @return	The list of currently active remote objects.
 */
const std::vector<RemoteObject> Controller::GetActivatedMatrixChannelRemoteObjects()
{
	std::vector<RemoteObject> activeRemoteObjects;
	for (auto const& processor : m_matrixChannelProcessors)
	{
		if ((processor->GetComsMode() & CM_Rx) == CM_Rx)
		{
			for (auto const& roi : MatrixChannelProcessor::GetUsedRemoteObjects())
			{
				auto sourceId = processor->GetMatrixChannelId();
				if (sourceId != INVALID_ADDRESS_VALUE)
				{
					if (ProcessingEngineConfig::IsRecordAddressingObject(roi))
						activeRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(sourceId, INVALID_ADDRESS_VALUE)));
					else if (!ProcessingEngineConfig::IsRecordAddressingObject(roi))
						activeRemoteObjects.push_back(RemoteObject(roi, RemoteObjectAddressing(sourceId, INVALID_ADDRESS_VALUE)));
				}
			}
		}
	}
	return activeRemoteObjects;
}

/**
 * Activates the remote objects in protocol bridge proxy corresponding to given source/mapping via proxy bridge object
 * @param soundobjectId	The soundsource object id to activate
 * @param mappingId	The soundsource mapping id to activate
 */
void Controller::ActivateSoundobjectId(SoundobjectId soundobjectId, MappingId mappingId)
{
	ignoreUnused(soundobjectId);
	ignoreUnused(mappingId);

	m_protocolBridge.UpdateActiveDS100RemoteObjectIds();
}

/**
 * Deactivates the remote objects in protocol bridge proxy corresponding to given source/mapping via proxy bridge object
 * @param soundobjectId	The soundsource object id to activate
 * @param mappingId	The soundsource mapping id to activate
 */
void Controller::DeactivateSoundobjectId(SoundobjectId soundobjectId, MappingId mappingId)
{
	ignoreUnused(soundobjectId);
	ignoreUnused(mappingId);

	m_protocolBridge.UpdateActiveDS100RemoteObjectIds();
}

/**
 * Method to set a list of soundsource ids to be selected, based on given list of processorIds.
 * This affects the internal map of soundsource select states and triggers setting/updating table/multislider pages.
 * The additional bool is used to indicate if the current selection shall be extended or cleared and be replaced by new selection.
 * @param processorIds	The list of processorIds to use to set internal map of soundsourceids selected state
 * @param clearPrevSelection	Use to indicate if previously active selection shall be replaced or extended.
 */
void Controller::SetSelectedSoundobjectProcessorIds(const std::vector<SoundobjectProcessorId>& processorIds, bool clearPrevSelection)
{
	if (clearPrevSelection)
	{
		// clear all selected soundobject ids
		m_soundObjectSelection.clear();

		// iterate through all processors and set each selected state based on given selection list
		for (auto const& processorId : GetSoundobjectProcessorIds())
		{
			auto processor = GetSoundobjectProcessor(processorId);
			if (processor)
				SetSoundobjectIdSelectState(processor->GetSoundobjectId(), std::find(processorIds.begin(), processorIds.end(), processorId) != processorIds.end());
		}
	}
	else
	{
		// iterate through selection list and set all contained processor ids to selected
		for (auto const& processorId : processorIds)
		{
			auto processor = GetSoundobjectProcessor(processorId);
			if (processor)
				SetSoundobjectIdSelectState(processor->GetSoundobjectId(), true);
		}
	}
}

/**
 * Method to get the list of currently selected processors.
 * This internally accesses the list of processors and selected soundsourceids and combines the info in new list.
 * @return The list of currently selected processors.
 */
const std::vector<SoundobjectProcessorId> Controller::GetSelectedSoundobjectProcessorIds()
{
	std::vector<SoundobjectProcessorId> processorIds;
	processorIds.reserve(m_soundObjectSelection.size());
	for (auto const& processor : m_soundobjectProcessors)
	{
		auto sourceId = processor->GetSoundobjectId();
		if ((m_soundObjectSelection.count(sourceId) > 0) && m_soundObjectSelection.at(sourceId))
			processorIds.push_back(processor->GetProcessorId());
	}

	return processorIds;
}

/**
 * Method to set a soundsource to be selected. This affects the internal map of soundsource select states
 * and triggers setting/updating table/multislider pages.
 * @param sourceId	The sourceId to modify regarding selected state
 * @param selected	The selected state to set.
 */
void Controller::SetSoundobjectIdSelectState(SoundobjectId soundobjectId, bool selected)
{
	m_soundObjectSelection[soundobjectId] = selected;
}

/**
 * Method to get a soundsource id selected state.
 * @param sourceId	The sourceId to modify regarding selected state
 * @param selected	The selected state to set.
 */
bool Controller::IsSoundobjectIdSelected(SoundobjectId soundobjectId)
{
	if (m_soundObjectSelection.count(soundobjectId) > 0)
		return m_soundObjectSelection.at(soundobjectId);
	else
		return false;
}


/**
 * Activates the remote objects in protocol bridge proxy corresponding to given MatrixChannel/mapping via proxy bridge object
 * @param matrixChannelId	The soundsource object id to activate
 */
void Controller::ActivateMatrixChannelId(MatrixChannelId matrixChannelId)
{
	ignoreUnused(matrixChannelId);

	m_protocolBridge.UpdateActiveDS100RemoteObjectIds();
}

/**
 * Deactivates the remote objects in protocol bridge proxy corresponding to given MatrixChannel/mapping via proxy bridge object
 * @param matrixChannelId	The soundsource object id to activate
 */
void Controller::DeactivateMatrixChannelId(MatrixChannelId matrixChannelId)
{
	ignoreUnused(matrixChannelId);

	m_protocolBridge.UpdateActiveDS100RemoteObjectIds();
}

/**
 * Method to set a list of soundsource ids to be selected, based on given list of processorIds.
 * This affects the internal map of soundsource select states and triggers setting/updating table/multislider pages.
 * The additional bool is used to indicate if the current selection shall be extended or cleared and be replaced by new selection.
 * @param processorIds	The list of processorIds to use to set internal map of soundsourceids selected state
 * @param clearPrevSelection	Use to indicate if previously active selection shall be replaced or extended.
 */
void Controller::SetSelectedMatrixChannelProcessorIds(const std::vector<MatrixChannelProcessorId>& processorIds, bool clearPrevSelection)
{
	if (clearPrevSelection)
	{
		// clear all selected soundobject ids
		m_matrixChannelSelection.clear();

		// iterate through all processors and set each selected state based on given selection list
		for (auto const& processorId : GetMatrixChannelProcessorIds())
		{
			auto processor = GetMatrixChannelProcessor(processorId);
			if (processor)
				SetMatrixChannelIdSelectState(processor->GetMatrixChannelId(), std::find(processorIds.begin(), processorIds.end(), processorId) != processorIds.end());
		}
	}
	else
	{
		// iterate through selection list and set all contained processor ids to selected
		for (auto const& processorId : processorIds)
		{
			auto processor = GetMatrixChannelProcessor(processorId);
			if (processor)
				SetMatrixChannelIdSelectState(processor->GetMatrixChannelId(), true);
		}
	}
}

/**
 * Method to get the list of currently selected processors.
 * This internally accesses the list of processors and selected MatrixChannelids and combines the info in new list.
 * @return The list of currently selected processors.
 */
const std::vector<MatrixChannelProcessorId> Controller::GetSelectedMatrixChannelProcessorIds()
{
	std::vector<MatrixChannelProcessorId> processorIds;
	processorIds.reserve(m_matrixChannelSelection.size());
	for (auto const& processor : m_matrixChannelProcessors)
	{
		auto sourceId = processor->GetMatrixChannelId();
		if ((m_matrixChannelSelection.count(sourceId) > 0) && m_matrixChannelSelection.at(sourceId))
			processorIds.push_back(processor->GetProcessorId());
	}

	return processorIds;
}

/**
 * Method to set a MatrixChannel to be selected. This affects the internal map of MatrixChannel select states
 * and triggers setting/updating MatrixChannel pages.
 * @param sourceId	The sourceId to modify regarding selected state
 * @param selected	The selected state to set.
 */
void Controller::SetMatrixChannelIdSelectState(MatrixChannelId matrixChannelId, bool selected)
{
	m_matrixChannelSelection[matrixChannelId] = selected;
}

/**
 * Method to get a MatrixChannel id selected state.
 * @param matrixChannelId	The sourceId to modify regarding selected state
 * @param selected	The selected state to set.
 */
bool Controller::IsMatrixChannelIdSelected(MatrixChannelId matrixChannelId)
{
	if (m_matrixChannelSelection.count(matrixChannelId) > 0)
		return m_matrixChannelSelection.at(matrixChannelId);
	else
		return false;
}



/**
 * Adds a given listener object to this controller instance's bridging wrapper object.
 * @param listener	The listener object to add to bridging wrapper.
 */
void Controller::AddProtocolBridgingWrapperListener(ProtocolBridgingWrapper::Listener* listener)
{
	m_protocolBridge.AddListener(listener);
}

/**
 * Getter for the active protocol bridging types (active protocols RoleB - those are used for bridging to DS100 running as RoleA, see RemoteProtocolBridge for details)
 * @return The bitfield containing all active bridging types
 */
ProtocolBridgingType Controller::GetActiveProtocolBridging()
{
	return m_protocolBridge.GetActiveBridgingProtocols();
}

/**
 * Getter for currently active bridging protocols count.
 * @return The number of currently active bridging protocols.
 */
int Controller::GetActiveProtocolBridgingCount()
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
	if ((activeBridging & PBT_YamahaOSC) == PBT_YamahaOSC)
		activeProtocolBridgingCount++;

	return activeProtocolBridgingCount;
}

/**
 * Setter for protocol bridging types that shall be active.
 * @param	bridgingTypes	Bitfield containing all types that are to be active.
 */
void Controller::SetActiveProtocolBridging(ProtocolBridgingType bridgingTypes)
{
	m_protocolBridge.SetActiveBridgingProtocols(bridgingTypes);
}

/**
 * Gets the mute state of the given source via proxy bridge object
 * @param sourceId The id of the source for which the mute state shall be returned
 * @return The mute state
 */
bool Controller::GetMuteBridgingSoundobjectId(ProtocolBridgingType bridgingType, SoundobjectId soundobjectId)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.GetMuteDiGiCoSoundobjectId(soundobjectId);
	case PBT_GenericOSC:
		return m_protocolBridge.GetMuteGenericOSCSoundobjectId(soundobjectId);
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.GetMuteRTTrPMSoundobjectId(soundobjectId);
	case PBT_GenericMIDI:
		return m_protocolBridge.GetMuteGenericMIDISoundobjectId(soundobjectId);
	case PBT_YamahaOSC:
		return m_protocolBridge.GetMuteYamahaOSCSoundobjectId(soundobjectId);
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
 * @param soundobjectId The id of the source that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool Controller::SetMuteBridgingSoundobjectId(ProtocolBridgingType bridgingType, SoundobjectId soundobjectId, bool mute)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.SetMuteDiGiCoSoundobjectId(soundobjectId, mute);
	case PBT_GenericOSC:
		return m_protocolBridge.SetMuteGenericOSCSoundobjectId(soundobjectId, mute);
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.SetMuteRTTrPMSoundobjectId(soundobjectId, mute);
	case PBT_GenericMIDI:
		return m_protocolBridge.SetMuteGenericMIDISoundobjectId(soundobjectId, mute);
	case PBT_YamahaOSC:
		return m_protocolBridge.SetMuteYamahaOSCSoundobjectId(soundobjectId, mute);
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

/**
 * Sets the given sources to be (un-)muted in DiGiCo protocol via proxy bridge object
 * @param sourceIds The ids of the sources that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool Controller::SetMuteBridgingSoundobjectIds(ProtocolBridgingType bridgingType, const std::vector<SoundobjectId>& soundobjectIds, bool mute)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.SetMuteDiGiCoSoundobjectIds(soundobjectIds, mute);
	case PBT_GenericOSC:
		return m_protocolBridge.SetMuteGenericOSCSoundobjectIds(soundobjectIds, mute);
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.SetMuteRTTrPMSoundobjectIds(soundobjectIds, mute);
	case PBT_GenericMIDI:
		return m_protocolBridge.SetMuteGenericMIDISoundobjectIds(soundobjectIds, mute);
	case PBT_YamahaOSC:
		return m_protocolBridge.SetMuteYamahaOSCSoundobjectIds(soundobjectIds, mute);
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

String Controller::GetBridgingIpAddress(ProtocolBridgingType bridgingType)
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
	case PBT_YamahaOSC:
		return m_protocolBridge.GetYamahaOSCIpAddress();
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	default:
		jassertfalse;
		return String();
	}
}

bool Controller::SetBridgingIpAddress(ProtocolBridgingType bridgingType, String ipAddress, bool dontSendNotification)
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
	case PBT_YamahaOSC:
		return m_protocolBridge.SetYamahaOSCIpAddress(ipAddress, dontSendNotification);
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	default:
		jassertfalse;
		return false;
	}
}

int Controller::GetBridgingListeningPort(ProtocolBridgingType bridgingType)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.GetDiGiCoListeningPort();
	case PBT_GenericOSC:
		return m_protocolBridge.GetGenericOSCListeningPort();
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.GetRTTrPMListeningPort();
	case PBT_YamahaOSC:
		return m_protocolBridge.GetYamahaOSCListeningPort();
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return INVALID_PORT_VALUE;
	}
}

bool Controller::SetBridgingListeningPort(ProtocolBridgingType bridgingType, int listeningPort, bool dontSendNotification)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.SetDiGiCoListeningPort(listeningPort, dontSendNotification);
	case PBT_GenericOSC:
		return m_protocolBridge.SetGenericOSCListeningPort(listeningPort, dontSendNotification);
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.SetRTTrPMListeningPort(listeningPort, dontSendNotification);
	case PBT_YamahaOSC:
		return m_protocolBridge.SetYamahaOSCListeningPort(listeningPort, dontSendNotification);
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

int Controller::GetBridgingRemotePort(ProtocolBridgingType bridgingType)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.GetDiGiCoRemotePort();
	case PBT_GenericOSC:
		return m_protocolBridge.GetGenericOSCRemotePort();
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.GetRTTrPMRemotePort();
	case PBT_YamahaOSC:
		return m_protocolBridge.GetYamahaOSCRemotePort();
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return INVALID_PORT_VALUE;
	}
}

bool Controller::SetBridgingRemotePort(ProtocolBridgingType bridgingType, int remotePort, bool dontSendNotification)
{
	switch (bridgingType)
	{
	case PBT_DiGiCo:
		return m_protocolBridge.SetDiGiCoRemotePort(remotePort, dontSendNotification);
	case PBT_GenericOSC:
		return m_protocolBridge.SetGenericOSCRemotePort(remotePort, dontSendNotification);
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.SetRTTrPMRemotePort(remotePort, dontSendNotification);
	case PBT_YamahaOSC:
		return m_protocolBridge.SetYamahaOSCRemotePort(remotePort, dontSendNotification);
	case PBT_GenericMIDI:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

int Controller::GetBridgingMappingArea(ProtocolBridgingType bridgingType)
{
	switch (bridgingType)
	{
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.GetRTTrPMMappingArea();
	case PBT_YamahaOSC:
		return m_protocolBridge.GetYamahaOSCMappingArea();
	case PBT_GenericMIDI:
		return m_protocolBridge.GetGenericMIDIMappingArea();
	case PBT_DiGiCo:
	case PBT_GenericOSC:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return INVALID_ADDRESS_VALUE;
	}
}

bool Controller::SetBridgingMappingArea(ProtocolBridgingType bridgingType, int mappingAreaId, bool dontSendNotification)
{
	switch (bridgingType)
	{
	case PBT_BlacktraxRTTrPM:
		return m_protocolBridge.SetRTTrPMMappingArea(mappingAreaId, dontSendNotification);
	case PBT_YamahaOSC:
		return m_protocolBridge.SetYamahaOSCMappingArea(mappingAreaId, dontSendNotification);
	case PBT_GenericMIDI:
		return m_protocolBridge.SetGenericMIDIMappingArea(mappingAreaId, dontSendNotification);
	case PBT_DiGiCo:
	case PBT_GenericOSC:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

String Controller::GetBridgingInputDeviceIdentifier(ProtocolBridgingType bridgingType)
{
	switch (bridgingType)
	{
	case PBT_GenericMIDI:
		return m_protocolBridge.GetGenericMIDIInputDeviceIdentifier();
	case PBT_YamahaOSC:
	case PBT_BlacktraxRTTrPM:
	case PBT_DiGiCo:
	case PBT_GenericOSC:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return String();
	}
}

bool Controller::SetBridgingInputDeviceIdentifier(ProtocolBridgingType bridgingType, const String& inputDeviceIdentifier, bool dontSendNotification)
{
	switch (bridgingType)
	{
	case PBT_GenericMIDI:
		return m_protocolBridge.SetGenericMIDIInputDeviceIdentifier(inputDeviceIdentifier, dontSendNotification);
	case PBT_BlacktraxRTTrPM:
	case PBT_DiGiCo:
	case PBT_GenericOSC:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

String Controller::GetBridgingOutputDeviceIdentifier(ProtocolBridgingType bridgingType)
{
	switch (bridgingType)
	{
	case PBT_GenericMIDI:
		return m_protocolBridge.GetGenericMIDIOutputDeviceIdentifier();
	case PBT_YamahaOSC:
	case PBT_BlacktraxRTTrPM:
	case PBT_DiGiCo:
	case PBT_GenericOSC:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return String();
	}
}

bool Controller::SetBridgingOutputDeviceIdentifier(ProtocolBridgingType bridgingType, const String& outputDeviceIdentifier, bool dontSendNotification)
{
	switch (bridgingType)
	{
	case PBT_GenericMIDI:
		return m_protocolBridge.SetGenericMIDIOutputDeviceIdentifier(outputDeviceIdentifier, dontSendNotification);
	case PBT_BlacktraxRTTrPM:
	case PBT_DiGiCo:
	case PBT_GenericOSC:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

JUCEAppBasics::MidiCommandRangeAssignment Controller::GetBridgingMidiAssignmentMapping(ProtocolBridgingType bridgingType, RemoteObjectIdentifier remoteObjectId)
{
	switch (bridgingType)
	{
	case PBT_GenericMIDI:
		return m_protocolBridge.GetGenericMIDIAssignmentMapping(remoteObjectId);
	case PBT_BlacktraxRTTrPM:
	case PBT_DiGiCo:
	case PBT_GenericOSC:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return JUCEAppBasics::MidiCommandRangeAssignment();
	}
}

bool Controller::SetBridgingMidiAssignmentMapping(ProtocolBridgingType bridgingType, RemoteObjectIdentifier remoteObjectId, const JUCEAppBasics::MidiCommandRangeAssignment& assignmentMapping, bool dontSendNotification)
{
	switch (bridgingType)
	{
	case PBT_GenericMIDI:
		return m_protocolBridge.SetGenericMIDIAssignmentMapping(remoteObjectId, assignmentMapping, dontSendNotification);
	case PBT_BlacktraxRTTrPM:
	case PBT_DiGiCo:
	case PBT_GenericOSC:
	case PBT_YamahaSQ:
	case PBT_HUI:
	case PBT_DS100:
	default:
		jassertfalse;
		return false;
	}
}

/**
 * Method to load a given input file as the new application configuration.
 * This tries to handle possible errors and shows a popup to the user in case an error was detected.
 * @param fileToLoadFrom	The input file to load the new app config from.
 * @return	True on succes, false on failure
 */
bool Controller::LoadConfigurationFile(const File& fileToLoadFrom)
{
	auto config = SoundscapeBridgeApp::AppConfiguration::getInstance();
	auto xmlConfig = juce::parseXML(fileToLoadFrom);

	if (!config)
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Error", "Loading failed du to internal error.");
	else if (!xmlConfig)
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Invalid config", "Loading failed du to invalid selected configuration file.");
	else if (!SoundscapeBridgeApp::AppConfiguration::isValid(xmlConfig))
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Loading failed", "Loading failed du to invalid configuration file contents.");
	else if (!config->resetConfigState(std::move(xmlConfig)))
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Loading failed", "Loading failed du to internal loading error.");
	else
	{
		SetParameterChanged(DCS_Init, DCT_AllConfigParameters);
		return true;
	}

	return false;
}

/**
 * Method to save the current application configuration to a given input file.
 * This tries to handle possible errors and shows a popup to the user in case an error was detected.
 * @param fileToSaveTo	The output file to save the current config to.
 * @return	True on succes, false on failure
 */
bool Controller::SaveConfigurationFile(const File& fileToSaveTo)
{
	auto config = SoundscapeBridgeApp::AppConfiguration::getInstance();
	auto xmlConfig = config->getConfigState();

	if (!config)
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Error", "Saving failed du to internal error.");
	else if (!xmlConfig)
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Invalid", "Loading failed du to invalid internal configuration.");
	else if (!xmlConfig->writeTo(fileToSaveTo))
		AlertWindow::showMessageBox(AlertWindow::AlertIconType::WarningIcon, "Saving failed", "Saving failed due to insufficient write access rights.");
	else
		return true;

	return false;
}

} // namespace SoundscapeBridgeApp
