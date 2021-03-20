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


#include "HeaderWithElmListComponent.h"

#include "../../../Controller.h"
#include "../../../LookAndFeel.h"

#include <Image_utils.h>


namespace SpaConBridge
{


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
		auto dontDelete = !component.second.second;
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
	m_activeToggleLabel->setText("Use " + headerText, dontSendNotification);

	auto font = m_headerLabel->getFont();
	font.setBold(true);
	m_headerLabel->setFont(font);
	m_headerLabel->setText(headerText + " Settings", dontSendNotification);
}

/**
 * Methdo to add a component to internal list of components that shall be layouted vertically.
 * @param compo	The component to add.
 * @param includeInLayout	Bool flag that can indicate if the component shall be made visible in this components 
 *							context but not layouted. (e.g. a lable that is already attached to another component)
 * @param takeOwnerShip		Bool flag that indicates if ownership of the given component shall be taken.
 */
void HeaderWithElmListComponent::addComponent(Component* compo, bool includeInLayout, bool takeOwnership)
{
	if (!compo)
		return;

	addAndMakeVisible(compo);
	m_components.push_back(std::make_pair(std::unique_ptr<Component>(compo), std::make_pair(includeInLayout, takeOwnership)));

	compo->setEnabled(m_toggleState);
}

/**
 * Reimplemented paint method from Component that uses colours from TableListBox to give
 * the user a similar impression as when using a table.
 */
void HeaderWithElmListComponent::paint(Graphics& g)
{
	auto w = getWidth();
	auto h = getHeight();

	if (m_toggleState)
		g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId));
	else
		g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId).darker());
	g.fillRect(0, 0, w, h);

	g.setColour(getLookAndFeel().findColour(TableListBox::outlineColourId));
	g.drawRect(0, 0, w, h);
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
	fb.items.add(
		FlexItem(headerFb)
			.withHeight(headerHeight)
			.withMargin(FlexItem::Margin(headerMargin, headerMargin, headerMargin, headerMargin)));
	// Add all the componentes that are flagged to be included in layouting
	for (auto const& component : m_components)
	{
		auto includeInLayout = component.second.first;
		if (includeInLayout)
		{
			fb.items.add(FlexItem(*component.first.get())
				.withHeight(itemHeight)
				.withMaxWidth(m_layoutItemWidth)
				.withMargin(FlexItem::Margin(itemMargin, itemMargin, itemMargin, 130 + itemMargin)));
			itemCount++;
		}
	}

	// Set the accumulated required size of the contents as new component size
	auto bounds = getLocalBounds();
	auto totalActiveToggleHeight = static_cast<int>(m_hasActiveToggle ? (activeToggleHeight + (2 * activeToggleMargin)) : 0);
	auto totalHeaderHeight = static_cast<int>(headerHeight + (2 * headerMargin));
	auto totalItemsHeight = static_cast<int>((itemHeight + (2 * itemMargin)) * itemCount);
	bounds.setHeight(totalActiveToggleHeight + totalHeaderHeight + totalItemsHeight + static_cast<int>(itemMargin));
	setSize(bounds.getWidth(), bounds.getHeight());

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

	// create the required button drawable images based on lookandfeel colours
	String imageName = BinaryData::help24px_svg;
	std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (dblookAndFeel)
	{
		JUCEAppBasics::Image_utils::getDrawableButtonImages(imageName, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

		m_helpButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
	}

	// set drawable button background colour according to the section enabled state
	if (m_toggleState)
		m_helpButton->setColour(DrawableButton::ColourIds::backgroundColourId, getLookAndFeel().findColour(TableListBox::backgroundColourId));
	else
		m_helpButton->setColour(DrawableButton::ColourIds::backgroundColourId, getLookAndFeel().findColour(TableListBox::backgroundColourId).darker());
}


} // namespace SpaConBridge
