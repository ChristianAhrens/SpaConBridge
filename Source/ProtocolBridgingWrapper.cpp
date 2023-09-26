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

#include "ProtocolBridgingWrapper.h"

#include "Controller.h"
#include "CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"

#include <ProcessingEngine/ObjectDataHandling/ObjectDataHandling_Abstract.h>


namespace SpaConBridge
{

/**
 * Constructor
 */
ProtocolBridgingWrapper::ProtocolBridgingWrapper()
	: m_bridgingXml(AppConfiguration::getTagName(AppConfiguration::TagID::BRIDGING))
{
	// Controller derives from ProcessingEngineNode::Listener
	m_processingNode.AddListener(this);

	SetupBridgingNode();
}

/**
 * Destructor
 */
ProtocolBridgingWrapper::~ProtocolBridgingWrapper()
{
	m_listeners.clear();
}

/**
 * Method to register a listener object to be called when the node has received the respective data via a node protocol.
 * @param	listener						The listener object to add to the internal list of listeners
 */
void ProtocolBridgingWrapper::AddListener(ProtocolBridgingWrapper::Listener* listener)
{
	if (listener)
		m_listeners.push_back(listener);
}

/**
 * Send a Message out via the active bridging node.
 * @param roi	The id of the remote object to be sent.
 * @param msgData	The message data to be sent.
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SendMessage(const RemoteObjectIdentifier roi, RemoteObjectMessageData& msgData)
{
	if (GetDS100ExtensionMode() == EM_Mirror)
	{
		// if the first DS100 is master, send data to it
		if ((GetProtocolState(DS100_1_PROCESSINGPROTOCOL_ID) & OHS_Protocol_Master) == OHS_Protocol_Master)
			return m_processingNode.SendMessageTo(DS100_1_PROCESSINGPROTOCOL_ID, roi, msgData, ASYNC_EXTID);

		// if the second DS100 is master, send data to it
		else if ((GetProtocolState(DS100_2_PROCESSINGPROTOCOL_ID) & OHS_Protocol_Master) == OHS_Protocol_Master)
			return m_processingNode.SendMessageTo(DS100_2_PROCESSINGPROTOCOL_ID, roi, msgData, ASYNC_EXTID);

		// of no master is present, we have an undefined state, cannot happen!
		else
			return false;
	}
	else if (GetDS100ExtensionMode() == EM_Extend)
	{
		if (msgData._addrVal._first > DS100_CHANNELCOUNT)
		{
			auto mappedChannel = static_cast<std::int32_t>(msgData._addrVal._first % DS100_CHANNELCOUNT);
			if (mappedChannel == 0)
				mappedChannel = static_cast<std::int32_t>(DS100_CHANNELCOUNT);
			msgData._addrVal._first = mappedChannel;

			return m_processingNode.SendMessageTo(DS100_2_PROCESSINGPROTOCOL_ID, roi, msgData, ASYNC_EXTID);
		}
		else
			return m_processingNode.SendMessageTo(DS100_1_PROCESSINGPROTOCOL_ID, roi, msgData, ASYNC_EXTID);
	}
	else if (GetDS100ExtensionMode() == EM_Parallel)
	{
		auto sendSuccess = m_processingNode.SendMessageTo(DS100_1_PROCESSINGPROTOCOL_ID, roi, msgData, ASYNC_EXTID);
		sendSuccess = m_processingNode.SendMessageTo(DS100_2_PROCESSINGPROTOCOL_ID, roi, msgData, ASYNC_EXTID) && sendSuccess;

		return sendSuccess;
	}
	else
	{
		return m_processingNode.SendMessageTo(DS100_1_PROCESSINGPROTOCOL_ID, roi, msgData, ASYNC_EXTID);
	}
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
 * @return	True if the reconnection was triggered, false if controller state is offline and therefor no reconnection is allowed
 */
bool ProtocolBridgingWrapper::Reconnect()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl || !ctrl->IsOnline()) // dont execute the reconnection if controller state suggests that the application shall be offline
		return false;

	Disconnect();

	auto nodeXmlElement = m_bridgingXml.getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));
	if (nodeXmlElement)
		SetBridgingNodeStateXml(nodeXmlElement, true);
	m_processingNode.Start();

	return true;
}

/**
 * Static helper method to query if the given ROI is not one of those marked for processing in application and only to be bridged to DS100.
 * @param roi	The object id to be checked regarding relevance for handling in application.
 * @return	True if the object shall be only bridged to DS100, false if it is also relevant for application
 */
bool ProtocolBridgingWrapper::IsBridgingObjectOnly(const RemoteObjectIdentifier roi)
{
	switch (roi)
	{
	case ROI_MatrixInput_Select:
	case ROI_RemoteProtocolBridge_SoundObjectSelect:
	case ROI_RemoteProtocolBridge_UIElementIndexSelect:
	case ROI_RemoteProtocolBridge_GetAllKnownValues:
	case ROI_RemoteProtocolBridge_SoundObjectGroupSelect:
	case ROI_RemoteProtocolBridge_MatrixInputGroupSelect:
	case ROI_RemoteProtocolBridge_MatrixOutputGroupSelect:
		return false;
	default:
		return true;
	}
}

/**
 * Reimplemented from ObjectDataHandling_Abstract::StateListener to get notified on state changes
 * in bridging object handling object regarding protocol state changes.
 * @param	id		The id of the protocol that the status has changed of.
 * @param	state	The new state value.
 */
void ProtocolBridgingWrapper::protocolStateChanged(ProtocolId id, ObjectHandlingState state)
{
	SetProtocolState(id, state);
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
	if (stateXml && (stateXml->getTagName() == AppConfiguration::getTagName(AppConfiguration::TagID::BRIDGING)))
	{
		m_bridgingXml = *stateXml;
		auto nodeXmlElement = stateXml->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE));
		if (nodeXmlElement)
		{
			// verify object handling element
			auto objectHandlingXmlElement = nodeXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING));
			if (!objectHandlingXmlElement)
				objectHandlingXmlElement = nodeXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING));
			if (objectHandlingXmlElement)
			{
				if (objectHandlingXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE)).isEmpty())
					objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Forward_only_valueChanges));

				// init precision element
				auto precisionXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
				if (!precisionXmlElement)
				{
					precisionXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
					precisionXmlElement->addTextElement(String(DS100_VALUCHANGE_SENSITIVITY));
				}

				// init typeA (DS100) to have the valueAck expected flag set
				auto protocolsAcknowledgeXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALUEACK));
				if (!protocolsAcknowledgeXmlElement)
				{
					protocolsAcknowledgeXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALUEACK));
					protocolsAcknowledgeXmlElement->addTextElement(String(DS100_ACKNOWLEDGES_SENT_VALUES_MASK));
				}

				// init reaction monitored protocols element
				auto reactMoniProtosTextValue = String(DS100_1_PROCESSINGPROTOCOL_ID) + "," + String(DS100_2_PROCESSINGPROTOCOL_ID);
				auto reactMoniProtosXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REACTMONIPROTOS));
				if (!reactMoniProtosXmlElement)
				{
					reactMoniProtosXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REACTMONIPROTOS));
					reactMoniProtosXmlElement->addTextElement(reactMoniProtosTextValue);
				}
			}

			// cache all bridging protocol elements
			auto digicoProtocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DIGICO_PROCESSINGPROTOCOL_ID));
			if (digicoProtocolXmlElement)
				m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_DiGiCo, *digicoProtocolXmlElement));
			auto DAWPluginProtocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DAWPLUGIN_PROCESSINGPROTOCOL_ID));
			if (DAWPluginProtocolXmlElement)
				m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_DAWPlugin, *DAWPluginProtocolXmlElement));
			auto RTTrPMProtocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(RTTRPM_PROCESSINGPROTOCOL_ID));
			if (RTTrPMProtocolXmlElement)
				m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_BlacktraxRTTrPM, *RTTrPMProtocolXmlElement));
			auto genericOSCProtocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(GENERICOSC_PROCESSINGPROTOCOL_ID));
			if (genericOSCProtocolXmlElement)
				m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_GenericOSC, *genericOSCProtocolXmlElement));
			auto genericMIDIProtocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(GENERICMIDI_PROCESSINGPROTOCOL_ID));
			if (genericMIDIProtocolXmlElement)
				m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_GenericMIDI, *genericMIDIProtocolXmlElement));
			auto admOSCProtocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(ADMOSC_PROCESSINGPROTOCOL_ID));
			if (admOSCProtocolXmlElement)
				m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_ADMOSC, *admOSCProtocolXmlElement));
			auto yamahaOSCProtocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(YAMAHAOSC_PROCESSINGPROTOCOL_ID));
			if (yamahaOSCProtocolXmlElement)
				m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_YamahaOSC, *yamahaOSCProtocolXmlElement));
			auto remapOSCProtocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(REMAPOSC_PROCESSINGPROTOCOL_ID));
			if (remapOSCProtocolXmlElement)
				m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_RemapOSC, *remapOSCProtocolXmlElement));

			return SetBridgingNodeStateXml(nodeXmlElement, true);
		}
		else
			return false;
	}
	else
	{
		SetupBridgingNode();

		return false;
	}

}

/**
 * Method to set an updated xml config element as new bridging node config.
 * This also takes care to re-register object handling module listeners after configuration change.
 * 
 * @param	stateXml		The new bridging node xml configuration to activate.
 * @return	True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetBridgingNodeStateXml(XmlElement* stateXml, bool dontSendNotification)
{
	// sanity check, if the incoming xml does make sense for this method
	if (!stateXml || (stateXml->getTagName() != ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::NODE)))
		return false;

	// To prevent that we end up in a recursive ::setStateXml situation, verify that this setStateXml method is not called by itself
	const ScopedXmlChangeLock lock(IsXmlChangeLocked());
	if (!lock.isLocked())
		return false;

	if (!dontSendNotification)
	{
		Controller* ctrl = Controller::GetInstance();
		if (ctrl)
			ctrl->SetParameterChanged(DCP_Host, DCT_BridgingConfig);
	}

	if (m_processingNode.setStateXml(stateXml))
	{
		if (auto objHandling = m_processingNode.GetObjectDataHandling())
			objHandling->AddStateListener(this);

		return true;
	}
	else
		return false;
}

/**
 * Method to create a basic configuration to use to setup the single supported
 * bridging node.
 * @param	bridgingProtocolsToActivate		Bitmask definition of what bridging protocols to activate in the bridging node about to be created.
 * @return	True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetupBridgingNode(const ProtocolBridgingType bridgingProtocolsToActivate)
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

		// update typeA (DS100) to have the valueAck expected flag set
		auto protocolsAcknowledgeXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALUEACK));
		if (!protocolsAcknowledgeXmlElement)
			protocolsAcknowledgeXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALUEACK));
		auto protocolsAcknowledgeTextXmlElement = protocolsAcknowledgeXmlElement->getFirstChildElement();
		if (protocolsAcknowledgeTextXmlElement && protocolsAcknowledgeTextXmlElement->isTextElement())
			protocolsAcknowledgeTextXmlElement->setText(String(DS100_ACKNOWLEDGES_SENT_VALUES_MASK));
		else
			protocolsAcknowledgeXmlElement->addTextElement(String(DS100_ACKNOWLEDGES_SENT_VALUES_MASK));

		// update reaction monitored protocols element
		auto reactMoniProtosTextValue = String(DS100_1_PROCESSINGPROTOCOL_ID) + "," + String(DS100_2_PROCESSINGPROTOCOL_ID);
		auto reactMoniProtosXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REACTMONIPROTOS));
		if (!reactMoniProtosXmlElement)
			reactMoniProtosXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REACTMONIPROTOS));
		auto reactMoniProtosTextXmlElement = reactMoniProtosXmlElement->getFirstChildElement();
		if (reactMoniProtosTextXmlElement && reactMoniProtosTextXmlElement->isTextElement())
			reactMoniProtosTextXmlElement->setText(reactMoniProtosTextValue);
		else
			reactMoniProtosXmlElement->addTextElement(reactMoniProtosTextValue);
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
		auto activeObjects = std::vector<RemoteObject>();
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

	// DAWPlugin protocol - RoleB
	{
		auto DAWPluginBridgingXmlElement = SetupDAWPluginBridgingProtocol();
		if (DAWPluginBridgingXmlElement)
		{
			m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_DAWPlugin, *DAWPluginBridgingXmlElement));

			if ((bridgingProtocolsToActivate & PBT_DAWPlugin) == PBT_DAWPlugin)
				nodeXmlElement->addChildElement(DAWPluginBridgingXmlElement.release());
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

	// ADM OSC protocol - RoleB
	{
		auto admOSCBridgingXmlElement = SetupADMOSCBridgingProtocol();
		if (admOSCBridgingXmlElement)
		{
			m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_ADMOSC, *admOSCBridgingXmlElement));

			if ((bridgingProtocolsToActivate & PBT_ADMOSC) == PBT_ADMOSC)
				nodeXmlElement->addChildElement(admOSCBridgingXmlElement.release());
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

	// Remap OSC protocol - RoleB
	{
		auto remapOSCBridgingXmlElement = SetupRemapOSCBridgingProtocol();
		if (remapOSCBridgingXmlElement)
		{
			m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_RemapOSC, *remapOSCBridgingXmlElement));

			if ((bridgingProtocolsToActivate & PBT_RemapOSC) == PBT_RemapOSC)
				nodeXmlElement->addChildElement(remapOSCBridgingXmlElement.release());
		}
	}

	if (SetBridgingNodeStateXml(nodeXmlElement.get(), true))
	{
		m_bridgingXml.addChildElement(nodeXmlElement.release());
		return true;
	}
	else
		return false;
}

/**
 * Method to create the default digico bridging protocol xml element.
 * @return	The protocol xml element that was created
 */
