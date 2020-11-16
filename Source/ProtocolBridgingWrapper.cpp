/*
  ==============================================================================

    ProtocolBridgingWrapper.cpp
    Created: 11 Aug 2020 12:20:42pm
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "ProtocolBridgingWrapper.h"


namespace SoundscapeBridgeApp
{

/**
 *
 */
ProtocolBridgingWrapper::ProtocolBridgingWrapper()
	: m_bridgingXml(AppConfiguration::getTagName(AppConfiguration::TagID::BRIDGING))
{

	// Controller derives from ProcessingEngineNode::Listener
	m_processingNode.AddListener(this);

	SetupBridgingNode();
}

/**
 *
 */
ProtocolBridgingWrapper::~ProtocolBridgingWrapper()
{
	m_listeners.clear();
}

/**
 * Method to register a listener object to be called when the node has received the respective data via a node protocol.
 * @param listener	The listener object to add to the internal list of listeners
 */
void ProtocolBridgingWrapper::AddListener(ProtocolBridgingWrapper::Listener* listener)
{
	if (listener)
		m_listeners.push_back(listener);
}

/**
 * Send a Message out via the active bridging node.
 * @param Id	The id of the remote object to be sent.
 * @param msgData	The message data to be sent.
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SendMessage(RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData)
{
	if (msgData.addrVal.first > DS100_CHANNELCOUNT)
		return m_processingNode.SendMessageTo(DS100_2_PROCESSINGPROTOCOL_ID, Id, msgData);
	else
		return m_processingNode.SendMessageTo(DS100_1_PROCESSINGPROTOCOL_ID, Id, msgData);
}

/**
 * Called when the OSCReceiver receives a new OSC message, since Controller inherits from OSCReceiver::Listener.
 * It forwards the message to all registered Processor objects.
 * @param nodeId	The bridging node that the message data was received on (only a single default id node supported currently).
 * @param senderProtocolId	The protocol that the message data was received on and was sent to controller from.
 * @param senderProtocolType	The protocol type that received the data and forwarded it to us.
 * @param objectId	The remote object id of the object that was received
 * @param msgData	The actual message data that was received
 */
void ProtocolBridgingWrapper::HandleNodeData(NodeId nodeId, ProtocolId senderProtocolId, ProtocolType senderProtocolType, RemoteObjectIdentifier objectId, RemoteObjectMessageData& msgData)
{
	ignoreUnused(senderProtocolType);

	for (auto l : m_listeners)
		l->HandleMessageData(nodeId, senderProtocolId, objectId, msgData);
}

/**
 * Disconnect the active bridging nodes' protocols.
 */
void ProtocolBridgingWrapper::Disconnect()
{
	m_processingNode.Stop();
}

/**
 * Disconnect and re-connect the OSCSender to a host specified by the current ip settings.
 */
void ProtocolBridgingWrapper::Reconnect()
{
	Disconnect();

	auto nodeXmlElement = m_bridgingXml.getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));
	if (nodeXmlElement)
		m_processingNode.setStateXml(nodeXmlElement);
	m_processingNode.Start();
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to dump this objects' settings
 * to a XML element structure that is returned and written to config file by the
 * singleton AppConfiguration class implementation.
 * @return	The XML element data that was created.
 */
std::unique_ptr<XmlElement> ProtocolBridgingWrapper::createStateXml()
{
	auto bridgingXmlElement = std::make_unique<XmlElement>(m_bridgingXml);

    return bridgingXmlElement;
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to set this objects' settings
 * from a XML element structure that passed as argument.
 * @param stateXml	The XML element containing this objects' configuration data
 * @return	True if the data was read and handled successfuly, false if not.
 */
bool ProtocolBridgingWrapper::setStateXml(XmlElement* stateXml)
{
	if (stateXml || (stateXml->getTagName() == AppConfiguration::getTagName(AppConfiguration::TagID::BRIDGING)))
	{
		m_bridgingXml = *stateXml;
		auto nodeXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));
		if (nodeXmlElement)
		{
			auto digicoProtocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DIGICO_PROCESSINGPROTOCOL_ID));
			if (digicoProtocolXmlElement)
				m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_DiGiCo, *digicoProtocolXmlElement));
			auto RTTrPMProtocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(RTTRPM_PROCESSINGPROTOCOL_ID));
			if (RTTrPMProtocolXmlElement)
				m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_BlacktraxRTTrPM, *RTTrPMProtocolXmlElement));
			auto genericOSCProtocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(GENERICOSC_PROCESSINGPROTOCOL_ID));
			if (genericOSCProtocolXmlElement)
				m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_GenericOSC, *genericOSCProtocolXmlElement));

			return m_processingNode.setStateXml(nodeXmlElement);
		}
	}
	else
	{
		SetupBridgingNode();
	}

	return false;
}

/**
 * Method to create a basic configuration to use to setup the single supported
 * bridging node.
 */
