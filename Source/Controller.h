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
#include "../submodules/RemoteProtocolBridge/Source/ProcessingEngineNode.h"


namespace SoundscapeBridgeApp
{


/**
 * Forward declarations.
 */
class SoundsourceProcessor;


/**
 * Class CController which takes care of OSC communication, including connection establishment
 * and sending/receiving of OSC messages over the network.
 * NOTE: This is a singleton class, i.e. there is only one instance.
 */
class CController :
	public ProcessingEngineNode::NodeListener,
	private Timer
{
public:
	CController();
	~CController() override;
	static CController* GetInstance();

	bool GetParameterChanged(DataChangeSource changeSource, DataChangeTypes change);
	bool PopParameterChanged(DataChangeSource changeSource, DataChangeTypes change);
	void SetParameterChanged(DataChangeSource changeSource, DataChangeTypes changeTypes);

	PluginId AddProcessor(SoundsourceProcessor* p);
	void RemoveProcessor(SoundsourceProcessor* p);
	int GetProcessorCount() const;
	SoundsourceProcessor* GetProcessor(PluginId idx) const;

	String GetIpAddress() const;
	static String GetDefaultIpAddress();
	void SetIpAddress(DataChangeSource changeSource, String ipAddress);

	int GetRate() const;
	void SetRate(DataChangeSource changeSource, int rate);
	static std::pair<int, int> GetSupportedRateRange();

	void InitGlobalSettings(DataChangeSource changeSource, String ipAddress, int rate);

	void Disconnect();
	void Reconnect();
	bool GetOnline() const;

	void CreateNodeConfiguration();
	void HandleNodeData(NodeId nodeId, ProtocolId senderProtocolId, ProtocolType senderProtocolType, RemoteObjectIdentifier objectId, RemoteObjectMessageData& msgData) override;
	bool SendMessage(RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData);

private:
	void timerCallback() override;

protected:
	/**
	 * The one and only instance of CController.
	 */
	static CController		*m_singleton;		

	/**
	 * List of registered Plug-in processor instances. 
	 * Incoming OSC messages will be forwarded to all processors on the list.
	 * When adding Plug-in instances to a project (i.e. one for each DAW track), this list will grow.
	 * When removing Plug-in instances from a project, this list will shrink. When the list becomes empty,
	 * The CController singleton object is no longer necessary and will destruct itself.
	 */
	Array<SoundsourceProcessor*>	m_processors;

	/**
	 * A processing engine node can send data to and receive data from multiple protocols that is encapsulates.
	 * Depending on the node configuration, there can exist two groups of protocols, A and B, that are handled 
	 * in a specific way to pass incoming and outgoing data to each other and this parent controller instance.
	 */
	ProcessingEngineNode	m_processingNode;	/**< The node that encapsulates the protocols that are used to send, receive and bridge data. */
	ProcessingEngineConfig	m_processingConfig;	/**< The configuration of the processing node. */

	/**
	 * IP Address where OSC messages will be sent to / received from.
	 */
	String					m_ipAddress;

	/**
	 * Interval at which OSC messages are sent to the host, in ms.
	 */
	int						m_oscMsgRate;

	/**
	 * Keep track of which OSC parameters have changed recently. 
	 * The array has one entry for each application module (see enum DataChangeSource).
	 */
	DataChangeTypes				m_parametersChanged[DCS_Max];

	/**
	 * Number of timer intervals since the last successful OSC message was received.
	 */
	int						m_heartBeatsRx;

	/**
	 * Number of timer intervals since the last OSC message was sent out.
	 */
	int						m_heartBeatsTx;

	/**
	 * A re-entrant mutex. Safety first.
	 */
	CriticalSection			m_mutex;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CController)
};


} // namespace SoundscapeBridgeApp
