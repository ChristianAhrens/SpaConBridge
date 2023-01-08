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


#include "HeaderWithElmListComponent.h"

#include "../../Controller.h"
#include "../../LookAndFeel.h"

#include <Image_utils.h>


namespace SpaConBridge
{

/*
===============================================================================
	Class HorizontalLayouterComponent
===============================================================================
*/

/**
 * Class constructor.
 */
HorizontalLayouterComponent::HorizontalLayouterComponent(const String& componentName) 
	: Component(componentName)
{
}

/**
 * Class destructor.
 */
HorizontalLayouterComponent::~HorizontalLayouterComponent()
{
}

/**
 * Helper method to add a component to the internal list of components to be layouted
 * that optionally takes a ratio value to take into account when layouting.
 * @param	compo			The component to add.
 * @param	layoutRatio		The ratio value to take into account for the component.
 */
void HorizontalLayouterComponent::AddComponent(Component* compo, float layoutRatio)
{
	addAndMakeVisible(compo);
	m_layoutComponents.push_back(compo);
	m_layoutRatios.push_back(layoutRatio);
}

/**
 * Helper method to remove a component from the internal list of components to be layouted.
 * @param	compo	The component to remove.
 * @return	False if the given component is not known internally, true if it is successfully removed.
 */
bool HorizontalLayouterComponent::RemoveComponent(Component* compo)
{
	auto compoIter = m_layoutComponents.begin();
	auto ratioIter = m_layoutRatios.begin();
	for (; compoIter != m_layoutComponents.end() && ratioIter != m_layoutRatios.end(); compoIter++, ratioIter++)
		if (*compoIter == compo)
			break;

	if (compoIter == m_layoutComponents.end() || ratioIter == m_layoutRatios.end())
		return false;

	removeChildComponent(compo);
	m_layoutComponents.erase(compoIter);
	m_layoutRatios.erase(ratioIter);
	return true;
}

/**
 * Setter for the internal layouting spacing value.
 * @param	spacing		The spacing value to set.
 */
void HorizontalLayouterComponent::SetSpacing(int spacing)
{
	m_spacing = spacing;
}

/**
 * Reimplemented from Component to dynamically arrange items in vertical direction.
 */
void HorizontalLayouterComponent::resized()
{
	// prepare width layouting
	auto ratioSum = 0.0f;
	for (auto const& ratio : m_layoutRatios)
		ratioSum += ratio;
	auto totalCompoWidth = getLocalBounds().getWidth() - ((m_layoutComponents.size() - 1) * m_spacing);
	auto widthPerRatioUnit = totalCompoWidth / ratioSum;

	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::row;
	auto compoCnt = m_layoutComponents.size();
	auto ratioCnt = m_layoutRatios.size();
	jassert(compoCnt == ratioCnt);
	for (int i = 0; i < compoCnt && i < ratioCnt; i++)
	{
		auto& compo = m_layoutComponents.at(i);
		auto& ratio = m_layoutRatios.at(i);
		fb.items.add(FlexItem(*compo).withWidth(widthPerRatioUnit * ratio));
		if (i < compoCnt - 1)
			fb.items.add(FlexItem().withWidth(static_cast<float>(m_spacing)));
	}
	fb.performLayout(getLocalBounds().toFloat());
}


/*
===============================================================================
	Class HeaderWithElmListComponent
===============================================================================
*/

/**
 * Class constructor.
 */
HeaderWithElmListComponent::HeaderWithElmListComponent(const String& componentName)
	: Component(componentName)
{
	m_headerLabel = std::make_unique<Label>();
	addAndMakeVisible(m_headerLabel.get());

	m_activeToggle = std::make_unique<ToggleButton>();
	m_activeToggle->onClick = [this] { onToggleActive(); };
	addAndMakeVisible(m_activeToggle.get());
	m_activeToggleLabel = std::make_unique<Label>();
	m_activeToggleLabel->attachToComponent(m_activeToggle.get(), true);
	addAndMakeVisible(m_activeToggleLabel.get());

	setElementsActiveState(m_toggleState);
}

/**
 * Class destructor.
 */
HeaderWithElmListComponent::~HeaderWithElmListComponent()
{
	for (auto& component : m_components)
	{
		auto dontDelete = !component.second._takeOwnership;
        if (dontDelete)
            component.first.release(); // release the pointer to not have the memory cleaned up for those elements that are still externally managed (flagged by second bool in second pair)
	}
}

/**
 *
 */
void HeaderWithElmListComponent::setToggleActiveState(bool toggleState)
{
	if (m_activeToggle)
		m_activeToggle->setToggleState(toggleState, dontSendNotification);

	m_toggleState = toggleState;

	setElementsActiveState(m_toggleState);
}

/**
 * 
 */
void HeaderWithElmListComponent::setElementsActiveState(bool toggleState)
{
	m_toggleState = toggleState;

	m_headerLabel->setEnabled(m_toggleState);
	for (auto const& component : m_components)
	{
		component.first->setEnabled(m_toggleState);
	}

	resized();
	repaint();
	lookAndFeelChanged();
}

/**
 * Helper to get the accumulated state if this instance doesn't use 
 * active toggling or is actually currently toggled active.
 * Both lead to all elements being displayed and the component being active.
 * @return The accumulated state as described.
 */
bool HeaderWithElmListComponent::IsActive()
{
	return m_toggleState || !m_hasActiveToggle;
}

/**
 * Callback method for when active/deactive toggle was triggered
 */
void HeaderWithElmListComponent::onToggleActive()
{
	if (m_activeToggle)
	{
		auto newActiveState = m_activeToggle->getToggleState();

		if (newActiveState == m_toggleState)
			return;

		setElementsActiveState(m_hasActiveToggle ? newActiveState : true);

		if (toggleIsActiveCallback)
			toggleIsActiveCallback(this, m_toggleState);
	}
}

/**
 * Setter for the private url member that defines the web help location
 * corresponding to this HWELC object's contents.
 */
void HeaderWithElmListComponent::setHelpUrl(const URL& helpUrl)
{
	m_helpUrl = std::make_unique<URL>(helpUrl);
	if (!m_helpButton)
	{
		m_helpButton = std::make_unique<DrawableButton>("Help", DrawableButton::ButtonStyle::ImageFitted);
		m_helpButton->onClick = [this] { 
			m_helpUrl->launchInDefaultBrowser();
		};
		addAndMakeVisible(m_helpButton.get());

		resized();
		lookAndFeelChanged();
	}
}

/**
 * Setter for the string that is painted in the background of the component
 * as user info on e.g. 'Alpha' to inform the user that the components held by
 * this object instance refer to a feature still under development.
 */
void HeaderWithElmListComponent::setBackgroundDecorationText(const std::string& text)
{
	m_backgroundDecorationText.clear();
	for (auto i = 0; i < 200; i++)
		m_backgroundDecorationText.append(text + "	");
}

/**
 * Method to set if this component shall display the enable/disable togglebutton
 * in its upper right corner or not.
 * @param hasActiveToggle	True if it shall show togglebutton, false if not
 */
void HeaderWithElmListComponent::setHasActiveToggle(bool hasActiveToggle)
{
	m_hasActiveToggle = hasActiveToggle;

	m_activeToggle->setVisible(hasActiveToggle);
	m_activeToggleLabel->setVisible(hasActiveToggle);

	setElementsActiveState(m_toggleState);
}

/**
 * Setter for the header text of this component.
 * @param headerText	The text to use as headline
 */
void HeaderWithElmListComponent::setHeaderText(String headerText)
{
	auto font = m_headerLabel->getFont();
	font.setBold(true);
	m_headerLabel->setFont(font);
	m_headerLabel->setText(headerText, dontSendNotification);
}

/**
 * Setter for the active toggle text of this component.
 * @param activeToggleText	The text to use in active toggle label
 */
void HeaderWithElmListComponent::setActiveToggleText(String activeToggleText)
{
	m_activeToggleLabel->setText(activeToggleText, dontSendNotification);
}

/**
 * Methdo to add a component to internal list of components that shall be layouted vertically.
 * @param compo	The component to add.
 * @param includeInLayout	Bool flag that can indicate if the component shall be made visible in this components 
 *							context but not layouted. (e.g. a lable that is already attached to another component)
 * @param takeOwnerShip		Bool flag that indicates if ownership of the given component shall be taken.
 * @param verticalSpan		count of vertical item height units the component shall use when layouted
 */
void HeaderWithElmListComponent::addComponent(Component* compo, bool includeInLayout, bool takeOwnership, int verticalSpan)
{
	if (!compo)
		return;

	addAndMakeVisible(compo);
	m_components.push_back(std::make_pair(std::unique_ptr<Component>(compo), LayoutingMetadata(includeInLayout, takeOwnership, verticalSpan)));

	compo->setEnabled(m_toggleState);
}

/**
 * Methdo to remove a component from internal list of components.
 * @param compo	The component to remove.
 */
void HeaderWithElmListComponent::removeComponent(Component* compo)
{
	if (!compo)
		return;

	removeChildComponent(compo);

	for (auto compoIter = m_components.begin(); compoIter != m_components.end(); compoIter++)
	{
		if (compoIter->first.get() == compo)
		{
			auto dontDelete = !compoIter->second._takeOwnership;
			if (dontDelete)
				compoIter->first.release(); // release the pointer to not have the memory cleaned up for those elements that are externally managed (flagged by second bool in second pair)

			m_components.erase(compoIter);
			break;
		}
	}
}

/**
 * Reimplemented paint method from Component that uses colours from TableListBox to give
 * the user a similar impression as when using a table.
 */
void HeaderWithElmListComponent::paint(Graphics& g)
{
	auto w = getWidth();
	auto h = getHeight();

	// paint the background depending on the enabled toggle state
	if (m_toggleState)
		g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId));
	else
		g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId).darker());
	g.fillRect(0, 0, w, h);

	g.setColour(getLookAndFeel().findColour(TableListBox::outlineColourId));
	g.drawRect(0, 0, w, h);

	// paint the info text
	if (!m_backgroundDecorationText.empty())
	{
		if (m_toggleState)
			g.setColour(getLookAndFeel().findColour(TableListBox::textColourId).withAlpha(0.10f));
		else
			g.setColour(getLookAndFeel().findColour(TableListBox::textColourId).withAlpha(0.05f));

		auto trans = AffineTransform();
		trans = trans.translated(-0.25f * w, 0);
		trans = trans.rotated(-0.5f);

		g.addTransform(trans);
		g.setFont(Font(100, Font::FontStyleFlags::bold));
		g.drawMultiLineText(String(m_backgroundDecorationText), 0, 0, static_cast<int>(1.5f * w));
	}
}