void ProtocolBridgingWrapper::SetupBridgingNode()
{
	auto nodeXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));

	nodeXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), DEFAULT_PROCNODE_ID);

	auto objectHandlingXmlElement = nodeXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING));
	if (objectHandlingXmlElement)
	{
		objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Bypass));
	}

	// DS100 protocol - RoleA
	auto protocolAXmlElement = nodeXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA));
	if (protocolAXmlElement)
	{
		protocolAXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), DS100_1_PROCESSINGPROTOCOL_ID);

		protocolAXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(PT_OSCProtocol));
		protocolAXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), 1);

		auto clientPortXmlElement = protocolAXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
		if (clientPortXmlElement)
			clientPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_DS100_DEVICE);

		auto hostPortXmlElement = protocolAXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
		if (hostPortXmlElement)
			hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_DS100_HOST);

		// Active objects preparation
		Array<RemoteObject> activeObjects;
		RemoteObject objectX, objectY;

		auto activeObjsXmlElement = protocolAXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ACTIVEOBJECTS));
		if (activeObjsXmlElement)
			ProcessingEngineConfig::WriteActiveObjects(activeObjsXmlElement, activeObjects);

		auto ipAdressXmlElement = protocolAXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
		if (ipAdressXmlElement)
			ipAdressXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS), PROTOCOL_DEFAULT_IP);

		auto pollIntervalXmlElement = protocolAXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
		if (pollIntervalXmlElement)
			pollIntervalXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL), ET_DefaultPollingRate);
	}

	// DiGiCo protocol - RoleB
	auto digicoBridgingXmlElement = SetupDiGiCoBridgingProtocol();
	if (digicoBridgingXmlElement)
	{
		m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_DiGiCo, *digicoBridgingXmlElement));
		nodeXmlElement->addChildElement(digicoBridgingXmlElement.release());
	}

	// RTTrPM protocol - RoleB
	auto RTTrPMBridgingXmlElement = SetupRTTrPMBridgingProtocol();
	if (RTTrPMBridgingXmlElement)
	{
		m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_BlacktraxRTTrPM, *RTTrPMBridgingXmlElement));
		nodeXmlElement->addChildElement(RTTrPMBridgingXmlElement.release());
	}

	// GenericOSC protocol - RoleB
	auto genericOSCBridgingXmlElement = SetupGenericOSCBridgingProtocol();
	if (genericOSCBridgingXmlElement)
	{
		m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_GenericOSC, *genericOSCBridgingXmlElement));
		nodeXmlElement->addChildElement(genericOSCBridgingXmlElement.release());
	}

	m_processingNode.setStateXml(nodeXmlElement.get());

	m_bridgingXml.addChildElement(nodeXmlElement.release());
}

/**
 * Method to create the default digico bridging protocol xml element.
 * @return	The protocol xml element that was created
 */
std::unique_ptr<XmlElement> ProtocolBridgingWrapper::SetupDiGiCoBridgingProtocol()
{
	// DiGiCo protocol - RoleB
	auto protocolBXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));
	if (protocolBXmlElement)
	{
		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), DIGICO_PROCESSINGPROTOCOL_ID);

		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(PT_OSCProtocol));
		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), 0);

		auto clientPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
		if (clientPortXmlElement)
			clientPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_DIGICO_DEVICE);

		auto hostPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
		if (hostPortXmlElement)
			hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_DIGICO_HOST);

		auto ipAdressXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
		if (ipAdressXmlElement)
			ipAdressXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS), PROTOCOL_DEFAULT_IP);

		protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDCHANNELS));
	}

	return protocolBXmlElement;
}

/**
 * Method to create the default blacktrax RTTrPM bridging protocol xml element.
 * @return	The protocol xml element that was created
 */
std::unique_ptr<XmlElement> ProtocolBridgingWrapper::SetupRTTrPMBridgingProtocol()
{
	// RTTrPM protocol - RoleB
	auto protocolBXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));
	if (protocolBXmlElement)
	{
		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), RTTRPM_PROCESSINGPROTOCOL_ID);

		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(PT_RTTrPMProtocol));
		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), 0);

		auto clientPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
		if (clientPortXmlElement)
			clientPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_RTTRPM_DEVICE);

		auto hostPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
		if (hostPortXmlElement)
			hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_RTTRPM_HOST);

		auto ipAdressXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
		if (ipAdressXmlElement)
			ipAdressXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS), PROTOCOL_DEFAULT_IP);

		auto mappingAreaIdXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREA));
		if (mappingAreaIdXmlElement)
			mappingAreaIdXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), PROTOCOL_DEFAULT_MAPPINGAREA);

		protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDCHANNELS));
	}

	return protocolBXmlElement;
}

/**
 * Method to create the default generic OSC bridging protocol xml element.
 * @return	The protocol xml element that was created
 */
std::unique_ptr<XmlElement> ProtocolBridgingWrapper::SetupGenericOSCBridgingProtocol()
{
	// GenericOSC protocol - RoleB
	auto protocolBXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));
	if (protocolBXmlElement)
	{
		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), GENERICOSC_PROCESSINGPROTOCOL_ID);

		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(PT_OSCProtocol));
		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), 0);

		auto clientPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
		if (clientPortXmlElement)
			clientPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_GENERICOSC_DEVICE);

		auto hostPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
		if (hostPortXmlElement)
			hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_GENERICOSC_HOST);

		auto ipAdressXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
		if (ipAdressXmlElement)
			ipAdressXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS), PROTOCOL_DEFAULT_IP);

		protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDCHANNELS));
	}

	return protocolBXmlElement;
}

/**
 * Gets the mute state of the given source of given protocol
 * @param protocolId The id of the protocol the muted state shall be returned of
 * @param sourceId The id of the source that shall be muted
 * @return True if mute is active, false if not
 */