std::unique_ptr<XmlElement> ProtocolBridgingWrapper::SetupDiGiCoBridgingProtocol()
{
	// DiGiCo protocol - RoleB
	auto protocolBXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));

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

	auto mutedObjsXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDOBJECTS));
	auto mutedObjects = std::vector<RemoteObject>();
	if (mutedObjsXmlElement)
		ProcessingEngineConfig::WriteMutedObjects(mutedObjsXmlElement, mutedObjects);

	return protocolBXmlElement;
}

/**
 * Method to create the default DAW Plugin bridging protocol xml element.
 * @return	The protocol xml element that was created
 */
std::unique_ptr<XmlElement> ProtocolBridgingWrapper::SetupDAWPluginBridgingProtocol()
{
	// DAWPlugin protocol - RoleB
	auto protocolBXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));

	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), DAWPLUGIN_PROCESSINGPROTOCOL_ID);

	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(PT_OSCProtocol));
	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), 0);

	auto clientPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
	if (clientPortXmlElement)
		clientPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_DS100_HOST);

	auto hostPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
	if (hostPortXmlElement)
		hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_DS100_DEVICE);

	auto ipAdressXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
	if (ipAdressXmlElement)
		ipAdressXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS), PROTOCOL_DEFAULT_PRVATELAN_IP);

	auto mutedObjsXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDOBJECTS));
	auto mutedObjects = std::vector<RemoteObject>();
	if (mutedObjsXmlElement)
		ProcessingEngineConfig::WriteMutedObjects(mutedObjsXmlElement, mutedObjects);

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

	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), RTTRPM_PROCESSINGPROTOCOL_ID);

	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(PT_RTTrPMProtocol));
	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), 0);

	auto hostPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
	if (hostPortXmlElement)
		hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_RTTRPM_HOST);

	auto mappingAreaIdXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREA));
	if (mappingAreaIdXmlElement)
		mappingAreaIdXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), PROTOCOL_DEFAULT_MAPPINGAREA);

	auto moduleTypeIdentifierXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PACKETMODULE));
	if (moduleTypeIdentifierXmlElement)
		moduleTypeIdentifierXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), "CentroidPosition;CentroidAccelerationAndVelocity");

	auto mutedObjsXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDOBJECTS));
	auto mutedObjects = std::vector<RemoteObject>();
	if (mutedObjsXmlElement)
		ProcessingEngineConfig::WriteMutedObjects(mutedObjsXmlElement, mutedObjects);

	auto mappingAreaRescaleXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREARESCALE));
	if (mappingAreaRescaleXmlElement)
		mappingAreaRescaleXmlElement->addTextElement("-3;3;-3;3");

	auto oscRemappingsXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REMAPPINGS));
	ignoreUnused(oscRemappingsXmlElement); // newly created element is taken into a variable above only for debugging, no need to have the unused-warning in the build output...

	auto xyAxisSwappedXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::XYSWAPPED));
	if (xyAxisSwappedXmlElement)
		xyAxisSwappedXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), 0);

	auto xAxisInvertedXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::XINVERTED));
	if (xAxisInvertedXmlElement)
		xAxisInvertedXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), 0);

	auto yAxisInvertedXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::YINVERTED));
	if (yAxisInvertedXmlElement)
		yAxisInvertedXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), 0);

	auto originOffsetXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ORIGINOFFSET));
	if (originOffsetXmlElement)
		originOffsetXmlElement->addTextElement("0;0");

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

	auto dataSendingDisabledXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATASENDINGDISABLED));
	if (dataSendingDisabledXmlElement)
		dataSendingDisabledXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), 0);

	auto mutedObjsXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDOBJECTS));
	auto mutedObjects = std::vector<RemoteObject>();
	if (mutedObjsXmlElement)
		ProcessingEngineConfig::WriteMutedObjects(mutedObjsXmlElement, mutedObjects);

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

	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), GENERICMIDI_PROCESSINGPROTOCOL_ID);

	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(PT_MidiProtocol));
	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), 0);

	auto inputDeviceIdentifierXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::INPUTDEVICE));
	if (inputDeviceIdentifierXmlElement)
		inputDeviceIdentifierXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DEVICEIDENTIFIER), String());

	auto outputDeviceIdentifierXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OUTPUTDEVICE));
	if (outputDeviceIdentifierXmlElement)
		outputDeviceIdentifierXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DEVICEIDENTIFIER), String());

	auto mappingAreaIdXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREA));
	if (mappingAreaIdXmlElement)
		mappingAreaIdXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), PROTOCOL_DEFAULT_MAPPINGAREA);

	auto mutedObjsXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDOBJECTS));
	auto mutedObjects = std::vector<RemoteObject>();
	if (mutedObjsXmlElement)
		ProcessingEngineConfig::WriteMutedObjects(mutedObjsXmlElement, mutedObjects);

	return protocolBXmlElement;
}

/**
 * Method to create the default ADM OSC bridging protocol xml element.
 * @return	The protocol xml element that was created
 */
std::unique_ptr<XmlElement> ProtocolBridgingWrapper::SetupADMOSCBridgingProtocol()
{
	// ADM OSC protocol - RoleB
	auto protocolBXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));

	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), ADMOSC_PROCESSINGPROTOCOL_ID);

	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(PT_ADMOSCProtocol));
	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), 0);

	auto clientPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
	if (clientPortXmlElement)
		clientPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_ADMOSC_DEVICE);

	auto hostPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
	if (hostPortXmlElement)
		hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_ADMOSC_HOST);

	auto ipAdressXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
	if (ipAdressXmlElement)
		ipAdressXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS), PROTOCOL_DEFAULT_IP);

	auto mappingAreaIdXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREA));
	if (mappingAreaIdXmlElement)
		mappingAreaIdXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), PROTOCOL_DEFAULT_MAPPINGAREA);

	auto xAxisInvertedXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::XINVERTED));
	if (xAxisInvertedXmlElement)
		xAxisInvertedXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), 0);

	auto yAxisInvertedXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::YINVERTED));
	if (yAxisInvertedXmlElement)
		yAxisInvertedXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), 0);

	auto xyAxisSwappedXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::XYSWAPPED));
	if (xyAxisSwappedXmlElement)
		xyAxisSwappedXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), 0);

	auto dataSendingDisabledXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATASENDINGDISABLED));
	if (dataSendingDisabledXmlElement)
		dataSendingDisabledXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), 0);

	auto xyMessageCombinedXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::XYMESSAGECOMBINED));
	if (xyMessageCombinedXmlElement)
		xyMessageCombinedXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), 0);

	auto mutedObjsXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDOBJECTS));
	auto mutedObjects = std::vector<RemoteObject>();
	if (mutedObjsXmlElement)
		ProcessingEngineConfig::WriteMutedObjects(mutedObjsXmlElement, mutedObjects);

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

	auto mutedObjsXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDOBJECTS));
	auto mutedObjects = std::vector<RemoteObject>();
	if (mutedObjsXmlElement)
		ProcessingEngineConfig::WriteMutedObjects(mutedObjsXmlElement, mutedObjects);

	return protocolBXmlElement;
}

/**
 * Method to create the default Remap OSC bridging protocol xml element.
 * @return	The protocol xml element that was created
 */
std::unique_ptr<XmlElement> ProtocolBridgingWrapper::SetupRemapOSCBridgingProtocol()
{
	// Remap OSC protocol - RoleB
	auto protocolBXmlElement = std::make_unique<XmlElement>(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLB));

	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), REMAPOSC_PROCESSINGPROTOCOL_ID);

	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(PT_RemapOSCProtocol));
	protocolBXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::USESACTIVEOBJ), 0);

	auto clientPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
	if (clientPortXmlElement)
		clientPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_REMAPOSC_DEVICE);

	auto hostPortXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
	if (hostPortXmlElement)
		hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), RX_PORT_REMAPOSC_HOST);

	auto ipAdressXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
	if (ipAdressXmlElement)
		ipAdressXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS), PROTOCOL_DEFAULT_IP);

	auto dataSendingDisabledXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATASENDINGDISABLED));
	if (dataSendingDisabledXmlElement)
		dataSendingDisabledXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), 0);

	auto mutedObjsXmlElement = protocolBXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDOBJECTS));
	auto mutedObjects = std::vector<RemoteObject>();
	if (mutedObjsXmlElement)
		ProcessingEngineConfig::WriteMutedObjects(mutedObjsXmlElement, mutedObjects);

	return protocolBXmlElement;
}

/**
 * Gets the mute state of the given source of given protocol
 * @param protocolProcessorId The id of the protocolProcessor the muted state shall be returned of
 * @param soundobjectId The id of the source that shall be muted
 * @return True if mute is active, false if not
 */
bool ProtocolBridgingWrapper::GetMuteProtocolSoundobjectProcessorId(ProtocolId protocolId, SoundobjectProcessorId soundobjectProcessorId)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;
		
	return GetProtocolRemoteObjectsMutedState(protocolId, ctrl->GetSoundobjectProcessorRemoteObjects(soundobjectProcessorId));
}

/**
 * Sets the given source of given protocol to be muted
 * @param protocolId The id of the protocol the source shall be muted of
 * @param soundobjectProcessorId The id of the source that shall be muted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteProtocolSoundobjectProcessorId(ProtocolId protocolId, SoundobjectId soundobjectProcessorId)
{
	std::vector<SoundobjectProcessorId> soundobjectProcessorIds{ soundobjectProcessorId };
	return SetMuteProtocolSoundobjectProcessorIds(protocolId, soundobjectProcessorIds);
}

/**
 * Sets the given sources of given protocol to be muted
 * @param protocolId The id of the protocol the source shall be muted of
 * @param soundobjectProcessorIds The ids of the soundobject processors that shall be muted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteProtocolSoundobjectProcessorIds(ProtocolId protocolId, const std::vector<SoundobjectProcessorId>& soundobjectProcessorIds)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	auto remoteObjects = GetMutedProtocolRemoteObjects(protocolId);
	for (auto const& soundobjectProcessorId : soundobjectProcessorIds)
		for (auto const& object : ctrl->GetSoundobjectProcessorRemoteObjects(soundobjectProcessorId))
			if (std::find(remoteObjects.begin(), remoteObjects.end(), object) == remoteObjects.end())
				remoteObjects.push_back(object);

	return SetProtocolRemoteObjectsStateMuted(protocolId, remoteObjects);
}

/**
 * Sets the given source of given protocol to be unmuted
 * @param protocolId The id of the protocol the source shall be unmuted of
 * @param soundobjectProcessorId The id of the soundobject processor that shall be unmuted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetUnmuteProtocolSoundobjectProcessorId(ProtocolId protocolId, SoundobjectProcessorId soundobjectProcessorId)
{
	std::vector<SoundobjectProcessorId> soundobjectProcessorIds{ soundobjectProcessorId };
	return SetUnmuteProtocolSoundobjectProcessorIds(protocolId, soundobjectProcessorIds);
}

/**
 * Sets the given sources of given protocol to be unmuted
 * @param protocolId The id of the protocol the sources shall be unmuted of
 * @param soundobjectProcessorIds The ids of the soundobject processor that shall be unmuted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetUnmuteProtocolSoundobjectProcessorIds(ProtocolId protocolId, const std::vector<SoundobjectProcessorId>& soundobjectProcessorIds)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	auto remoteObjects = std::vector<RemoteObject>();
	for (auto const& soundobjectProcessorId : soundobjectProcessorIds)
		for (auto const& object : ctrl->GetSoundobjectProcessorRemoteObjects(soundobjectProcessorId))
			remoteObjects.push_back(object);

	return SetProtocolRemoteObjectsStateUnmuted(protocolId, remoteObjects);
}

/**
 * Gets the mute state of the given matrixInput of given protocol
 * @param protocolId The id of the protocol the muted state shall be returned of
 * @param matrixInputProcessorId The id of the matrixInputProcessor that shall be muted
 * @return True if mute is active, false if not
 */
