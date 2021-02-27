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

#include "SoundobjectTableComponent.h"

#include "../../../Controller.h"


namespace SoundscapeBridgeApp
{


/*
===============================================================================
	Class SoundobjectTableComponent
===============================================================================
*/

/**
 * Class constructor.
 */
SoundobjectTableComponent::SoundobjectTableComponent()
	: TableModelComponent()
{
	// This fills m_ids.
	RecreateTableRowIds();

	SetModel(this);

	// collect required info for table columns
	std::map<CustomTableHeaderComponent::TableColumn, CustomTableHeaderComponent::ColumnProperties> tableColumns;
	int tableHeaderFlags = (TableHeaderComponent::visible | TableHeaderComponent::sortable);
	tableColumns[CustomTableHeaderComponent::TC_TrackID] = CustomTableHeaderComponent::ColumnProperties("", 40, 40, -1, tableHeaderFlags);
	tableColumns[CustomTableHeaderComponent::TC_SoundobjectID] = CustomTableHeaderComponent::ColumnProperties("Object #", 60, 60, -1, tableHeaderFlags);
	tableColumns[CustomTableHeaderComponent::TC_Mapping] = CustomTableHeaderComponent::ColumnProperties("Mapping", 60, 60, -1, tableHeaderFlags);
	tableColumns[CustomTableHeaderComponent::TC_ComsMode] = CustomTableHeaderComponent::ColumnProperties("Mode", 90, 90, -1, tableHeaderFlags);
	tableColumns[CustomTableHeaderComponent::TC_BridgingMute] = CustomTableHeaderComponent::ColumnProperties("", 90, 90, -1, tableHeaderFlags);

	GetTable().setHeader(std::make_unique<CustomTableHeaderComponent>(tableColumns, CustomTableHeaderComponent::TC_SoundobjectID));
	GetTable().setRowHeight(33);
	GetTable().setOutlineThickness(1);
	GetTable().setClickingTogglesRowSelection(false);
	GetTable().setMultipleSelectionEnabled(true);
}

/**
 * Class destructor.
 */
SoundobjectTableComponent::~SoundobjectTableComponent()
{
}


/**
 * This clears and re-fills m_processorIds.
 */
void SoundobjectTableComponent::RecreateTableRowIds()
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
void SoundobjectTableComponent::UpdateTable()
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
int SoundobjectTableComponent::getNumRows()
{
	int ret = 0;

	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
		ret = ctrl->GetSoundobjectProcessorCount();

	return ret;
}

/**
 * This is overloaded from TableListBoxModel, and tells us that the row selection has changed.
 * @param lastRowSelected	The last of the now selected rows.
 */
void SoundobjectTableComponent::selectedRowsChanged(int lastRowSelected)
{
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->SetSelectedSoundobjectProcessorIds(GetProcessorIdsForRows(GetSelectedRows()), true);

	TableModelComponent::selectedRowsChanged(lastRowSelected);
}


} // namespace SoundscapeBridgeApp
