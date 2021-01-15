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

#include "ProtocolBridgingWrapper.h"

#include "Controller.h"
#include "SoundsourceProcessor/SoundsourceProcessor.h"


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
	if (msgData._addrVal._first > DS100_CHANNELCOUNT)
		return m_processingNode.SendMessageTo(DS100_2_PROCESSINGPROTOCOL_ID, Id, msgData);
	else
		return m_processingNode.SendMessageTo(DS100_1_PROCESSINGPROTOCOL_ID, Id, msgData);
}

/**
 * Called when the OSCReceiver receives a new OSC message, since Controller inherits from OSCReceiver::Listener.
 * It forwards the message to all registered Processor objects.
 * @param callbackMessage	The node data to handle encapsulated in a calbackmsg struct..
 */
void ProtocolBridgingWrapper::HandleNodeData(const ProcessingEngineNode::NodeCallbackMessage* callbackMessage)
{
    if (!callbackMessage)
        return;
    
	for (auto l : m_listeners)
        l->HandleMessageData(callbackMessage->_protocolMessage._nodeId, callbackMessage->_protocolMessage._senderProtocolId, callbackMessage->_protocolMessage._Id, callbackMessage->_protocolMessage._msgData);
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
 * Static helper method to query if the given ROI is not one of those marked for processing in application and only to be bridged to DS100.
 * @param id	The object id to be checked regarding relevance for handling in application.
 * @return	True if the object shall be only bridged to DS100, false if it is also relevant for application
 */
bool ProtocolBridgingWrapper::IsBridgingObjectOnly(RemoteObjectIdentifier id)
{
	switch (id)
	{
	case ROI_MatrixInput_Select:
	case ROI_RemoteProtocolBridge_SoundObjectSelect:
	case ROI_RemoteProtocolBridge_UIElementIndexSelect:
		return false;
	default:
		return true;
	}
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
			auto genericMIDIProtocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(GENERICMIDI_PROCESSINGPROTOCOL_ID));
			if (genericMIDIProtocolXmlElement)
				m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_GenericMIDI, *genericMIDIProtocolXmlElement));
			auto yamahaOSCProtocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(YAMAHAOSC_PROCESSINGPROTOCOL_ID));
			if (yamahaOSCProtocolXmlElement)
				m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_YamahaOSC, *yamahaOSCProtocolXmlElement));

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
 * @param bridgingProtocolsToActivate	Bitmask definition of what bridging protocols to activate in the bridging node about to be created.
 */