bool ProtocolBridgingWrapper::GetMuteProtocolMatrixInputProcessorId(ProtocolId protocolId, MatrixInputId matrixInputProcessorId)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	return GetProtocolRemoteObjectsMutedState(protocolId, ctrl->GetMatrixInputProcessorRemoteObjects(matrixInputProcessorId));
}

/**
 * Sets the given matrixInput of given protocol to be muted
 * @param protocolId The id of the protocol the matrixInput shall be muted of
 * @param matrixInputProcessorId The id of the matrixInputProcessor that shall be muted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteProtocolMatrixInputProcessorId(ProtocolId protocolId, MatrixInputProcessorId matrixInputProcessorId)
{
	std::vector<MatrixInputProcessorId> matrixInputProcessorIds{ matrixInputProcessorId };
	return SetMuteProtocolMatrixInputProcessorIds(protocolId, matrixInputProcessorIds);
}

/**
 * Sets the given matrixInputProcessors of given protocol to be muted
 * @param protocolId The id of the protocol the matrixInput shall be muted of
 * @param matrixInputProcessorIds The ids of the matrixInputProcessors that shall be muted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteProtocolMatrixInputProcessorIds(ProtocolId protocolId, const std::vector<MatrixInputProcessorId>& matrixInputProcessorIds)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	auto remoteObjects = GetMutedProtocolRemoteObjects(protocolId);
	for (auto const& matrixInputProcessorId : matrixInputProcessorIds)
		for (auto const& object : ctrl->GetMatrixInputProcessorRemoteObjects(matrixInputProcessorId))
			if (std::find(remoteObjects.begin(), remoteObjects.end(), object) == remoteObjects.end())
				remoteObjects.push_back(object);

	return SetProtocolRemoteObjectsStateMuted(protocolId, remoteObjects);
}

/**
 * Sets the given matrixInput of given protocol to be unmuted
 * @param protocolId The id of the protocol the matrixInput shall be unmuted of
 * @param matrixInputProcessorId The id of the matrixInputProcessor that shall be unmuted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetUnmuteProtocolMatrixInputProcessorId(ProtocolId protocolId, MatrixInputId matrixInputProcessorId)
{
	std::vector<MatrixInputProcessorId> matrixInputProcessorIds{ matrixInputProcessorId };
	return SetUnmuteProtocolSoundobjectProcessorIds(protocolId, matrixInputProcessorIds);
}

/**
 * Sets the given matrixInputs of given protocol to be unmuted
 * @param protocolId The id of the protocol the matrixInputs shall be unmuted of
 * @param matrixInputIdProcessors The ids of the matrixInputProcessors that shall be unmuted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetUnmuteProtocolMatrixInputProcessorIds(ProtocolId protocolId, const std::vector<MatrixInputProcessorId>& matrixInputProcessorIds)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	auto remoteObjects = std::vector<RemoteObject>();
	for (auto const& matrixInputProcessorId : matrixInputProcessorIds)
		for (auto const& object : ctrl->GetMatrixInputProcessorRemoteObjects(matrixInputProcessorId))
			remoteObjects.push_back(object);

	return SetProtocolRemoteObjectsStateUnmuted(protocolId, remoteObjects);
}

/**
 * Gets the mute state of the given matrixOutput of given protocol
 * @param protocolId The id of the protocol the muted state shall be returned of
 * @param matrixOutputProcessorId The id of the matrixOutputProcessor that shall be muted
 * @return True if mute is active, false if not
 */
bool ProtocolBridgingWrapper::GetMuteProtocolMatrixOutputProcessorId(ProtocolId protocolId, MatrixOutputId matrixOutputProcessorId)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	return GetProtocolRemoteObjectsMutedState(protocolId, ctrl->GetMatrixOutputProcessorRemoteObjects(matrixOutputProcessorId));
}

/**
 * Sets the given matrixOutput of given protocol to be muted
 * @param protocolId The id of the protocol the matrixOutput shall be muted of
 * @param matrixOutputProcessorId The id of the matrixOutputProcessor that shall be muted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteProtocolMatrixOutputProcessorId(ProtocolId protocolId, MatrixOutputId matrixOutputProcessorId)
{
	std::vector<MatrixOutputId> matrixOutputProcessorIds{ matrixOutputProcessorId };
	return SetMuteProtocolMatrixOutputProcessorIds(protocolId, matrixOutputProcessorIds);
}

/**
 * Sets the given matrixOutputs of given protocol to be muted
 * @param protocolId The id of the protocol the matrixOutput shall be muted of
 * @param matrixOutputProcessorIds The ids of the matrixOutputProcessors that shall be muted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetMuteProtocolMatrixOutputProcessorIds(ProtocolId protocolId, const std::vector<MatrixOutputProcessorId>& matrixOutputProcessorIds)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	auto remoteObjects = GetMutedProtocolRemoteObjects(protocolId);
	for (auto const& matrixOutputProcessorId : matrixOutputProcessorIds)
		for (auto const& object : ctrl->GetMatrixOutputProcessorRemoteObjects(matrixOutputProcessorId))
			if (std::find(remoteObjects.begin(), remoteObjects.end(), object) == remoteObjects.end())
				remoteObjects.push_back(object);

	return SetProtocolRemoteObjectsStateMuted(protocolId, remoteObjects);
}

/**
 * Sets the given matrixOutput of given protocol to be unmuted
 * @param protocolId The id of the protocol the matrixOutput shall be unmuted of
 * @param matrixOutputProcessorId The id of the matrixOutput that shall be unmuted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetUnmuteProtocolMatrixOutputProcessorId(ProtocolId protocolId, MatrixOutputId matrixOutputProcessorId)
{
	std::vector<MatrixOutputProcessorId> matrixOutputProcessorIds{ matrixOutputProcessorId };
	return SetUnmuteProtocolMatrixOutputProcessorIds(protocolId, matrixOutputProcessorIds);
}

/**
 * Sets the given matrixOutputs of given protocol to be unmuted
 * @param protocolId The id of the protocol the matrixOutputs shall be unmuted of
 * @param matrixOutputProcessorIds The ids of the matrixOutputProcessors that shall be unmuted
 * @return True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetUnmuteProtocolMatrixOutputProcessorIds(ProtocolId protocolId, const std::vector<MatrixOutputProcessorId>& matrixOutputProcessorIds)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	auto remoteObjects = std::vector<RemoteObject>();
	for (auto const& matrixOutputProcessorId : matrixOutputProcessorIds)
		for (auto const& object : ctrl->GetMatrixOutputProcessorRemoteObjects(matrixOutputProcessorId))
			remoteObjects.push_back(object);

	return SetProtocolRemoteObjectsStateUnmuted(protocolId, remoteObjects);
}

/**
 * Internal helper method to get an accumulated mute state of a list of given remote objects.
 * The mute states are read from cached bridging node configuration xml element.
 * @param protocolId	The id of the protocol to get the mute states for.
 * @param objects		The list of objects of the protocol to get the mute states for.
 * @return				True if all given objects of the given protocol are muted, false if not.
 */
bool ProtocolBridgingWrapper::GetProtocolRemoteObjectsMutedState(ProtocolId protocolId, const std::vector<RemoteObject>& objects)
{
	auto mutedObjects = GetMutedProtocolRemoteObjects(protocolId);

	if (mutedObjects.empty())
		return false;

	auto muted = true;
	for (auto const& object : objects)
		muted = muted && std::find(mutedObjects.begin(), mutedObjects.end(), object) != mutedObjects.end();

	return muted;
}

/**
 * Internal helper method to set a list of given remote objects to mute state 'true'.
 * The mute state 'true' is set to cached bridging node configuration xml element.
 * @param protocolId	The id of the protocol to set the mute states for.
 * @param objects		The list of objects of the protocol to set the mute states for.
 * @return				True setting mute state succeeded, false if not.
 */
bool ProtocolBridgingWrapper::SetProtocolRemoteObjectsStateMuted(ProtocolId protocolId, const std::vector<RemoteObject>& objects)
{
	return UpdateMutedProtocolRemoteObjects(protocolId, objects, false);
}

/**
 * Helper method to set a list of given remote objects to mute state 'false'.
 * The mute state 'false' is set to cached bridging node configuration xml element.
 * @param protocolId	The id of the protocol to set the mute states for.
 * @param objects		The list of objects of the protocol to set the mute states for.
 * @return				True setting mute state succeeded, false if not.
 */
bool ProtocolBridgingWrapper::SetProtocolRemoteObjectsStateUnmuted(ProtocolId protocolId, const std::vector<RemoteObject>& objects)
{
	return UpdateMutedProtocolRemoteObjects(protocolId, objects, true);
}

/**
 * Private helper method to modify a list of given remote objects to given mute state.
 * @param	protocolId		The id of the protocol to set the mute states for.
 * @param	objects			The list of objects of the protocol to set the mute states for.
 * @param	unmuteObjects	True if the given objects should be unmuted, false if they shall be muted.
 * @return				True setting mute state succeeded, false if not.
 */
bool ProtocolBridgingWrapper::UpdateMutedProtocolRemoteObjects(ProtocolId protocolId, const std::vector<RemoteObject>& objects, bool unmuteObjects)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto mutedObjsXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDOBJECTS));
			auto mutedObjects = std::vector<RemoteObject>();
			ProcessingEngineConfig::ReadMutedObjects(mutedObjsXmlElement, mutedObjects);
			auto oldMutedObjects = mutedObjects;

			if (unmuteObjects)
			{
				// if list objects shall be unmuted, iterate through new list and remove them if they are present
				for (auto const& object : objects)
				{
					auto objIter = std::find(mutedObjects.begin(), mutedObjects.end(), object);
					if (objIter != mutedObjects.end())
						mutedObjects.erase(objIter);
				}
			}
			else if (!unmuteObjects)
			{
				// iterate through old list of muted objects and remove all that are not contained in the list of new objects
				auto remainingMutedObjects = std::vector<RemoteObject>();
				for (auto const& mutedObject : mutedObjects)
				{
					auto objIter = std::find(objects.begin(), objects.end(), mutedObject);
					if (objIter != objects.end())
						remainingMutedObjects.push_back(mutedObject);
				}
				mutedObjects = remainingMutedObjects;
				// iterate through new list of objects and add them if not yet present
				for (auto const& object : objects)
				{
					auto objIter = std::find(mutedObjects.begin(), mutedObjects.end(), object);
					if (objIter == mutedObjects.end())
						mutedObjects.push_back(object);
				}
			}

			// if there the list was actually modified, apply the modifications
			if (oldMutedObjects != mutedObjects)
			{
				ProcessingEngineConfig::WriteMutedObjects(mutedObjsXmlElement, mutedObjects);
				m_bridgingProtocolMutedObjects[protocolId] = mutedObjects;

				SetBridgingNodeStateXml(nodeXmlElement, true);

				Controller* ctrl = Controller::GetInstance();
				if (ctrl)
					ctrl->SetParameterChanged(DCP_Host, DCT_MuteState);

				return true;
			}
		}
	}

	return false;
}

/**
 * Private helper method to get the list of remote objects that are currently muted for the given bridging protocol.
 * @param	protocolId		The id of the protocol to get the mute states for.
 * @return	The requested list of muted objects. Can be empty.
 */
const std::vector<RemoteObject> ProtocolBridgingWrapper::GetMutedProtocolRemoteObjects(ProtocolId protocolId)
{
	auto mutedObjects = std::vector<RemoteObject>();

	if (0 != m_bridgingProtocolMutedObjects.count(protocolId))
	{
		mutedObjects = m_bridgingProtocolMutedObjects.at(protocolId);
	}
	else
	{
		// if our cache turned out to be empty, fill in what can be read from xml config
		auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
		if (nodeXmlElement)
		{
			auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
			if (protocolXmlElement)
			{
				auto mutedObjsXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MUTEDOBJECTS));
				ProcessingEngineConfig::ReadMutedObjects(mutedObjsXmlElement, mutedObjects);
				m_bridgingProtocolMutedObjects[protocolId] = mutedObjects;
			}
		}
	}

	return mutedObjects;
}

/**
 * Gets the protocol's currently set client ip address.
 * @param protocolId The id of the protocol for which to get the currently configured ip address
 * @return	The ip address string
 */
juce::IPAddress ProtocolBridgingWrapper::GetProtocolIpAddress(ProtocolId protocolId)
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
				return juce::IPAddress(ipAddressXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS)));
			}
		}
	}

	return juce::IPAddress(INVALID_IPADDRESS_VALUE);
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
bool ProtocolBridgingWrapper::SetProtocolIpAddress(ProtocolId protocolId, juce::IPAddress ipAddress, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto ipAddressXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
			if (!ipAddressXmlElement)
				ipAddressXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::IPADDRESS));
			if (ipAddressXmlElement)
				ipAddressXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ADRESS), ipAddress.toString());
			else
				return false;
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
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
			if (!listeningPortXmlElement)
				listeningPortXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
			if (listeningPortXmlElement)
				listeningPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), listeningPort);
			else
				return false;
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
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
			if (!remotePortXmlElement)
				remotePortXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
			if (remotePortXmlElement)
				remotePortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), remotePort);
			else
				return false;
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
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

	return MappingAreaId::MAI_Invalid;
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

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the protocol's currently set offset from origin value, if available for the given protocol.
 * @param protocolId The id of the protocol for which to get the currently configured offset from origin value
 * @return	The offset from origin value, 0/0 if not available
 */
