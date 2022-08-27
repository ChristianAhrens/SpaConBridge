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


#pragma once

#include "../../SpaConBridgeCommon.h"


namespace SpaConBridge
{


/**
 * Helper class to allow horizontal layouting of child components
 * and use the instance of this class as a single component embedded
 * in other layouts.
 */
class HorizontalLayouterComponent : public Component
{
public:
	explicit HorizontalLayouterComponent(const String& componentName = String());
	~HorizontalLayouterComponent() override;

	//==============================================================================
	void AddComponent(Component* compo, float layoutRatio = 1.0f);
	bool RemoveComponent(Component* compo);
	void SetSpacing(int spacing);
	void resized() override;

private:
	//==============================================================================
	std::vector<Component*>	m_layoutComponents;
	std::vector<float>		m_layoutRatios;
	int m_spacing{ 0 };
};

/**
 * HeaderWithElmListComponent is a component to hold a header component with multiple other components in a specific layout.
 */
class HeaderWithElmListComponent : public Component
{
public:
	struct LayoutingMetadata
	{
		LayoutingMetadata(bool includeInLayout, bool takeOwnership, int verticalSpan)
		{
			_includeInLayout = includeInLayout;
			_takeOwnership = takeOwnership;
			_verticalSpan = verticalSpan;
		}

		bool	_includeInLayout;
		bool	_takeOwnership;
		int		_verticalSpan;
	};

public:
	HeaderWithElmListComponent(const String& componentName = String());
	~HeaderWithElmListComponent() override;

	//==============================================================================
	void setHasActiveToggle(bool hasActiveToggle);
	void setActiveToggleText(String activeToggleText);
	void setHeaderText(String headerText);
	void addComponent(Component* compo, bool includeInLayout = true, bool takeOwnership = true, int verticalSpan = 1);
	void removeComponent(Component* compo);
	void setToggleActiveState(bool toggleState);
	
	void onToggleActive();

	//==============================================================================
	void setHelpUrl(const URL& helpUrl);
	void setBackgroundDecorationText(const std::string& text);

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
	bool																	m_hasActiveToggle{ false };
	bool																	m_toggleState{ true };
	std::unique_ptr<ToggleButton>											m_activeToggle;
	std::unique_ptr<Label>													m_activeToggleLabel;
	std::unique_ptr<Label>													m_headerLabel;
	std::unique_ptr<DrawableButton>											m_helpButton;
	std::unique_ptr<URL>													m_helpUrl;
	std::string																m_backgroundDecorationText;
	std::vector<std::pair<std::unique_ptr<Component>, LayoutingMetadata>>	m_components;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderWithElmListComponent)
};


} // namespace SpaConBridge