void ProtocolBridgingWrapper::SetupBridgingNode(const ProtocolBridgingType bridgingProtocolsToActivate)
{
	auto nodeXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));

	nodeXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), DEFAULT_PROCNODE_ID);

	auto objectHandlingXmlElement = nodeXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING));
	if (objectHandlingXmlElement)
	{
		objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Forward_only_valueChanges));
        
        // update precision element
        auto precisionXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
        if (!precisionXmlElement)
            precisionXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
        auto precisionTextXmlElement = precisionXmlElement->getFirstChildElement();
        if (precisionTextXmlElement && precisionTextXmlElement->isTextElement())
            precisionTextXmlElement->setText(String(DS100_VALUCHANGE_SENSITIVITY));
        else
            precisionXmlElement->addTextElement(String(DS100_VALUCHANGE_SENSITIVITY));
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
		std::vector<RemoteObject> activeObjects;
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
	{
		auto digicoBridgingXmlElement = SetupDiGiCoBridgingProtocol();
		if (digicoBridgingXmlElement)
		{
			m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_DiGiCo, *digicoBridgingXmlElement));

			if ((bridgingProtocolsToActivate & PBT_DiGiCo) == PBT_DiGiCo)
				nodeXmlElement->addChildElement(digicoBridgingXmlElement.release());
		}
	}

	// RTTrPM protocol - RoleB
	{
		auto RTTrPMBridgingXmlElement = SetupRTTrPMBridgingProtocol();
		if (RTTrPMBridgingXmlElement)
		{
			m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_BlacktraxRTTrPM, *RTTrPMBridgingXmlElement));

			if ((bridgingProtocolsToActivate & PBT_BlacktraxRTTrPM) == PBT_BlacktraxRTTrPM)
				nodeXmlElement->addChildElement(RTTrPMBridgingXmlElement.release());
		}
	}

	// GenericOSC protocol - RoleB
	{
		auto genericOSCBridgingXmlElement = SetupGenericOSCBridgingProtocol();
		if (genericOSCBridgingXmlElement)
		{
			m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_GenericOSC, *genericOSCBridgingXmlElement));

			if ((bridgingProtocolsToActivate & PBT_GenericOSC) == PBT_GenericOSC)
				nodeXmlElement->addChildElement(genericOSCBridgingXmlElement.release());
		}
	}

	// GenericMIDI protocol - RoleB
	{
		auto genericMIDIBridgingXmlElement = SetupGenericMIDIBridgingProtocol();
		if (genericMIDIBridgingXmlElement)
		{
			m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_GenericMIDI, *genericMIDIBridgingXmlElement));

			if ((bridgingProtocolsToActivate & PBT_GenericMIDI) == PBT_GenericMIDI)
				nodeXmlElement->addChildElement(genericMIDIBridgingXmlElement.release());
		}
	}

	// Yamaha OSC protocol - RoleB
	{
		auto yamahaOSCBridgingXmlElement = SetupYamahaOSCBridgingProtocol();
		if (yamahaOSCBridgingXmlElement)
		{
			m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_YamahaOSC, *yamahaOSCBridgingXmlElement));

			if ((bridgingProtocolsToActivate & PBT_YamahaOSC) == PBT_YamahaOSC)
				nodeXmlElement->addChildElement(yamahaOSCBridgingXmlElement.release());
		}
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

		auto hostPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
		if (hostPortXmlElement)
			hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_RTTRPM_HOST);

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
 * Method to create the default generic MIDI bridging protocol xml element.
 * @return	The protocol xml element that was created
 */
std::unique_ptr<XmlElement> ProtocolBridgingWrapper::SetupGenericMIDIBridgingProtocol()
{
	// GenericOSC protocol - RoleB
	auto protocolBXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));
	if (protocolBXmlElement)
	{
		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), GENERICMIDI_PROCESSINGPROTOCOL_ID);

		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(PT_MidiProtocol));
		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), 0);

		auto inputDeviceIndexXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::INPUTDEVICE));
		if (inputDeviceIndexXmlElement)
			inputDeviceIndexXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DEVICEINDEX), PROTOCOL_DEFAULT_INPUTDEVICEINDEX);

		protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDCHANNELS));
	}

	return protocolBXmlElement;
}

/**
 * Method to create the default Yamaha OSC bridging protocol xml element.
 * @return	The protocol xml element that was created
 */
std::unique_ptr<XmlElement> ProtocolBridgingWrapper::SetupYamahaOSCBridgingProtocol()
{
	// Yamaha OSC protocol - RoleB
	auto protocolBXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));
	if (protocolBXmlElement)
	{
		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), YAMAHAOSC_PROCESSINGPROTOCOL_ID);

		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(PT_YamahaOSCProtocol));
		protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), 0);

		auto clientPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
		if (clientPortXmlElement)
			clientPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_YAMAHAOSC_DEVICE);

		auto hostPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
		if (hostPortXmlElement)
			hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_YAMAHAOSC_HOST);

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

				Controller* ctrl = Controller::GetInstance();
				if (ctrl)
					ctrl->SetParameterChanged(DCS_Host, DCT_MuteState);

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

				Controller* ctrl = Controller::GetInstance();
				if (ctrl)
					ctrl->SetParameterChanged(DCS_Host, DCT_MuteState);

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
		{
			Controller* ctrl = Controller::GetInstance();
			if (ctrl)
				ctrl->SetParameterChanged(DCS_Host, DCT_BridgingConfig);
		}

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
		{
			Controller* ctrl = Controller::GetInstance();
			if (ctrl)
				ctrl->SetParameterChanged(DCS_Host, DCT_BridgingConfig);
		}

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
		{
			Controller* ctrl = Controller::GetInstance();
			if (ctrl)
				ctrl->SetParameterChanged(DCS_Host, DCT_BridgingConfig);
		}

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
		{
			Controller* ctrl = Controller::GetInstance();
			if (ctrl)
				ctrl->SetParameterChanged(DCS_Host, DCT_BridgingConfig);
		}

		return true;
	}
	else
		return false;
}

