/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file was originally part of the Soundscape VST, AU, and AAX Plug-in and now in a derived version is part of SpaConBridge.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY d&b audiotechnik GmbH & Co. KG "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================
*/


#include "TableModelComponent.h"

#include "TableControlBarComponent.h"
#include "TableEditorComponents.h"
#include "BridgingAwareTableHeaderComponent.h"

#include "../../CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"
#include "../../CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessorEditor.h"
#include "../../CustomAudioProcessors/MatrixInputProcessor/MatrixInputProcessor.h"
#include "../../CustomAudioProcessors/MatrixInputProcessor/MatrixInputProcessorEditor.h"
#include "../../CustomAudioProcessors/MatrixOutputProcessor/MatrixOutputProcessor.h"
#include "../../CustomAudioProcessors/MatrixOutputProcessor/MatrixOutputProcessorEditor.h"

#include "../../SurfaceSlider.h"
#include "../../Controller.h"
#include "../../LookAndFeel.h"

#include <Image_utils.h>


namespace SpaConBridge
{


/*
===============================================================================
 Class TableModelComponent
===============================================================================
*/

/**
 * Class constructor.
 */
	TableModelComponent::TableModelComponent(ControlBarPosition pos, bool tableCanCollapse)
	{
		// Create our table component and add it to this component.
		m_table = std::make_unique<TableListBox>();
		addAndMakeVisible(m_table.get());
		m_tableControlBar = std::make_unique<TableControlBarComponent>(tableCanCollapse);
		addAndMakeVisible(m_tableControlBar.get());

		m_tableControlBar->onAddClick = [=] { onAddProcessor(); };
		m_tableControlBar->onRemoveClick = [=] { onRemoveProcessor(); };
		m_tableControlBar->onSelectAllClick = [=] { onSelectAllProcessors(); };
		m_tableControlBar->onSelectNoneClick = [=] { onDeselectAllProcessors(); };
		m_tableControlBar->onHeightChanged = [=](int height) { onRowHeightSlided(height); };
		m_tableControlBar->onCollapsClick = [=](bool collapsed) { onCollapseToggled(collapsed); };

	SetControlBarPosition(pos);
}

/**
 * Class destructor.
 */
TableModelComponent::~TableModelComponent()
{
}

/**
* Helper method to set the internal table type identifier.
* @param	tt	The table type to set to internal member.
*/
void TableModelComponent::SetTableType(TableType tt)
{
	m_tableType = tt;
}

/**
* Helper method to get the internal table type identifier.
* @return	The internal table type value.
*/
TableType TableModelComponent::GetTableType()
{
	return m_tableType;
}

/**
* Helper method to set the model to internal table component.
* @param	model	The model object to set in table object.
*/
void TableModelComponent::SetModel(TableListBoxModel* model)
{
	if (m_table)
		m_table->setModel(model);
}

/**
 * Setter for the member defining where the table control bar shall be positioned when resizing
 * @param	pos	The position relative to the table component where to put the control bar.
 */
void TableModelComponent::SetControlBarPosition(ControlBarPosition pos)
{
	m_controlBarPosition = pos;

	if (m_tableControlBar)
	{
		if (pos == CBP_Bottom || pos == CBP_Top)
			m_tableControlBar->SetLayoutDirection(TableControlBarComponent::LD_Horizontal);
		else if (pos == CBP_Left || pos == CBP_Right)
			m_tableControlBar->SetLayoutDirection(TableControlBarComponent::LD_Vertical);
	}
}

/**
 * Get the ID of the procssor instance corresponding to the given table row number.
 * @param rowNumber	The desired row number (starts at 0).
 * @return	The ID of the procssor instance at that row number, if any.
 */
juce::int32 TableModelComponent::GetProcessorIdForRow(int rowNumber) const
{
	if ((unsigned int)rowNumber > (m_processorIds.size() - 1) || m_processorIds.empty())
	{
		jassertfalse; // Unexpected row number!
		return 0;
	}

	return m_processorIds.at(rowNumber);
}

/**
 * Get the IDs of the procssor instances corresponding to the given table row numbers.
 * @param rowNumbers	A list of desired row numbers.
 * @return	A list of IDs of the procssor instances at those rows.
 */
std::vector<juce::int32> TableModelComponent::GetProcessorIdsForRows(const std::vector<int>& rowNumbers) const
{
	std::vector<juce::int32> ids;
	ids.reserve(rowNumbers.size());
	for (auto const& rowNumber : rowNumbers)
		ids.push_back(GetProcessorIdForRow(rowNumber));

	return ids;
}

/**
 * Get the table row number to the given processorId
 * @param processorId	The id of the processor to get the corresp. row for.
 * @return	The table row of the processor.
 */
int TableModelComponent::GetRowForProcessorId(juce::int32 processorId) const
{
	auto processorIdIter = std::find(m_processorIds.begin(), m_processorIds.end(), processorId);
	if (processorIdIter == m_processorIds.end())
	{
		jassertfalse;
		return -1;
	}

	return static_cast<int>(processorIdIter - m_processorIds.begin());
}

/**
 * Get the table row numbers to the given processorIds
 * @param processorIds	A list of desired processorIds.
 * @return	A list of table rows corresp. to the given processorIds.
 */
std::vector<int> TableModelComponent::GetRowsForProcessorIds(const std::vector<juce::int32>& processorIds) const
{
	std::vector<int> rows;
	rows.reserve(processorIds.size());
	for (auto const& processorId : processorIds)
	{
		auto rowNumber = GetRowForProcessorId(processorId);
		if (rowNumber >= 0)
			rows.push_back(rowNumber);
	}

	return rows;
}

/**
 * Helper method to get the row height of internal tableListBox member.
 *
 * @return	The row height of internal table member
 */
int TableModelComponent::GetRowHeight()
{
	// set the new row height to tableListBox member 
	if (m_table)
		return m_table->getRowHeight();
	else
		return 0;
}

/**
 * Helper method to set a new row height to internal tableListBox member
 * and at the same time trigger resizing of the container component
 * 
 * @param	rowHeight	The new height value to set as row height.
 */
void TableModelComponent::SetRowHeight(int rowHeight)
{
	// set the new row height to tableListBox member 
	if (m_table)
		m_table->setRowHeight(rowHeight);

	// set the new row height to rowheight slider member 
	if (m_tableControlBar)
		m_tableControlBar->SetRowHeightSliderValue(rowHeight);

	// trigger overall resizing
	resized();
}

/**
 * Helper method to get the collapsed state of the internal table & control bar components.
 *
 * @return	True if collapsed, false if not
 */
bool TableModelComponent::IsCollapsed()
{
	return m_tableControlBar->GetCollapsed();
}

/**
 * Helper method to set a new collapsed state of the internal table & control bar components
 * and at the same time trigger resizing of the container component
 *
 * @param	collapsed	The new collapsed state of the internal table & control bar components
 */
void TableModelComponent::SetCollapsed(bool collapsed)
{
	m_tableControlBar->SetCollapsed(collapsed);

	onCollapseToggled(collapsed);

	// trigger overall resizing
	resized();
}

/**
 * Get the list of rows which are currently selected on the table.
 * @return	A std::vector containing all selected row numbers.
 */
std::vector<int> TableModelComponent::GetSelectedRows() const
{
	std::vector<int> selectedRows;

	if (m_table)
	{
		auto sr = m_table->getSelectedRows();
		selectedRows.reserve(sr.size());
		for (int i = 0; i < sr.size(); ++i)
			selectedRows.push_back(sr[i]);
	}

	return selectedRows;
}

/**
 * Set the list of rows which are currently selected on the table.
 * @param rows	The std::vector containing all row numbers to be selected.
 */
void TableModelComponent::SetSelectedRows(const std::vector<int>& rows)
{
	if (m_table)
	{
		m_table->deselectAllRows();

		for (auto const& row : rows)
			m_table->selectRow(row, true, false);
	}
}

/**
 * Select all (or none) of the rows on the table.
 * @param all	True to select all rows. False to de-select all (clear selection).
 */
void TableModelComponent::SelectAllRows(bool all)
{
	if (m_table)
	{
		if (all)
			m_table->selectRangeOfRows(0, m_table->getNumRows(), true /* Do not scroll */);
		else
			m_table->deselectAllRows();
	}
}

/**
 * Helper sorting function used by std::sort(). This version is used to sort by procssor's SoundobjectId.
 * @param pId1	Id of the first processor.
 * @param pId2	Id of the second processor.
 * @return	True if the first procssor's SoundobjectId is less than the second's.
 */
bool TableModelComponent::LessThanSoundobjectId(juce::int32 pId1, juce::int32 pId2)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	auto processorIds = ctrl->GetSoundobjectProcessorIds();
	auto maxProcessorIdIter = std::max_element(processorIds.begin(), processorIds.end());
	if (maxProcessorIdIter == processorIds.end())
		return false;
	auto maxProcessorId = *maxProcessorIdIter;
	if ((pId1 <= maxProcessorId) && (pId2 <= maxProcessorId))
	{
		auto p1 = ctrl->GetSoundobjectProcessor(pId1);
		auto p2 = ctrl->GetSoundobjectProcessor(pId2);
		if (p1 && p2)
			return (p1->GetSoundobjectId() < p2->GetSoundobjectId());
	}

