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


#pragma once

#include <JuceHeader.h>


namespace SpaConBridge
{


/**
 * Abstract class PageComponentBase.
 * Must be reimplemented to provide a component for an app page.
 */
class PageComponentBase : public Component
{
public:

	/**
	 * Overlay types. There can only be one active at the time.
	 */
	enum PageComponentType
	{
		PCT_Unknown = 0,
		PCT_Overview,
		PCT_MultiSlide,
		PCT_Settings,
		PCT_Statistics,
		PCT_About
	};

	explicit PageComponentBase(PageComponentType type);
	~PageComponentBase() override;

	//==============================================================================
	PageComponentType GetPageComponentType() const;

	//==============================================================================
	virtual void UpdateGui(bool init) = 0;

private:
	PageComponentType	m_pageComponentType;	/**> Type of page as specified by the PageComponentType enum. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PageComponentBase)
};


} // namespace SpaConBridge
