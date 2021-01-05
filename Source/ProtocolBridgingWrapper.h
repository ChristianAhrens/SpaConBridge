/* Copyright (c) 2020-2021, Christian Ahrens
 *
 * This file is part of SoundscapeBridgeApp <https://github.com/ChristianAhrens/SoundscapeBridgeApp>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

#include "SoundscapeBridgeAppCommon.h"
#include "AppConfiguration.h"

#include <ProcessingEngine/ProcessingEngineNode.h>

#include <JuceHeader.h>


namespace SoundscapeBridgeApp
{

static const String PROTOCOL_DEFAULT_IP("127.0.0.1");	//< Default IP Address
static const String PROTOCOL_DEFAULT2_IP("127.0.0.2");	//< Default IP Address

static constexpr int PROTOCOL_DEFAULT_MAPPINGAREA = 1;	//< Mapping Area Id to use as default

static constexpr int PROTOCOL_DEFAULT_INPUTDEVICEINDEX = 0;	//< Input Device Index to use as default

static constexpr int RX_PORT_DS100_DEVICE = 50010;		//< UDP port which the DS100 is listening to for OSC
static constexpr int RX_PORT_DS100_HOST = 50011;		//< UDP port to which the DS100 will send OSC replies

static constexpr int RX_PORT_DIGICO_DEVICE = 50012;		//< UDP port which the DiGiCo console is listening to for OSC
static constexpr int RX_PORT_DIGICO_HOST = 50013;		//< UDP port to which the DiGiCo console will send OSC replies

static constexpr int RX_PORT_GENERICOSC_DEVICE = 50014;	//< UDP port which the generic OSC device is listening to for OSC
static constexpr int RX_PORT_GENERICOSC_HOST = 50015;	//< UDP port to which the generic OSC device will send OSC replies

static constexpr int RX_PORT_RTTRPM_HOST = 24100;		//< UDP port to which the Blacktrax tracker device will send RTTrPM data replies to (us)

static constexpr int RX_PORT_YAMAHAOSC_DEVICE = 50016;	//< UDP port which the Yamaha Rivage console is listening to for OSC
static constexpr int RX_PORT_YAMAHAOSC_HOST = 50017;	//< UDP port to which the Yamaha Rivage console will send OSC replies

/**
 * Pre-define processing bridge config values
 */
static constexpr int DEFAULT_PROCNODE_ID = 1;
static constexpr int DS100_1_PROCESSINGPROTOCOL_ID = 2;
static constexpr int DIGICO_PROCESSINGPROTOCOL_ID = 3;
static constexpr int RTTRPM_PROCESSINGPROTOCOL_ID = 5;
static constexpr int GENERICOSC_PROCESSINGPROTOCOL_ID = 4;
static constexpr int DS100_2_PROCESSINGPROTOCOL_ID = 6;
static constexpr int GENERICMIDI_PROCESSINGPROTOCOL_ID = 7;
static constexpr int YAMAHAOSC_PROCESSINGPROTOCOL_ID = 8;

class ProtocolBridgingWrapper :
	public ProcessingEngineNode::NodeListener,
	public AppConfiguration::XmlConfigurableElement
{
public:
	/**
	 * Abstract embedded interface class for message data handling
	 */
	class Listener
	{
	public:
		Listener() {};
		virtual ~Listener() {};

		/**
		 * Method to be overloaded by ancestors to act as an interface
		 * for handling of received message data
		 */
		virtual void HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData) = 0;
	};