	jassertfalse; // Index out of range!
	return false;
}

/**
 * Helper sorting function used by std::sort(). This version is used to sort by procssor's MatrixInputId.
 * @param pId1	Id of the first processor.
 * @param pId2	Id of the second processor.
 * @return	True if the first procssor's SoundobjectId is less than the second's.
 */
bool TableModelComponent::LessThanMatrixInputId(juce::int32 pId1, juce::int32 pId2)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	auto processorIds = ctrl->GetMatrixInputProcessorIds();
	auto maxProcessorIdIter = std::max_element(processorIds.begin(), processorIds.end());
	if (maxProcessorIdIter == processorIds.end())
		return false;
	auto maxProcessorId = *maxProcessorIdIter;
	if ((pId1 <= maxProcessorId) && (pId2 <= maxProcessorId))
	{
		auto p1 = ctrl->GetMatrixInputProcessor(pId1);
		auto p2 = ctrl->GetMatrixInputProcessor(pId2);
		if (p1 && p2)
			return (p1->GetMatrixInputId() < p2->GetMatrixInputId());
	}

	jassertfalse; // Index out of range!
	return false;
}

/**
 * Helper sorting function used by std::sort(). This version is used to sort by procssor's SoundobjectId.
 * @param pId1	Id of the first processor.
 * @param pId2	Id of the second processor.
 * @return	True if the first procssor's SoundobjectId is less than the second's.
 */
