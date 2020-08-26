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

	// CController derives from ProcessingEngineNode::Listener
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
	return m_processingNode.SendMessageTo(DS100_PROCESSINGPROTOCOL_ID, Id, msgData);
}

/**
 * Called when the OSCReceiver receives a new OSC message, since CController inherits from OSCReceiver::Listener.
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

	//auto bridgingXmlElement = controllerXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::BRIDGING));
	//if (bridgingXmlElement)
	//{
	//	auto nodeXmlElement = m_processingNode.createStateXml();
	//	if (nodeXmlElement)
	//	{
	//		bridgingXmlElement->addChildElement(nodeXmlElement.release());
	//		m_bridgingXml = *bridgingXmlElement;
	//	}
	//	else
	//	{
	//		controllerXmlElement->replaceChildElement(bridgingXmlElement, std::make_unique<XmlElement>(m_bridgingXml).release());
	//	}
	//}

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
			return m_processingNode.setStateXml(nodeXmlElement);
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

	auto protocolAXmlElement = nodeXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLA));
	if (protocolAXmlElement)
	{
		protocolAXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), DS100_PROCESSINGPROTOCOL_ID);

		protocolAXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(PT_OSCProtocol));
		protocolAXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), 1);

		auto clientPortXmlElement = protocolAXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
		if (clientPortXmlElement)
			clientPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_DS100);

		auto hostPortXmlElement = protocolAXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
		if (hostPortXmlElement)
			hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_HOST);

		// Active objects preparation
		Array<RemoteObject> activeObjects;
		RemoteObject objectX, objectY;
		//
		//objectX.Id = ROI_SoundObject_Position_X;
		//objectY.Id = ROI_SoundObject_Position_Y;
		//for (int16 i = 1; i <= 16; ++i)
		//{
		//	RemoteObjectAddressing addr;
		//	addr.first = i; //channel = source
		//	addr.second = 1; //record = mapping
		//
		//	objectX.Addr = addr;
		//	objectY.Addr = addr;
		//
		//	activeObjects.add(objectX);
		//	activeObjects.add(objectY);
		//}
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

	m_processingNode.setStateXml(nodeXmlElement.get());

	m_bridgingXml.addChildElement(nodeXmlElement.release());
}

/**
 * Sets the given soundobject/mapping as to be activly handled.
 * This method inserts the object into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param sourceId	The id of the new soundsource object to activate
 * @param mappingId The mapping id that is to be activated for the soundsource object
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::ActivateDS100SourceId(juce::int16 sourceId, juce::int16 mappingId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
		{
			auto activeObjsXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ACTIVEOBJECTS));
			if (activeObjsXmlElement)
			{
				juce::Array<RemoteObject> activeObjects;
				ProcessingEngineConfig::ReadActiveObjects(activeObjsXmlElement, activeObjects);
				for (int roi = ROI_SoundObject_Position_XY; roi < ROI_UserMAX; roi++)
				{
					RemoteObject newSourceObject;
					newSourceObject.Id = static_cast<RemoteObjectIdentifier>(roi);
					newSourceObject.Addr.first = sourceId;
					if (roi == ROI_SoundObject_Position_X || roi == ROI_SoundObject_Position_Y || roi == ROI_SoundObject_Position_XY)
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
		triggerConfigurationUpdate();

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
bool ProtocolBridgingWrapper::DeactivateDS100SourceId(juce::int16 sourceId, juce::int16 mappingId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
		{
			auto activeObjsXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ACTIVEOBJECTS));
			if (activeObjsXmlElement)
			{
				juce::Array<RemoteObject> activeObjects;
				ProcessingEngineConfig::ReadActiveObjects(activeObjsXmlElement, activeObjects);
				for (int roi = ROI_SoundObject_Position_XY; roi < ROI_UserMAX; roi++)
				{
					RemoteObject newSourceObject;
					newSourceObject.Id = static_cast<RemoteObjectIdentifier>(roi);
					newSourceObject.Addr.first = sourceId;
					if (roi == ROI_SoundObject_Position_X || roi == ROI_SoundObject_Position_Y || roi == ROI_SoundObject_Position_XY)
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
		triggerConfigurationUpdate();

		return true;
	}
	else
		return false;
}

/**
 * Sets the desired protocol client ip address.
 * This method inserts the ip address into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param ipAddress	The new ip address string
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetDS100IpAddress(String ipAddress)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_PROCESSINGPROTOCOL_ID));
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
		triggerConfigurationUpdate();

		return true;
	}
	else
		return false;
}

/**
 * Sets the desired message rate for protocol polling.
 * This method inserts the rate value into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param msgRate	The new message rate value in ms
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetDS100MsgRate(int msgRate)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_PROCESSINGPROTOCOL_ID));
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
		triggerConfigurationUpdate();

		return true;
	}
	else
		return false;
}

}
