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

#include "MatrixInputTableComponent.h"

#include "../../../Controller.h"


namespace SoundscapeBridgeApp
{


/*
===============================================================================
	Class MatrixInputTableComponent
===============================================================================
*/

/**
 * Class constructor.
 */
MatrixInputTableComponent::MatrixInputTableComponent()
	: MatrixChannelTableComponentBase()
{
	// This fills m_ids.
	RecreateTableRowIds();

	SetModel(this);

	// collect required info for table columns
	std::map<CustomTableHeaderComponent::TableColumn, CustomTableHeaderComponent::ColumnProperties> tableColumns;
	int tableHeaderFlags = (TableHeaderComponent::visible | TableHeaderComponent::sortable);
	tableColumns[CustomTableHeaderComponent::TC_TrackID] = CustomTableHeaderComponent::ColumnProperties("", getColumnAutoSizeWidth(CustomTableHeaderComponent::TC_TrackID), getColumnAutoSizeWidth(CustomTableHeaderComponent::TC_TrackID), -1, tableHeaderFlags);
	tableColumns[CustomTableHeaderComponent::TC_InputID] = CustomTableHeaderComponent::ColumnProperties("Input #", getColumnAutoSizeWidth(CustomTableHeaderComponent::TC_InputID), getColumnAutoSizeWidth(CustomTableHeaderComponent::TC_InputID), -1, tableHeaderFlags);
	tableColumns[CustomTableHeaderComponent::TC_InputEditor] = CustomTableHeaderComponent::ColumnProperties("Matrix Input", getColumnAutoSizeWidth(CustomTableHeaderComponent::TC_InputEditor), getColumnAutoSizeWidth(CustomTableHeaderComponent::TC_InputEditor), -1, tableHeaderFlags);
	tableColumns[CustomTableHeaderComponent::TC_ComsMode] = CustomTableHeaderComponent::ColumnProperties("Mode", getColumnAutoSizeWidth(CustomTableHeaderComponent::TC_ComsMode), getColumnAutoSizeWidth(CustomTableHeaderComponent::TC_ComsMode), -1, tableHeaderFlags);
	tableColumns[CustomTableHeaderComponent::TC_BridgingMute] = CustomTableHeaderComponent::ColumnProperties("", getColumnAutoSizeWidth(CustomTableHeaderComponent::TC_BridgingMute), getColumnAutoSizeWidth(CustomTableHeaderComponent::TC_BridgingMute), -1, tableHeaderFlags);

	GetTable().setHeader(std::make_unique<CustomTableHeaderComponent>(tableColumns));

	SetRowHeight(33);
}

/**
 * Class destructor.
 */
MatrixInputTableComponent::~MatrixInputTableComponent()
{
}

/**
 * This clears and re-fills m_processorIds.
 */
void MatrixInputTableComponent::RecreateTableRowIds()
{
	GetProcessorIds().clear();
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
	{
		GetProcessorIds().reserve(ctrl->GetSoundobjectProcessorCount());
		for (auto const& processorId : ctrl->GetSoundobjectProcessorIds())
			GetProcessorIds().push_back(processorId);
	}
	
	// Clear row selection, since rows may have changed.
	auto currentSelectedRows = GetTable().getSelectedRows();
	if (!currentSelectedRows.isEmpty())
	{
		GetTable().deselectAllRows();
		GetTable().selectRow(currentSelectedRows[currentSelectedRows.size() - 1]);
	}
}

/**
 * This refreshes the table contents.
 */
void MatrixInputTableComponent::UpdateTable()
{
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
	{
		auto selectedProcessorIds = ctrl->GetSelectedSoundobjectProcessorIds();
		auto selectedRows = GetRowsForProcessorIds(selectedProcessorIds);
		if (GetSelectedRows() != selectedRows)
			SetSelectedRows(selectedRows);
	}
	
	// Refresh table
	GetTable().updateContent();
	
	// Refresh table header
	auto customTableHeader = dynamic_cast<CustomTableHeaderComponent*>(&GetTable().getHeader());
	if (customTableHeader)
		customTableHeader->updateBridgingTitles();
}

/**
 * This is overloaded from TableListBoxModel, and must return the total number of rows in our table.
 * @return	Number of rows on the table, equal to number of procssor instances.
 */
int MatrixInputTableComponent::getNumRows()
{
	int ret = 0;

	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
		ret = ctrl->GetMatrixInputProcessorCount();

	return ret;
}


} // namespace SoundscapeBridgeApp
