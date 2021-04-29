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

#include "../../SpaConBridgeCommon.h"


namespace SpaConBridge
{


/**
 * HeaderWithElmListComponent is a component to hold a header component with multiple other components in a specific layout.
 */
class HeaderWithElmListComponent : public Component
{
public:
	HeaderWithElmListComponent(const String& componentName = String());
	~HeaderWithElmListComponent() override;

	//==============================================================================
	void setHasActiveToggle(bool hasActiveToggle);
	void setActiveToggleText(String activeToggleText);
	void setHeaderText(String headerText);
	void addComponent(Component* compo, bool includeInLayout = true, bool takeOwnership = true);
	void setToggleActiveState(bool toggleState);
	
	void onToggleActive();

	//==============================================================================
	void setHelpUrl(const URL& helpUrl);

	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

	//==============================================================================
	void lookAndFeelChanged() override;

	//==============================================================================
	std::function<void(HeaderWithElmListComponent*, bool)>	toggleIsActiveCallback;

	//==============================================================================
	static constexpr std::int32_t	m_attachedItemWidth{ 150 };
	static constexpr std::int32_t	m_layoutItemWidth{ 205 };

protected:
	//==============================================================================
	void setElementsActiveState(bool toggleState);

private:
	//==============================================================================
	bool																		m_hasActiveToggle{ false };
	bool																		m_toggleState{ true };
	std::unique_ptr<ToggleButton>												m_activeToggle;
	std::unique_ptr<Label>														m_activeToggleLabel;
	std::unique_ptr<Label>														m_headerLabel;
	std::unique_ptr<DrawableButton>												m_helpButton;
	std::unique_ptr<URL>														m_helpUrl;
	std::vector<std::pair<std::unique_ptr<Component>, std::pair<bool, bool>>>	m_components;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderWithElmListComponent)
};


} // namespace SpaConBridge