public:
	ProtocolBridgingWrapper();
	~ProtocolBridgingWrapper();

	//==========================================================================
	void AddListener(ProtocolBridgingWrapper::Listener* listener);

	//==========================================================================
	void HandleNodeData(const ProcessingEngineNode::NodeCallbackMessage* callbackMessage) override;
	bool SendMessage(RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData);

	//==========================================================================
	ProtocolBridgingType GetActiveBridgingProtocols();
	void SetActiveBridgingProtocols(ProtocolBridgingType desiredActiveBridgingTypes);

	//==========================================================================
	bool UpdateActiveDS100SourceIds();

	String GetDS100IpAddress();
	bool SetDS100IpAddress(String ipAddress, bool dontSendNotification = false);

	int GetDS100MsgRate();
	bool SetDS100MsgRate(int msgRate, bool dontSendNotification = false);

	String GetSecondDS100IpAddress();
	bool SetSecondDS100IpAddress(String ipAddress, bool dontSendNotification = false);

	ExtensionMode GetDS100ExtensionMode();
	bool SetDS100ExtensionMode(ExtensionMode mode, bool dontSendNotification = false);

	//==========================================================================
	bool GetMuteDiGiCoSourceId(SourceId sourceId);
	bool SetMuteDiGiCoSourceId(SourceId sourceId, bool mute = true);
	bool SetMuteDiGiCoSourceIds(const std::vector<SourceId>& sourceIds, bool mute = true);

	String GetDiGiCoIpAddress();
	bool SetDiGiCoIpAddress(String ipAddress, bool dontSendNotification = false);
	int GetDiGiCoListeningPort();
	bool SetDiGiCoListeningPort(int listeningPort, bool dontSendNotification = false);
	int GetDiGiCoRemotePort();
	bool SetDiGiCoRemotePort(int remotePort, bool dontSendNotification = false);

	//==========================================================================
	bool GetMuteRTTrPMSourceId(SourceId sourceId);
	bool SetMuteRTTrPMSourceId(SourceId sourceId, bool mute = true);
	bool SetMuteRTTrPMSourceIds(const std::vector<SourceId>& sourceIds, bool mute = true);

	String GetRTTrPMIpAddress();
	bool SetRTTrPMIpAddress(String ipAddress, bool dontSendNotification = false);
	int GetRTTrPMListeningPort();
	bool SetRTTrPMListeningPort(int listeningPort, bool dontSendNotification = false);
	int GetRTTrPMRemotePort();
	bool SetRTTrPMRemotePort(int remotePort, bool dontSendNotification = false);
	int GetRTTrPMMappingArea();
	bool SetRTTrPMMappingArea(int mappingAreaId, bool dontSendNotification = false);

	//==========================================================================
	bool GetMuteGenericOSCSourceId(SourceId sourceId);
	bool SetMuteGenericOSCSourceId(SourceId sourceId, bool mute = true);
	bool SetMuteGenericOSCSourceIds(const std::vector<SourceId>& sourceIds, bool mute = true);

	String GetGenericOSCIpAddress();
	bool SetGenericOSCIpAddress(String ipAddress, bool dontSendNotification = false);
	int GetGenericOSCListeningPort();
	bool SetGenericOSCListeningPort(int listeningPort, bool dontSendNotification = false);
	int GetGenericOSCRemotePort();
	bool SetGenericOSCRemotePort(int remotePort, bool dontSendNotification = false);

	//==========================================================================
	bool GetMuteGenericMIDISourceId(SourceId sourceId);
	bool SetMuteGenericMIDISourceId(SourceId sourceId, bool mute = true);
	bool SetMuteGenericMIDISourceIds(const std::vector<SourceId>& sourceIds, bool mute = true);

	int GetGenericMIDIInputDeviceIndex();
	bool SetGenericMIDIInputDeviceIndex(int MIDIInputDeviceIndex, bool dontSendNotification = false);

	//==========================================================================
	bool GetMuteYamahaOSCSourceId(SourceId sourceId);
	bool SetMuteYamahaOSCSourceId(SourceId sourceId, bool mute = true);
	bool SetMuteYamahaOSCSourceIds(const std::vector<SourceId>& sourceIds, bool mute = true);

	String GetYamahaOSCIpAddress();
	bool SetYamahaOSCIpAddress(String ipAddress, bool dontSendNotification = false);
	int GetYamahaOSCListeningPort();
	bool SetYamahaOSCListeningPort(int listeningPort, bool dontSendNotification = false);
	int GetYamahaOSCRemotePort();
	bool SetYamahaOSCRemotePort(int remotePort, bool dontSendNotification = false);
	int GetYamahaOSCMappingArea();
	bool SetYamahaOSCMappingArea(int mappingAreaId, bool dontSendNotification = false);

	//==========================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==========================================================================
	void Disconnect();
	void Reconnect();

	//==========================================================================
	static bool IsBridgingObjectOnly(RemoteObjectIdentifier id);

private:
	//==========================================================================
	bool GetMuteProtocolSourceId(ProtocolId protocolId, SourceId sourceId);
	bool SetMuteProtocolSourceId(ProtocolId protocolId, SourceId sourceId);
	bool SetMuteProtocolSourceIds(ProtocolId protocolId, const std::vector<SourceId>& sourceIds);
	bool SetUnmuteProtocolSourceId(ProtocolId protocolId, SourceId sourceId);
	bool SetUnmuteProtocolSourceIds(ProtocolId protocolId, const std::vector<SourceId>& sourceIds);

	String GetProtocolIpAddress(ProtocolId protocolId);
	bool SetProtocolIpAddress(ProtocolId protocolId, String ipAddress, bool dontSendNotification = false);
	int GetProtocolListeningPort(ProtocolId protocolId);
	bool SetProtocolListeningPort(ProtocolId protocolId, int listeningPort, bool dontSendNotification = false);
	int GetProtocolRemotePort(ProtocolId protocolId);
	bool SetProtocolRemotePort(ProtocolId protocolId, int remotePort, bool dontSendNotification = false);
	int GetProtocolMappingArea(ProtocolId protocolId);
	bool SetProtocolMappingArea(ProtocolId protocolId, int mappingAreaId, bool dontSendNotification = false);
	int GetProtocolInputDeviceIndex(ProtocolId protocolId);
	bool SetProtocolInputDeviceIndex(ProtocolId protocolId, int inputDeviceIndex, bool dontSendNotification = false);

	//==========================================================================
	void SetupBridgingNode(const ProtocolBridgingType bridgingProtocolsToActivate = PBT_None);
	std::unique_ptr<XmlElement> SetupDiGiCoBridgingProtocol();
	std::unique_ptr<XmlElement> SetupRTTrPMBridgingProtocol();
	std::unique_ptr<XmlElement> SetupGenericOSCBridgingProtocol();
	std::unique_ptr<XmlElement> SetupGenericMIDIBridgingProtocol();
	std::unique_ptr<XmlElement> SetupYamahaOSCBridgingProtocol();

	/**
	 * A processing engine node can send data to and receive data from multiple protocols that is encapsulates.
	 * Depending on the node configuration, there can exist two groups of protocols, A and B, that are handled
	 * in a specific way to pass incoming and outgoing data to each other and this parent controller instance.
	 */
	ProcessingEngineNode							m_processingNode;	/**< The node that encapsulates the protocols that are used to send, receive and bridge data. */
	XmlElement										m_bridgingXml;		/**< The current xml config for bridging (contains node xml). */
	std::map<ProtocolBridgingType, XmlElement>		m_bridgingProtocolCacheMap;	/**< Map that holds the xml config elements of bridging elements when currently not active, 
																				 * to be able to reactivate correct previous config on request. */

	std::vector<ProtocolBridgingWrapper::Listener*>	m_listeners;		/**< The listner objects, for message data handling callback. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProtocolBridgingWrapper)
};

}
