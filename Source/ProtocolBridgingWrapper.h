/* Copyright (c) 2020-2023, Christian Ahrens
 *
 * This file is part of SpaConBridge <https://github.com/ChristianAhrens/SpaConBridge>
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

#include "SpaConBridgeCommon.h"
#include "AppConfiguration.h"

#include <ProcessingEngine/ProcessingEngineNode.h>
#include <ProcessingEngine/ObjectDataHandling/ObjectDataHandling_Abstract.h>

#include "../submodules/JUCE-AppBasics/Source/MidiCommandRangeAssignment.h"

#include <JuceHeader.h>


namespace SpaConBridge
{

static const String PROTOCOL_DEFAULT_IP("127.0.0.1");	//< Default IP Address
static const String PROTOCOL_DEFAULT2_IP("127.0.0.2");	//< Default IP Address
static const String PROTOCOL_DEFAULT_PRVATELAN_IP("192.168.1.101");	//< Default IP Address

static constexpr int PROTOCOL_DEFAULT_MAPPINGAREA = 1;	//< Mapping Area Id to use as default

static constexpr int PROTOCOL_DEFAULT_INPUTDEVICEINDEX = 0;	//< Input Device Index to use as default

static constexpr int RX_PORT_DS100_DEVICE_OCP1 = 50014;		//< TCP port which the DS100 is listening to for OCP1 (OCA/AES70) connections

static constexpr int RX_PORT_DS100_DEVICE = 50010;		//< UDP port which the DS100 is listening to for OSC
static constexpr int RX_PORT_DS100_HOST = 50011;		//< UDP port to which the DS100 will send OSC replies

static constexpr int RX_PORT_DIGICO_DEVICE = 50012;		//< UDP port which the DiGiCo console is listening to for OSC
static constexpr int RX_PORT_DIGICO_HOST = 50013;		//< UDP port to which the DiGiCo console will send OSC replies

static constexpr int RX_PORT_GENERICOSC_DEVICE = 50014;	//< UDP port which the generic OSC device is listening to for OSC
static constexpr int RX_PORT_GENERICOSC_HOST = 50015;	//< UDP port to which the generic OSC device will send OSC replies

static constexpr int RX_PORT_RTTRPM_HOST = 24002;		//< UDP port to which the Blacktrax tracker device will send RTTrPM data replies to (us)

static constexpr int RX_PORT_YAMAHAOSC_DEVICE = 50016;	//< UDP port which the Yamaha Rivage console is listening to for OSC
static constexpr int RX_PORT_YAMAHAOSC_HOST = 50017;	//< UDP port to which the Yamaha Rivage console will send OSC replies

static constexpr int RX_PORT_ADMOSC_DEVICE = 50018;		//< Default UDP port to use for ADM OSC device side
static constexpr int RX_PORT_ADMOSC_HOST = 50019;		//< Default UDP port to use for ADM OSC host (spaconbridge) side

static constexpr int RX_PORT_REMAPOSC_DEVICE = 50020;	//< Default UDP port to use for Remap OSC device side
static constexpr int RX_PORT_REMAPOSC_HOST = 50021;		//< Default UDP port to use for Remap OSC host (spaconbridge) side

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
static constexpr int ADMOSC_PROCESSINGPROTOCOL_ID = 9;
static constexpr int DAWPLUGIN_PROCESSINGPROTOCOL_ID = 10;
static constexpr int REMAPOSC_PROCESSINGPROTOCOL_ID = 11;

class ProtocolBridgingWrapper :
	public ProcessingEngineNode::NodeListener,
	public AppConfiguration::XmlConfigurableElement,
	public ObjectDataHandling_Abstract::StateListener
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
		virtual void HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier roi, const RemoteObjectMessageData& msgData) = 0;
	};

public:
	ProtocolBridgingWrapper();
	~ProtocolBridgingWrapper();

	//==========================================================================
	void AddListener(ProtocolBridgingWrapper::Listener* listener);

	//==========================================================================
	void HandleNodeData(const ProcessingEngineNode::NodeCallbackMessage* callbackMessage) override;
	bool SendMessage(RemoteObjectIdentifier roi, RemoteObjectMessageData& msgData);

	//==========================================================================
	void SetOnline(bool online);

	//==========================================================================
	void Disconnect();
	bool Reconnect();

	//==========================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==========================================================================
	void protocolStateChanged(ProtocolId id, ObjectHandlingState state) override;

	//==========================================================================
	ProtocolBridgingType GetActiveBridgingProtocols();
	void SetActiveBridgingProtocols(ProtocolBridgingType desiredActiveBridgingTypes);

	//==========================================================================
	bool UpdateActiveDS100RemoteObjectIds();

	ProtocolType GetDS100ProtocolType();
	bool SetDS100ProtocolType(ProtocolType protocolType, bool dontSendNotification = false);

	juce::IPAddress GetDS100IpAddress();
	bool SetDS100IpAddress(juce::IPAddress ipAddress, bool dontSendNotification = false);

	int GetDS100Port();
	bool SetDS100Port(int port, bool dontSendNotification = false);

	int GetDS100MsgRate();
	bool SetDS100MsgRate(int msgRate, bool dontSendNotification = false);

	juce::IPAddress GetSecondDS100IpAddress();
	bool SetSecondDS100IpAddress(juce::IPAddress ipAddress, bool dontSendNotification = false);

	int GetSecondDS100Port();
	bool SetSecondDS100Port(int port, bool dontSendNotification = false);

	ExtensionMode GetDS100ExtensionMode();
	bool SetDS100ExtensionMode(ExtensionMode mode, bool dontSendNotification = false);

	ActiveParallelModeDS100 GetActiveParallelModeDS100();
	bool SetActiveParallelModeDS100(ActiveParallelModeDS100 activeParallelModeDS100, bool dontSendNotification = false);

	ObjectHandlingState GetDS100State() const;
	void SetDS100State(ObjectHandlingState state);
	ObjectHandlingState GetSecondDS100State() const;
	void SetSecondDS100State(ObjectHandlingState state);

	//==========================================================================
	static bool IsBridgingObjectOnly(const RemoteObjectIdentifier roi);

	//==========================================================================
	bool GetMuteProtocolSoundobjectProcessorId(ProtocolId protocolId, SoundobjectProcessorId soundobjectProcessorId);
	bool SetMuteProtocolSoundobjectProcessorId(ProtocolId protocolId, SoundobjectProcessorId soundobjectProcessorId);
	bool SetMuteProtocolSoundobjectProcessorIds(ProtocolId protocolId, const std::vector<SoundobjectProcessorId>& soundobjectProcessorIds);
	bool SetUnmuteProtocolSoundobjectProcessorId(ProtocolId protocolId, SoundobjectProcessorId soundobjectProcessorId);
	bool SetUnmuteProtocolSoundobjectProcessorIds(ProtocolId protocolId, const std::vector<SoundobjectProcessorId>& soundobjectProcessorIds);

	bool GetMuteProtocolMatrixInputProcessorId(ProtocolId protocolId, MatrixInputProcessorId matrixInputProcessorId);
	bool SetMuteProtocolMatrixInputProcessorId(ProtocolId protocolId, MatrixInputProcessorId matrixInputProcessorId);
	bool SetMuteProtocolMatrixInputProcessorIds(ProtocolId protocolId, const std::vector<MatrixInputProcessorId>& matrixInputProcessorIds);
	bool SetUnmuteProtocolMatrixInputProcessorId(ProtocolId protocolId, MatrixInputProcessorId matrixInputProcessorId);
	bool SetUnmuteProtocolMatrixInputProcessorIds(ProtocolId protocolId, const std::vector<MatrixInputProcessorId>& matrixInputProcessorIds);

	bool GetMuteProtocolMatrixOutputProcessorId(ProtocolId protocolId, MatrixOutputProcessorId matrixOutputProcessorId);
	bool SetMuteProtocolMatrixOutputProcessorId(ProtocolId protocolId, MatrixOutputProcessorId matrixOutputProcessorId);
	bool SetMuteProtocolMatrixOutputProcessorIds(ProtocolId protocolId, const std::vector<MatrixOutputProcessorId>& matrixOutputProcessorIds);
	bool SetUnmuteProtocolMatrixOutputProcessorId(ProtocolId protocolId, MatrixOutputProcessorId matrixOutputProcessorId);
	bool SetUnmuteProtocolMatrixOutputProcessorIds(ProtocolId protocolId, const std::vector<MatrixOutputProcessorId>& matrixOutputProcessorIds);

	bool GetProtocolRemoteObjectsMutedState(ProtocolId protocolId, const std::vector<RemoteObject>& objects);
	bool SetProtocolRemoteObjectsStateMuted(ProtocolId protocolId, const std::vector<RemoteObject>& objects);
	bool SetProtocolRemoteObjectsStateUnmuted(ProtocolId protocolId, const std::vector<RemoteObject>& objects);

	juce::IPAddress GetProtocolIpAddress(ProtocolId protocolId);
	bool SetProtocolIpAddress(ProtocolId protocolId, juce::IPAddress ipAddress, bool dontSendNotification = false);
	int GetProtocolListeningPort(ProtocolId protocolId);
	bool SetProtocolListeningPort(ProtocolId protocolId, int listeningPort, bool dontSendNotification = false);
	int GetProtocolRemotePort(ProtocolId protocolId);
	bool SetProtocolRemotePort(ProtocolId protocolId, int remotePort, bool dontSendNotification = false);
	int GetProtocolMappingArea(ProtocolId protocolId);
	bool SetProtocolMappingArea(ProtocolId protocolId, int mappingAreaId, bool dontSendNotification = false);

	const juce::Point<float> GetProtocolOriginOffset(ProtocolId protocolId);
	bool SetProtocolOriginOffset(ProtocolId protocolId, const juce::Point<float>& originOffset, bool dontSendNotification = false);

	const std::pair<juce::Range<float>, juce::Range<float>> GetProtocolMappingRange(ProtocolId protocolId);
	bool SetProtocolMappingRange(ProtocolId protocolId, const std::pair<juce::Range<float>, juce::Range<float>>& mappingRange, bool dontSendNotification = false);

	String GetProtocolInputDeviceIdentifier(ProtocolId protocolId);
	bool SetProtocolInputDeviceIdentifier(ProtocolId protocolId, const String& inputDeviceIdentifier, bool dontSendNotification = false);
	String GetProtocolOutputDeviceIdentifier(ProtocolId protocolId);
	bool SetProtocolOutputDeviceIdentifier(ProtocolId protocolId, const String& outputDeviceIdentifier, bool dontSendNotification = false);
	JUCEAppBasics::MidiCommandRangeAssignment GetMidiAssignmentMapping(ProtocolId protocolId, const RemoteObjectIdentifier roi);
	bool SetMidiAssignmentMapping(ProtocolId protocolId, RemoteObjectIdentifier remoteObjectId, const JUCEAppBasics::MidiCommandRangeAssignment& assignmentMapping, bool dontSendNotification = false);
	std::map<String, JUCEAppBasics::MidiCommandRangeAssignment> GetMidiScenesAssignmentMapping(ProtocolId protocolId, const RemoteObjectIdentifier roi);
	bool SetMidiScenesAssignmentMapping(ProtocolId protocolId, const RemoteObjectIdentifier roi, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& assignmentMapping, bool dontSendNotification = false);
	int GetProtocolXAxisInverted(ProtocolId protocolId);
	bool SetProtocolXAxisInverted(ProtocolId protocolId, int inverted, bool dontSendNotification = false);
	int GetProtocolYAxisInverted(ProtocolId protocolId);
	bool SetProtocolYAxisInverted(ProtocolId protocolId, int inverted, bool dontSendNotification = false);
	int GetProtocolXYAxisSwapped(ProtocolId protocolId);
	bool SetProtocolXYAxisSwapped(ProtocolId protocolId, int swapped, bool dontSendNotification = false);
	int GetProtocolDataSendingDisabled(ProtocolId protocolId);
	bool SetProtocolDataSendingDisabled(ProtocolId protocolId, int disabled, bool dontSendNotification = false);
	bool GetProtocolBridgingXYMessageCombined(ProtocolId protocolId);
	bool SetProtocolBridgingXYMessageCombined(ProtocolId protocolId, bool combined, bool dontSendNotification = false);
	std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>> GetProtocolOscRemapAssignments(ProtocolId protocolId);
	bool SetProtocolOscRemapAssignments(ProtocolId protocolId, const std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>& oscRemapAssignments, bool dontSendNotification = false);
	const String GetProtocolModuleTypeIdentifier(ProtocolId protocolId);
	bool SetProtocolModuleTypeIdentifier(ProtocolId protocolId, const String& moduleTypeIdentifier, bool dontSendNotification = false);
	std::map<int, ChannelId> GetProtocolChannelRemapAssignments(ProtocolId protocolId);
	bool SetProtocolChannelRemapAssignments(ProtocolId protocolId, const std::map<int, ChannelId>& channelRemapAssignments, bool dontSendNotification = false);

private:
	//==========================================================================
	ObjectHandlingState GetProtocolState(ProtocolId protocolId) const;
	void SetProtocolState(ProtocolId protocolId, ObjectHandlingState state);

	//==========================================================================
	bool SetBridgingNodeStateXml(XmlElement* stateXml, bool dontSendNotification = false);
	bool SetupBridgingNode(const ProtocolBridgingType bridgingProtocolsToActivate = PBT_None);
	std::unique_ptr<XmlElement> SetupDiGiCoBridgingProtocol();
	std::unique_ptr<XmlElement> SetupDAWPluginBridgingProtocol();
	std::unique_ptr<XmlElement> SetupRTTrPMBridgingProtocol();
	std::unique_ptr<XmlElement> SetupGenericOSCBridgingProtocol();
	std::unique_ptr<XmlElement> SetupGenericMIDIBridgingProtocol();
	std::unique_ptr<XmlElement> SetupADMOSCBridgingProtocol();
	std::unique_ptr<XmlElement> SetupYamahaOSCBridgingProtocol();
	std::unique_ptr<XmlElement> SetupRemapOSCBridgingProtocol();

	//==========================================================================
	bool UpdateMutedProtocolRemoteObjects(ProtocolId protocolId, const std::vector<RemoteObject>& objects, bool unmuteObjects);
	const std::vector<RemoteObject> GetMutedProtocolRemoteObjects(ProtocolId protocolId);

	/**
	 * A processing engine node can send data to and receive data from multiple protocols that is encapsulates.
	 * Depending on the node configuration, there can exist two groups of protocols, A and B, that are handled
	 * in a specific way to pass incoming and outgoing data to each other and this parent controller instance.
	 */
	ProcessingEngineNode								m_processingNode;				/**< The node that encapsulates the protocols that are used to send, receive and bridge data. */
	XmlElement											m_bridgingXml;					/**< The current xml config for bridging (contains node xml). */
	std::map<ProtocolBridgingType, XmlElement>			m_bridgingProtocolCacheMap;		/**< Map that holds the xml config elements of bridging elements when currently not active, to be able to reactivate correct previous config on request. */
	std::map<ProtocolId, ObjectHandlingState>			m_bridgingProtocolState;		/**< Map that holds the current protocol status as were communicated by protocol processing engine node data handling object. */
	std::map<ProtocolId, std::vector<RemoteObject>>		m_bridgingProtocolMutedObjects;	/**< Map that holds (caches) the currently muted objects per protocol. */
	std::vector<ProtocolBridgingWrapper::Listener*>		m_listeners;					/**< The listner objects, for message data handling callback. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProtocolBridgingWrapper)
};

}