const juce::Point<float> ProtocolBridgingWrapper::GetProtocolOriginOffset(ProtocolId protocolId)
{
	auto absoluteOrigin = juce::Point<float>(0.0f, 0.0f);

	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto originOffsetXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ORIGINOFFSET));
			if (originOffsetXmlElement)
			{
				auto originOffsetTextElement = originOffsetXmlElement->getFirstChildElement();
				if (originOffsetTextElement && originOffsetTextElement->isTextElement())
				{
					auto rangeRescaleValues = StringArray();
					if (2 == rangeRescaleValues.addTokens(originOffsetTextElement->getText(), ";", ""))
					{
						auto xOrigOffset = rangeRescaleValues[0].getFloatValue();
						auto yOrigOffset = rangeRescaleValues[1].getFloatValue();

						absoluteOrigin = juce::Point<float>(xOrigOffset, yOrigOffset);
					}
				}
			}
		}
	}

	return absoluteOrigin;
}

/**
 * Sets the given protocol offset from origin value.
 * This method inserts the mapping range into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId			The id of the protocol for which to set the offset from origin
 * @param originOffset			The new offset from origin value
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolOriginOffset(ProtocolId protocolId, const juce::Point<float>& originOffset, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto originOffsetString = juce::String(originOffset.getX()) + ";" + juce::String(originOffset.getY());
			auto originOffsetXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ORIGINOFFSET));
			if (originOffsetXmlElement)
			{
				auto originOffsetTextElement = originOffsetXmlElement->getFirstChildElement();
				if (originOffsetTextElement && originOffsetTextElement->isTextElement())
					originOffsetTextElement->setText(originOffsetString);
				else
					originOffsetXmlElement->addTextElement(originOffsetString);
			}
			else
			{
				originOffsetXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ORIGINOFFSET));
				originOffsetXmlElement->addTextElement(originOffsetString);
			}
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the protocol's currently set mapping range x/y min/max value, if available for the given protocol.
 * @param protocolId The id of the protocol for which to get the currently configured mapping range x/y min/max value
 * @return	The mapping range x/y min/max value, -3..3 min/max range if not available
 */
const std::pair<juce::Range<float>, juce::Range<float>> ProtocolBridgingWrapper::GetProtocolMappingRange(ProtocolId protocolId)
{
	auto mappingAreaRescaleRangeX = juce::Range<float>(-3.0f, 3.0f);
	auto mappingAreaRescaleRangeY = juce::Range<float>(-3.0f, 3.0f);

	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto mappingAreaRescaleXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREARESCALE));
			if (mappingAreaRescaleXmlElement)
			{
				auto mappingAreaRescaleTextElement = mappingAreaRescaleXmlElement->getFirstChildElement();
				if (mappingAreaRescaleTextElement && mappingAreaRescaleTextElement->isTextElement())
				{
					auto rangeRescaleValues = StringArray();
					if (4 != rangeRescaleValues.addTokens(mappingAreaRescaleTextElement->getText(), ";", ""))
					{
						mappingAreaRescaleRangeX = juce::Range<float>(-3.0f, 3.0f);
						mappingAreaRescaleRangeY = juce::Range<float>(-3.0f, 3.0f);
					}
					else
					{
						auto minX = rangeRescaleValues[0].getFloatValue();
						auto maxX = rangeRescaleValues[1].getFloatValue();
						auto minY = rangeRescaleValues[2].getFloatValue();
						auto maxY = rangeRescaleValues[3].getFloatValue();

						mappingAreaRescaleRangeX = juce::Range<float>(minX, maxX);
						mappingAreaRescaleRangeY = juce::Range<float>(minY, maxY);
					}
				}
			}
		}
	}

	return std::make_pair(mappingAreaRescaleRangeX, mappingAreaRescaleRangeY);
}

/**
 * Sets the given protocol mapping range x/y min/max values.
 * This method inserts the mapping range into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId			The id of the protocol for which to set the mapping range
 * @param mappingRange			The new mapping range value, consisting of two ranges. first for x, second for y.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolMappingRange(ProtocolId protocolId, const std::pair<juce::Range<float>, juce::Range<float>>& mappingRange, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto mappingRangeString = juce::String(mappingRange.first.getStart()) + ";" + juce::String(mappingRange.first.getEnd()) + ";" + juce::String(mappingRange.second.getStart()) + ";" + juce::String(mappingRange.second.getEnd());
			auto mappingAreaRescaleXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREARESCALE));
			if (mappingAreaRescaleXmlElement)
			{
				auto mappingAreaRescaleTextElement = mappingAreaRescaleXmlElement->getFirstChildElement();
				if (mappingAreaRescaleTextElement && mappingAreaRescaleTextElement->isTextElement())
					mappingAreaRescaleTextElement->setText(mappingRangeString);
				else
					mappingAreaRescaleXmlElement->addTextElement(mappingRangeString);
			}
			else
			{
				mappingAreaRescaleXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::MAPPINGAREARESCALE));
				mappingAreaRescaleXmlElement->addTextElement(mappingRangeString);
			}
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the protocol's currently set module type string, if available for the given protocol.
 * @param protocolId The id of the protocol for which to get the currently configured module type string
 * @return	The module type string
 */
const String ProtocolBridgingWrapper::GetProtocolModuleTypeIdentifier(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto moduleTypeForPositioningXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PACKETMODULE));
			if (moduleTypeForPositioningXmlElement)
			{
				return moduleTypeForPositioningXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE));
			}
		}
	}
	
	return String();
}

/**
 * Sets the given protocol module type identifier string.
 * This method inserts the module type identifier string into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId The id of the protocol for which to set the ip address
 * @param moduleTypeIdentifier	The new module type identifier string.
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolModuleTypeIdentifier(ProtocolId protocolId, const String& moduleTypeIdentifier, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto moduleTypeForPositioningXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PACKETMODULE));
			if (!moduleTypeForPositioningXmlElement)
				moduleTypeForPositioningXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PACKETMODULE));
			if (moduleTypeForPositioningXmlElement)
				moduleTypeForPositioningXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), moduleTypeIdentifier);
			else
				return false;
		}
		else
			return false;
	
		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the protocol's currently set input device identifier, if available, for the given protocol.
 * @param protocolId The id of the protocol for which to get the currently configured inputdevice identifier
 * @return	The input device dentifier
 */
String ProtocolBridgingWrapper::GetProtocolInputDeviceIdentifier(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto inputDeviceIdentifierXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::INPUTDEVICE));
			if (inputDeviceIdentifierXmlElement)
			{
				return inputDeviceIdentifierXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DEVICEIDENTIFIER));
			}
		}
	}

	return String();
}

/**
 * Sets the given protocol device string identifier.
 * This method inserts the mapping area id into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId The id of the protocol for which to set the identifier
 * @param inputDeviceIdentifier	The new device identifier to set as input device
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolInputDeviceIdentifier(ProtocolId protocolId, const String& inputDeviceIdentifier, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto inputDeviceIdentifierXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::INPUTDEVICE));
			if (!inputDeviceIdentifierXmlElement)
				inputDeviceIdentifierXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::INPUTDEVICE));
			if (inputDeviceIdentifierXmlElement)
				inputDeviceIdentifierXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DEVICEIDENTIFIER), inputDeviceIdentifier);
			else
				return false;
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the protocol's currently set output device identifier, if available, for the given protocol.
 * @param protocolId The id of the protocol for which to get the currently configured outputdevice identifier
 * @return	The output device identifier
 */
String ProtocolBridgingWrapper::GetProtocolOutputDeviceIdentifier(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto outputDeviceIdentifierXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OUTPUTDEVICE));
			if (outputDeviceIdentifierXmlElement)
			{
				return outputDeviceIdentifierXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DEVICEIDENTIFIER));
			}
		}
	}

	return String();
}

/**
 * Sets the given protocol device string identifier.
 * This method inserts the mapping area id into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId The id of the protocol for which to set the identifier
 * @param outputDeviceIdentifier	The new device identifier to set as output device
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolOutputDeviceIdentifier(ProtocolId protocolId, const String& outputDeviceIdentifier, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto outputDeviceIdentifierXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OUTPUTDEVICE));
			if (!outputDeviceIdentifierXmlElement)
				outputDeviceIdentifierXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OUTPUTDEVICE));
			if (outputDeviceIdentifierXmlElement)
				outputDeviceIdentifierXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::DEVICEIDENTIFIER), outputDeviceIdentifier);
			else
				return false;
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the currently set midi assignment mapping for a given remote object, if available, for the given protocol.
 * @param protocolId	The id of the protocol to get the midi assignment for.
 * @param roi	The remote object to get the midi mapping for.
 * @return	The requested midi assignment mapping
 */
JUCEAppBasics::MidiCommandRangeAssignment ProtocolBridgingWrapper::GetMidiAssignmentMapping(ProtocolId protocolId, const RemoteObjectIdentifier roi)
{
    auto midiAssiMap = JUCEAppBasics::MidiCommandRangeAssignment();
    
    auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
    if (nodeXmlElement)
    {
        auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
        if (protocolXmlElement)
        {
            auto assiMapXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::GetObjectDescription(roi).removeCharacters(" "));
            if (assiMapXmlElement)
            {
                auto assiMapHexStringTextXmlElement = assiMapXmlElement->getFirstChildElement();
                if (assiMapHexStringTextXmlElement && assiMapHexStringTextXmlElement->isTextElement())
                {
					auto midiAssiStr = assiMapHexStringTextXmlElement->getText();
					if (midiAssiStr.isNotEmpty())
						midiAssiMap.deserializeFromHexString(midiAssiStr);
                }
            }
        }
    }

	return midiAssiMap;
}

/**
 * Sets the desired midi assignment mapping for a given remote object.
 * This method inserts the mapping area id into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId	The id of the protocol to set the midi assignment for.
 * @param roi	The remote object to set the midi mapping for.
 * @param assignmentMapping	The midi mapping to set for the remote object.
 * @param dontSendNotification	Flag if change notification shall be broadcasted.
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetMidiAssignmentMapping(ProtocolId protocolId, const RemoteObjectIdentifier roi, const JUCEAppBasics::MidiCommandRangeAssignment& assignmentMapping, bool dontSendNotification)
{
    auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
    if (nodeXmlElement)
    {
        auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
        if (protocolXmlElement)
        {
            auto assiMapHexString = assignmentMapping.serializeToHexString();
            
            auto assiMapXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::GetObjectDescription(roi).removeCharacters(" "));
            if (assiMapXmlElement)
            {
                auto assiMapHexStringTextXmlElement = assiMapXmlElement->getFirstChildElement();
                if (assiMapHexStringTextXmlElement && assiMapHexStringTextXmlElement->isTextElement())
                    assiMapHexStringTextXmlElement->setText(assiMapHexString);
                else
                    assiMapXmlElement->addTextElement(assiMapHexString);
            }
            else
            {
                assiMapXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::GetObjectDescription(roi).removeCharacters(" "));
                assiMapXmlElement->addTextElement(assiMapHexString);
            }
        }
        else
            return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
    }
    else
        return false;
}

/**
 * Gets the currently set scenes to midi assignment mapping for a given remote object, if available, for the given protocol.
 * @param protocolId	The id of the protocol to get the scenes to midi assignment for.
 * @param roi	The remote object to get the scenes to midi mapping for.
 * @return	The requested scenes to midi assignment mapping
 */
std::map<String, JUCEAppBasics::MidiCommandRangeAssignment> ProtocolBridgingWrapper::GetMidiScenesAssignmentMapping(ProtocolId protocolId, const RemoteObjectIdentifier roi)
{
	auto scenesToMidiAssiMap = std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>();

	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto assiMapXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::GetObjectDescription(roi).removeCharacters(" "));
			if (assiMapXmlElement)
			{
				if (assiMapXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MULTIVALUE)) == 1)
				{
					for (auto assiMapSubXmlElement : assiMapXmlElement->getChildIterator())
					{
						auto scnIdx = assiMapSubXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::VALUE));
						auto assiMapSubHexStringTextXmlElement = assiMapSubXmlElement->getFirstChildElement();
						if (assiMapSubHexStringTextXmlElement && assiMapSubHexStringTextXmlElement->isTextElement())
						{
							auto midiAssi = JUCEAppBasics::MidiCommandRangeAssignment();

							auto midiAssiStr = assiMapSubHexStringTextXmlElement->getText();
							if (midiAssiStr.isNotEmpty() && midiAssi.deserializeFromHexString(midiAssiStr))
								scenesToMidiAssiMap.insert(std::make_pair(scnIdx, midiAssi));
						}
					}
				}
			}
		}
	}

	return scenesToMidiAssiMap;
}

