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


#include "TableControlBarComponent.h"

#include "../../SpaConBridgeCommon.h"

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
TableControlBarComponent::TableControlBarComponent(bool canCollapse, bool canToggleSingleSelectionOnly, const String& componentName)
	: Component(componentName)
{
	m_canCollapse = canCollapse;
	m_singleSelectionOnlyTogglable = canToggleSingleSelectionOnly;

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

	// Create selection mode toggle button
	m_singleSelectionOnly = std::make_unique<DrawableButton>("singleSelOnly", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_singleSelectionOnly->setClickingTogglesState(true);
	m_singleSelectionOnly->addListener(this);
	m_singleSelectionOnly->setTooltip("Disable multiselection");
	addAndMakeVisible(m_singleSelectionOnly.get());

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
 * Helper method to get the allowingsingleselection state
 * @return	The internal allowingsingleselection state value
 */
bool TableControlBarComponent::IsSingleSelectionOnlyTogglable()
{
	return m_singleSelectionOnlyTogglable;
}

/**
 * Helper method to set the allowingsingleselection button to enabled/disabled
 * @param	enabled		Determines if the allowingsingleselection should be set to enabled or disabled
 */
void TableControlBarComponent::SetSingleSelectionOnlyTogglable(bool togglable)
{
	m_singleSelectionOnlyTogglable = togglable;

	resized();
}

/**
 * Helper method to get the toggle state of single selection only button
 * @return	True if only single selection is allowed, false if not
 */
bool TableControlBarComponent::IsSingleSelectionOnly()
{
	if (m_singleSelectionOnly)
		return m_singleSelectionOnly->getToggleState();
	else
		return false;
}

/**
 * Helper method to set the toggle state of single selection only button
 * @param	singleSelectionOnly		Determines if only single selection is allowed
 */
void TableControlBarComponent::SetSingleSelectionOnly(bool singleSelectionOnly)
{
	if (m_singleSelectionOnly)
		m_singleSelectionOnly->setToggleState(singleSelectionOnly, dontSendNotification);
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

	// Update drawable button images with updated lookAndFeel colours
	UpdateDrawableButtonImages(m_addInstance, BinaryData::add24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_removeInstance, BinaryData::remove24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_addMultipleInstances, BinaryData::add_batch24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_singleSelectionOnly, BinaryData::rule_one24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_selectAll, BinaryData::rule_checked24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_selectNone, BinaryData::rule_unchecked24px_svg, &getLookAndFeel());

	UpdateCollapsedButton();
}

/**
 * Helper method to recreate svg drawable contents of collapsed button
 * depending on the collapsed and layoutdirection state of the control bar.
 */
void TableControlBarComponent::UpdateCollapsedButton()
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

		// Update drawable button images with updated lookAndFeel colours
		UpdateDrawableButtonImages(m_toggleCollapse, imageName, &getLookAndFeel());
	}
}

/**
 * Reimplemented from Button::Listener, gets called whenever the buttons are clicked.
 * @param button	The button which has been clicked.
 */