bool TableModelComponent::LessThanMatrixOutputId(juce::int32 pId1, juce::int32 pId2)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	auto processorIds = ctrl->GetMatrixOutputProcessorIds();
	auto maxProcessorIdIter = std::max_element(processorIds.begin(), processorIds.end());
	if (maxProcessorIdIter == processorIds.end())
		return false;
	auto maxProcessorId = *maxProcessorIdIter;
	if ((pId1 <= maxProcessorId) && (pId2 <= maxProcessorId))
	{
		auto p1 = ctrl->GetMatrixOutputProcessor(pId1);
		auto p2 = ctrl->GetMatrixOutputProcessor(pId2);
		if (p1 && p2)
			return (p1->GetMatrixOutputId() < p2->GetMatrixOutputId());
	}

	jassertfalse; // Index out of range!
	return false;
}

/**
 * Helper sorting function used by std::sort(). This version is used to sort by procssor's MappingId.
 * @param pId1	Id of the first processor.
 * @param pId2	Id of the second processor.
 * @return	True if the first MappingId is less than the second.
 */
bool TableModelComponent::LessThanMapping(juce::int32 pId1, juce::int32 pId2)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	auto processorIds = ctrl->GetSoundobjectProcessorIds();
	auto maxProcessorIdIter = std::max_element(processorIds.begin(), processorIds.end());
	if (maxProcessorIdIter == processorIds.end())
		return false;
	auto maxProcessorId = *maxProcessorIdIter;
	if ((pId1 <= maxProcessorId) && (pId2 <= maxProcessorId))
	{
		auto p1 = ctrl->GetSoundobjectProcessor(pId1);
		auto p2 = ctrl->GetSoundobjectProcessor(pId2);
		if (p1 && p2)
			return (p1->GetMappingId() < p2->GetMappingId());
	}

	jassertfalse; // Index out of range!
	return false;
}

/**
 * Helper sorting function used by std::sort(). This version is used to sort by procssor's ComsMode. 
 * @param pId1	Id of the first processor.
 * @param pId2	Id of the second processor.
 * @return	True if the first procssor's ComsMode is less than the second's.
 */