/**
 * Sets the desired scenes to midi assignment mapping for a given remote object.
 * @param protocolId	The id of the protocol to set the scenes to midi assignment for.
 * @param roi	The remote object to set the scenes to midi mapping for.
 * @param assignmentMapping	The scenes to midi mapping to set for the remote object.
 * @param dontSendNotification	Flag if change notification shall be broadcasted.
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetMidiScenesAssignmentMapping(ProtocolId protocolId, const RemoteObjectIdentifier roi, const std::map<juce::String, JUCEAppBasics::MidiCommandRangeAssignment>& assignmentMapping, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), juce::String(protocolId));
		if (protocolXmlElement)
		{
			auto assiMapXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::GetObjectDescription(roi).removeCharacters(" "));
			if (assiMapXmlElement)
			{
				// collect the xml elements that are no longer used according to new incoming mappings
				std::vector<XmlElement*> noLongerUsedElements;
				auto assiXmlElement = assiMapXmlElement->getFirstChildElement();
				while (nullptr != assiXmlElement)
				{
					if (assignmentMapping.count(assiXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::VALUE))) < 1)
						noLongerUsedElements.push_back(assiXmlElement);
					assiXmlElement = assiXmlElement->getNextElement();
				}
				// and remove them
				for (auto const& childToRemove : noLongerUsedElements)
					assiMapXmlElement->removeChildElement(childToRemove, true);

				// create or update the xml elements according to new incoming mappings
				assiMapXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MULTIVALUE), 1);
				for (auto const& assi : assignmentMapping)
				{
					auto assiMapHexString = assi.second.serializeToHexString();

					auto assiMapSubXmlElement = assiMapXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::VALUE), assi.first);
					if (assiMapSubXmlElement)
					{
						auto assiMapSubHexStringTextXmlElement = assiMapSubXmlElement->getFirstChildElement();
						if (assiMapSubHexStringTextXmlElement && assiMapSubHexStringTextXmlElement->isTextElement())
							assiMapSubHexStringTextXmlElement->setText(assiMapHexString);
						else
							assiMapSubXmlElement->addTextElement(assiMapHexString);
					}
					else
					{
						assiMapSubXmlElement = assiMapXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALTOCMDASSI));
						assiMapSubXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::VALUE), assi.first);
						assiMapSubXmlElement->addTextElement(assiMapHexString);
					}
				}
			}
			else
			{
				assiMapXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::GetObjectDescription(roi).removeCharacters(" "));
				assiMapXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MULTIVALUE), 1);
				for (auto const& assi : assignmentMapping)
				{
					auto assiMapSubXmlElement = assiMapXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALTOCMDASSI));
					assiMapSubXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::VALUE), assi.first);
					auto assiMapHexString = assi.second.serializeToHexString();
					assiMapSubXmlElement->addTextElement(assiMapHexString);
				}
			}
		}
		else
			return false;
	
		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the currently set x-Axis inverted flag for the given protocol.
 * @param protocolId	The id of the protocol to get the flag.
 * @return	The requested flag value.
 */
int ProtocolBridgingWrapper::GetProtocolXAxisInverted(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto xAxisInvertedXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::XINVERTED));
			if (xAxisInvertedXmlElement)
			{
				return xAxisInvertedXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE));
			}
		}
	}

	return 0;
}

/**
 * Sets the desired x-Axis inverted flag for the given protocol.
 * This method inserts the value into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId	The id of the protocol to set the flag value for.
 * @param inverted		The x-Axis inverted flag value.
 * @param dontSendNotification	Flag if change notification shall be broadcasted.
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolXAxisInverted(ProtocolId protocolId, int inverted, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto xAxisInvertedXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::XINVERTED));
			if (xAxisInvertedXmlElement)
			{
				xAxisInvertedXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), inverted);
			}
			else
				return false;
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the currently set y-Axis inverted flag for the given protocol.
 * @param protocolId	The id of the protocol to get the flag.
 * @return	The requested flag value.
 */
int ProtocolBridgingWrapper::GetProtocolYAxisInverted(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto yAxisInvertedXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::YINVERTED));
			if (yAxisInvertedXmlElement)
			{
				return yAxisInvertedXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE));
			}
		}
	}

	return 0;
}

/**
 * Sets the desired x-Axis inverted flag for the given protocol.
 * This method inserts the value into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId	The id of the protocol to set the flag value for.
 * @param inverted		The ^^-Axis inverted flag value.
 * @param dontSendNotification	Flag if change notification shall be broadcasted.
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolYAxisInverted(ProtocolId protocolId, int inverted, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto yAxisInvertedXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::YINVERTED));
			if (yAxisInvertedXmlElement)
			{
				yAxisInvertedXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), inverted);
			}
			else
				return false;
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the currently set xy-Axis-Swapped inverted flag for the given protocol.
 * @param protocolId	The id of the protocol to get the flag.
 * @return	The requested flag value.
 */
int ProtocolBridgingWrapper::GetProtocolXYAxisSwapped(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto xySwappedXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::XYSWAPPED));
			if (xySwappedXmlElement)
			{
				return xySwappedXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE));
			}
		}
	}

	return 0;
}

/**
 * Sets the desired xy-Axis-swapped flag for the given protocol.
 * This method inserts the value into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId	The id of the protocol to set the flag value for.
 * @param inverted		The xy-Axis-swapped flag value.
 * @param dontSendNotification	Flag if change notification shall be broadcasted.
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolXYAxisSwapped(ProtocolId protocolId, int swapped, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto xySwappedXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::XYSWAPPED));
			if (xySwappedXmlElement)
			{
				xySwappedXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), swapped);
			}
			else
				return false;
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the currently set data sending disabled flag for the given protocol.
 * @param protocolId	The id of the protocol to get the flag.
 * @return	The requested flag value.
 */
int ProtocolBridgingWrapper::GetProtocolDataSendingDisabled(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto dataSendingDisabledXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATASENDINGDISABLED));
			if (dataSendingDisabledXmlElement)
			{
				return dataSendingDisabledXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE));
			}
		}
	}

	return 0;
}

/**
 * Sets the desired data sending disabled flag for the given protocol.
 * This method inserts the value into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId	The id of the protocol to set the flag value for.
 * @param inverted		The data sending disabled flag value.
 * @param dontSendNotification	Flag if change notification shall be broadcasted.
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetProtocolDataSendingDisabled(ProtocolId protocolId, int disabled, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto dataSendingDisabledXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATASENDINGDISABLED));
			if (dataSendingDisabledXmlElement)
			{
				dataSendingDisabledXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), disabled);
			}
			else
				return false;
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the currently set xy combine flag for the given protocol.
 * @param protocolId	The id of the protocol to get the flag.
 * @return	The requested flag value.
 */
bool ProtocolBridgingWrapper::GetProtocolBridgingXYMessageCombined(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto xyMessageCombinedXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::XYMESSAGECOMBINED));
			if (xyMessageCombinedXmlElement)
			{
				return xyMessageCombinedXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE));
			}
		}
	}

	return 0;
}

/**
 * Sets the desired xy combine flag for the given protocol.
 * This method inserts the value into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId	The id of the protocol to set the flag value for.
 * @param combined		The xy combined flag value.
 * @param dontSendNotification	Flag if change notification shall be broadcasted.
 * @return	True on succes, false on failure
 */
bool ProtocolBridgingWrapper::SetProtocolBridgingXYMessageCombined(ProtocolId protocolId, bool combined, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto xyMessageCombinedXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::XYMESSAGECOMBINED));
			if (xyMessageCombinedXmlElement)
			{
				xyMessageCombinedXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::STATE), combined);
			}
			else
				return false;
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the currently set osc remappings for the given protocol.
 * @param protocolId	The id of the protocol to get the assignments for.
 * @return	The requested osc remapping assignments.
 */
std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>> ProtocolBridgingWrapper::GetProtocolOscRemapAssignments(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto oscRemappingsXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REMAPPINGS));
			if (oscRemappingsXmlElement)
			{
				std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>> oscRemappings;
				auto oscRemappingXmlElement = oscRemappingsXmlElement->getFirstChildElement();
				while (nullptr != oscRemappingXmlElement)
				{
					for (int i = ROI_Invalid + 1; i < ROI_BridgingMAX; ++i)
					{
						auto ROId = static_cast<RemoteObjectIdentifier>(i);
						if (oscRemappingXmlElement->getTagName() == ProcessingEngineConfig::GetObjectDescription(ROId).removeCharacters(" "))
						{
							auto oscRemappingTextElement = oscRemappingXmlElement->getFirstChildElement();
							if (oscRemappingTextElement && oscRemappingTextElement->isTextElement())
							{
								auto remapPattern = oscRemappingTextElement->getText();
								auto minVal = static_cast<float>(oscRemappingXmlElement->getDoubleAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MINVALUE)));
								auto maxVal = static_cast<float>(oscRemappingXmlElement->getDoubleAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MAXVALUE)));
								oscRemappings.insert(std::make_pair(ROId, std::make_pair(remapPattern, juce::Range<float>(minVal, maxVal))));
							}
						}
					}

					oscRemappingXmlElement = oscRemappingXmlElement->getNextElement();
				}

				return oscRemappings;
			}
		}
	}

	return std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>();
}

/**
 * Sets the given osc remappings for the given protocol.
 * This method inserts the value into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId			The id of the protocol to set the flag value for.
 * @param oscRemapAssignments	The new osc remapping assignments value.
 * @param dontSendNotification	Flag if change notification shall be broadcasted.
 * @return	True on succes, false on failure
 */
bool ProtocolBridgingWrapper::SetProtocolOscRemapAssignments(ProtocolId protocolId, const std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>& oscRemapAssignments, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto oscRemappingsXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REMAPPINGS));
			if (!oscRemappingsXmlElement)
				oscRemappingsXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REMAPPINGS));
			if (oscRemappingsXmlElement)
			{
				// collect the xml elements that are no longer used according to new incoming assignments
				std::vector<XmlElement*> noLongerUsedElements;
				auto remappingXmlElement = oscRemappingsXmlElement->getFirstChildElement();
				while (nullptr != remappingXmlElement)
				{
					bool stillInUse = false;
					for (auto const& assi : oscRemapAssignments)
						if (ProcessingEngineConfig::GetObjectDescription(assi.first).removeCharacters(" ") == remappingXmlElement->getTagName())
							stillInUse = true;
					if (!stillInUse)
						noLongerUsedElements.push_back(remappingXmlElement);

					remappingXmlElement = remappingXmlElement->getNextElement();
				}
				// and remove them
				for (auto const& childToRemove : noLongerUsedElements)
					oscRemappingsXmlElement->removeChildElement(childToRemove, true);

				// create or update the xml elements according to new incoming assignments
				for (auto const& assi : oscRemapAssignments)
				{
					auto oscRemappingXmlElement = oscRemappingsXmlElement->getChildByName(ProcessingEngineConfig::GetObjectDescription(assi.first).removeCharacters(" "));
					if (oscRemappingXmlElement)
					{
						auto oscRemappingTextXmlElement = oscRemappingXmlElement->getFirstChildElement();
						if (oscRemappingTextXmlElement && oscRemappingTextXmlElement->isTextElement())
							oscRemappingTextXmlElement->setText(assi.second.first);
						else
							oscRemappingTextXmlElement->addTextElement(assi.second.first);
						oscRemappingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MINVALUE), static_cast<double>(assi.second.second.getStart()));
						oscRemappingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MAXVALUE), static_cast<double>(assi.second.second.getEnd()));
					}
					else
					{
						oscRemappingXmlElement = oscRemappingsXmlElement->createNewChildElement(ProcessingEngineConfig::GetObjectDescription(assi.first).removeCharacters(" "));
						oscRemappingXmlElement->addTextElement(assi.second.first);
						oscRemappingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MINVALUE), static_cast<double>(assi.second.second.getStart()));
						oscRemappingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MAXVALUE), static_cast<double>(assi.second.second.getEnd()));
					}
				}
			}
			else
				return false;
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the currently set index to channel remappings for the given protocol.
 * @param protocolId	The id of the protocol to get the assignments for.
 * @return	The requested index to channel remapping assignments.
 */
std::map<int, ChannelId> ProtocolBridgingWrapper::GetProtocolChannelRemapAssignments(ProtocolId protocolId)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto channelRemappingsXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REMAPPINGS));
			if (channelRemappingsXmlElement)
			{
				std::map<int, ChannelId> channelRemappings;
				auto channelRemappingXmlElement = channelRemappingsXmlElement->getFirstChildElement();
				while (nullptr != channelRemappingXmlElement)
				{
					if (channelRemappingXmlElement->getTagName() == ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REMAPPINGS))
					{
						auto channelRemappingTextElement = channelRemappingXmlElement->getFirstChildElement();
						if (channelRemappingTextElement && channelRemappingTextElement->isTextElement())
						{
							auto index = channelRemappingXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), -1);
							auto channel = ChannelId(channelRemappingTextElement->getText().getIntValue());
							channelRemappings.insert(std::make_pair(index, channel));
						}
					}

					channelRemappingXmlElement = channelRemappingXmlElement->getNextElement();
				}

				return channelRemappings;
			}
		}
	}

	return std::map<int, ChannelId>();
}

