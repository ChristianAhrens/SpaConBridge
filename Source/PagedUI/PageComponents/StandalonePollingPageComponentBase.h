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

#include "PageComponentBase.h"
#include "HeaderWithElmListComponent.h"

#include "../../StandalonePollingBase.h"


namespace SpaConBridge
{


/**
 * Helper class to allow horizontal layouting of child components
 * and use the instance of this class as a single component embedded
 * in other layouts.
 */
class BorderedComponentContainer : public Component
{
public:
	void SetComponent(Component* compo)
	{
		addAndMakeVisible(compo);
		m_borderedComponent = compo;

		auto newHeight = compo->getHeight() + m_topBorder + m_bottomBorder;
		auto newWidth = compo->getWidth() + m_leftBorder + m_rightBorder;

		setSize(newWidth, newHeight);
	}
	bool RemoveComponent(Component* compo)
	{
		if (compo != m_borderedComponent)
			return false;

		removeChildComponent(compo);
		m_borderedComponent = nullptr;
		return true;
	}
	void SetBorder(int border)
	{
		SetBorder(border, border, border, border);
	}
	void SetBorder(int topBorder, int rightBorder, int bottomBorder, int leftBorder)
	{
		m_topBorder = topBorder;
		m_rightBorder = rightBorder;
		m_bottomBorder = bottomBorder;
		m_leftBorder = leftBorder;

		if (m_borderedComponent)
		{
			auto newHeight = m_borderedComponent->getHeight() + m_topBorder + m_bottomBorder;
			auto newWidth = m_borderedComponent->getWidth() + m_leftBorder + m_rightBorder;

			setSize(newWidth, newHeight);
		}
	}
	int GetBorder()
	{
		auto maxBorder = m_topBorder;
		if (maxBorder < m_rightBorder)
			maxBorder = m_rightBorder;
		if (maxBorder < m_bottomBorder)
			maxBorder = m_bottomBorder;
		if (maxBorder < m_leftBorder)
			maxBorder = m_leftBorder;

		return maxBorder;
	}

	void resized() override
	{
		if (m_borderedComponent)
		{
			auto bounds = getLocalBounds();
			bounds.removeFromTop(m_topBorder);
			bounds.removeFromRight(m_rightBorder);
			bounds.removeFromBottom(m_bottomBorder);
			bounds.removeFromLeft(m_leftBorder);
			m_borderedComponent->setBounds(bounds);
		}
	}

	int GetBorderedHeight()
	{
		if (m_borderedComponent)
			return m_borderedComponent->getHeight() + m_bottomBorder + m_topBorder;
		else
			return getHeight();
	}
	int GetBorderedWidth()
	{
		if (m_borderedComponent)
			return m_borderedComponent->getWidth() + m_leftBorder + m_rightBorder;
		else
			return getWidth();
	}

private:
	Component* m_borderedComponent{ nullptr };
	int m_topBorder{ 0 };
	int m_rightBorder{ 0 };
	int m_bottomBorder{ 0 };
	int m_leftBorder{ 0 };
};

/**
 * class StandalonePollingPageComponentBase is supposed to be used
 * as base component for pages that use remote objects for internal use only without
 * submitting them as active for bridging.
 */
class StandalonePollingPageComponentBase :	public PageComponentBase, public StandalonePollingBase
{
public:
	explicit StandalonePollingPageComponentBase(UIPageId id);
	~StandalonePollingPageComponentBase() override;

	HeaderWithElmListComponent* GetElementsContainer();

	//==============================================================================
	void UpdateGui(bool init) override;

	//==========================================================================
	void paint(Graphics&) override;
	void resized() override;

private:
	//==============================================================================
	std::unique_ptr<HeaderWithElmListComponent>	m_elementsContainer;
	std::unique_ptr<BorderedComponentContainer>	m_borderedElementsContainer;
	std::unique_ptr<Viewport>					m_elementsContainerViewport;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandalonePollingPageComponentBase)
};


} // namespace SpaConBridge