/**
 * Gets the protocol's currently set input device index, if available for the given protocol.
 * @param protocolId The id of the protocol for which to get the currently configured inputdevice index
 * @return	The input device index
 */
int ProtocolBridgingWrapper::GetProtocolInputDeviceIndex(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto inputDeviceIndexXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::INPUTDEVICE));
			if (inputDeviceIndexXmlElement)
			{
				return inputDeviceIndexXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DEVICEINDEX));
			}
		}
	}

	return INVALID_ADDRESS_VALUE;
}

/**
 * Sets the given protocol mapping area id.
 * This method inserts the mapping area id into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId The id of the protocol for which to set the ip address
 * @param inputDeviceIndex	The new device index to set as input device
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolInputDeviceIndex(ProtocolId protocolId, int inputDeviceIndex, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto inputDeviceIndexXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::INPUTDEVICE));
			if (inputDeviceIndexXmlElement)
			{
				inputDeviceIndexXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DEVICEINDEX), inputDeviceIndex);
			}
			else
				return false;
		}
		else
			return false;

		m_processingNode.setStateXml(nodeXmlElement);

		if (!dontSendNotification)
		{
			Controller* ctrl = Controller::GetInstance();
			if (ctrl)
				ctrl->SetParameterChanged(DCS_Host, DCT_BridgingConfig);
		}

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

		protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(GENERICMIDI_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
			activeBridgingTypes |= PBT_GenericMIDI;

		protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(YAMAHAOSC_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
			activeBridgingTypes |= PBT_YamahaOSC;
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
		auto addGenericMIDIBridging = ((desiredActiveBridgingTypes & PBT_GenericMIDI) && !(currentlyActiveBridgingTypes & PBT_GenericMIDI));
		auto removeGenericMIDIBridging = (!(desiredActiveBridgingTypes & PBT_GenericMIDI) && (currentlyActiveBridgingTypes & PBT_GenericMIDI));
		auto addYamahaOSCBridging = ((desiredActiveBridgingTypes & PBT_YamahaOSC) && !(currentlyActiveBridgingTypes & PBT_YamahaOSC));
		auto removeYamahaOSCBridging = (!(desiredActiveBridgingTypes & PBT_YamahaOSC) && (currentlyActiveBridgingTypes & PBT_YamahaOSC));

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

			if (addGenericMIDIBridging)
			{
				nodeXmlElement->addChildElement(std::make_unique<XmlElement>(m_bridgingProtocolCacheMap.at(PBT_GenericMIDI)).release());
			}
			else if (removeGenericMIDIBridging)
			{
				auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(GENERICMIDI_PROCESSINGPROTOCOL_ID));
				if (protocolXmlElement)
				{
					m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_GenericMIDI, *protocolXmlElement));
					nodeXmlElement->removeChildElement(protocolXmlElement, true);
				}
			}

			if (addYamahaOSCBridging)
			{
				nodeXmlElement->addChildElement(std::make_unique<XmlElement>(m_bridgingProtocolCacheMap.at(PBT_YamahaOSC)).release());
			}
			else if (removeYamahaOSCBridging)
			{
				auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(YAMAHAOSC_PROCESSINGPROTOCOL_ID));
				if (protocolXmlElement)
				{
					m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_YamahaOSC, *protocolXmlElement));
					nodeXmlElement->removeChildElement(protocolXmlElement, true);
				}
			}

			m_processingNode.setStateXml(nodeXmlElement);

			Controller* ctrl = Controller::GetInstance();
			if (ctrl)
				ctrl->SetParameterChanged(DCS_Host, DCT_NumBridgingModules);
		}
	}
}

/**
 * Updates the active soundobject ids in DS100 device protocol configuration of bridging node.
 * This method gets all current active remote objects from controller, maps them to one or two (64ch vs 128ch)
 * DS100 devices and inserts them into an xml element that is then set as new config to bridging node.
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::UpdateActiveDS100SourceIds()
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		// we can't do anything here without the controller
		auto ctrl = Controller::GetInstance();
		if (!ctrl)
			return false;

		// Get currently active objects from controller and split them 
		// into those relevant for first and second DS100
		auto activeObjects = ctrl->GetActivatedRemoteObjects();
		auto activeObjectsOnFirstDS100 = std::vector<RemoteObject>{};
		auto activeObjectsOnSecondDS100 = std::vector<RemoteObject>{};
		for (auto const& ro : activeObjects)
		{
			auto sourceId = ro._Addr._first;

			// We do not support anything exceeding two DS100 (ext. mode) channelcount wise
			if (sourceId > DS100_EXTMODE_CHANNELCOUNT)
				continue;

			// If the sourceId is out of range for a single DS100, take it as relevant 
			// for second and map the sourceid back into a single DS100's source range
			if (sourceId > DS100_CHANNELCOUNT)
			{
				activeObjectsOnSecondDS100.push_back(ro);
				activeObjectsOnSecondDS100.back()._Addr._first = static_cast<int>(sourceId - DS100_CHANNELCOUNT);
			}
			// Otherwise simply take it as relevant for first DS100
			else
			{
				activeObjectsOnFirstDS100.push_back(ro);
			}
		}

		// insert active objects for first DS100 into its xml element
		auto protocolXmlElement1stDS100 = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_1_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement1stDS100)
		{
			auto activeObjsXmlElement = protocolXmlElement1stDS100->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ACTIVEOBJECTS));
			if (activeObjsXmlElement)
			{
				ProcessingEngineConfig::ReplaceActiveObjects(activeObjsXmlElement, activeObjectsOnFirstDS100);
			}
			else
				return false;
		}
		// first DS100 existence is mandatory, we can assume that an error occured if the corresp. xml element is not available (second DS100 xml element is optional)
		else
			return false;

		// insert active objects for second DS100 into its xml element
		auto protocolXmlElement2ndDS100 = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_2_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement2ndDS100)
		{
			auto activeObjsXmlElement = protocolXmlElement2ndDS100->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ACTIVEOBJECTS));
			if (activeObjsXmlElement)
			{
				ProcessingEngineConfig::ReplaceActiveObjects(activeObjsXmlElement, activeObjectsOnSecondDS100);
			}
			else
				return false;
		}

		// set updated xml config live
		m_processingNode.setStateXml(nodeXmlElement);

		// broadcast that bridging config has changed
		ctrl->SetParameterChanged(DCS_Host, DCT_BridgingConfig);

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
		{
			Controller* ctrl = Controller::GetInstance();
			if (ctrl)
				ctrl->SetParameterChanged(DCS_Host, DCT_BridgingConfig);
		}

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
			case OHM_Mux_nA_to_mB_withValFilter:
				return EM_Extend;
			case OHM_Forward_only_valueChanges:
				return EM_Off;
			case OHM_Bypass:
			case OHM_Invalid:
			case OHM_Mux_nA_to_mB:
			case OHM_Remap_A_X_Y_to_B_XY:
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
				objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Forward_only_valueChanges));
				// remove elements that are not used by this ohm
				auto protocolAChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLACHCNT));
				if (protocolAChCntXmlElement)
					objectHandlingXmlElement->removeChildElement(protocolAChCntXmlElement, true);
				auto protocolBChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLBCHCNT));
				if (protocolBChCntXmlElement)
					objectHandlingXmlElement->removeChildElement(protocolBChCntXmlElement, true);

				// update precision element
				auto precisionXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
				if (!precisionXmlElement)
					precisionXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
				auto precisionTextXmlElement = precisionXmlElement->getFirstChildElement();
				if (precisionTextXmlElement && precisionTextXmlElement->isTextElement())
					precisionTextXmlElement->setText(String(DS100_VALUCHANGE_SENSITIVITY));
				else
					precisionXmlElement->addTextElement(String(DS100_VALUCHANGE_SENSITIVITY));
			}
			break;
			case EM_Extend:
			{
				// EM_Extend refers to Multiplex nA to mB object handling mode with channel A and B parameter attributes
				objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Mux_nA_to_mB_withValFilter));
				
				// update first DS100 channel count elements
				auto protocolAChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLACHCNT));
				if (!protocolAChCntXmlElement)
					protocolAChCntXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLACHCNT));
				auto protocolAChCntTextXmlElement = protocolAChCntXmlElement->getFirstChildElement();
				if (protocolAChCntTextXmlElement && protocolAChCntTextXmlElement->isTextElement())
					protocolAChCntTextXmlElement->setText(String(DS100_CHANNELCOUNT));
				else
					protocolAChCntXmlElement->addTextElement(String(DS100_CHANNELCOUNT));

				// update first DS100 channel count elements
				auto protocolBChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLBCHCNT));
				if (!protocolBChCntXmlElement)
					protocolBChCntXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLBCHCNT));
				auto protocolBChCntTextXmlElement = protocolBChCntXmlElement->getFirstChildElement();
				if (protocolBChCntTextXmlElement && protocolBChCntTextXmlElement->isTextElement())
					protocolBChCntTextXmlElement->setText(String(DS100_EXTMODE_CHANNELCOUNT));
				else
					protocolBChCntXmlElement->addTextElement(String(DS100_EXTMODE_CHANNELCOUNT));

				// update precision element
				auto precisionXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
				if (!precisionXmlElement)
					precisionXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
				auto precisionTextXmlElement = precisionXmlElement->getFirstChildElement();
				if (precisionTextXmlElement && precisionTextXmlElement->isTextElement())
					precisionTextXmlElement->setText(String(DS100_VALUCHANGE_SENSITIVITY));
				else
					precisionXmlElement->addTextElement(String(DS100_VALUCHANGE_SENSITIVITY));
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
		{
			Controller* ctrl = Controller::GetInstance();
			if (ctrl)
				ctrl->SetParameterChanged(DCS_Host, DCT_BridgingConfig);
		}

		return true;
	}
	else
		return false;
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

/**
 * Gets the mute state of the given source
 * @param sourceId The id of the source for which the mute state shall be returned
 * @return The mute state
 */