/**
 * Sets the given index to channel remappings for the given protocol.
 * This method inserts the value into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param protocolId				The id of the protocol to set the flag value for.
 * @param channelRemapAssignments	The new index to channel remapping assignments value.
 * @param dontSendNotification		Flag if change notification shall be broadcasted.
 * @return	True on succes, false on failure
 */
bool ProtocolBridgingWrapper::SetProtocolChannelRemapAssignments(ProtocolId protocolId, const std::map<int, ChannelId>& channelRemapAssignments, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(protocolId));
		if (protocolXmlElement)
		{
			auto channelRemappingsXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REMAPPINGS));
			if (!channelRemappingsXmlElement)
				channelRemappingsXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REMAPPINGS));
			if (channelRemappingsXmlElement)
			{
				// collect the xml elements that are no longer used according to new incoming assignments
				std::vector<XmlElement*> noLongerUsedElements;
				auto remappingXmlElement = channelRemappingsXmlElement->getFirstChildElement();
				while (nullptr != remappingXmlElement)
				{
					auto index = remappingXmlElement->getIntAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), -1);

					bool stillInUse = false;
					for (auto const& assi : channelRemapAssignments)
					{
						if (assi.first == index)
						{
							stillInUse = true;
							break;
						}
					}
					if (!stillInUse)
						noLongerUsedElements.push_back(remappingXmlElement);

					remappingXmlElement = remappingXmlElement->getNextElement();
				}
				// and remove them
				for (auto const& childToRemove : noLongerUsedElements)
					channelRemappingsXmlElement->removeChildElement(childToRemove, true);

				// create or update the xml elements according to new incoming assignments
				for (auto const& assi : channelRemapAssignments)
				{
					auto channelRemappingXmlElement = channelRemappingsXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), juce::String(assi.first));
					if (channelRemappingXmlElement)
					{
						auto channelRemappingTextXmlElement = channelRemappingXmlElement->getFirstChildElement();
						if (channelRemappingTextXmlElement && channelRemappingTextXmlElement->isTextElement())
							channelRemappingTextXmlElement->setText(juce::String(assi.second));
						else
							channelRemappingTextXmlElement->addTextElement(juce::String(assi.second));
					}
					else
					{
						channelRemappingXmlElement = channelRemappingsXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REMAPPINGS));
						channelRemappingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), assi.first);
						channelRemappingXmlElement->addTextElement(juce::String(assi.second));
					}
				}
			}
			else
				return false;
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Getter for the controller-understandable status per protocol as can be presented to the user.
 * @param	protocolId	The id of the protocol to get the status for
 * @return	The status as requested
 */
ObjectHandlingState ProtocolBridgingWrapper::GetProtocolState(ProtocolId protocolId) const
{
	if (m_bridgingProtocolState.count(protocolId) < 1)
		return OHS_Invalid;
	else
		return m_bridgingProtocolState.at(protocolId);
}

/***
 * Setter for the protocol status values hashed by protocol id.
 * @param	protocolId	The id of the protocol to set a status value for
 * @param	state		The state to set for the protocol
 */
void ProtocolBridgingWrapper::SetProtocolState(ProtocolId protocolId, ObjectHandlingState state)
{
	if (GetProtocolState(protocolId) != state)
	{
		m_bridgingProtocolState[protocolId] = state;

		if (!Controller::Exists()) // avoid creating controller singleton here
			return;
		auto ctrl = Controller::GetInstance();
		if (ctrl)
			ctrl->SetParameterChanged(DCP_Protocol, DCT_Connected);
	}
}

/**
 * Method to toggle the bridging module online state.
 * @param	online	Bool indicator if connection shall be established or reset.
 */
void ProtocolBridgingWrapper::SetOnline(bool online)
{
	if (online)
		Reconnect();
	else
		Disconnect();
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

		protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DAWPLUGIN_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
			activeBridgingTypes |= PBT_DAWPlugin;

		protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(RTTRPM_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
			activeBridgingTypes |= PBT_BlacktraxRTTrPM;

		protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(GENERICOSC_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
			activeBridgingTypes |= PBT_GenericOSC;

		protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(GENERICMIDI_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
			activeBridgingTypes |= PBT_GenericMIDI;

		protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(ADMOSC_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
			activeBridgingTypes |= PBT_ADMOSC;

		protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(YAMAHAOSC_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
			activeBridgingTypes |= PBT_YamahaOSC;

		protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(REMAPOSC_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
			activeBridgingTypes |= PBT_RemapOSC;
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
		auto addDAWPluginBridging = ((desiredActiveBridgingTypes & PBT_DAWPlugin) && !(currentlyActiveBridgingTypes & PBT_DAWPlugin));
		auto removeDAWPluginBridging = (!(desiredActiveBridgingTypes & PBT_DAWPlugin) && (currentlyActiveBridgingTypes & PBT_DAWPlugin));
		auto addRTTrPMBridging = ((desiredActiveBridgingTypes & PBT_BlacktraxRTTrPM) && !(currentlyActiveBridgingTypes & PBT_BlacktraxRTTrPM));
		auto removeRTTrPMBridging = (!(desiredActiveBridgingTypes & PBT_BlacktraxRTTrPM) && (currentlyActiveBridgingTypes & PBT_BlacktraxRTTrPM));
		auto addGenericOSCBridging = ((desiredActiveBridgingTypes & PBT_GenericOSC) && !(currentlyActiveBridgingTypes & PBT_GenericOSC));
		auto removeGenericOSCBridging = (!(desiredActiveBridgingTypes & PBT_GenericOSC) && (currentlyActiveBridgingTypes & PBT_GenericOSC));
		auto addGenericMIDIBridging = ((desiredActiveBridgingTypes & PBT_GenericMIDI) && !(currentlyActiveBridgingTypes & PBT_GenericMIDI));
		auto removeGenericMIDIBridging = (!(desiredActiveBridgingTypes & PBT_GenericMIDI) && (currentlyActiveBridgingTypes & PBT_GenericMIDI));
		auto addADMOSCBridging = ((desiredActiveBridgingTypes & PBT_ADMOSC) && !(currentlyActiveBridgingTypes & PBT_ADMOSC));
		auto removeADMOSCBridging = (!(desiredActiveBridgingTypes & PBT_ADMOSC) && (currentlyActiveBridgingTypes & PBT_ADMOSC));
		auto addYamahaOSCBridging = ((desiredActiveBridgingTypes & PBT_YamahaOSC) && !(currentlyActiveBridgingTypes & PBT_YamahaOSC));
		auto removeYamahaOSCBridging = (!(desiredActiveBridgingTypes & PBT_YamahaOSC) && (currentlyActiveBridgingTypes & PBT_YamahaOSC));
		auto addRemapOSCBridging = ((desiredActiveBridgingTypes & PBT_RemapOSC) && !(currentlyActiveBridgingTypes & PBT_RemapOSC));
		auto removeRemapOSCBridging = (!(desiredActiveBridgingTypes & PBT_RemapOSC) && (currentlyActiveBridgingTypes & PBT_RemapOSC));

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

			if (addDAWPluginBridging)
			{
				nodeXmlElement->addChildElement(std::make_unique<XmlElement>(m_bridgingProtocolCacheMap.at(PBT_DAWPlugin)).release());
			}
			else if (removeDAWPluginBridging)
			{
				auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DAWPLUGIN_PROCESSINGPROTOCOL_ID));
				if (protocolXmlElement)
				{
					m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_DAWPlugin, *protocolXmlElement));
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

			if (addADMOSCBridging)
			{
				nodeXmlElement->addChildElement(std::make_unique<XmlElement>(m_bridgingProtocolCacheMap.at(PBT_ADMOSC)).release());
			}
			else if (removeADMOSCBridging)
			{
				auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(ADMOSC_PROCESSINGPROTOCOL_ID));
				if (protocolXmlElement)
				{
					m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_ADMOSC, *protocolXmlElement));
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

			if (addRemapOSCBridging)
			{
				nodeXmlElement->addChildElement(std::make_unique<XmlElement>(m_bridgingProtocolCacheMap.at(PBT_RemapOSC)).release());
			}
			else if (removeRemapOSCBridging)
			{
				auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(REMAPOSC_PROCESSINGPROTOCOL_ID));
				if (protocolXmlElement)
				{
					m_bridgingProtocolCacheMap.insert(std::make_pair(PBT_RemapOSC, *protocolXmlElement));
					nodeXmlElement->removeChildElement(protocolXmlElement, true);
				}
			}

			SetBridgingNodeStateXml(nodeXmlElement);

			Controller* ctrl = Controller::GetInstance();
			if (ctrl)
				ctrl->SetParameterChanged(DCP_Host, DCT_NumBridgingModules);
		}
	}
}

/**
 * Updates the active soundobject ids in DS100 device protocol configuration of bridging node.
 * This method gets all current active remote objects from controller, maps them to one or two (64ch vs 128ch)
 * DS100 devices and inserts them into an xml element that is then set as new config to bridging node.
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::UpdateActiveDS100RemoteObjectIds()
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (!nodeXmlElement)
		return false;

	auto objectHandlingXmlElement = nodeXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING));
	if (!objectHandlingXmlElement)
		return false;

	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	auto extensionMode = ctrl->GetExtensionMode();

	// Get currently active objects from controller and split them 
	// into those relevant for first and second DS100
	auto activeSoObjects = ctrl->GetActivatedSoundObjectRemoteObjects();
	auto activeMiObjects = ctrl->GetActivatedMatrixInputRemoteObjects();
	auto activeMoObjects = ctrl->GetActivatedMatrixOutputRemoteObjects();

	auto activeObjects = std::vector<RemoteObject>();
	activeObjects.insert(activeObjects.end(), activeSoObjects.begin(), activeSoObjects.end());
	activeObjects.insert(activeObjects.end(), activeMiObjects.begin(), activeMiObjects.end());
	activeObjects.insert(activeObjects.end(), activeMoObjects.begin(), activeMoObjects.end());

	auto activeObjectsOnFirstDS100 = std::vector<RemoteObject>{};
	auto activeObjectsOnSecondDS100 = std::vector<RemoteObject>{};
	for (auto const& ro : activeObjects)
	{
		auto objectId = ro._Addr._first;

		switch (extensionMode)
		{
		case EM_Off:
			// We do not support anything exceeding one DS100 channelcount wise
			if (objectId <= DS100_CHANNELCOUNT)
				activeObjectsOnFirstDS100.push_back(ro);
			break;
		case EM_Extend:
			// We do not support anything exceeding two DS100 (ext. mode) channelcount wise
			if (objectId > DS100_EXTMODE_CHANNELCOUNT)
				break;

			// If the soundobjectId is out of range for a single DS100, take it as relevant 
			// for second and map the soundobjectId back into a single DS100's source range
			if (objectId > DS100_CHANNELCOUNT)
			{
				activeObjectsOnSecondDS100.push_back(ro);
				activeObjectsOnSecondDS100.back()._Addr._first = static_cast<int>(objectId - DS100_CHANNELCOUNT);
			}
			// Otherwise simply take it as relevant for first DS100
			else
			{
				activeObjectsOnFirstDS100.push_back(ro);
			}
			break;
		case EM_Parallel:
		case EM_Mirror:
			// We do not support anything exceeding one DS100 channelcount wise
			if (objectId <= DS100_CHANNELCOUNT)
			{
				activeObjectsOnFirstDS100.push_back(ro);
				activeObjectsOnSecondDS100.push_back(ro);
			}
			break;
		default:
			break;
		}
	}

	// insert active objects for first DS100 into its xml element
	auto protocolXmlElement1stDS100 = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_1_PROCESSINGPROTOCOL_ID));
	if (protocolXmlElement1stDS100)
	{
		auto activeObjsXmlElement = protocolXmlElement1stDS100->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::ACTIVEOBJECTS));
		if (activeObjsXmlElement)
		{
			ProcessingEngineConfig::WriteActiveObjects(activeObjsXmlElement, activeObjectsOnFirstDS100);
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
			ProcessingEngineConfig::WriteActiveObjects(activeObjsXmlElement, activeObjectsOnSecondDS100);
		}
		else
			return false;
	}

	// set updated xml config live
	return SetBridgingNodeStateXml(nodeXmlElement);
}

/**
 * Getter method for the active DS100 communication protocol type.
 * This does not return a member variable value but contains logic to derive the mode from internal cached xml element configuration.
 * @return The DS100 communication protocol type as results from cached xml config. Default PT_OSCProtocol if xml is invalid.
 */
