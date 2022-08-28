/* Copyright (c) 2020-2022, Christian Ahrens
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

#include "../../Controller.h"


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
StandalonePollingPageComponentBase::StandalonePollingPageComponentBase(UIPageId id)
	: PageComponentBase(id)
{
	m_elementsContainer = std::make_unique<HeaderWithElmListComponent>();
	m_borderedElementsContainer = std::make_unique<BorderedComponentContainer>();
	m_borderedElementsContainer->SetComponent(m_elementsContainer.get());
	m_borderedElementsContainer->SetBorder(3);
	m_elementsContainerViewport = std::make_unique<Viewport>();
	m_elementsContainerViewport->setViewedComponent(m_borderedElementsContainer.get(), false);
	addAndMakeVisible(m_elementsContainerViewport.get());

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
 * Getter for the private elements container component to be able to externally add items.
 * @return	The container object pointer, if existing. Otherwise nullptr.
 */
HeaderWithElmListComponent* StandalonePollingPageComponentBase::GetElementsContainer()
{
	return m_elementsContainer.get();
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
 * Reimplemented to resize elements container component.
 */
void StandalonePollingPageComponentBase::resized()
{
	auto bounds = getLocalBounds().reduced(5);

	if (m_elementsContainerViewport)
		m_elementsContainerViewport->setBounds(bounds);

	auto minWidth = HeaderWithElmListComponent::m_attachedItemWidth + HeaderWithElmListComponent::m_layoutItemWidth + 2 * m_borderedElementsContainer->GetBorder();
	auto minHeight = m_borderedElementsContainer->GetBorderedHeight();

	if (bounds.getWidth() < minWidth)
		bounds.setWidth(minWidth);
	if (bounds.getHeight() < minHeight)
		bounds.setHeight(minHeight);

	if (m_borderedElementsContainer && m_elementsContainerViewport)
	{
		if (m_elementsContainerViewport->canScrollVertically() || m_elementsContainerViewport->canScrollHorizontally())
		{
			auto boundsWithoutScrollbars = bounds;

			if (m_elementsContainerViewport->canScrollVertically())
				boundsWithoutScrollbars.setWidth(bounds.getWidth() - m_elementsContainerViewport->getVerticalScrollBar().getWidth());

			if (m_elementsContainerViewport->canScrollHorizontally())
				boundsWithoutScrollbars.setHeight(bounds.getHeight() - m_elementsContainerViewport->getHorizontalScrollBar().getHeight());

			m_borderedElementsContainer->setBounds(boundsWithoutScrollbars);
		}
		else
			m_borderedElementsContainer->setBounds(bounds);
	}
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

	// from here on we require the controller singleton to be available
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// enshure that in parallel extension mode, only the polling request answers of the active device are processed
	if (ctrl->GetExtensionMode() == EM_Parallel)
	{
		// Do neither handle any protocol data from second DS100 if first is set as active one...
		if (ctrl->GetActiveParallelModeDS100() == APM_1st && senderProtocolId != DS100_1_PROCESSINGPROTOCOL_ID)
			return;
		// ...nor any protocol data from first DS100 if second is set as active one in parallel extension mode.
		if (ctrl->GetActiveParallelModeDS100() == APM_2nd && senderProtocolId != DS100_2_PROCESSINGPROTOCOL_ID)
			return;
	}

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