bool ProtocolBridgingWrapper::GetMuteProtocolSourceId(ProtocolId protocolId, SourceId sourceId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto mutedObjChsXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDCHANNELS));
			Array<int> mutedChannels;
			ProcessingEngineConfig::ReadMutedObjectChannels(mutedObjChsXmlElement, mutedChannels);
			
			return mutedChannels.contains(sourceId);
		}
	}

	return false;
}

/**
 * Sets the given source of given protocol to be muted
 * @param protocolId The id of the protocol the source shall be muted of
 * @param sourceId The id of the source that shall be muted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteProtocolSourceId(ProtocolId protocolId, SourceId sourceId)
{
	std::vector<SourceId> sourceIds{ sourceId };
	return SetMuteProtocolSourceIds(protocolId, sourceIds);
}

/**
 * Sets the given sources of given protocol to be muted
 * @param protocolId The id of the protocol the source shall be muted of
 * @param sourceIds The ids of the sources that shall be muted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteProtocolSourceIds(ProtocolId protocolId, const std::vector<SourceId>& sourceIds)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto mutedObjChsXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDCHANNELS));
			Array<int> mutedChannels;
			ProcessingEngineConfig::ReadMutedObjectChannels(mutedObjChsXmlElement, mutedChannels);
			auto oldMutedChannels = mutedChannels;
			for (auto sourceId : sourceIds)
				if (!mutedChannels.contains(sourceId))
					mutedChannels.add(sourceId);

			if (oldMutedChannels != mutedChannels)
			{
				ProcessingEngineConfig::WriteMutedObjectChannels(mutedObjChsXmlElement, mutedChannels);

				m_processingNode.setStateXml(nodeXmlElement);
				triggerConfigurationUpdate(false);

				return true;
			}
		}
	}

	return false;
}

/**
 * Sets the given source of given protocol to be unmuted
 * @param protocolId The id of the protocol the source shall be unmuted of
 * @param sourceId The id of the source that shall be unmuted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetUnmuteProtocolSourceId(ProtocolId protocolId, SourceId sourceId)
{
	std::vector<SourceId> sourceIds{ sourceId };
	return SetUnmuteProtocolSourceIds(protocolId, sourceIds);
}

/**
 * Sets the given sources of given protocol to be unmuted
 * @param protocolId The id of the protocol the sources shall be unmuted of
 * @param sourceIds The ids of the sources that shall be unmuted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetUnmuteProtocolSourceIds(ProtocolId protocolId, const std::vector<SourceId>& sourceIds)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto mutedObjChsXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDCHANNELS));
			Array<int> mutedChannels;
			ProcessingEngineConfig::ReadMutedObjectChannels(mutedObjChsXmlElement, mutedChannels);
			auto oldMutedChannels = mutedChannels;
			for (auto sourceId : sourceIds)
				if (mutedChannels.contains(sourceId))
					mutedChannels.removeAllInstancesOf(sourceId);

			if (oldMutedChannels != mutedChannels)
			{
				ProcessingEngineConfig::WriteMutedObjectChannels(mutedObjChsXmlElement, mutedChannels);

				m_processingNode.setStateXml(nodeXmlElement);
				triggerConfigurationUpdate(false);

				return true;
			}
		}
	}

	return false;
}

/**
 * Gets the protocol's currently set client ip address.
 * @param protocolId The id of the protocol for which to get the currently configured ip address
 * @return	The ip address string
 */
String ProtocolBridgingWrapper::GetProtocolIpAddress(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto ipAddressXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
			if (ipAddressXmlElement)
			{
				return ipAddressXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS));
			}
		}
	}

	return INVALID_IPADDRESS_VALUE;
}

/**
 * Sets the desired protocol client ip address.
 * This method inserts the ip address into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId The id of the protocol for which to set the ip address
 * @param ipAddress	The new ip address string
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolIpAddress(ProtocolId protocolId, String ipAddress, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto ipAddressXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
			if (ipAddressXmlElement)
			{
				ipAddressXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS), ipAddress);
			}
			else
				return false;
		}
		else
			return false;

		m_processingNode.setStateXml(nodeXmlElement);

		if (!dontSendNotification)
			triggerConfigurationUpdate(false);

		return true;
	}
	else
		return false;
}

/**
 * Gets the protocol's currently set listening (local host) port.
 * @param protocolId The id of the protocol for which to get the currently configured port
 * @return	The port number
 */
int ProtocolBridgingWrapper::GetProtocolListeningPort(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto listeningPortXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
			if (listeningPortXmlElement)
			{
				return listeningPortXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT));
			}
		}
	}

	return INVALID_PORT_VALUE;
}

/**
 * Sets the desired protocol listening port.
 * This method inserts the port number into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId The id of the protocol for which to set the ip address
 * @param listeningPort	The new port number to listen on
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolListeningPort(ProtocolId protocolId, int listeningPort, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto listeningPortXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
			if (listeningPortXmlElement)
			{
				listeningPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), listeningPort);
			}
			else
				return false;
		}
		else
			return false;

		m_processingNode.setStateXml(nodeXmlElement);

		if (!dontSendNotification)
			triggerConfigurationUpdate(false);

		return true;
	}
	else
		return false;
}

/**
 * Gets the protocol's currently set remote (target client) port.
 * @param protocolId The id of the protocol for which to get the currently configured port
 * @return	The port number
 */
