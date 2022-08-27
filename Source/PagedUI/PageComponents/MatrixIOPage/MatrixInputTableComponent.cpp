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

#include "MatrixInputTableComponent.h"

#include "../BridgingAwareTableHeaderComponent.h"

#include "../../../Controller.h"
#include "../../../CustomAudioProcessors/MatrixInputProcessor/MatrixInputProcessor.h"
#include "../../../RowHeightSlider.h"
#include "../../../DelayedRecursiveFunctionCaller.h"


namespace SpaConBridge
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
	SetTableType(TT_MatrixInputs);

	// This fills m_ids.
	RecreateTableRowIds();

	SetModel(this);

	// collect required info for table columns
	std::map<BridgingAwareTableHeaderComponent::TableColumn, BridgingAwareTableHeaderComponent::ColumnProperties> tableColumns;
	int tableHeaderFlags = (TableHeaderComponent::visible | TableHeaderComponent::sortable);
	tableColumns[BridgingAwareTableHeaderComponent::TC_EmptyHandleCellID] = BridgingAwareTableHeaderComponent::ColumnProperties("", getColumnAutoSizeWidth(BridgingAwareTableHeaderComponent::TC_EmptyHandleCellID), getColumnAutoSizeWidth(BridgingAwareTableHeaderComponent::TC_EmptyHandleCellID), -1, tableHeaderFlags);
	tableColumns[BridgingAwareTableHeaderComponent::TC_InputID] = BridgingAwareTableHeaderComponent::ColumnProperties("Input #", getColumnAutoSizeWidth(BridgingAwareTableHeaderComponent::TC_InputID), getColumnAutoSizeWidth(BridgingAwareTableHeaderComponent::TC_InputID), -1, tableHeaderFlags);
	tableColumns[BridgingAwareTableHeaderComponent::TC_InputEditor] = BridgingAwareTableHeaderComponent::ColumnProperties("Matrix Input", getColumnAutoSizeWidth(BridgingAwareTableHeaderComponent::TC_InputEditor), getColumnAutoSizeWidth(BridgingAwareTableHeaderComponent::TC_InputEditor), -1, tableHeaderFlags);
	tableColumns[BridgingAwareTableHeaderComponent::TC_ComsMode] = BridgingAwareTableHeaderComponent::ColumnProperties("Mode", getColumnAutoSizeWidth(BridgingAwareTableHeaderComponent::TC_ComsMode), getColumnAutoSizeWidth(BridgingAwareTableHeaderComponent::TC_ComsMode), -1, tableHeaderFlags);
	tableColumns[BridgingAwareTableHeaderComponent::TC_BridgingMute] = BridgingAwareTableHeaderComponent::ColumnProperties("", getColumnAutoSizeWidth(BridgingAwareTableHeaderComponent::TC_BridgingMute), getColumnAutoSizeWidth(BridgingAwareTableHeaderComponent::TC_BridgingMute), -1, tableHeaderFlags);

	auto table = GetTable();
	if (table)
		table->setHeader(std::make_unique<BridgingAwareTableHeaderComponent>(tableColumns));

	SetRowHeight(RowHeightSlider::_Min + RowHeightSlider::_Interval);
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
		GetProcessorIds().reserve(ctrl->GetMatrixInputProcessorCount());
		for (auto const& processorId : ctrl->GetMatrixInputProcessorIds())
			GetProcessorIds().push_back(processorId);
	}
	
	auto table = GetTable();
	if (table)
	{
		// Clear row selection, since rows may have changed.
		auto currentSelectedRows = table->getSelectedRows();
		if (!currentSelectedRows.isEmpty())
		{
			table->deselectAllRows();
			table->selectRow(currentSelectedRows[currentSelectedRows.size() - 1]);
		}
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
		auto selectedProcessorIds = ctrl->GetSelectedMatrixInputProcessorIds();
		auto selectedRows = GetRowsForProcessorIds(selectedProcessorIds);
		if (GetSelectedRows() != selectedRows)
			SetSelectedRows(selectedRows);
	}
	
	auto table = GetTable();
	if (table)
	{
		// Refresh table
		table->updateContent();

		// Refresh table header
		auto customTableHeader = dynamic_cast<BridgingAwareTableHeaderComponent*>(&table->getHeader());
		if (customTableHeader)
			customTableHeader->updateBridgingTitles();
	}
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

/**
 * This is overloaded from TableListBoxModel, and tells us that the row selection has changed.
 * @param lastRowSelected	The last of the now selected rows.
 */