void TableControlBarComponent::buttonClicked(Button* button)
{
	if (nullptr == button)
	{
		return;
	}
	else if (button == m_singleSelectionOnly.get())
	{
		if (onSingleSelectionOnlyClick)
			onSingleSelectionOnlyClick(m_singleSelectionOnly->getToggleState());

		m_selectAll->setEnabled(!m_singleSelectionOnly->getToggleState());
		m_selectNone->setEnabled(!m_singleSelectionOnly->getToggleState());
	}
	else if (button == m_selectAll.get())
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
		// collect required widths for items...
		auto itemWidthSymMargin = 2 + 25 + 2;
		auto itemWidthAsymMargin = 2 + 25 + 4;
		auto sliderWidthAsymMargin = 2 + 90 + 4;
		auto minRequiredWidth = 0;
		if (m_canCollapse && m_toggleCollapse)
			minRequiredWidth += itemWidthSymMargin + itemWidthAsymMargin;
		else
			minRequiredWidth += itemWidthAsymMargin;
		if (IsSingleSelectionOnlyTogglable())
			minRequiredWidth += 5 * itemWidthSymMargin + sliderWidthAsymMargin;
		else
			minRequiredWidth += 4 * itemWidthSymMargin + sliderWidthAsymMargin;
		// ...and decide on that basis what items cannot be shown
		auto sliderCanBeShown = true;
		auto selectCtlsCanBeShown = true;
		auto nothingCanBeShown = false;
		if (getWidth() <= minRequiredWidth)
		{
			sliderCanBeShown = false;
			if (getWidth() <= (minRequiredWidth - sliderWidthAsymMargin + 2))
			{
				selectCtlsCanBeShown = false;
				if (getWidth() <= (minRequiredWidth - sliderWidthAsymMargin - 2 * itemWidthSymMargin))
					nothingCanBeShown = true;
			}
		}

		// hide/show items tepending on the outcome above
		if (m_toggleCollapse)
			m_toggleCollapse->setVisible(!nothingCanBeShown);
		m_addInstance->setVisible(!nothingCanBeShown);
		m_addMultipleInstances->setVisible(!nothingCanBeShown);
		m_removeInstance->setVisible(!nothingCanBeShown);
		m_singleSelectionOnly->setVisible(!nothingCanBeShown);
		m_selectAll->setVisible(!nothingCanBeShown && selectCtlsCanBeShown);
		m_selectNone->setVisible(!nothingCanBeShown && selectCtlsCanBeShown);
		m_heightSlider->setVisible(!nothingCanBeShown && sliderCanBeShown);
		if (nothingCanBeShown)
			return;

		// perform the actual layouting
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

		mainFB.items.add(FlexItem(*m_addMultipleInstances.get()).withWidth(25).withMargin(FlexItem::Margin(2, 2, 3, 2)));
		mainFB.items.add(FlexItem(*m_removeInstance.get()).withWidth(25).withMargin(FlexItem::Margin(2, 2, 3, 2)));
		mainFB.items.add(FlexItem().withFlex(1).withHeight(30));
		if (IsSingleSelectionOnlyTogglable())
		{
			if (selectCtlsCanBeShown)
				mainFB.items.add(FlexItem(*m_singleSelectionOnly.get()).withWidth(25).withMargin(FlexItem::Margin(2, 2, 3, 2)));
			else
				mainFB.items.add(FlexItem(*m_singleSelectionOnly.get()).withWidth(25).withMargin(FlexItem::Margin(2, 4, 3, 2)));
		}
		if (selectCtlsCanBeShown)
		{
			mainFB.items.add(FlexItem(*m_selectAll.get()).withWidth(25).withMargin(FlexItem::Margin(2, 2, 3, 2)));
			if (sliderCanBeShown)
				mainFB.items.add(FlexItem(*m_selectNone.get()).withWidth(25).withMargin(FlexItem::Margin(2, 2, 3, 2)));
			else
				mainFB.items.add(FlexItem(*m_selectNone.get()).withWidth(25).withMargin(FlexItem::Margin(2, 4, 3, 2)));
		}
		if (sliderCanBeShown)
			mainFB.items.add(FlexItem(*m_heightSlider.get()).withWidth(90).withMargin(FlexItem::Margin(2, 4, 3, 2)));

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

		if (IsSingleSelectionOnlyTogglable())
			mainFB.items.addArray({
				FlexItem(*m_addMultipleInstances.get()).withHeight(25).withMargin(FlexItem::Margin(2, 2, 2, 3)),
				FlexItem(*m_removeInstance.get()).withHeight(25).withMargin(FlexItem::Margin(2, 2, 2, 3)),
				FlexItem().withFlex(1).withWidth(30),
				FlexItem(*m_singleSelectionOnly.get()).withHeight(25).withMargin(FlexItem::Margin(2, 2, 2, 3)),
				FlexItem(*m_selectAll.get()).withHeight(25).withMargin(FlexItem::Margin(2, 2, 2, 3)),
				FlexItem(*m_selectNone.get()).withHeight(25).withMargin(FlexItem::Margin(2, 2, 2, 3)),
				FlexItem(*m_heightSlider.get()).withHeight(90).withMargin(FlexItem::Margin(2, 2, 4, 3)),
				});
		else
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