bool ProtocolBridgingWrapper::GetMuteGenericMIDISourceId(SourceId sourceId)
{
	return GetMuteProtocolSourceId(GENERICMIDI_PROCESSINGPROTOCOL_ID, sourceId);
}

/**
 * Sets the given source to be (un-)muted
 * @param sourceId The id of the source that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteGenericMIDISourceId(SourceId sourceId, bool mute)
{
	if (mute)
		return SetMuteProtocolSourceId(GENERICMIDI_PROCESSINGPROTOCOL_ID, sourceId);
	else
		return SetUnmuteProtocolSourceId(GENERICMIDI_PROCESSINGPROTOCOL_ID, sourceId);
}

/**
 * Sets the given sources to be (un-)muted
 * @param sourceIds The ids of the sources that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteGenericMIDISourceIds(const std::vector<SourceId>& sourceIds, bool mute)
{
	if (mute)
		return SetMuteProtocolSourceIds(GENERICMIDI_PROCESSINGPROTOCOL_ID, sourceIds);
	else
		return SetUnmuteProtocolSourceIds(GENERICMIDI_PROCESSINGPROTOCOL_ID, sourceIds);
}

/**
 * Gets the desired protocol input device index.
 * This method forwards the call to the generic implementation.
 * @return	The requested input device index
 */