bool TableModelComponent::LessThanComsMode(juce::int32 pId1, juce::int32 pId2)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	auto processorIds = ctrl->GetSoundobjectProcessorIds();
	auto maxProcessorIdIter = std::max_element(processorIds.begin(), processorIds.end());
	if (maxProcessorIdIter == processorIds.end())
		return false;
	auto maxProcessorId = *maxProcessorIdIter;
	if ((pId1 <= maxProcessorId) && (pId2 <= maxProcessorId))
	{
		auto p1 = ctrl->GetSoundobjectProcessor(pId1);
		auto p2 = ctrl->GetSoundobjectProcessor(pId2);
		if (p1 && p2)
			return (p1->GetComsMode() < p2->GetComsMode());
	}

	jassertfalse; // Index out of range!
	return false;
}

/**
 * Helper sorting function used by std::sort(). This version is used to sort by mute states.
 * @param pId1	Id of the first processor.
 * @param pId2	Id of the second processor.
 * @return	True if the first procssor's mute states are 'muted' for more protocols than the second's.
 */
bool TableModelComponent::LessThanSoundobjectBridgingMute(juce::int32 pId1, juce::int32 pId2)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	// Comparing mutes does not make too much sense.
	// Nevertheless, to have some defined behaviour, we use the collected 
	// count of muted briding protocols of every soundsource for this.
	// (Nothing muted < some protocols muted < all protocols muted)
	auto processorIds = ctrl->GetSoundobjectProcessorIds();
	auto maxProcessorIdIter = std::max_element(processorIds.begin(), processorIds.end());
	if (maxProcessorIdIter == processorIds.end())
		return false;
	auto maxProcessorId = *maxProcessorIdIter;
	if ((pId1 <= maxProcessorId) && (pId2 <= maxProcessorId))
	{
		auto p1 = ctrl->GetSoundobjectProcessor(pId1);
		auto p2 = ctrl->GetSoundobjectProcessor(pId2);
		if (p1 && p2)
		{
			auto mutedProtocolCountP1 = 0;
			auto mutedProtocolCountP2 = 0;

			auto soundobjectId1 = p1->GetSoundobjectId();
			auto soundobjectId2 = p2->GetSoundobjectId();
			for (auto bridgingType : ProtocolBridgingTypes)
			{
				auto activeBridging = ctrl->GetActiveProtocolBridging();
				if ((activeBridging & bridgingType) == bridgingType)
				{
					mutedProtocolCountP1 += ctrl->GetMuteBridgingSoundobjectId(bridgingType, soundobjectId1) ? 1 : 0;
					mutedProtocolCountP2 += ctrl->GetMuteBridgingSoundobjectId(bridgingType, soundobjectId2) ? 1 : 0;
				}
			}

			return (mutedProtocolCountP1 < mutedProtocolCountP2);
		}
	}
	
	jassertfalse;
	return false;
}

/**
 * Helper sorting function used by std::sort(). This version is used to sort by mute states.
 * @param pId1	Id of the first processor.
 * @param pId2	Id of the second processor.
 * @return	True if the first procssor's mute states are 'muted' for more protocols than the second's.
 */
bool TableModelComponent::LessThanMatrixInputBridgingMute(juce::int32 pId1, juce::int32 pId2)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	// Comparing mutes does not make too much sense.
	// Nevertheless, to have some defined behaviour, we use the collected 
	// count of muted briding protocols of every soundsource for this.
	// (Nothing muted < some protocols muted < all protocols muted)
	auto processorIds = ctrl->GetMatrixInputProcessorIds();
	auto maxProcessorIdIter = std::max_element(processorIds.begin(), processorIds.end());
	if (maxProcessorIdIter == processorIds.end())
		return false;
	auto maxProcessorId = *maxProcessorIdIter;
	if ((pId1 <= maxProcessorId) && (pId2 <= maxProcessorId))
	{
		auto p1 = ctrl->GetMatrixInputProcessor(pId1);
		auto p2 = ctrl->GetMatrixInputProcessor(pId2);
		if (p1 && p2)
		{
			auto mutedProtocolCountP1 = 0;
			auto mutedProtocolCountP2 = 0;

			auto matrixInputId1 = p1->GetMatrixInputId();
			auto matrixInputId2 = p2->GetMatrixInputId();
			for (auto bridgingType : ProtocolBridgingTypes)
			{
				auto activeBridging = ctrl->GetActiveProtocolBridging();
				if ((activeBridging & bridgingType) == bridgingType)
				{
					mutedProtocolCountP1 += ctrl->GetMuteBridgingMatrixInputId(bridgingType, matrixInputId1) ? 1 : 0;
					mutedProtocolCountP2 += ctrl->GetMuteBridgingMatrixInputId(bridgingType, matrixInputId2) ? 1 : 0;
				}
			}

			return (mutedProtocolCountP1 < mutedProtocolCountP2);
		}
	}

	jassertfalse;
	return false;
}

