/* Copyright (c) 2020-2021, Christian Ahrens
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


#include "StandalonePollingPageComponentBase.h"


namespace SpaConBridge
{


/*
===============================================================================
 Class StandalonePollingPageComponentBase
===============================================================================
*/

/**
 * Class constructor.
 */
StandalonePollingPageComponentBase::StandalonePollingPageComponentBase(PageComponentType type)
	: PageComponentBase(type)
{
	auto ctrl = SpaConBridge::Controller::GetInstance();
	if (ctrl)
		ctrl->AddProtocolBridgingWrapperListener(this);
}

/**
 * Class destructor.
 */
StandalonePollingPageComponentBase::~StandalonePollingPageComponentBase()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void StandalonePollingPageComponentBase::paint(Graphics& g)
{
	// Paint background to cover the controls behind this overlay.
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(getLocalBounds());
}

/**
 * Method to handle object data responses.
 * This includes filtering objects that have been added to internal 'monitoring' list and forwarding that data to
 * internal handling method of derived class instance.
 * @param	objectId	The remote object identifier of the object that shall be handled.
 * @param	msgData		The remote object message data that was received and shall be handled.
 */
void StandalonePollingPageComponentBase::HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData)
{
	// disregard all data that does not originate from relevant processing node (may not occur, since this application only uses a single bridging node!)
	if (nodeId != DEFAULT_PROCNODE_ID)
		return;

	// disregard all data that does not originate from a DS100
	if (senderProtocolId != DS100_1_PROCESSINGPROTOCOL_ID && senderProtocolId != DS100_2_PROCESSINGPROTOCOL_ID)
		return;

	// only forward the data corresponding to relevant remote objects to derived implementations
	if (m_objectsForStandalonePolling.count(objectId) > 0)
		if (std::find(m_objectsForStandalonePolling.at(objectId).begin(), m_objectsForStandalonePolling.at(objectId).end(), msgData._addrVal) != m_objectsForStandalonePolling.at(objectId).end())
			HandleObjectDataInternal(objectId, msgData);
}

/**
 * Reimplemtend refresh method that is cyclically called by pagecontainercomponent parent
 * and used here to trigger a single burst of polling messages for the objects that are registered for 'monitoring'.
 * @param init	Ignored here
 */
void StandalonePollingPageComponentBase::UpdateGui(bool init)
{
	ignoreUnused(init);

	triggerPollOnce();
}

/**
 * Helper method to do the sending of poll request messages through controller interface.
 */
void StandalonePollingPageComponentBase::triggerPollOnce()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// send an empty polling request message for each object in our container
	for (auto const& object : m_objectsForStandalonePolling)
	{
		for (auto const& addressing : object.second)
		{
			RemoteObjectMessageData romd(addressing, ROVT_NONE, 0, nullptr, 0);
			ctrl->SendMessageDataDirect(object.first, romd);
		}
	}
}

/**
 * Getter for the map of vectors of the objects that are registered for 'monitoring'.
 * @return	The map of vectors of the objects that are registered for 'monitoring'.
 */
const std::map<RemoteObjectIdentifier, std::vector<RemoteObjectAddressing>>& StandalonePollingPageComponentBase::GetStandalonePollingObjects() const
{
	return m_objectsForStandalonePolling;
}

/**
 * Setter for the map of vectors of the objects that are registered for 'monitoring'.
 * @param	objects	The map of vectors of the objects that shall be used for 'monitoring'.
 */
void StandalonePollingPageComponentBase::SetStandalonePollingObjects(const std::map<RemoteObjectIdentifier, std::vector<RemoteObjectAddressing>>& objects)
{
	m_objectsForStandalonePolling = objects;
}

/**
 * Helper method to add a single remote object (incl. addressing) to the map of vectors of the objects that are registered for 'monitoring'.
 * @param	roi		The id of the remote object to add.
 * @param	addressing	The addressing of the remote object.
 */
void StandalonePollingPageComponentBase::AddStandalonePollingObject(const RemoteObjectIdentifier roi, const RemoteObjectAddressing& addressing)
{
	if (m_objectsForStandalonePolling.count(roi) > 0)
		if (std::find(m_objectsForStandalonePolling.at(roi).begin(), m_objectsForStandalonePolling.at(roi).end(), addressing) != m_objectsForStandalonePolling.at(roi).end())
			return; // abort and dont append the incoming object, since it already is present in our object container

	m_objectsForStandalonePolling[roi].push_back(addressing);
}


} // namespace SpaConBridge