int ProtocolBridgingWrapper::GetProtocolRemotePort(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto remotePortXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
			if (remotePortXmlElement)
			{
				return remotePortXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT));
			}
		}
	}

	return INVALID_PORT_VALUE;
}

/**
 * Sets the desired protocol remote (client) port.
 * This method inserts the port number into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId The id of the protocol for which to set the ip address
 * @param remotePort	The new port number to send to
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolRemotePort(ProtocolId protocolId, int remotePort, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto remotePortXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
			if (remotePortXmlElement)
			{
				remotePortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), remotePort);
			}
			else
				return false;
		}
		else
			return false;

		m_processingNode.setStateXml(nodeXmlElement);

		if (!dontSendNotification)
			triggerConfigurationUpdate(false);

		return true;
	}
	else
		return false;
}

/**
 * Gets the protocol's currently set mapping area id, if available for the given protocol.
 * @param protocolId The id of the protocol for which to get the currently configured mappingarea id
 * @return	The mapping area id
 */
int ProtocolBridgingWrapper::GetProtocolMappingArea(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto mappingAreaIdXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREA));
			if (mappingAreaIdXmlElement)
			{
				return mappingAreaIdXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID));
			}
		}
	}

	return INVALID_PORT_VALUE;
}

/**
 * Sets the given protocol mapping area id.
 * This method inserts the mapping area id into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId The id of the protocol for which to set the ip address
 * @param remotePort	The new port number to send to
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolMappingArea(ProtocolId protocolId, int mappingAreaId, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto mappingAreaIdXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREA));
			if (mappingAreaIdXmlElement)
			{
				mappingAreaIdXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), mappingAreaId);
			}
			else
				return false;
		}
		else
			return false;

		m_processingNode.setStateXml(nodeXmlElement);

		if (!dontSendNotification)
			triggerConfigurationUpdate(false);

		return true;
	}
	else
		return false;
}

/**
 * Getter for the active protocol bridging types (active protocols RoleB - those are used for bridging to DS100 running as RoleA, see RemoteProtocolBridge for details)
 * @return The bitfield containing all active bridging types
 */
ProtocolBridgingType ProtocolBridgingWrapper::GetActiveBridgingProtocols()
{
	ProtocolBridgingType activeBridgingTypes = PBT_None;

	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DIGICO_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
			activeBridgingTypes |= PBT_DiGiCo;

		protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(RTTRPM_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
			activeBridgingTypes |= PBT_BlacktraxRTTrPM;

		protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(GENERICOSC_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
			activeBridgingTypes |= PBT_GenericOSC;
	}

	return activeBridgingTypes;
}

/**
 * Setter for protocol bridging types that shall be active.
 * @param	desiredActiveBridgingTypes	Bitfield containing all types that are to be active.
 */
void ProtocolBridgingWrapper::SetActiveBridgingProtocols(ProtocolBridgingType desiredActiveBridgingTypes)
{
	auto currentlyActiveBridgingTypes = GetActiveBridgingProtocols();

	if (currentlyActiveBridgingTypes != desiredActiveBridgingTypes)
	{
		// we need to do something, since currently active protocols are not what the user wants any more
		auto addDiGiCoBridging = ((desiredActiveBridgingTypes & PBT_DiGiCo) && !(currentlyActiveBridgingTypes & PBT_DiGiCo));
		auto removeDiGiCoBridging = (!(desiredActiveBridgingTypes & PBT_DiGiCo) && (currentlyActiveBridgingTypes & PBT_DiGiCo));
		auto addRTTrPMBridging = ((desiredActiveBridgingTypes & PBT_BlacktraxRTTrPM) && !(currentlyActiveBridgingTypes & PBT_BlacktraxRTTrPM));
		auto removeRTTrPMBridging = (!(desiredActiveBridgingTypes & PBT_BlacktraxRTTrPM) && (currentlyActiveBridgingTypes & PBT_BlacktraxRTTrPM));
		auto addGenericOSCBridging = ((desiredActiveBridgingTypes & PBT_GenericOSC) && !(currentlyActiveBridgingTypes & PBT_GenericOSC));
		auto removeGenericOSCBridging = (!(desiredActiveBridgingTypes & PBT_GenericOSC) && (currentlyActiveBridgingTypes & PBT_GenericOSC));

		auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
		if (nodeXmlElement)
		{
			if (addDiGiCoBridging)
			{
				nodeXmlElement->addChildElement(std::make_unique<XmlElement>(m_bridgingProtocolCacheMap.at(PBT_DiGiCo)).release());
			}
			else if (removeDiGiCoBridging)
			{
				auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DIGICO_PROCESSINGPROTOCOL_ID));
				if (protocolXmlElement)
				{
					m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_DiGiCo, *protocolXmlElement));
					nodeXmlElement->removeChildElement(protocolXmlElement, true);
				}
			}

			if (addRTTrPMBridging)
			{
				nodeXmlElement->addChildElement(std::make_unique<XmlElement>(m_bridgingProtocolCacheMap.at(PBT_BlacktraxRTTrPM)).release());
			}
			else if (removeRTTrPMBridging)
			{
				auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(RTTRPM_PROCESSINGPROTOCOL_ID));
				if (protocolXmlElement)
				{
					m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_BlacktraxRTTrPM, *protocolXmlElement));
					nodeXmlElement->removeChildElement(protocolXmlElement, true);
				}
			}

			if (addGenericOSCBridging)
			{
				nodeXmlElement->addChildElement(std::make_unique<XmlElement>(m_bridgingProtocolCacheMap.at(PBT_GenericOSC)).release());
			}
			else if (removeGenericOSCBridging)
			{
				auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(GENERICOSC_PROCESSINGPROTOCOL_ID));
				if (protocolXmlElement)
				{
					m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_GenericOSC, *protocolXmlElement));
					nodeXmlElement->removeChildElement(protocolXmlElement, true);
				}
			}

			m_processingNode.setStateXml(nodeXmlElement);
			triggerConfigurationUpdate(true);
		}
	}
}