/**
 * Helper sorting function used by std::sort(). This version is used to sort by mute states.
 * @param pId1	Id of the first processor.
 * @param pId2	Id of the second processor.
 * @return	True if the first procssor's mute states are 'muted' for more protocols than the second's.
 */
bool TableModelComponent::LessThanMatrixOutputBridgingMute(juce::int32 pId1, juce::int32 pId2)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return false;

	// Comparing mutes does not make too much sense.
	// Nevertheless, to have some defined behaviour, we use the collected 
	// count of muted briding protocols of every soundsource for this.
	// (Nothing muted < some protocols muted < all protocols muted)
	auto processorIds = ctrl->GetMatrixOutputProcessorIds();
	auto maxProcessorIdIter = std::max_element(processorIds.begin(), processorIds.end());
	if (maxProcessorIdIter == processorIds.end())
		return false;
	auto maxProcessorId = *maxProcessorIdIter;
	if ((pId1 <= maxProcessorId) && (pId2 <= maxProcessorId))
	{
		auto p1 = ctrl->GetMatrixOutputProcessor(pId1);
		auto p2 = ctrl->GetMatrixOutputProcessor(pId2);
		if (p1 && p2)
		{
			auto mutedProtocolCountP1 = 0;
			auto mutedProtocolCountP2 = 0;

			auto matrixOutputId1 = p1->GetMatrixOutputId();
			auto matrixOutputId2 = p2->GetMatrixOutputId();
			for (auto bridgingType : ProtocolBridgingTypes)
			{
				auto activeBridging = ctrl->GetActiveProtocolBridging();
				if ((activeBridging & bridgingType) == bridgingType)
				{
					mutedProtocolCountP1 += ctrl->GetMuteBridgingMatrixOutputId(bridgingType, matrixOutputId1) ? 1 : 0;
					mutedProtocolCountP2 += ctrl->GetMuteBridgingMatrixOutputId(bridgingType, matrixOutputId2) ? 1 : 0;
				}
			}

			return (mutedProtocolCountP1 < mutedProtocolCountP2);
		}
	}

	jassertfalse;
	return false;
}


/**
 * This can be overridden to react to the user double-clicking on a part of the list where there are no rows. 
 * @param event	Contains position and status information about a mouse event.
 */
void TableModelComponent::backgroundClicked(const MouseEvent &event)
{
	if (m_table)
	{
		// Clear selection
		m_table->deselectAllRows();
	}

	// Base class implementation.
	TableListBoxModel::backgroundClicked(event);
}

/**
 * This is overloaded from TableListBoxModel, and should fill in the background of the whole row.
 * @param g					Graphics context that must be used to do the drawing operations.
 * @param rowNumber			Number of row to paint.
 * @param width				Width of area to paint.
 * @param height			Height of area to paint.
 * @param rowIsSelected		True if row is currently selected.
 */
void TableModelComponent::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
	ignoreUnused(rowNumber);

	// Selected rows have a different background color.
	if (rowIsSelected)
		g.setColour(getLookAndFeel().findColour(TableHeaderComponent::highlightColourId));
	else
		g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId));
	g.fillRect(0, 0, width, height - 1);

	// Line between rows.
	g.setColour(getLookAndFeel().findColour(ListBox::outlineColourId));
	g.fillRect(0, height - 1, width, height - 1);
}

/**
 * This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom components.
 * This reimplementation does nothing (all cells use custom components).
 * @param g					Graphics context that must be used to do the drawing operations.
 * @param rowNumber			Number of row to paint (starts at 0)
 * @param columnId			Number of column to paint (starts at 1).
 * @param width				Width of area to paint.
 * @param height			Height of area to paint.
 * @param rowIsSelected		True if row is currently selected.
 */
void TableModelComponent::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	ignoreUnused(g);
	ignoreUnused(rowNumber);
	ignoreUnused(columnId);
	ignoreUnused(width);
	ignoreUnused(height);
	ignoreUnused(rowIsSelected);
}

/**
 * This is overloaded from TableListBoxModel, and tells us that the user has clicked a table header
 * to change the sort order.
 * @param newSortColumnId ID of the column selected for sorting.
 * @param isForwards True if sorting from smallest to largest.
 */