int ProtocolBridgingWrapper::GetGenericMIDIInputDeviceIndex()
{
	return GetProtocolInputDeviceIndex(GENERICMIDI_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol input device index.
 * This method forwards the call to the generic implementation.
 * @param	inputDeviceIndex	The protocol input device index to set
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetGenericMIDIInputDeviceIndex(int inputDeviceIndex, bool dontSendNotification)
{
	return SetProtocolInputDeviceIndex(GENERICMIDI_PROCESSINGPROTOCOL_ID, inputDeviceIndex, dontSendNotification);
}

/**
 * Gets the mute state of the given source
 * @param sourceId The id of the source for which the mute state shall be returned
 * @return The mute state
 */
bool ProtocolBridgingWrapper::GetMuteYamahaOSCSourceId(SourceId sourceId)
{
	return GetMuteProtocolSourceId(YAMAHAOSC_PROCESSINGPROTOCOL_ID, sourceId);
}

/**
 * Sets the given source to be (un-)muted
 * @param sourceId The id of the source that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteYamahaOSCSourceId(SourceId sourceId, bool mute)
{
	if (mute)
		return SetMuteProtocolSourceId(YAMAHAOSC_PROCESSINGPROTOCOL_ID, sourceId);
	else
		return SetUnmuteProtocolSourceId(YAMAHAOSC_PROCESSINGPROTOCOL_ID, sourceId);
}

/**
 * Sets the given sources to be (un-)muted
 * @param sourceIds The ids of the sources that shall be muted
 * @param mute Set to true for mute and false for unmute
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteYamahaOSCSourceIds(const std::vector<SourceId>& sourceIds, bool mute)
{
	if (mute)
		return SetMuteProtocolSourceIds(YAMAHAOSC_PROCESSINGPROTOCOL_ID, sourceIds);
	else
		return SetUnmuteProtocolSourceIds(YAMAHAOSC_PROCESSINGPROTOCOL_ID, sourceIds);
}

/**
 * Gets the currently set DiGiCo client ip address.
 * This method forwards the call to the generic implementation.
 * @return	The ip address string
 */
String ProtocolBridgingWrapper::GetYamahaOSCIpAddress()
{
	return GetProtocolIpAddress(YAMAHAOSC_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol client ip address.
 * This method forwards the call to the generic implementation.
 * @param ipAddress	The new ip address string
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetYamahaOSCIpAddress(String ipAddress, bool dontSendNotification)
{
	return SetProtocolIpAddress(YAMAHAOSC_PROCESSINGPROTOCOL_ID, ipAddress, dontSendNotification);
}

/**
 * Gets the desired protocol listening port.
 * This method forwards the call to the generic implementation.
 * @return	The requested listening port
 */
int ProtocolBridgingWrapper::GetYamahaOSCListeningPort()
{
	return GetProtocolListeningPort(YAMAHAOSC_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol listening port.
 * This method forwards the call to the generic implementation.
 * @param	listeningPort	The protocol port to set as listening port
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetYamahaOSCListeningPort(int listeningPort, bool dontSendNotification)
{
	return SetProtocolListeningPort(YAMAHAOSC_PROCESSINGPROTOCOL_ID, listeningPort, dontSendNotification);
}

/**
 * Gets the desired protocol remote (target client) port.
 * This method forwards the call to the generic implementation.
 * @return	The requested remote port
 */
int ProtocolBridgingWrapper::GetYamahaOSCRemotePort()
{
	return GetProtocolRemotePort(YAMAHAOSC_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol remote port.
 * This method forwards the call to the generic implementation.
 * @param	remotePort	The protocol port to set as remote port
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetYamahaOSCRemotePort(int remotePort, bool dontSendNotification)
{
	return SetProtocolRemotePort(YAMAHAOSC_PROCESSINGPROTOCOL_ID, remotePort, dontSendNotification);
}

/**
 * Gets the desired protocol mapping area id.
 * This method forwards the call to the generic implementation.
 * @return	The requested mapping area id
 */
int ProtocolBridgingWrapper::GetYamahaOSCMappingArea()
{
	return GetProtocolMappingArea(YAMAHAOSC_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the desired protocol mapping area id.
 * This method forwards the call to the generic implementation.
 * @param	mappingAreaId	The protocol mapping area id to set
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetYamahaOSCMappingArea(int mappingAreaId, bool dontSendNotification)
{
	return SetProtocolMappingArea(YAMAHAOSC_PROCESSINGPROTOCOL_ID, mappingAreaId, dontSendNotification);
}

}