ProtocolType ProtocolBridgingWrapper::GetDS100ProtocolType()
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_1_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
		{
			return ProcessingEngineConfig::ProtocolTypeFromString(protocolXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE)));
		}
	}

	return PT_OSCProtocol;
}

/**
 * Sets the desired protocol type for communication with the DS100(s).
 * This method inserts the rate value into the cached xml element,
 * pushes the updated xml element into processing node and triggers configuration updating.
 * @param msgRate	The new message rate value in ms
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetDS100ProtocolType(ProtocolType protocolType, bool dontSendNotification)
{
	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		// First DS100 configuration
		auto protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_1_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
		{
			auto protocolTypeString = ProcessingEngineConfig::ProtocolTypeToString(protocolType);
			bool protocolTypeChanged = (protocolTypeString != protocolXmlElement->getStringAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE)));
			// Set the new protocolType. This is responsible for the instantiation of correct ProtocolProcessor type in RPBC when parsing the updated xml
			protocolXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), protocolTypeString);

			if (protocolType != PT_NoProtocol)
			{
				// DS100 uses different ports for OSC (udp) and OCA (tcp)
				auto clientPortXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
				auto clientPortElmWasNewlyCreated = false;
				if (!clientPortXmlElement)
				{
					clientPortXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
					clientPortElmWasNewlyCreated = true;
				}
				if (protocolTypeChanged || clientPortElmWasNewlyCreated)
					clientPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), (protocolType == PT_OSCProtocol ? RX_PORT_DS100_DEVICE : RX_PORT_DS100_DEVICE_OCP1));

				auto hostPortXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
				auto hostPortElmWasNewlyCreated = false;
				if (!hostPortXmlElement)
				{
					hostPortXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
					hostPortElmWasNewlyCreated = true;
				}
				if (protocolTypeChanged || hostPortElmWasNewlyCreated)
					hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), (protocolType == PT_OSCProtocol ? RX_PORT_DS100_HOST : 0)); // hostport is not used in Ocp1 RPBC implementation

				// Ocp1 requires the server/client mode to be set. Therefor we need to create the corresp. xml entry if Ocp1 or delete it if OCA becomes active.
				auto ocp1ConnectionModeXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OCP1CONNECTIONMODE));
				if (protocolType == PT_OCP1Protocol)
				{
					if (!ocp1ConnectionModeXmlElement)
						ocp1ConnectionModeXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OCP1CONNECTIONMODE));
					if (ocp1ConnectionModeXmlElement->getFirstChildElement() && ocp1ConnectionModeXmlElement->getFirstChildElement()->isTextElement())
						ocp1ConnectionModeXmlElement->getFirstChildElement()->setText("client");
					else
						ocp1ConnectionModeXmlElement->addTextElement("client");
				}
				else if (ocp1ConnectionModeXmlElement)
					protocolXmlElement->removeChildElement(ocp1ConnectionModeXmlElement, true);

				// Ocp1 heartbeat timing is handled internally and should not be interfered with from a potentially leftover OSC polling interval 
				if (protocolType == PT_OCP1Protocol)
				{
					auto pollingIntervalXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
					if (pollingIntervalXmlElement)
						protocolXmlElement->removeChildElement(pollingIntervalXmlElement, true);
				}
				else if (protocolType == PT_OSCProtocol)
				{
					auto pollingIntervalXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
					if (!pollingIntervalXmlElement)
					{
						pollingIntervalXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
						pollingIntervalXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL), ET_DefaultPollingRate);
					}
				}
			}
		}
		else
			return false;

		// Second DS100 configuration (if existing)
		protocolXmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_2_PROCESSINGPROTOCOL_ID));
		if (protocolXmlElement)
		{
			// Set the new protocolType. This is responsible for the instantiation of correct ProtocolProcessor type in RPBC when parsing the updated xml
			protocolXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::TYPE), ProcessingEngineConfig::ProtocolTypeToString(protocolType));

			if (protocolType != PT_NoProtocol)
			{
				// DS100 uses different ports for OSC (udp) and OCA (tcp)
				auto clientPortXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
				if (!clientPortXmlElement)
					clientPortXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::CLIENTPORT));
				clientPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), (protocolType == PT_OSCProtocol ? RX_PORT_DS100_DEVICE : RX_PORT_DS100_DEVICE_OCP1));

				auto hostPortXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
				if (!hostPortXmlElement)
					hostPortXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::HOSTPORT));
				hostPortXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::PORT), (protocolType == PT_OSCProtocol ? RX_PORT_DS100_HOST : 0)); // hostport is not used in Ocp1 RPBC implementation

				// Ocp1 requires the server/client mode to be set. Therefor we need to create the corresp. xml entry if Ocp1 or delete it if OCA becomes active.
				auto ocp1ConnectionModeXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OCP1CONNECTIONMODE));
				if (protocolType == PT_OCP1Protocol)
				{
					if (!ocp1ConnectionModeXmlElement)
						ocp1ConnectionModeXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OCP1CONNECTIONMODE));
					if (ocp1ConnectionModeXmlElement->getFirstChildElement() && ocp1ConnectionModeXmlElement->getFirstChildElement()->isTextElement())
						ocp1ConnectionModeXmlElement->getFirstChildElement()->setText("client");
					else
						ocp1ConnectionModeXmlElement->addTextElement("client");
				}
				else if (ocp1ConnectionModeXmlElement)
					protocolXmlElement->removeChildElement(ocp1ConnectionModeXmlElement, true);

				// Ocp1 heartbeat timing is handled internally and should not be interfered with from a potentially leftover OSC polling interval 
				if (protocolType == PT_OCP1Protocol)
				{
					auto pollingIntervalXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
					if (pollingIntervalXmlElement)
						protocolXmlElement->removeChildElement(pollingIntervalXmlElement, true);
				}
				else if (protocolType == PT_OSCProtocol)
				{
					auto pollingIntervalXmlElement = protocolXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
					if (!pollingIntervalXmlElement)
					{
						pollingIntervalXmlElement = protocolXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::POLLINGINTERVAL));
						pollingIntervalXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::INTERVAL), ET_DefaultPollingRate);
					}
				}
			}
		}
		// its ok to have no else return false here, since the second DS100 is not mandatory in config!

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Gets the currently set DS100 client ip address.
 * This method forwards the call to the generic implementation.
 * @return	The ip address string
 */
juce::IPAddress ProtocolBridgingWrapper::GetDS100IpAddress()
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
bool ProtocolBridgingWrapper::SetDS100IpAddress(juce::IPAddress ipAddress, bool dontSendNotification)
{
	return SetProtocolIpAddress(DS100_1_PROCESSINGPROTOCOL_ID, ipAddress, dontSendNotification);
}

/**
 * Gets the currently used port for the connection to the first DS100 device.
 * This method forwards the call to the generic implementation.
 * @return	The port number used.
 */