/**
 * Sets the given soundobject/mapping as to be activly handled.
 * This method inserts the object into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param sourceId	The id of the new soundsource object to activate
 * @param mappingId The mapping id that is to be activated for the soundsource object
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::ActivateDS100SourceId(SourceId sourceId, juce::int16 mappingId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto mappedSourceId = sourceId;
		auto DS100ProtocolId = DS100_1_PROCESSINGPROTOCOL_ID;

		if (sourceId > DS100_CHANNELCOUNT)
		{
			mappedSourceId = sourceId - DS100_CHANNELCOUNT;
			DS100ProtocolId = DS100_2_PROCESSINGPROTOCOL_ID;
		}

		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100ProtocolId));
		if (protocolXmlElement)
		{
			auto activeObjsXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ACTIVEOBJECTS));
			if (activeObjsXmlElement)
			{
				juce::Array<RemoteObject> activeObjects;
				ProcessingEngineConfig::ReadActiveObjects(activeObjsXmlElement, activeObjects);
				for(auto roi : m_activeObjectsPerSource)
				{
					RemoteObject newSourceObject;
					newSourceObject.Id = roi;
					newSourceObject.Addr.first = mappedSourceId;
					if (roi == ROI_CoordinateMapping_SourcePosition_X || roi == ROI_CoordinateMapping_SourcePosition_Y || roi == ROI_CoordinateMapping_SourcePosition_XY)
						newSourceObject.Addr.second = mappingId;
					else
						newSourceObject.Addr.second = INVALID_ADDRESS_VALUE;
			
					if (!activeObjects.contains(newSourceObject))
						activeObjects.add(newSourceObject);
				}
				ProcessingEngineConfig::ReplaceActiveObjects(activeObjsXmlElement, activeObjects);
			}
			else
				return false;
		}
		else
			return false;

		m_processingNode.setStateXml(nodeXmlElement);
		triggerConfigurationUpdate(false);

		return true;
	}
	else
		return false;
}

/**
 * Removes the given soundobject/mapping as to no longer be activly handled.
 * This method removes the object from the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param sourceId	The id of the new soundsource object to deactivate
 * @param mappingId The mapping id that is to be deactivated for the soundsource object
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::DeactivateDS100SourceId(SourceId sourceId, juce::int16 mappingId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto mappedSourceId = sourceId;
		auto DS100ProtocolId = DS100_1_PROCESSINGPROTOCOL_ID;

		if (sourceId > DS100_CHANNELCOUNT)
		{
			mappedSourceId = sourceId - DS100_CHANNELCOUNT;
			DS100ProtocolId = DS100_2_PROCESSINGPROTOCOL_ID;
		}

		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100ProtocolId));
		if (protocolXmlElement)
		{
			auto activeObjsXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ACTIVEOBJECTS));
			if (activeObjsXmlElement)
			{
				juce::Array<RemoteObject> activeObjects;
				ProcessingEngineConfig::ReadActiveObjects(activeObjsXmlElement, activeObjects);
				for (auto roi : m_activeObjectsPerSource)
				{
					RemoteObject newSourceObject;
					newSourceObject.Id = static_cast<RemoteObjectIdentifier>(roi);
					newSourceObject.Addr.first = mappedSourceId;
					if (roi == ROI_CoordinateMapping_SourcePosition_X || roi == ROI_CoordinateMapping_SourcePosition_Y || roi == ROI_CoordinateMapping_SourcePosition_XY)
						newSourceObject.Addr.second = mappingId;
					else
						newSourceObject.Addr.second = INVALID_ADDRESS_VALUE;

					if (activeObjects.contains(newSourceObject))
					{
						auto idx = activeObjects.indexOf(newSourceObject);
						activeObjects.remove(idx);
					}
				}
				ProcessingEngineConfig::ReplaceActiveObjects(activeObjsXmlElement, activeObjects);
			}
			else
				return false;
		}
		else
			return false;

		m_processingNode.setStateXml(nodeXmlElement);
		triggerConfigurationUpdate(false);

		return true;
	}
	else
		return false;
}

/**
 * Gets the currently set DS100 client ip address.
 * This method forwards the call to the generic implementation.
 * @return	The ip address string
 */
