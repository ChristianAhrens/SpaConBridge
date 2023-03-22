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


#pragma once

#include <JuceHeader.h>

#include "../../SpaConBridgeCommon.h"


namespace SpaConBridge
{


/**
 * Abstract class PageComponentBase.
 * Must be reimplemented to provide a component for an app page.
 */
class PageComponentBase : public Component
{
public:
	explicit PageComponentBase(UIPageId id);
	~PageComponentBase() override;

	//==============================================================================
	UIPageId GetPageId() const;

	//==============================================================================
	bool IsPageInitializing() const;
	void SetPageIsInitializing(bool initializing);

	//==============================================================================
	bool IsPageVisible() const;
	virtual void SetPageIsVisible(bool visible);

	//==============================================================================
	void userTriedToCloseWindow() override;

	//==============================================================================
	virtual void UpdateGui(bool init) = 0;
	virtual void NotifyPageWasWindowed(UIPageId pageId, bool windowed) { ignoreUnused(pageId); ignoreUnused(windowed); };

protected:
	bool	IsPortraitAspectRatio();

private:
	UIPageId	m_pageId{ UIPageId::UPI_InvalidMin };	/**> Type of page as specified by the UIPageId enum. */
	bool		m_isInitializing{ false };
	bool		m_isVisible{ false };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PageComponentBase)
};


} // namespace SpaConBridge
