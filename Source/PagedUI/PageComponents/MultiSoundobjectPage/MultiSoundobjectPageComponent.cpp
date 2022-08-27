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
	: PageComponentBase(PCT_MultiSlide)
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
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void MultiSoundobjectPageComponent::resized()
{
	auto pageManager = PageComponentManager::GetInstance();
	if (pageManager)
	{
		auto& multiSoundobjectComponent = pageManager->GetMultiSoundobjectComponent();
		if (multiSoundobjectComponent)
			multiSoundobjectComponent->setBounds(getLocalBounds());
	}
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
