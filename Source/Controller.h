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


#pragma once

#include "SoundscapeBridgeAppCommon.h"
#include "AppConfiguration.h"
#include "ProtocolBridgingWrapper.h"



namespace SoundscapeBridgeApp
{


/**
 * Forward declarations.
 */
class SoundsourceProcessor;


/**
 * Class CController which takes care of protocol communication through protocolbridging wrapper, including connection establishment
 * and sending/receiving of messages over the network.
 * NOTE: This is a singleton class, i.e. there is only one instance.
 */
class CController :
	private Timer,
	public AppConfiguration::XmlConfigurableElement,
	public ProtocolBridgingWrapper::Listener
{
public:
	CController();
	~CController() override;
	static CController* GetInstance();
	void DestroyInstance();

	bool GetParameterChanged(DataChangeSource changeSource, DataChangeType change);
	bool PopParameterChanged(DataChangeSource changeSource, DataChangeType change);
	void SetParameterChanged(DataChangeSource changeSource, DataChangeType changeTypes);

	void createNewProcessor();
	ProcessorId AddProcessor(DataChangeSource changeSource, SoundsourceProcessor* p);
	void RemoveProcessor(SoundsourceProcessor* p);
	int GetProcessorCount() const;
	SoundsourceProcessor* GetProcessor(ProcessorId idx) const;

	//==========================================================================
	String GetIpAddress() const;
	static String GetDefaultIpAddress();
	void SetIpAddress(DataChangeSource changeSource, String ipAddress, bool dontSendNotification = false);

	//==========================================================================
	int GetRate() const;
	void SetRate(DataChangeSource changeSource, int rate, bool dontSendNotification = false);
	static std::pair<int, int> GetSupportedRateRange();

	//==========================================================================
	void ActivateSoundSourceId(SourceId sourceId, MappingId mappingId);
	void DeactivateSoundSourceId(SourceId sourceId, MappingId mappingId);

	//==========================================================================
	ProtocolBridgingType GetActiveProtocolBridging();
	void SetActiveProtocolBridging(ProtocolBridgingType bridgingType);
	int GetActiveProtocolBridgingCount();
	
	bool GetMuteBridgingSourceId(ProtocolBridgingType bridgingType, juce::int16 sourceId);
	bool SetMuteBridgingSourceId(ProtocolBridgingType bridgingType, juce::int16 sourceId, bool mute);

	String GetBridgingIpAddress(ProtocolBridgingType bridgingType);
	bool SetBridgingIpAddress(ProtocolBridgingType bridgingType, String ipAddress, bool dontSendNotification = false);
	int GetBridgingListeningPort(ProtocolBridgingType bridgingType);
	bool SetBridgingListeningPort(ProtocolBridgingType bridgingType, int listeningPort, bool dontSendNotification = false);
	int GetBridgingRemotePort(ProtocolBridgingType bridgingType);
	bool SetBridgingRemotePort(ProtocolBridgingType bridgingType, int remotePort, bool dontSendNotification = false);
	int GetBridgingMappingArea(ProtocolBridgingType bridgingType);
	bool SetBridgingMappingArea(ProtocolBridgingType bridgingType, int mappingAreaId, bool dontSendNotification = false);

	//==========================================================================
	void InitGlobalSettings(DataChangeSource changeSource, String ipAddress, int rate);

	//==========================================================================
	void Disconnect();
	void Reconnect();
	bool GetOnline() const;

	//==========================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==========================================================================
	void AddProtocolBridgingWrapperListener(ProtocolBridgingWrapper::Listener* listener);

	//==========================================================================
	void HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData) override;

private:
	void timerCallback() override;

protected:
	
	static CController				*m_singleton;		/**< The one and only instance of CController. */
	Array<SoundsourceProcessor*>	m_processors;		/**< List of registered Plug-in processor instances.
														 * Incoming OSC messages will be forwarded to all processors on the list.
														 * When adding Plug-in instances to a project (i.e. one for each DAW track), this list will grow.
														 * When removing Plug-in instances from a project, this list will shrink. When the list becomes empty,
														 * The CController singleton object is no longer necessary and will destruct itself.
														 */
	ProtocolBridgingWrapper			m_protocolBridge;	/**< The wrapper for protocol bridging node, allowing to easily interface with it. */
	String							m_ipAddress;		/**< IP Address where OSC messages will be sent to / received from. */
	int								m_oscMsgRate;		/**< Interval at which OSC messages are sent to the host, in ms. */
	DataChangeType					m_parametersChanged[DCS_Max];	/**< Keep track of which OSC parameters have changed recently.
																	 * The array has one entry for each application module (see enum DataChangeSource). */
	int								m_heartBeatsRx;		/**< Number of timer intervals since the last successful OSC message was received. */
	int								m_heartBeatsTx;		/**< Number of timer intervals since the last OSC message was sent out. */
	CriticalSection					m_mutex;			/**< A re-entrant mutex. Safety first. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CController)
};


} // namespace SoundscapeBridgeApp
