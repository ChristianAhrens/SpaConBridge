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


#include "PageComponentBase.h"


namespace SpaConBridge
{


/*
===============================================================================
 Class PageComponentBase
===============================================================================
*/

/**
 * Class constructor.
 */
PageComponentBase::PageComponentBase(PageComponentType type)
	: Component()
{
	m_pageComponentType = type;
}

/**
 * Class destructor.
 */
PageComponentBase::~PageComponentBase()
{
}

/**
 * Get this PageComponent's type.
 */
PageComponentBase::PageComponentType PageComponentBase::GetPageComponentType() const
{
	return m_pageComponentType;
}

/**
 * Get this PageComponent's initializing state.
 * @return	True if the page is currently set to initializing state, false if not.
 */
bool PageComponentBase::IsPageInitializing() const
{
	return m_isInitializing;
}

/**
 * Set this PageComponent's initializing state.
 * @param	initializing	The initializing state to set.
 */
void PageComponentBase::SetPageIsInitializing(bool initializing)
{
	m_isInitializing = initializing;
}

/**
 * Minimal helper method to determine if aspect ratio of currently
 * available screen realestate suggests we are in portrait or landscape orientation
 * and be able to use the same determination code in multiple places.
 *
 * @return	True if we are in portrait, false if in landscape aspect ratio.
 */
bool PageComponentBase::IsPortraitAspectRatio()
{
	// determine the layout direction (we want a ratio of 0.75 to be the switching point)
	auto layoutSwitchAspectRatio = 0.75f;
	auto w = getLocalBounds().getWidth();
	auto h = getLocalBounds().getHeight();
	auto aspectRatio = h / (w != 0.0f ? w : 1.0f);

	return layoutSwitchAspectRatio < aspectRatio;
}


} // namespace SpaConBridge