void TableModelComponent::sortOrderChanged(int newSortColumnId, bool isForwards)
{
	// Remember row selection so it can be restored after sorting.
	auto selectedProcessors = GetProcessorIdsForRows(GetSelectedRows());
	if (m_table)
		m_table->deselectAllRows();

	// Use a different helper sorting function depending on which column is selected for sorting.
	switch (newSortColumnId)
	{
	case BridgingAwareTableHeaderComponent::TC_SoundobjectID:
		std::sort(m_processorIds.begin(), m_processorIds.end(), TableModelComponent::LessThanSoundobjectId);
		break;
	case BridgingAwareTableHeaderComponent::TC_InputID:
	case BridgingAwareTableHeaderComponent::TC_InputEditor:
		std::sort(m_processorIds.begin(), m_processorIds.end(), TableModelComponent::LessThanMatrixInputId);
		break;
	case BridgingAwareTableHeaderComponent::TC_OutputID:
	case BridgingAwareTableHeaderComponent::TC_OutputEditor:
		std::sort(m_processorIds.begin(), m_processorIds.end(), TableModelComponent::LessThanMatrixOutputId);
		break;
	case BridgingAwareTableHeaderComponent::TC_Mapping:
		std::sort(m_processorIds.begin(), m_processorIds.end(), TableModelComponent::LessThanMapping);
		break;
	case BridgingAwareTableHeaderComponent::TC_ComsMode:
		std::sort(m_processorIds.begin(), m_processorIds.end(), TableModelComponent::LessThanComsMode);
		break;
	case BridgingAwareTableHeaderComponent::TC_BridgingMute:
		if (GetTableType() == TT_Soundobjects)
			std::sort(m_processorIds.begin(), m_processorIds.end(), TableModelComponent::LessThanSoundobjectBridgingMute);
		else if (GetTableType() == TT_MatrixInputs)
			std::sort(m_processorIds.begin(), m_processorIds.end(), TableModelComponent::LessThanMatrixInputBridgingMute);
		else if (GetTableType() == TT_MatrixOutputs)
			std::sort(m_processorIds.begin(), m_processorIds.end(), TableModelComponent::LessThanMatrixOutputBridgingMute);
		break;
	default:
		break;
	}

	// If reverse order is selected, reverse the list.
	if (!isForwards)
		std::reverse(m_processorIds.begin(), m_processorIds.end());

	if (m_table)
	{
		m_table->updateContent();

		// Restore row selection after sorting order has been changed, BUT make sure that
		// it is the same Processors which are selected after the sorting, NOT the same rows.
		for (auto processorId : selectedProcessors)
		{
			int rowNo = static_cast<int>(std::find(m_processorIds.begin(), m_processorIds.end(), processorId) - m_processorIds.begin());
			m_table->selectRow(rowNo, true /* don't scroll */, false /* do not deselect other rows*/);
		}
	}
}

/**
 * This is overloaded from TableListBoxModel, and must update any custom components that we're using.
 * @param rowNumber			Number of row on the table (starting at 0).
 * @param columnId			Number of column on the table (starting at 1).
 * @param isRowSelected		True if row is currently selected.
 * @param existingComponentToUpdate		Pointer to existing component for this cell. Null if no component exists yet.
 * @return	Pointer to component which should be used for this cell. Null if no component is necessary.
 */