int ProtocolBridgingWrapper::GetDS100Port()
{
	return GetProtocolRemotePort(DS100_1_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the port number to currently use the connection to the first DS100 device.
 * This method forwards the call to the generic implementation.
 * @param port					The new port number to use
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetDS100Port(int port, bool dontSendNotification)
{
	return SetProtocolRemotePort(DS100_1_PROCESSINGPROTOCOL_ID, port, dontSendNotification);
}

/**
 * Gets the currently set cascade DS100 client ip address.
 * This method forwards the call to the generic implementation.
 * @return	The ip address string
 */
juce::IPAddress ProtocolBridgingWrapper::GetSecondDS100IpAddress()
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
bool ProtocolBridgingWrapper::SetSecondDS100IpAddress(juce::IPAddress ipAddress, bool dontSendNotification)
{
	return SetProtocolIpAddress(DS100_2_PROCESSINGPROTOCOL_ID, ipAddress, dontSendNotification);
}


/**
 * Gets the currently used port for the connection to the second DS100 device.
 * This method forwards the call to the generic implementation.
 * @return	The port number used.
 */
int ProtocolBridgingWrapper::GetSecondDS100Port()
{
	return GetProtocolRemotePort(DS100_2_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the port number to currently use the connection to the second DS100 device.
 * This method forwards the call to the generic implementation.
 * @param port					The new port number to use
 * @param dontSendNotification	Flag if the app configuration should be triggered to be updated
 * @return	True on succes, false if failure
 */
bool ProtocolBridgingWrapper::SetSecondDS100Port(int port, bool dontSendNotification)
{
	return SetProtocolRemotePort(DS100_2_PROCESSINGPROTOCOL_ID, port, dontSendNotification);
}

/**
 * Gets the currently active message rate for protocol polling.
 * @return	The message rate currently set in xml config
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
 * @param msgRate	The new protocol type enum id
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
		// its ok to have no else return false here, since the second DS100 is not mandatory in config!

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
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
			case OHM_Mirror_dualA_withValFilter:
				return EM_Mirror;
			case OHM_Mux_nA_to_mB_withValFilter:
				return EM_Extend;
			case OHM_Forward_only_valueChanges:
			case OHM_A1active_withValFilter:
			case OHM_A2active_withValFilter:
				{
				auto protocol1XmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_1_PROCESSINGPROTOCOL_ID));
				auto protocol2XmlElement = nodeXmlElement->getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_2_PROCESSINGPROTOCOL_ID));
				// if two DS100 protocol processors are configured for OHM_Forward_only_valueChanges, we are in parallel mode
				if (protocol1XmlElement && protocol2XmlElement)
					return EM_Parallel;
				// if only one is configured, we are in off mode
				else if (protocol1XmlElement)
					return EM_Off;
				// if no one is configured, we are in undefined mode and fallthrough into default and assert
				}
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
					// EM_Off refers to valuechange forwarding object handling mode without any channelcount parameter attributes
					objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Forward_only_valueChanges));

					// remove elements that are not used by this ohm
					auto protocolAChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLACHCNT));
					if (protocolAChCntXmlElement)
						objectHandlingXmlElement->removeChildElement(protocolAChCntXmlElement, true);
					auto protocolBChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLBCHCNT));
					if (protocolBChCntXmlElement)
						objectHandlingXmlElement->removeChildElement(protocolBChCntXmlElement, true);
					auto protoFailoverTimeXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::FAILOVERTIME));
					if (protoFailoverTimeXmlElement)
						objectHandlingXmlElement->removeChildElement(protoFailoverTimeXmlElement, true);

					// update precision element
					auto precisionXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
					if (!precisionXmlElement)
						precisionXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
					auto precisionTextXmlElement = precisionXmlElement->getFirstChildElement();
					if (precisionTextXmlElement && precisionTextXmlElement->isTextElement())
						precisionTextXmlElement->setText(String(DS100_VALUCHANGE_SENSITIVITY));
					else
						precisionXmlElement->addTextElement(String(DS100_VALUCHANGE_SENSITIVITY));

					// update typeA (DS100) to have the valueAck expected flag set
					auto protocolsAcknowledgeXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALUEACK));
					if (!protocolsAcknowledgeXmlElement)
						protocolsAcknowledgeXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALUEACK));
					auto protocolsAcknowledgeTextXmlElement = protocolsAcknowledgeXmlElement->getFirstChildElement();
					if (protocolsAcknowledgeTextXmlElement && protocolsAcknowledgeTextXmlElement->isTextElement())
						protocolsAcknowledgeTextXmlElement->setText(String(DS100_ACKNOWLEDGES_SENT_VALUES_MASK));
					else
						protocolsAcknowledgeXmlElement->addTextElement(String(DS100_ACKNOWLEDGES_SENT_VALUES_MASK));

					// update reaction monitored protocols element
					auto reactMoniProtosTextValue = String(DS100_1_PROCESSINGPROTOCOL_ID) + "," + String(DS100_2_PROCESSINGPROTOCOL_ID);
					auto reactMoniProtosXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REACTMONIPROTOS));
					if (!reactMoniProtosXmlElement)
						reactMoniProtosXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REACTMONIPROTOS));
					auto reactMoniProtosTextXmlElement = reactMoniProtosXmlElement->getFirstChildElement();
					if (reactMoniProtosTextXmlElement && reactMoniProtosTextXmlElement->isTextElement())
						reactMoniProtosTextXmlElement->setText(reactMoniProtosTextValue);
					else
						reactMoniProtosXmlElement->addTextElement(reactMoniProtosTextValue);
				}
				break;
			case EM_Parallel:
				{
					// EM_Parallel refers to valuechange forwarding object handling mode, restricted to either A1 or A2 poll request answer forwarding, without any channelcount parameter attributes
					auto parallelObjectHandlingMode = (GetActiveParallelModeDS100() == APM_2nd ? OHM_A2active_withValFilter : OHM_A1active_withValFilter);
					objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(parallelObjectHandlingMode));

					// remove elements that are not used by this ohm
					auto protocolAChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLACHCNT));
					if (protocolAChCntXmlElement)
						objectHandlingXmlElement->removeChildElement(protocolAChCntXmlElement, true);
					auto protocolBChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLBCHCNT));
					if (protocolBChCntXmlElement)
						objectHandlingXmlElement->removeChildElement(protocolBChCntXmlElement, true);
					auto protoFailoverTimeXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::FAILOVERTIME));
					if (protoFailoverTimeXmlElement)
						objectHandlingXmlElement->removeChildElement(protoFailoverTimeXmlElement, true);

					// update precision element
					auto precisionXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
					if (!precisionXmlElement)
						precisionXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
					auto precisionTextXmlElement = precisionXmlElement->getFirstChildElement();
					if (precisionTextXmlElement && precisionTextXmlElement->isTextElement())
						precisionTextXmlElement->setText(String(DS100_VALUCHANGE_SENSITIVITY));
					else
						precisionXmlElement->addTextElement(String(DS100_VALUCHANGE_SENSITIVITY));

					// update typeA (DS100) to have the valueAck expected flag set
					auto protocolsAcknowledgeXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALUEACK));
					if (!protocolsAcknowledgeXmlElement)
						protocolsAcknowledgeXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALUEACK));
					auto protocolsAcknowledgeTextXmlElement = protocolsAcknowledgeXmlElement->getFirstChildElement();
					if (protocolsAcknowledgeTextXmlElement && protocolsAcknowledgeTextXmlElement->isTextElement())
						protocolsAcknowledgeTextXmlElement->setText(String(DS100_ACKNOWLEDGES_SENT_VALUES_MASK));
					else
						protocolsAcknowledgeXmlElement->addTextElement(String(DS100_ACKNOWLEDGES_SENT_VALUES_MASK));

					// update reaction monitored protocols element
					auto reactMoniProtosTextValue = String(DS100_1_PROCESSINGPROTOCOL_ID) + "," + String(DS100_2_PROCESSINGPROTOCOL_ID);
					auto reactMoniProtosXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REACTMONIPROTOS));
					if (!reactMoniProtosXmlElement)
						reactMoniProtosXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REACTMONIPROTOS));
					auto reactMoniProtosTextXmlElement = reactMoniProtosXmlElement->getFirstChildElement();
					if (reactMoniProtosTextXmlElement && reactMoniProtosTextXmlElement->isTextElement())
						reactMoniProtosTextXmlElement->setText(reactMoniProtosTextValue);
					else
						reactMoniProtosXmlElement->addTextElement(reactMoniProtosTextValue);
				}
				break;
			case EM_Extend:
				{
					// EM_Extend refers to Multiplex nA to mB object handling mode with channel A and B parameter attributes
					objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Mux_nA_to_mB_withValFilter));

					// remove elements that are not used by this ohm
					auto protoFailoverTimeXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::FAILOVERTIME));
					if (protoFailoverTimeXmlElement)
						objectHandlingXmlElement->removeChildElement(protoFailoverTimeXmlElement, true);
				
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

					// update typeA (DS100) to have the valueAck expected flag set
					auto protocolsAcknowledgeXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALUEACK));
					if (!protocolsAcknowledgeXmlElement)
						protocolsAcknowledgeXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALUEACK));
					auto protocolsAcknowledgeTextXmlElement = protocolsAcknowledgeXmlElement->getFirstChildElement();
					if (protocolsAcknowledgeTextXmlElement && protocolsAcknowledgeTextXmlElement->isTextElement())
						protocolsAcknowledgeTextXmlElement->setText(String(DS100_ACKNOWLEDGES_SENT_VALUES_MASK));
					else
						protocolsAcknowledgeXmlElement->addTextElement(String(DS100_ACKNOWLEDGES_SENT_VALUES_MASK));

					// update reaction monitored protocols element
					auto reactMoniProtosTextValue = String(DS100_1_PROCESSINGPROTOCOL_ID) + "," + String(DS100_2_PROCESSINGPROTOCOL_ID);
					auto reactMoniProtosXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REACTMONIPROTOS));
					if (!reactMoniProtosXmlElement)
						reactMoniProtosXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REACTMONIPROTOS));
					auto reactMoniProtosTextXmlElement = reactMoniProtosXmlElement->getFirstChildElement();
					if (reactMoniProtosTextXmlElement && reactMoniProtosTextXmlElement->isTextElement())
						reactMoniProtosTextXmlElement->setText(reactMoniProtosTextValue);
					else
						reactMoniProtosXmlElement->addTextElement(reactMoniProtosTextValue);
				}
				break;																					
			case EM_Mirror:
				{
					// EM_Extend refers to Multiplex nA to mB object handling mode with channel A and B parameter attributes
					objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(OHM_Mirror_dualA_withValFilter));
				
					// remove elements that are not used by this ohm
					auto protocolAChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLACHCNT));
					if (protocolAChCntXmlElement)
						objectHandlingXmlElement->removeChildElement(protocolAChCntXmlElement, true);
					auto protocolBChCntXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::PROTOCOLBCHCNT));
					if (protocolBChCntXmlElement)
						objectHandlingXmlElement->removeChildElement(protocolBChCntXmlElement, true);

					// update failover time element
					auto protoFailoverTimeXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::FAILOVERTIME));
					if (!protoFailoverTimeXmlElement)
						protoFailoverTimeXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::FAILOVERTIME));
					auto protoFailoverTimeTextXmlElement = protoFailoverTimeXmlElement->getFirstChildElement();
					if (protoFailoverTimeTextXmlElement && protoFailoverTimeTextXmlElement->isTextElement())
						protoFailoverTimeTextXmlElement->setText("1000");
					else
						protoFailoverTimeXmlElement->addTextElement("1000");

					// update precision element
					auto precisionXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
					if (!precisionXmlElement)
						precisionXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::DATAPRECISION));
					auto precisionTextXmlElement = precisionXmlElement->getFirstChildElement();
					if (precisionTextXmlElement && precisionTextXmlElement->isTextElement())
						precisionTextXmlElement->setText(String(DS100_VALUCHANGE_SENSITIVITY));
					else if (precisionTextXmlElement)
						precisionXmlElement->addTextElement(String(DS100_VALUCHANGE_SENSITIVITY));

					// update typeA (DS100) to have the valueAck expected flag set
					auto protocolsAcknowledgeXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALUEACK));
					if (!protocolsAcknowledgeXmlElement)
						protocolsAcknowledgeXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::VALUEACK));
					auto protocolsAcknowledgeTextXmlElement = protocolsAcknowledgeXmlElement->getFirstChildElement();
					if (protocolsAcknowledgeTextXmlElement && protocolsAcknowledgeTextXmlElement->isTextElement())
						protocolsAcknowledgeTextXmlElement->setText(String(DS100_ACKNOWLEDGES_SENT_VALUES_MASK));
					else
						protocolsAcknowledgeXmlElement->addTextElement(String(DS100_ACKNOWLEDGES_SENT_VALUES_MASK));

					// update reaction monitored protocols element
					auto reactMoniProtosTextValue = String(DS100_1_PROCESSINGPROTOCOL_ID) + "," + String(DS100_2_PROCESSINGPROTOCOL_ID);
					auto reactMoniProtosXmlElement = objectHandlingXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REACTMONIPROTOS));
					if (!reactMoniProtosXmlElement)
						reactMoniProtosXmlElement = objectHandlingXmlElement->createNewChildElement(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::REACTMONIPROTOS));
					auto reactMoniProtosTextXmlElement = reactMoniProtosXmlElement->getFirstChildElement();
					if (reactMoniProtosTextXmlElement && reactMoniProtosTextXmlElement->isTextElement())
						reactMoniProtosTextXmlElement->setText(reactMoniProtosTextValue);
					else
						reactMoniProtosXmlElement->addTextElement(reactMoniProtosTextValue);
				}
				break;
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

				auto ctrl = Controller::GetInstance();
				if (ctrl)
					ctrl->SetSecondDS100IpAndPort(DCP_Init, juce::IPAddress(), 0xffff, dontSendNotification);
			}
			break;
			case EM_Extend:
			case EM_Parallel:
			case EM_Mirror:
			{
				// EM_Extend/EM_Parallel/EM_Mirror require the second DS100 protocol to be present
				if (!protocolA2XmlElement)
				{
					protocolA2XmlElement = std::make_unique<XmlElement>(*protocolA1XmlElement).release();
					protocolA2XmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DS100_2_PROCESSINGPROTOCOL_ID));
					nodeXmlElement->addChildElement(protocolA2XmlElement);

					auto ctrl = Controller::GetInstance();
					if (ctrl)
						ctrl->SetSecondDS100IpAndPort(DCP_Init, juce::IPAddress(PROTOCOL_DEFAULT2_IP), RX_PORT_DS100_DEVICE, dontSendNotification);
				}
			}
			break;
			default:
				break;
			}
		}
		else
			return false;

		return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
	}
	else
		return false;
}

/**
 * Getter method for active DS100 in extension mode "parallel".
 * This does not return a member variable value but contains logic to derive the mode from internal cached xml element configuration.
 * @return The DS100 currently set as active one for extension mode "parallel" in cached xml config.
 */
ActiveParallelModeDS100 ProtocolBridgingWrapper::GetActiveParallelModeDS100()
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
			case OHM_A1active_withValFilter:
				return APM_1st;
			case OHM_A2active_withValFilter:
				return APM_2nd;
			case OHM_Mirror_dualA_withValFilter:
			case OHM_Mux_nA_to_mB_withValFilter:
			case OHM_Forward_only_valueChanges:
			case OHM_Bypass:
			case OHM_Invalid:
			case OHM_Mux_nA_to_mB:
			case OHM_Remap_A_X_Y_to_B_XY:
			case OHM_DS100_DeviceSimulation:
			case OHM_Forward_A_to_B_only:
			case OHM_Reverse_B_to_A_only:
				return APM_None;
			case OHM_UserMAX:
			default:
				jassertfalse;
				return APM_None;
			}
		}
	}

	return APM_None;
}

/**
 * Setter method for active DS100 in extension mode "parallel".
 * This does not set a member variable but contains logic to reconfigure the cached xml element according to the given mode value.
 * @param activeParallelModeDS100	The DS100 to set as active one.
 * @param dontSendNotification	-Ignored-
 * @return	True on success, false on failure
 */
bool ProtocolBridgingWrapper::SetActiveParallelModeDS100(ActiveParallelModeDS100 activeParallelModeDS100, bool dontSendNotification)
{
	ignoreUnused(dontSendNotification);

	// chicken exit - no changes to cached xml element and still successful return if no activeParallelMode shall be set
	if (activeParallelModeDS100 == APM_None)
		return true;

	if (GetDS100ExtensionMode() != EM_Parallel)
	{
		// makes no sense to set the details of parallel extension mode when parallel mode itself is not active
		jassertfalse;
		SetDS100ExtensionMode(EM_Parallel);
	}

	auto nodeXmlElement = m_bridgingXml.getChildByAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::ID), String(DEFAULT_PROCNODE_ID));
	if (nodeXmlElement)
	{
		auto objectHandlingXmlElement = nodeXmlElement->getChildByName(ProcessingEngineConfig::getTagName(ProcessingEngineConfig::TagID::OBJECTHANDLING));
		if (objectHandlingXmlElement)
		{
			if (activeParallelModeDS100 == APM_2nd)
				objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(OHM_A2active_withValFilter));
			else
				objectHandlingXmlElement->setAttribute(ProcessingEngineConfig::getAttributeName(ProcessingEngineConfig::AttributeID::MODE), ProcessingEngineConfig::ObjectHandlingModeToString(OHM_A1active_withValFilter));

			return SetBridgingNodeStateXml(nodeXmlElement, dontSendNotification);
		}
		else
			return false;
	}
	else
		return false;
}

/**
 * Gets the status of the first DS100 protocol connection.
 * This forwards the call to the generic implementation that itself gets the info from BridgingWrapper.
 * @return	The protocol status
 */
ObjectHandlingState ProtocolBridgingWrapper::GetDS100State() const
{
	return GetProtocolState(DS100_1_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the status of the first DS100 protocol connection.
 * This forwards the call to the generic implementation that itself gets the info from BridgingWrapper.
 * @param	state	The protocol state
 */
void ProtocolBridgingWrapper::SetDS100State(ObjectHandlingState state)
{
	SetProtocolState(DS100_1_PROCESSINGPROTOCOL_ID, state);
}

/**
 * Gets the status of the second DS100 protocol connection.
 * This forwards the call to the generic implementation that itself gets the info from BridgingWrapper.
 * @return	The protocol status
 */
ObjectHandlingState ProtocolBridgingWrapper::GetSecondDS100State() const
{
	return GetProtocolState(DS100_2_PROCESSINGPROTOCOL_ID);
}

/**
 * Sets the status of the first DS100 protocol connection.
 * This forwards the call to the generic implementation that itself gets the info from BridgingWrapper.
 * @param	state	The protocol state
 */
void ProtocolBridgingWrapper::SetSecondDS100State(ObjectHandlingState state)
{
	SetProtocolState(DS100_2_PROCESSINGPROTOCOL_ID, state);
}


}
