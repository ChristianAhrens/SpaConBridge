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

#include "../../RowHeightSlider.h"

namespace SpaConBridge
{


/**
 * TableControlBarComponent class provides a read-only slider that uses HorizontalBar Sliderstyle.
 */
class TableControlBarComponent	:	public Component,
									public Button::Listener,
									public RowHeightSlider::RowHeightListener
{
public:
	enum LayoutDirection
	{
		LD_Horizontal,
		LD_Vertical
	};
public:
	TableControlBarComponent(bool canCollapse = false, const String& componentName = String());
	~TableControlBarComponent() override;

	void SetLayoutDirection(LayoutDirection direction);
	void SetRemoveEnabled(bool enabled);
	void SetRowHeightSliderValue(int rowHeight);
	void SetCollapsed(bool collapsed);
	bool GetCollapsed();

	void UpdateCollapsedButton();

	//==============================================================================
	void lookAndFeelChanged() override;

	//==============================================================================
	void buttonClicked(Button*) override;

	//==========================================================================
	void rowHeightChanged(int rowHeight) override;

	//==============================================================================
	std::function<void(bool collapsed)> onCollapsClick;
	std::function<void()> onAddClick;
	std::function<void()> onAddMultipleClick;
	std::function<void()> onRemoveClick;
	std::function<void()> onSelectAllClick;
	std::function<void()> onSelectNoneClick;
	std::function<void(int height)> onHeightChanged;

protected:
	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

private:
	//==============================================================================
	std::unique_ptr<DrawableButton>		m_toggleCollapse;		/**> Button to toggle the table expand/collapse state. */
	std::unique_ptr<DrawableButton>		m_addInstance;			/**> Button to add a processor instance */
	std::unique_ptr<DrawableButton>		m_removeInstance;		/**> Button to remove the selected processor instance */
	std::unique_ptr<DrawableButton>		m_addMultipleInstances;	/**> Button to add multiple processor instances */
	std::unique_ptr<RowHeightSlider>	m_heightSlider;			/**> Special slider component instance to modify table row height. */
	std::unique_ptr<DrawableButton>		m_selectAll;			/**> Select all rows button. */
	std::unique_ptr<DrawableButton>		m_selectNone;			/**> Select no rows button. */

	bool			m_canCollapse{ false };				/**> Bool flag to indicate if this table control bar instance shall allow toggling collapse state or not. */
	bool			m_collapsed{ false };				/**> Bool flag holding the current collapsed state or false if this instance is not able to collapse. */
	LayoutDirection	m_layoutDirection{ LD_Horizontal };	/**> Determines if elements should be arranged in a vertical or horizontal layout. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TableControlBarComponent)
};


} // namespace SpaConBridge
