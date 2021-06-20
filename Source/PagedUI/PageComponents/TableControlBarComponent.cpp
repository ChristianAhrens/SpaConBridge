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


#include "TableControlBarComponent.h"

#include "../../LookAndFeel.h"

#include <Image_utils.h>


namespace SpaConBridge
{


/*
===============================================================================
 Class TableControlBarComponent
===============================================================================
*/

/**
 * Object constructor.
 */
TableControlBarComponent::TableControlBarComponent(bool canCollapse, const String& componentName)
	: Component(componentName)
{
	m_canCollapse = canCollapse;

	// optional Collapse Button
	if (m_canCollapse)
	{
		m_toggleCollapse = std::make_unique<DrawableButton>("collapse", DrawableButton::ButtonStyle::ImageFitted);
		m_toggleCollapse->setClickingTogglesState(false);
		m_toggleCollapse->addListener(this);
		addAndMakeVisible(m_toggleCollapse.get());
	}

	// Add/Remove Buttons
	m_addInstance = std::make_unique<DrawableButton>("add", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_addInstance->setClickingTogglesState(false);
	m_addInstance->addListener(this);
	m_addInstance->setTooltip("Add row");
	addAndMakeVisible(m_addInstance.get());
	m_removeInstance = std::make_unique<DrawableButton>("remove", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_removeInstance->setClickingTogglesState(false);
	m_removeInstance->setEnabled(false);
	m_removeInstance->addListener(this);
	m_removeInstance->setTooltip("Remove selected row(s)");
	addAndMakeVisible(m_removeInstance.get());
	m_addMultipleInstances = std::make_unique<DrawableButton>("add_batch", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_addMultipleInstances->setClickingTogglesState(false);
	m_addMultipleInstances->addListener(this);
	m_addMultipleInstances->setTooltip("Add multiple rows");
	addAndMakeVisible(m_addMultipleInstances.get());

	// Create quick selection buttons
	m_selectAll = std::make_unique<DrawableButton>("all", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_selectAll->setClickingTogglesState(false);
	m_selectAll->addListener(this);
	m_selectAll->setTooltip("Select all rows");
	addAndMakeVisible(m_selectAll.get());

	m_selectNone = std::make_unique<DrawableButton>("none", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_selectNone->setClickingTogglesState(false);
	m_selectNone->addListener(this);
	m_selectNone->setTooltip("Deselect all rows");
	addAndMakeVisible(m_selectNone.get());

	// row height slider
	m_heightSlider = std::make_unique<RowHeightSlider>("rowHeight");
	m_heightSlider->SetListener(this);
	addAndMakeVisible(m_heightSlider.get());

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
	if (m_layoutDirection == direction)
		return;

	m_layoutDirection = direction;

	UpdateCollapsedButton();

	resized();
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
 * Helper method to set the current value of row height slider
 * @param	rowHeight		The row height value to set as current
 */
void TableControlBarComponent::SetRowHeightSliderValue(int rowHeight)
{
	if (m_heightSlider)
		m_heightSlider->SetSliderValue(rowHeight);
}

/**
 * Helper method to set the remove button to enabled/disabled
 * @param	enabled		Determines if the remove button should be set to enabled or disabled
 */
void TableControlBarComponent::SetCollapsed(bool collapsed)
{
	if (!m_canCollapse)
		return;

	m_collapsed = collapsed;
	UpdateCollapsedButton();
}

/**
 * Helper method to get the collapsed state
 * @return	The internal collapsed state value (True for collapsed, false for not collapsed)
 */
bool TableControlBarComponent::GetCollapsed()
{
	return m_collapsed;
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
	std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (dblookAndFeel)
	{
		// add images
		JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::add24px_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
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
		JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::remove24px_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

		m_removeInstance->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());

		// batch-add images
		JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::add_batch24dp_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

		m_addMultipleInstances->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());

		// select all images
		JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::rule_checked24px_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

		m_selectAll->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());

		// select none images
		JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::rule_unchecked24px_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

		m_selectNone->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());

		UpdateCollapsedButton();
	}
}

/**
 * Helper method to recreate svg drawable contents of collapsed button
 * depending on the collapsed and layoutdirection state of the control bar.
 */
void TableControlBarComponent::UpdateCollapsedButton()
{
	// create the required button drawable images based on lookandfeel colours
	std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (dblookAndFeel)
	{
		if (m_canCollapse && m_toggleCollapse)
		{
			auto imageName = String();

			if (m_layoutDirection == LD_Horizontal)
			{
				if (m_collapsed)
					imageName = BinaryData::keyboard_arrow_right24px_svg;
				else
					imageName = BinaryData::keyboard_arrow_up24px_svg;
			}
			else if (m_layoutDirection == LD_Vertical)
			{
				if (m_collapsed)
					imageName = BinaryData::keyboard_arrow_down24px_svg;
				else
					imageName = BinaryData::keyboard_arrow_right24px_svg;
			}

			// collapse/expand images
			JUCEAppBasics::Image_utils::getDrawableButtonImages(imageName, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
				dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
				dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
				dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
				dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
				dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
				dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
				dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
				dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

			m_toggleCollapse->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
		}
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
	else if (button == m_addMultipleInstances.get())
	{
		if (onAddMultipleClick)
			onAddMultipleClick();
	}
	else if (button == m_toggleCollapse.get())
	{
		m_collapsed = !m_collapsed;

		UpdateCollapsedButton();

		if (onCollapsClick)
			onCollapsClick(m_collapsed);
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

		if (m_canCollapse && m_toggleCollapse)
		{
			mainFB.items.addArray({
				FlexItem(*m_toggleCollapse.get()).withWidth(25).withMargin(FlexItem::Margin(2, 2, 3, 4)),
				FlexItem(*m_addInstance.get()).withWidth(25).withMargin(FlexItem::Margin(2, 2, 3, 2)),
				});
		}
		else
			mainFB.items.add(FlexItem(*m_addInstance.get()).withWidth(25).withMargin(FlexItem::Margin(2, 2, 3, 4)));

		mainFB.items.addArray({
			FlexItem(*m_addMultipleInstances.get()).withWidth(25).withMargin(FlexItem::Margin(2, 2, 3, 2)),
			FlexItem(*m_removeInstance.get()).withWidth(25).withMargin(FlexItem::Margin(2, 2, 3, 2)),
			FlexItem().withFlex(1).withHeight(30),
			FlexItem(*m_selectAll.get()).withWidth(25).withMargin(FlexItem::Margin(2, 2, 3, 2)),
			FlexItem(*m_selectNone.get()).withWidth(25).withMargin(FlexItem::Margin(2, 2, 3, 2)),
			FlexItem(*m_heightSlider.get()).withWidth(90).withMargin(FlexItem::Margin(2, 4, 3, 2)),
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

		if (m_canCollapse && m_toggleCollapse)
		{
			mainFB.items.addArray({
				FlexItem(*m_toggleCollapse.get()).withHeight(25).withMargin(FlexItem::Margin(4, 2, 2, 3)),
				FlexItem(*m_addInstance.get()).withHeight(25).withMargin(FlexItem::Margin(2, 2, 2, 3)),
				});
		}
		else
			mainFB.items.add(FlexItem(*m_addInstance.get()).withHeight(25).withMargin(FlexItem::Margin(4, 2, 2, 3)));

		mainFB.items.addArray({
			FlexItem(*m_addMultipleInstances.get()).withHeight(25).withMargin(FlexItem::Margin(2, 2, 2, 3)),
			FlexItem(*m_removeInstance.get()).withHeight(25).withMargin(FlexItem::Margin(2, 2, 2, 3)),
			FlexItem().withFlex(1).withWidth(30),
			FlexItem(*m_selectAll.get()).withHeight(25).withMargin(FlexItem::Margin(2, 2, 2, 3)),
			FlexItem(*m_selectNone.get()).withHeight(25).withMargin(FlexItem::Margin(2, 2, 2, 3)),
			FlexItem(*m_heightSlider.get()).withHeight(90).withMargin(FlexItem::Margin(2, 2, 4, 3)),
			});

		mainFB.performLayout(bounds.reduced(1, 0));
	}
}


} // namespace SpaConBridge