Component* TableModelComponent::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate)
{
	ignoreUnused(isRowSelected);

	Component* ret = nullptr;

	switch (columnId)
	{

	case BridgingAwareTableHeaderComponent::TC_EmptyHandleCellID:
		// empty cell does not use any component
		break;

	case BridgingAwareTableHeaderComponent::TC_Mapping:
		{
			ComboBoxContainer* comboBox = static_cast<ComboBoxContainer*> (existingComponentToUpdate);

			// If an existing component is being passed-in for updating, we'll re-use it, but
			// if not, we'll have to create one.
			if (comboBox == nullptr)
				comboBox = new ComboBoxContainer(*this);

			// Ensure that the comboBox knows which row number it is located at.
			comboBox->SetRow(rowNumber);

			// Return a pointer to the comboBox.
			ret = comboBox;
		}
		break;

	case BridgingAwareTableHeaderComponent::TC_SoundobjectID:
		{
			TextEditorContainer* textEdit = static_cast<TextEditorContainer*> (existingComponentToUpdate);

			// If an existing component is being passed-in for updating, we'll re-use it, but
			// if not, we'll have to create one.
			if (textEdit == nullptr)
				textEdit = new TextEditorContainer(*this);

			// Ensure that the component knows which row number it is located at.
			textEdit->SetRow(rowNumber);

			// Return a pointer to the component.
			ret = textEdit;
		}
		break;

	case BridgingAwareTableHeaderComponent::TC_InputID:
		{
			TextEditorContainer* textEdit = static_cast<TextEditorContainer*> (existingComponentToUpdate);

			// If an existing component is being passed-in for updating, we'll re-use it, but
			// if not, we'll have to create one.
			if (textEdit == nullptr)
				textEdit = new TextEditorContainer(*this);

			// Ensure that the component knows which row number it is located at.
			textEdit->SetRow(rowNumber);

			// Return a pointer to the component.
			ret = textEdit;
		}
		break;

	case BridgingAwareTableHeaderComponent::TC_OutputID:
		{
			TextEditorContainer* textEdit = static_cast<TextEditorContainer*> (existingComponentToUpdate);

			// If an existing component is being passed-in for updating, we'll re-use it, but
			// if not, we'll have to create one.
			if (textEdit == nullptr)
				textEdit = new TextEditorContainer(*this);

			// Ensure that the component knows which row number it is located at.
			textEdit->SetRow(rowNumber);

			// Return a pointer to the component.
			ret = textEdit;
		}
		break;

	case BridgingAwareTableHeaderComponent::TC_ComsMode:
		{
			RadioButtonContainer* radioButton = static_cast<RadioButtonContainer*> (existingComponentToUpdate);

			// If an existing component is being passed-in for updating, we'll re-use it, but
			// if not, we'll have to create one.
			if (radioButton == nullptr)
				radioButton = new RadioButtonContainer(*this);

			// Ensure that the component knows which row number it is located at.
			radioButton->SetRow(rowNumber);

			// Return a pointer to the component.
			ret = radioButton;
		}
		break;

	case BridgingAwareTableHeaderComponent::TC_BridgingMute:
		{
			MuteButtonContainer* muteButton = static_cast<MuteButtonContainer*> (existingComponentToUpdate);

			// If an existing component is being passed-in for updating, we'll re-use it, but
			// if not, we'll have to create one.
			if (muteButton == nullptr)
				muteButton = new MuteButtonContainer(*this);

			// Ensure that the component knows which row number it is located at.
			muteButton->SetRow(rowNumber);
			muteButton->updateBridgingMuteButtons();

			// Return a pointer to the component.
			ret = muteButton;
		}
		break;

	case BridgingAwareTableHeaderComponent::TC_InputEditor:
		{
			MatrixInputProcessorEditor* matrixInputEditor = static_cast<MatrixInputProcessorEditor*> (existingComponentToUpdate);
			
			auto ctrl = Controller::GetInstance();
			if (ctrl)
			{
				auto processor = ctrl->GetMatrixInputProcessor(GetProcessorIdForRow(rowNumber));
				if (processor)
				{
					// If an existing component is being passed-in for updating, we'll re-use it, but
					// if not, we'll have to create one.
					if ((matrixInputEditor == nullptr) || (processor->GetMatrixInputId() != matrixInputEditor->GetMatrixInputId()))
					{
						auto processorEditor = processor->createEditorIfNeeded();
						auto mipEditor = dynamic_cast<MatrixInputProcessorEditor*>(processorEditor);
						if (mipEditor)
							matrixInputEditor = mipEditor;
					}
				}
			}

			if (matrixInputEditor)
				matrixInputEditor->UpdateGui(true);

			// Return a pointer to the component.
			ret = matrixInputEditor;
		}
		break;

	case BridgingAwareTableHeaderComponent::TC_OutputEditor:
		{
			MatrixOutputProcessorEditor* matrixOutputEditor = static_cast<MatrixOutputProcessorEditor*> (existingComponentToUpdate);

			auto ctrl = Controller::GetInstance();
			if (ctrl)
			{
				auto processor = ctrl->GetMatrixOutputProcessor(GetProcessorIdForRow(rowNumber));
				if (processor)
				{
					// If an existing component is being passed-in for updating, we'll re-use it, but
					// if not, we'll have to create one.
					if ((matrixOutputEditor == nullptr) || (processor->GetMatrixOutputId() != matrixOutputEditor->GetMatrixOutputId()))
					{
						auto processorEditor = processor->createEditorIfNeeded();
						auto mopEditor = dynamic_cast<MatrixOutputProcessorEditor*>(processorEditor);
						if (mopEditor)
							matrixOutputEditor = mopEditor;
					}
				}
			}

			if (matrixOutputEditor)
				matrixOutputEditor->UpdateGui(true);

			// Return a pointer to the component.
			ret = matrixOutputEditor;
		}
		break;

	default:
		jassert(existingComponentToUpdate == nullptr);
		break;
	}

	return ret;
}