void MatrixInputTableComponent::selectedRowsChanged(int lastRowSelected)
{
	Controller* ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->SetSelectedMatrixInputProcessorIds(GetProcessorIdsForRows(GetSelectedRows()), true);

	TableModelComponent::selectedRowsChanged(lastRowSelected);
}

/**
 * Reimplemented pure virtual method that is used as std::function callback in table control bar
 */
void MatrixInputTableComponent::onAddProcessor()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	ctrl->createNewMatrixInputProcessor();
}

/**
 * Reimplemented pure virtual method that is used as std::function callback in table control bar
 */
void MatrixInputTableComponent::onAddMultipleProcessors()
{
	auto w = std::make_unique<AlertWindow>("Matrix Inputs", "Choose how many to add", MessageBoxIconType::NoIcon).release();
	w->addTextEditor("processor_count", "1");
    w->getTextEditor("processor_count")->setInputRestrictions(3, "0123456789");
    w->getTextEditor("processor_count")->setKeyboardType(TextInputTarget::VirtualKeyboardType::phoneNumberKeyboard);
	w->addButton("OK", 1, KeyPress(KeyPress::returnKey, 0, 0));
	w->addButton("Cancel", 0, KeyPress(KeyPress::escapeKey, 0, 0));

	// lambda to be called with the result of the modal processor count choise dialog
	auto countChoiceCallbackFunctionBody = ([w](int result)
		{
			if (w && result == 1)
			{
				// get the requested count of new processors and start adding them if greater zero
				int newProcessorsCount = w->getTextEditorContents("processor_count").getIntValue();
				if (newProcessorsCount > 0)
				{
					auto config = SpaConBridge::AppConfiguration::getInstance();
					if (config)
						config->SetFlushAndUpdateDisabled();

					auto functionCaller = std::make_unique<DelayedRecursiveFunctionCaller>([]
						{
							auto ctrl = Controller::GetInstance();
							if (ctrl)
								ctrl->createNewMatrixInputProcessor();
						}, newProcessorsCount, true);
					functionCaller->SetFinalFunctionCall([]
						{
							auto config = SpaConBridge::AppConfiguration::getInstance();
							if (config)
								config->ResetFlushAndUpdateDisabled();
						});
					functionCaller->Run();
					functionCaller.release();
				}
			}
		});
	auto modalCallback = juce::ModalCallbackFunction::create(countChoiceCallbackFunctionBody);

	// Run asynchronously
	w->enterModalState(true, modalCallback, true);
}

/**
 * Reimplemented pure virtual method that is used as std::function callback in table control bar
 */
void MatrixInputTableComponent::onRemoveProcessor()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

    auto const& selectedProcessorIds = GetProcessorIdsForRows(GetSelectedRows());
    
	if (ctrl->GetMatrixInputProcessorCount() <= selectedProcessorIds.size())
	{
		if (onCurrentSelectedProcessorChanged)
			onCurrentSelectedProcessorChanged(INVALID_PROCESSOR_ID);
	}
	else
	{
		auto processorCount = ctrl->GetSoundobjectProcessorCount();
		auto currentLastProcessorId = processorCount - 1;
		auto selectedProcessorsToRemoveCount = selectedProcessorIds.size();
		auto nextStillExistingId = static_cast<MatrixInputId>(currentLastProcessorId - selectedProcessorsToRemoveCount);
		selectedRowsChanged(nextStillExistingId);
	}

	// when processors are being deleted in next step, the current selection will be queried, which is why clearing the selection before is neccessary
	SetSelectedRows(std::vector<juce::int32>());

	// Iterate through the processor ids once more to destroy the selected processors themselves.
	if (selectedProcessorIds.size() > 0 && ctrl->GetMatrixInputProcessorCount() > 0)
	{
		auto functionCaller = std::make_unique<DelayedRecursiveFunctionCaller>([](int processorId)
			{
				auto ctrl = Controller::GetInstance();
				if (ctrl && ctrl->GetMatrixInputProcessorCount() >= 1)
				{
					auto processor = std::unique_ptr<MatrixInputProcessor>(ctrl->GetMatrixInputProcessor(processorId)); // when processor goes out of scope, it is destroyed and the destructor does handle unregistering from ccontroller by itself
					processor->releaseResources();
				}
			}, selectedProcessorIds, true);
		functionCaller->Run();
		functionCaller.release();
	}
}


} // namespace SpaConBridge