String ProtocolBridgingWrapper::GetDS100IpAddress()
{
	return GetProtocolIpAddress(DS100_1_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol client ip address.
 * This method forwards the call to the generic implementation.
 * @param ipAddress	The new ip address string
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetDS100IpAddress(String ipAddress, bool dontSendNotification)
{
	return SetProtocolIpAddress(DS100_1_PROCESSINGPROTOCOL_ID, ipAddress, dontSendNotification);
}

/**
 * Gets the currently set cascade DS100 client ip address.
 * This method forwards the call to the generic implementation.
 * @return	The ip address string
 */
String ProtocolBridgingWrapper::GetSecondDS100IpAddress()
{
	return GetProtocolIpAddress(DS100_2_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired cascade DS100 client ip address.
 * This method forwards the call to the generic implementation.
 * @param ipAddress	The new ip address string
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetSecondDS100IpAddress(String ipAddress, bool dontSendNotification)
{
	return SetProtocolIpAddress(DS100_2_PROCESSINGPROTOCOL_ID, ipAddress, dontSendNotification);
}

/**
 * Gets the currently active message rate for protocol polling.
 * @return	True on succes, false if failure
 */
int ProtocolBridgingWrapper::GetDS100MsgRate()
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_1_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
		{
			auto pollingIntervalXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
			if (pollingIntervalXmlElement)
			{
				return pollingIntervalXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL));
			}
		}
	}
	
	return INVALID_RATE_VALUE;
}

/**
 * Sets the desired message rate for protocol polling.
 * This method inserts the rate value into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param msgRate	The new message rate value in ms
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetDS100MsgRate(int msgRate, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_1_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
		{
			auto pollingIntervalXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
			if (pollingIntervalXmlElement)
			{
				pollingIntervalXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL), msgRate);
			}
			else
				return false;
		}
		else
			return false;

		protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_2_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
		{
			auto pollingIntervalXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
			if (pollingIntervalXmlElement)
			{
				pollingIntervalXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL), msgRate);
			}
			else
				return false;
		}
		else
			return false;

		m_processingNode.setStateXml(nodeXmlElement);

		if (!dontSendNotification)
			triggerConfigurationUpdate(false);

		return true;
	}
	else
		return false;
}

/**
 * Getter method for the active DS100 extension mode.
 * This does not return a member variable value but contains logic to derive the mode from internal cached xml element configuration.
 * @return The DS100 extension mode value as results from cached xml config.
 */
ExtensionMode ProtocolBridgingWrapper::GetDS100ExtensionMode()
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto objectHandlingXmlElement = nodeXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING));
		if (objectHandlingXmlElement)
		{
			auto objectHandlingMode = ProcessingEngineConfig::ObjectHandlingModeFromString(objectHandlingXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE)));
			switch (objectHandlingMode)
			{
			case OHM_Mux_nA_to_mB:
				return EM_Extend;
			case OHM_Bypass:
				return EM_Off;
			case OHM_Invalid:
			case OHM_Remap_A_X_Y_to_B_XY:
			case OHM_Forward_only_valueChanges:
			case OHM_DS100_DeviceSimulation:
			case OHM_Forward_A_to_B_only:
			case OHM_Reverse_B_to_A_only:
			case OHM_UserMAX:
			default:
				jassertfalse;
				return EM_Off;
			}
		}
	}

	return EM_Off;
}

/**
 * Setter method for DS100 Extension Mode.
 * This does not set a member variable but contains logic to reconfigure the cached xml element according to the given mode value.
 * @param mode	The mode to activate
 * @param dontSendNotification	-Ignored-
 * @return	True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetDS100ExtensionMode(ExtensionMode mode, bool dontSendNotification)
{
	ignoreUnused(dontSendNotification);

	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto objectHandlingXmlElement = nodeXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING));
		if (objectHandlingXmlElement)
		{
			switch (mode)
			{
			case EM_Off:
			{
				// EM_Off refers to Bypass object handling mode without any channelcount parameter attributes
				objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Bypass));
				auto protocolAChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLACHCNT));
				if (protocolAChCntXmlElement)
					objectHandlingXmlElement->removeChildElement(protocolAChCntXmlElement, true);
				auto protocolBChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLBCHCNT));
				if (protocolBChCntXmlElement)
					objectHandlingXmlElement->removeChildElement(protocolBChCntXmlElement, true);
			}
			break;
			case EM_Extend:
			{
				// EM_Extend refers to Multiplex nA to mB object handling mode with channel A and B parameter attributes
				objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Mux_nA_to_mB));
				auto protocolAChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLACHCNT));
				if (!protocolAChCntXmlElement)
					protocolAChCntXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLACHCNT));
				protocolAChCntXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), DS100_CHANNELCOUNT);
				auto protocolBChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLBCHCNT));
				if (!protocolBChCntXmlElement)
					protocolBChCntXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLBCHCNT));
				protocolBChCntXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::COUNT), DS100_EXTMODE_CHANNELCOUNT);
			}
			break;
			case EM_Mirror:
			default:
				break;
			}
		}
		else
			return false;

		auto protocolA1XmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_1_PROCESSINGPROTOCOL_ID));
		auto protocolA2XmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_2_PROCESSINGPROTOCOL_ID));

		if (protocolA1XmlElement) // primary DS100 must always be existing, otherwise something in the config is broken
		{
			switch (mode)
			{
			case EM_Off:
			{
				// EM_Off requires the second DS100 protocol to not be present
				if (protocolA2XmlElement)
					nodeXmlElement->removeChildElement(protocolA2XmlElement, true);
			}
			break;
			case EM_Extend:
			{
				// EM_Extend requires the second DS100 protocol to be present
				if (!protocolA2XmlElement)
				{
					protocolA2XmlElement = std::make_unique<XmlElement>(*protocolA1XmlElement).release();
					protocolA2XmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_2_PROCESSINGPROTOCOL_ID));
					auto ipAddressXmlElement = protocolA2XmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
					if (ipAddressXmlElement)
					{
						ipAddressXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS), PROTOCOL_DEFAULT2_IP);
					}
					else
						return false;

					nodeXmlElement->addChildElement(protocolA2XmlElement);
				}
			}
			break;
			case EM_Mirror:
			default:
				break;
			}
		}
		else
			return false;

		m_processingNode.setStateXml(nodeXmlElement);

		if (!dontSendNotification)
			triggerConfigurationUpdate(false);

		return true;
	}
	else
		return false;

	return true;
}

/**
 * Gets the mute state of the given source
 * @param sourceId The id of the source for which the mute state shall be returned
 * @return The mute state
 */
