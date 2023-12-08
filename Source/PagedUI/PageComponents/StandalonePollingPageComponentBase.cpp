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
	: PageComponentBase(id), Controller::StandaloneActiveObjectsListener()
{
	m_elementsContainer = std::make_unique<HeaderWithElmListComponent>();
	m_borderedElementsContainer = std::make_unique<BorderedComponentContainer>();
	m_borderedElementsContainer->SetComponent(m_elementsContainer.get());
	m_borderedElementsContainer->SetBorder(3);
	m_elementsContainerViewport = std::make_unique<Viewport>();
	m_elementsContainerViewport->setViewedComponent(m_borderedElementsContainer.get(), false);
	addAndMakeVisible(m_elementsContainerViewport.get());

	// add this class to be notified of object value changes to controller
	auto const ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->AddStandaloneActiveObjectsListener(this);
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

	if (m_borderedElementsContainer && m_elementsContainerViewport)
	{
		m_elementsContainerViewport->setBounds(bounds);

		auto minWidth = HeaderWithElmListComponent::m_attachedItemWidth + HeaderWithElmListComponent::m_layoutItemWidth + 2 * m_borderedElementsContainer->GetBorder();
		auto minHeight = m_borderedElementsContainer->GetBorderedHeight();

		if (bounds.getWidth() < minWidth)
			bounds.setWidth(minWidth);
		if (bounds.getHeight() < minHeight)
			bounds.setHeight(minHeight);

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
 * Getter for the map of vectors of the objects that are registered for 'monitoring'.
 * @return	The map of vectors of the objects that are registered for 'monitoring'.
 */
const std::vector<RemoteObject>& StandalonePollingPageComponentBase::GetStandalonePollingObjects()
{
	auto const ctrl = Controller::GetInstance();
	if (ctrl)
		return ctrl->GetStandaloneActiveRemoteObjects(this);
	else
		return {};
}

/**
 * Setter for the map of vectors of the objects that are registered for 'monitoring'.
 * @param	objects	The map of vectors of the objects that shall be used for 'monitoring'.
 */
void StandalonePollingPageComponentBase::SetStandalonePollingObjects(const std::map<RemoteObjectIdentifier, std::vector<RemoteObjectAddressing>>& objects)
{
	for (auto const& objectsPerROI : objects)
	{
		auto const& roi = objectsPerROI.first;
		for (auto const& objectAddr : objectsPerROI.second)
			AddStandalonePollingObject(roi, objectAddr);
	}
}

/**
 * Helper method to add a single remote object (incl. addressing) to the map of vectors of the objects that are registered for 'monitoring'.
 * @param	roi		The id of the remote object to add.
 * @param	addressing	The addressing of the remote object.
 */
void StandalonePollingPageComponentBase::AddStandalonePollingObject(const RemoteObjectIdentifier& roi, const RemoteObjectAddressing& addressing)
{
	auto const ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->AddStandaloneActiveRemoteObject(this, { roi, addressing });
}

/**
 * Helper method to confirm active objects through controller interface.
 * This might result in a single poll of all objects or subscription established
 * verification, depending on the protocol in use.
 */
void StandalonePollingPageComponentBase::TriggerConfirmActiveObjects()
{
	auto const ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->TriggerConfirmStandaloneActiveObjects(this);
}


} // namespace SpaConBridge
