/* Copyright (c) 2020-2021, Christian Ahrens
 *
 * This file is part of SoundscapeBridgeApp <https://github.com/ChristianAhrens/SoundscapeBridgeApp>
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


#include "TableControlBarComponent.h"

#include "../../LookAndFeel.h"

#include <Image_utils.h>


namespace SoundscapeBridgeApp
{


/*
===============================================================================
 Class TableControlBarComponent
===============================================================================
*/

/**
 * Object constructor.
 */
TableControlBarComponent::TableControlBarComponent(const String& componentName)
	: Component(componentName)
{
	m_layoutDirection = LD_Horizontal;

	// Add/Remove Buttons
	m_addInstance = std::make_unique<DrawableButton>("add", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_addInstance->setClickingTogglesState(false);
	m_addInstance->addListener(this);
	addAndMakeVisible(m_addInstance.get());
	m_removeInstance = std::make_unique<DrawableButton>("remove", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_removeInstance->setClickingTogglesState(false);
	m_removeInstance->setEnabled(false);
	m_removeInstance->addListener(this);
	addAndMakeVisible(m_removeInstance.get());

	// row height slider
	m_heightSlider = std::make_unique<RowHeightSlider>("rowHeight");
	m_heightSlider->SetSliderRange(33, 66, 11);
	m_heightSlider->SetListener(this);
	addAndMakeVisible(m_heightSlider.get());

	// Create quick selection buttons
	m_selectLabel = std::make_unique<Label>("Select:", "Select:");
	m_selectLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(m_selectLabel.get());

	m_selectAll = std::make_unique<TextButton>();
	m_selectAll->setClickingTogglesState(false);
	m_selectAll->setButtonText("All");
	m_selectAll->setEnabled(true);
	m_selectAll->addListener(this);
	addAndMakeVisible(m_selectAll.get());

	m_selectNone = std::make_unique<TextButton>();
	m_selectNone->setClickingTogglesState(false);
	m_selectNone->setButtonText("None");
	m_selectNone->setEnabled(true);
	m_selectNone->addListener(this);
	addAndMakeVisible(m_selectNone.get());

	// trigger lookandfeel update
	lookAndFeelChanged();
}

/**
 * Object destructor.
 */
TableControlBarComponent::~TableControlBarComponent()
{
}

/**
 * Setter for the layout direction member
 * @param	direction	The new direction to set as value for layout direction member
 */
void TableControlBarComponent::SetLayoutDirection(LayoutDirection direction)
{
	m_layoutDirection = direction;
}

/**
 * Helper method to set the remove button to enabled/disabled
 * @param	enabled		Determines if the remove button should be set to enabled or disabled
 */
void TableControlBarComponent::SetRemoveEnabled(bool enabled)
{
	if (m_removeInstance)
		m_removeInstance->setEnabled(enabled);
}

/**
 * Reimplemented method to handle changed look and feel data.
 * This makes shure the add/remove buttons' svg images are colored correctly.
 */
void TableControlBarComponent::lookAndFeelChanged()
{
	// first forward the call to base implementation
	Component::lookAndFeelChanged();

	// create the required button drawable images based on lookandfeel colours
	String addImageName = BinaryData::add24px_svg;
	String removeImageName = BinaryData::remove24px_svg;
	std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (dblookAndFeel)
	{
		// add images
		JUCEAppBasics::Image_utils::getDrawableButtonImages(addImageName, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

		m_addInstance->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());

		// remove images
		JUCEAppBasics::Image_utils::getDrawableButtonImages(removeImageName, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

		m_removeInstance->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
	}
}

/**
 * Reimplemented from Button::Listener, gets called whenever the buttons are clicked.
 * @param button	The button which has been clicked.
 */
void TableControlBarComponent::buttonClicked(Button* button)
{
	if (button == m_selectAll.get())
	{
		if (onSelectAllClick)
			onSelectAllClick();
	}
	else if (button == m_selectNone.get())
	{
		if (onSelectNoneClick)
			onSelectNoneClick();
	}
	else if (button == m_addInstance.get())
	{
		if (onAddClick)
			onAddClick();
	}
	else if (button == m_removeInstance.get())
	{
		if (onRemoveClick)
			onRemoveClick();
	}
}

/**
 * Reimplemented to handle the updated rowheightslider row height value changes
 * @param	rowHeight	The new configured row height to set as new row height value into table memeber
 */
void TableControlBarComponent::rowHeightChanged(int rowHeight)
{
	if (onHeightChanged)
		onHeightChanged(rowHeight);
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void TableControlBarComponent::paint(Graphics& g)
{
	auto bounds = getLocalBounds();

	// background
	g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId));
	g.fillRect(bounds);

	g.setColour(getLookAndFeel().findColour(TableListBox::outlineColourId));
	if (m_layoutDirection == LD_Horizontal)
	{
		// frame (left, bottom, right - not top)
		auto leftLine = juce::Line<float>(bounds.getTopLeft().toFloat(), bounds.getBottomLeft().toFloat());
		auto bottomLine = juce::Line<float>(bounds.getBottomLeft().toFloat(), bounds.getBottomRight().toFloat());
		auto rightLine = juce::Line<float>(bounds.getTopRight().toFloat(), bounds.getBottomRight().toFloat());
		g.drawLine(leftLine, 2);
		g.drawLine(bottomLine, 2);
		g.drawLine(rightLine, 2);
	}
	else if (m_layoutDirection == LD_Vertical)
	{
		// frame (top, left, bottom - not right)
		// frame (left, bottom, right - not top)
		auto leftLine = juce::Line<float>(bounds.getTopLeft().toFloat(), bounds.getBottomLeft().toFloat());
		auto topLine = juce::Line<float>(bounds.getTopLeft().toFloat(), bounds.getTopRight().toFloat());
		auto bottomLine = juce::Line<float>(bounds.getBottomLeft().toFloat(), bounds.getBottomRight().toFloat());
		g.drawLine(leftLine, 2);
		g.drawLine(topLine, 2);
		g.drawLine(bottomLine, 2);
	}
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void TableControlBarComponent::resized()
{
	auto bounds = getLocalBounds();
	if (m_layoutDirection == LD_Horizontal)
	{
		// flexbox for bottom buttons
		FlexBox mainFB;
		mainFB.flexDirection = FlexBox::Direction::row;
		mainFB.justifyContent = FlexBox::JustifyContent::center;
		mainFB.alignContent = FlexBox::AlignContent::center;
		mainFB.items.addArray({
			FlexItem(*m_addInstance.get()).withFlex(1).withMaxWidth(30).withMargin(FlexItem::Margin(2, 2, 3, 4)),
			FlexItem(*m_removeInstance.get()).withFlex(1).withMaxWidth(30).withMargin(FlexItem::Margin(2, 2, 3, 2)),
			FlexItem().withFlex(2).withHeight(30),
			FlexItem(*m_heightSlider.get()).withFlex(1).withMaxWidth(100).withMargin(FlexItem::Margin(2, 2, 3, 2)),
			FlexItem(*m_selectLabel.get()).withFlex(1).withMaxWidth(80),
			FlexItem(*m_selectAll.get()).withFlex(1).withMaxWidth(40).withMargin(FlexItem::Margin(2, 2, 3, 2)),
			FlexItem(*m_selectNone.get()).withFlex(1).withMaxWidth(46).withMargin(FlexItem::Margin(2, 4, 3, 2)),
			});

		mainFB.performLayout(bounds.reduced(0, 1));
	}
	else if (m_layoutDirection == LD_Vertical)
	{
		// flexbox for bottom buttons
		FlexBox mainFB;
		mainFB.flexDirection = FlexBox::Direction::column;
		mainFB.justifyContent = FlexBox::JustifyContent::center;
		mainFB.alignContent = FlexBox::AlignContent::center;
		mainFB.items.addArray({
			FlexItem(*m_addInstance.get()).withFlex(1).withMaxHeight(30).withMargin(FlexItem::Margin(2, 2, 3, 4)),
			FlexItem(*m_removeInstance.get()).withFlex(1).withMaxHeight(30).withMargin(FlexItem::Margin(2, 2, 3, 2)),
			FlexItem().withFlex(2).withWidth(30),
			FlexItem(*m_heightSlider.get()).withFlex(1).withMaxHeight(100).withMargin(FlexItem::Margin(2, 2, 3, 2)),
			FlexItem(*m_selectLabel.get()).withFlex(1).withMaxHeight(80),
			FlexItem(*m_selectAll.get()).withFlex(1).withMaxHeight(40).withMargin(FlexItem::Margin(2, 2, 3, 2)),
			FlexItem(*m_selectNone.get()).withFlex(1).withMaxHeight(46).withMargin(FlexItem::Margin(2, 4, 3, 2)),
			});

		mainFB.performLayout(bounds.reduced(1, 0));
	}
}


} // namespace SoundscapeBridgeApp