bool ProtocolBridgingWrapper::GetMuteDiGiCoSourceId(SourceId sourceId)
{
	return GetMuteProtocolSourceId(DIGICO_PROCESSINGPROTOCOL_ID, sourceId);
}

/**
 * Sets the given source to be (un-)muted
 * @param sourceId The id of the source that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteDiGiCoSourceId(SourceId sourceId, bool mute)
{
	if (mute)
		return SetMuteProtocolSourceId(DIGICO_PROCESSINGPROTOCOL_ID, sourceId);
	else
		return SetUnmuteProtocolSourceId(DIGICO_PROCESSINGPROTOCOL_ID, sourceId);
}

/**
 * Sets the given sources to be (un-)muted
 * @param sourceIds The ids of the sources that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteDiGiCoSourceIds(const std::vector<SourceId>& sourceIds, bool mute)
{
	if (mute)
		return SetMuteProtocolSourceIds(DIGICO_PROCESSINGPROTOCOL_ID, sourceIds);
	else
		return SetUnmuteProtocolSourceIds(DIGICO_PROCESSINGPROTOCOL_ID, sourceIds);
}

/**
 * Gets the currently set DiGiCo client ip address.
 * This method forwards the call to the generic implementation.
 * @return	The ip address string
 */
String ProtocolBridgingWrapper::GetDiGiCoIpAddress()
{
	return GetProtocolIpAddress(DIGICO_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol client ip address.
 * This method forwards the call to the generic implementation.
 * @param ipAddress	The new ip address string
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetDiGiCoIpAddress(String ipAddress, bool dontSendNotification)
{
	return SetProtocolIpAddress(DIGICO_PROCESSINGPROTOCOL_ID, ipAddress, dontSendNotification);
}

/**
 * Gets the desired protocol listening port.
 * This method forwards the call to the generic implementation.
 * @return	The requested listening port
 */
int ProtocolBridgingWrapper::GetDiGiCoListeningPort()
{
	return GetProtocolListeningPort(DIGICO_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol listening port.
 * This method forwards the call to the generic implementation.
 * @param	listeningPort	The protocol port to set as listening port
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetDiGiCoListeningPort(int listeningPort, bool dontSendNotification)
{
	return SetProtocolListeningPort(DIGICO_PROCESSINGPROTOCOL_ID, listeningPort, dontSendNotification);
}

/**
 * Gets the desired protocol remote (target client) port.
 * This method forwards the call to the generic implementation.
 * @return	The requested remote port
 */
int ProtocolBridgingWrapper::GetDiGiCoRemotePort()
{
	return GetProtocolRemotePort(DIGICO_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol remote port.
 * This method forwards the call to the generic implementation.
 * @param	remotePort	The protocol port to set as remote port
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetDiGiCoRemotePort(int remotePort, bool dontSendNotification)
{
	return SetProtocolRemotePort(DIGICO_PROCESSINGPROTOCOL_ID, remotePort, dontSendNotification);
}

/**
 * Gets the mute state of the given source
 * @param sourceId The id of the source for which the mute state shall be returned
 * @return The mute state
 */
bool ProtocolBridgingWrapper::GetMuteRTTrPMSourceId(SourceId sourceId)
{
	return GetMuteProtocolSourceId(RTTRPM_PROCESSINGPROTOCOL_ID, sourceId);
}

/**
 * Sets the given source to be (un-)muted
 * @param sourceId The id of the source that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteRTTrPMSourceId(SourceId sourceId, bool mute)
{
	if (mute)
		return SetMuteProtocolSourceId(RTTRPM_PROCESSINGPROTOCOL_ID, sourceId);
	else
		return SetUnmuteProtocolSourceId(RTTRPM_PROCESSINGPROTOCOL_ID, sourceId);
}

/**
 * Sets the given sources to be (un-)muted
 * @param sourceIds The ids of the sources that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteRTTrPMSourceIds(const std::vector<SourceId>& sourceIds, bool mute)
{
	if (mute)
		return SetMuteProtocolSourceIds(RTTRPM_PROCESSINGPROTOCOL_ID, sourceIds);
	else
		return SetUnmuteProtocolSourceIds(RTTRPM_PROCESSINGPROTOCOL_ID, sourceIds);
}

/**
 * Gets the currently set DiGiCo client ip address.
 * This method forwards the call to the generic implementation.
 * @return	The ip address string
 */
String ProtocolBridgingWrapper::GetRTTrPMIpAddress()
{
	return GetProtocolIpAddress(RTTRPM_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol client ip address.
 * This method forwards the call to the generic implementation.
 * @param ipAddress	The new ip address string
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetRTTrPMIpAddress(String ipAddress, bool dontSendNotification)
{
	return SetProtocolIpAddress(RTTRPM_PROCESSINGPROTOCOL_ID, ipAddress, dontSendNotification);
}

/**
 * Gets the desired protocol listening port.
 * This method forwards the call to the generic implementation.
 * @return	The requested listening port
 */
int ProtocolBridgingWrapper::GetRTTrPMListeningPort()
{
	return GetProtocolListeningPort(RTTRPM_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol listening port.
 * This method forwards the call to the generic implementation.
 * @param	listeningPort	The protocol port to set as listening port
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetRTTrPMListeningPort(int listeningPort, bool dontSendNotification)
{
	return SetProtocolListeningPort(RTTRPM_PROCESSINGPROTOCOL_ID, listeningPort, dontSendNotification);
}

/**
 * Gets the desired protocol remote (target client) port.
 * This method forwards the call to the generic implementation.
 * @return	The requested remote port
 */
int ProtocolBridgingWrapper::GetRTTrPMRemotePort()
{
	return GetProtocolRemotePort(RTTRPM_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol remote port.
 * This method forwards the call to the generic implementation.
 * @param	remotePort	The protocol port to set as remote port
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetRTTrPMRemotePort(int remotePort, bool dontSendNotification)
{
	return SetProtocolRemotePort(RTTRPM_PROCESSINGPROTOCOL_ID, remotePort, dontSendNotification);
}

/**
 * Gets the desired protocol mapping area id.
 * This method forwards the call to the generic implementation.
 * @return	The requested mapping area id 
 */
int ProtocolBridgingWrapper::GetRTTrPMMappingArea()
{
	return GetProtocolMappingArea(RTTRPM_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol mapping area id.
 * This method forwards the call to the generic implementation.
 * @param	mappingAreaId	The protocol mapping area id to set
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetRTTrPMMappingArea(int mappingAreaId, bool dontSendNotification)
{
	return SetProtocolMappingArea(RTTRPM_PROCESSINGPROTOCOL_ID, mappingAreaId, dontSendNotification);
}

/**
 * Gets the mute state of the given source
 * @param sourceId The id of the source for which the mute state shall be returned
 * @return The mute state
 */
bool ProtocolBridgingWrapper::GetMuteGenericOSCSourceId(SourceId sourceId)
{
	return GetMuteProtocolSourceId(GENERICOSC_PROCESSINGPROTOCOL_ID, sourceId);
}

/**
 * Sets the given source to be (un-)muted
 * @param sourceId The id of the source that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteGenericOSCSourceId(SourceId sourceId, bool mute)
{
	if (mute)
		return SetMuteProtocolSourceId(GENERICOSC_PROCESSINGPROTOCOL_ID, sourceId);
	else
		return SetUnmuteProtocolSourceId(GENERICOSC_PROCESSINGPROTOCOL_ID, sourceId);
}

/**
 * Sets the given sources to be (un-)muted
 * @param sourceIds The ids of the sources that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteGenericOSCSourceIds(const std::vector<SourceId>& sourceIds, bool mute)
{
	if (mute)
		return SetMuteProtocolSourceIds(GENERICOSC_PROCESSINGPROTOCOL_ID, sourceIds);
	else
		return SetUnmuteProtocolSourceIds(GENERICOSC_PROCESSINGPROTOCOL_ID, sourceIds);
}

/**
 * Gets the currently set GenericOSC client ip address.
 * This method forwards the call to the generic implementation.
 * @return	The ip address string
 */
String ProtocolBridgingWrapper::GetGenericOSCIpAddress()
{
	return GetProtocolIpAddress(GENERICOSC_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol client ip address.
 * This method forwards the call to the generic implementation.
 * @param ipAddress	The new ip address string
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetGenericOSCIpAddress(String ipAddress, bool dontSendNotification)
{
	return SetProtocolIpAddress(GENERICOSC_PROCESSINGPROTOCOL_ID, ipAddress, dontSendNotification);
}

/**
 * Gets the desired protocol listening port.
 * This method forwards the call to the generic implementation.
 * @return	The requested listening port
 */
int ProtocolBridgingWrapper::GetGenericOSCListeningPort()
{
	return GetProtocolListeningPort(GENERICOSC_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol listening port.
 * This method forwards the call to the generic implementation.
 * @param	listeningPort	The protocol port to set as listening port
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetGenericOSCListeningPort(int listeningPort, bool dontSendNotification)
{
	return SetProtocolListeningPort(GENERICOSC_PROCESSINGPROTOCOL_ID, listeningPort, dontSendNotification);
}

/**
 * Gets the desired protocol remote (target client) port.
 * This method forwards the call to the generic implementation.
 * @return	The requested remote port
 */
int ProtocolBridgingWrapper::GetGenericOSCRemotePort()
{
	return GetProtocolRemotePort(GENERICOSC_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol remote port.
 * This method forwards the call to the generic implementation.
 * @param	remotePort	The protocol port to set as remote port
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetGenericOSCRemotePort(int remotePort, bool dontSendNotification)
{
	return SetProtocolRemotePort(GENERICOSC_PROCESSINGPROTOCOL_ID, remotePort, dontSendNotification);
}

}