/**
 *  This is overloaded from TableListBoxModel, and should choose the best width for the specified column.
 * @param columnId	Desired column ID.
 * @return	Width to be used for the desired column.
 */
int TableModelComponent::getColumnAutoSizeWidth(int columnId)
{
	switch (columnId)
	{
	case BridgingAwareTableHeaderComponent::TC_EmptyHandleCellID:
		return 40;
	case BridgingAwareTableHeaderComponent::TC_SoundobjectID:
		return 80;
	case BridgingAwareTableHeaderComponent::TC_InputID:
		return 70;
	case BridgingAwareTableHeaderComponent::TC_OutputID:
		return 70;
	case BridgingAwareTableHeaderComponent::TC_InputEditor:
		return 190;
	case BridgingAwareTableHeaderComponent::TC_OutputEditor:
		return 190;
	case BridgingAwareTableHeaderComponent::TC_Mapping:
		return 80;
	case BridgingAwareTableHeaderComponent::TC_ComsMode:
		return 110;
	case BridgingAwareTableHeaderComponent::TC_BridgingMute:
		return 70;
	default:
		break;
	}

	return 0;
}

/**
 * This is overloaded from TableListBoxModel, and tells us that the row selection has changed.
 * @param lastRowSelected	The last of the now selected rows.
 */
void TableModelComponent::selectedRowsChanged(int lastRowSelected)
{
	if (m_table)
	{
		if (m_table->getSelectedRows().isEmpty() || m_table->getSelectedRows().size() > 1)
		{
			if (onCurrentSelectedProcessorChanged)
				onCurrentSelectedProcessorChanged(SpaConBridge::INVALID_PROCESSOR_ID);
			m_tableControlBar->SetRemoveEnabled(false);
		}
		else
		{
			if (onCurrentSelectedProcessorChanged)
				onCurrentSelectedProcessorChanged(GetProcessorIdForRow(lastRowSelected));
			m_tableControlBar->SetRemoveEnabled(true);
		}
	}
}

/**
 *  This is overloaded from Component, and will reposition the TableListBox inside it. 
 */
void TableModelComponent::resized()
{
	auto tableBounds = getLocalBounds();
	auto tableControlBarBounds = juce::Rectangle<int>();

	switch (m_controlBarPosition)
	{
	case CBP_Left:
		tableControlBarBounds = tableBounds.removeFromLeft(32);
		break;
	case CBP_Right:
		tableControlBarBounds = tableBounds.removeFromRight(32);
		break;
	case CBP_Top:
		tableControlBarBounds = tableBounds.removeFromTop(32);
		break;
	case CBP_Bottom:
	default:
		tableControlBarBounds = tableBounds.removeFromBottom(32);
		break;
	}

	if (m_table)
		m_table->setBounds(tableBounds);

	if (m_tableControlBar)
		m_tableControlBar->setBounds(tableControlBarBounds);
}

/**
 * Helper method to be used as function callback to trigger selecting all rows of the member table
 * by forwarding the call to SelectAllRows with parameter set to true.
 */
void TableModelComponent::onSelectAllProcessors()
{
	SelectAllRows(true);
}

/**
 * Helper method to be used as function callback to trigger deselecting all rows of the member table
 * by forwarding the call to SelectAllRows with parameter set to false.
 */
void TableModelComponent::onDeselectAllProcessors()
{
	SelectAllRows(false);
}

/**
 * Helper method to be used as function callback to trigger applying new row height value.
 * @param	rowHeight	The new row height.
 */
void TableModelComponent::onRowHeightSlided(int rowHeight)
{
	// set the new row height to tableListBox member 
	if (m_table)
		m_table->setRowHeight(rowHeight);

	if (onCurrentRowHeightChanged)
		onCurrentRowHeightChanged(rowHeight);

	resized();
}

/**
 * Helper method to be used as function callback to trigger toggling the collapsed state (table visibility).
 * @param	collapsed	The new collapsed state (true=collapsed, false=expanded)
 */
void TableModelComponent::onCollapseToggled(bool collapsed)
{
	if (onCurrentCollapseStateChanged)
		onCurrentCollapseStateChanged(collapsed);
}


} // namespace SpaConBridge