/**
 * Reimplemented from Component to dynamically arrange items in vertical direction.
 */
void HeaderWithElmListComponent::resized()
{
	if (!m_headerLabel)
		return;

	Font f = m_headerLabel->getFont();
	auto headerTextWidth = f.getStringWidth(m_headerLabel->getText());

	auto activeToggleHeight = 20.0f;
	auto activeToggleMargin = 2.0f;
	auto headerHeight = 25.0f;
	auto headerMargin = 2.0f;
	auto itemHeight = headerHeight;
	auto itemMargin = 5.0f;
	auto itemCount = 0;

	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.justifyContent = FlexBox::JustifyContent::flexStart;
	// Add the enable/disable section toggle, if this section is configured to be toggleable
	if (m_hasActiveToggle)
	{
		fb.items.add(
			FlexItem(*m_activeToggle.get())
			.withAlignSelf(FlexItem::AlignSelf::flexEnd)
			.withWidth(activeToggleHeight + activeToggleMargin)
			.withHeight(activeToggleHeight)
			.withMargin(FlexItem::Margin(activeToggleMargin, activeToggleMargin, 0, activeToggleMargin)));
	}
	// Add the headline section label
	FlexBox headerFb;
	headerFb.flexDirection = FlexBox::Direction::row;
	headerFb.justifyContent = FlexBox::JustifyContent::flexStart;
	headerFb.items.add(FlexItem(*m_headerLabel.get())
		.withAlignSelf(FlexItem::AlignSelf::flexStart)
		.withWidth(headerTextWidth + headerMargin)
		.withHeight(headerHeight));
	if (m_helpButton) headerFb.items.add(FlexItem(*m_helpButton.get())
		.withAlignSelf(FlexItem::AlignSelf::flexStart)
		.withWidth(headerHeight)
		.withHeight(headerHeight));

	if (IsActive())
	{
		fb.items.add(
		FlexItem(headerFb)
			.withHeight(headerHeight)
			.withMargin(FlexItem::Margin(headerMargin, headerMargin, headerMargin, headerMargin)));

		// Add all the componentes that are flagged to be included in layouting
		for (auto const& component : m_components)
		{
			auto includeInLayout = component.second._includeInLayout;
			auto itemVerticalSpan = component.second._verticalSpan;
			auto flexItemHeight = (itemHeight * itemVerticalSpan) + (2 * itemMargin * (itemVerticalSpan - 1));
			if (includeInLayout)
			{
				fb.items.add(FlexItem(*component.first.get())
					.withHeight(flexItemHeight)
					.withMaxWidth(m_layoutItemWidth)
					.withMargin(FlexItem::Margin(itemMargin, itemMargin, itemMargin, 130 + itemMargin)));
				itemCount += itemVerticalSpan;
			}
		}
	}

	// Set the accumulated required size of the contents as new component size

	auto totalActiveToggleHeight = static_cast<int>(m_hasActiveToggle ? (activeToggleHeight + (2 * activeToggleMargin)) : 0);
	auto totalHeaderHeight = static_cast<int>(headerHeight + (2 * headerMargin));
	auto totalItemsHeight = static_cast<int>((itemHeight + (2 * itemMargin)) * itemCount);

	auto totalHeight = static_cast<int>(itemMargin);
	if (IsActive())
		totalHeight += totalHeaderHeight;
	totalHeight += totalActiveToggleHeight;
	totalHeight += totalItemsHeight;

	auto bounds = getLocalBounds();
	bounds.setHeight(totalHeight);
	setSize(getLocalBounds().getWidth(), bounds.getHeight());

	// Trigger the actual layouting based on the calculated bounds
	fb.performLayout(bounds);

//#ifdef DEBUG
//	DBG(getName() + "::" + __FUNCTION__ + " " + String(bounds.getWidth()) + "x" + String(bounds.getHeight()));
//#endif
}

/**
 * Reimplemented method to handle changed look and feel data.
 * This makes shure the help buttons' svg images are colored correctly.
 */
void HeaderWithElmListComponent::lookAndFeelChanged()
{
	// first forward the call to base implementation
	Component::lookAndFeelChanged();

	// all following is about the help button - if it does not exist, do not continue!
	if (!m_helpButton)
		return;

	// Update drawable button images with updated lookAndFeel colours
	UpdateDrawableButtonImages(m_helpButton, BinaryData::help24px_svg, &getLookAndFeel());

	// set drawable button background colour according to the section enabled state
	if (m_toggleState)
		m_helpButton->setColour(DrawableButton::ColourIds::backgroundColourId, getLookAndFeel().findColour(TableListBox::backgroundColourId));
	else
		m_helpButton->setColour(DrawableButton::ColourIds::backgroundColourId, getLookAndFeel().findColour(TableListBox::backgroundColourId).darker());
}


} // namespace SpaConBridge
