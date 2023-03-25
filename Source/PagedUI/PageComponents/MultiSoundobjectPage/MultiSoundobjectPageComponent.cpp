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


#include "MultiSoundobjectPageComponent.h"

#include "../../PageComponentManager.h"
#include "../../../MultiSoundobjectComponent.h"

#include <Image_utils.h>


namespace SpaConBridge
{


/*
===============================================================================
 Class MultiSoundobjectPageComponent
===============================================================================
*/

/**
 * Class constructor.
 */
MultiSoundobjectPageComponent::MultiSoundobjectPageComponent()
	: PageComponentBase(UIPageId::UPI_MultiSoundobjects)
{
	auto pageManager = PageComponentManager::GetInstance();
	if (pageManager)
	{
		auto& multiSoundobjectComponent = pageManager->GetMultiSoundobjectComponent();
		if (multiSoundobjectComponent)
			addAndMakeVisible(multiSoundobjectComponent.get());
	}
}

/**
 * Class destructor.
 */
MultiSoundobjectPageComponent::~MultiSoundobjectPageComponent()
{
	auto pageManager = PageComponentManager::GetInstance();
	if (pageManager)
	{
		auto& multiSoundobjectComponent = pageManager->GetMultiSoundobjectComponent();
		if (multiSoundobjectComponent)
			removeChildComponent(multiSoundobjectComponent.get());
	}
}

/**
 * Reimplemented to paint background and frame.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void MultiSoundobjectPageComponent::paint(Graphics& g)
{
	// Background
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(getLocalBounds());
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void MultiSoundobjectPageComponent::resized()
{
	if (!IsPageVisible())
		return;

	auto pageManager = PageComponentManager::GetInstance();
	if (pageManager)
	{
		auto& multiSoundobjectComponent = pageManager->GetMultiSoundobjectComponent();
		if (multiSoundobjectComponent)
		{
			if (this != multiSoundobjectComponent->getParentComponent())
				addAndMakeVisible(multiSoundobjectComponent.get());

			multiSoundobjectComponent->setBounds(getLocalBounds());
		}
	}
}

/**
 * Reimplemented from PageComponentBase to add or remove the multiSoundobject component to this page's layouting
 * depending on visibility.
 * Call is forwarded to baseimplementation afterwards.
 * @param	initializing	The visible state to set.
 */
void MultiSoundobjectPageComponent::SetPageIsVisible(bool visible)
{
	auto& multiSoundobjectComponent = PageComponentManager::GetInstance()->GetMultiSoundobjectComponent();
	if (multiSoundobjectComponent)
	{
		if (!visible && !isOnDesktop())
		{
			removeChildComponent(multiSoundobjectComponent.get());
		}
		else if (visible)
		{
			multiSoundobjectComponent->SetHandleSelectedOnly(false);
		}
	}

	PageComponentBase::SetPageIsVisible(visible);

	resized();
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the procssor parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void MultiSoundobjectPageComponent::UpdateGui(bool init)
{
	auto pageManager = PageComponentManager::GetInstance();
	if (pageManager)
	{
		auto& multiSoundobjectComponent = pageManager->GetMultiSoundobjectComponent();
		if (multiSoundobjectComponent)
			multiSoundobjectComponent->UpdateGui(init);
	}
}


} // namespace SpaConBridge
