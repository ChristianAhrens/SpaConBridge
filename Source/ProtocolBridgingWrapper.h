/*
  ==============================================================================

    ProtocolBridgingWrapper.h
    Created: 11 Aug 2020 12:20:42pm
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "AppConfiguration.h"

#include <ProcessingEngine/ProcessingEngineNode.h>

#include <JuceHeader.h>


namespace SoundscapeBridgeApp
{

static const String PROTOCOL_DEFAULT_IP("127.0.0.1");	//< Default IP Address

static constexpr int RX_PORT_DS100_DEVICE = 50010;		//< UDP port which the DS100 is listening to for OSC
static constexpr int RX_PORT_DS100_HOST = 50011;		//< UDP port to which the DS100 will send OSC replies

static constexpr int RX_PORT_DIGICO_DEVICE = 50012;		//< UDP port which the DS100 is listening to for OSC
static constexpr int RX_PORT_DIGICO_HOST = 50013;		//< UDP port to which the DS100 will send OSC replies

static constexpr int RX_PORT_GENERICOSC_DEVICE = 50014;		//< UDP port which the DS100 is listening to for OSC
static constexpr int RX_PORT_GENERICOSC_HOST = 50015;		//< UDP port to which the DS100 will send OSC replies

/**
 * Pre-define processing bridge config values
 */
static constexpr int DEFAULT_PROCNODE_ID = 1;
static constexpr int DS100_PROCESSINGPROTOCOL_ID = 2;
static constexpr int DIGICO_PROCESSINGPROTOCOL_ID = 3;
static constexpr int GENERICOSC_PROCESSINGPROTOCOL_ID = 4;

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
		virtual void HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData) = 0;
	};

public:
	ProtocolBridgingWrapper();
	~ProtocolBridgingWrapper();

	void AddListener(ProtocolBridgingWrapper::Listener* listener);

	void SetupBridgingNode();
	void HandleNodeData(NodeId nodeId, ProtocolId senderProtocolId, ProtocolType senderProtocolType, RemoteObjectIdentifier objectId, RemoteObjectMessageData& msgData) override;
	bool SendMessage(RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData);

	//==========================================================================
	bool ActivateDS100SourceId(juce::int16 sourceId, juce::int16 mappingId);
	bool DeactivateDS100SourceId(juce::int16 sourceId, juce::int16 mappingId);

	String GetDS100IpAddress();
	bool SetDS100IpAddress(String ipAddress, bool dontSendNotification = false);

	int GetDS100MsgRate();
	bool SetDS100MsgRate(int msgRate, bool dontSendNotification = false);

	//==========================================================================
	bool GetMuteDiGiCoSourceId(juce::int16 sourceId);
	bool SetMuteDiGiCoSourceId(juce::int16 sourceId, bool mute = true);

	String GetDiGiCoIpAddress();
	bool SetDiGiCoIpAddress(String ipAddress, bool dontSendNotification = false);

	//==========================================================================
	bool GetMuteGenericOSCSourceId(juce::int16 sourceId);
	bool SetMuteGenericOSCSourceId(juce::int16 sourceId, bool mute = true);

	String GetGenericOSCIpAddress();
	bool SetGenericOSCIpAddress(String ipAddress, bool dontSendNotification = false);

	//==========================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

	//==========================================================================
	void Disconnect();
	void Reconnect();

private:
	//==========================================================================
	bool GetMuteProtocolSourceId(ProtocolId protocolId, juce::int16 sourceId);
	bool SetMuteProtocolSourceId(ProtocolId protocolId, juce::int16 sourceId);
	bool SetUnmuteProtocolSourceId(ProtocolId protocolId, juce::int16 sourceId);

	String GetProtocolIpAddress(ProtocolId protocolId);
	bool SetProtocolIpAddress(ProtocolId protocolId, String ipAddress, bool dontSendNotification = false);

	/**
	 * A processing engine node can send data to and receive data from multiple protocols that is encapsulates.
	 * Depending on the node configuration, there can exist two groups of protocols, A and B, that are handled
	 * in a specific way to pass incoming and outgoing data to each other and this parent controller instance.
	 */
	ProcessingEngineNode							m_processingNode;	/**< The node that encapsulates the protocols that are used to send, receive and bridge data. */
	XmlElement										m_bridgingXml;		/**< The current xml config for bridging (contains node xml). */

	std::vector<ProtocolBridgingWrapper::Listener*>	m_listeners;		/**< The listner objects, for message data handling callback. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProtocolBridgingWrapper)
};

}