/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file is part of the Soundscape VST, AU, and AAX Plug-in.

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


#include "OverviewTable.h"

#include "../SoundsourceProcessor/SoundsourceProcessor.h"
#include "../SoundsourceProcessor/SoundsourceProcessorEditor.h"
#include "../SoundsourceProcessor/SurfaceSlider.h"

#include "../Controller.h"
#include "../LookAndFeel.h"

#include <Image_utils.hpp>


namespace SoundscapeBridgeApp
{


/*
===============================================================================
 Class OverviewTableContainer
===============================================================================
*/

/**
 * Class constructor.
 */
OverviewTableContainer::OverviewTableContainer()
	: OverlayBase(OT_Overview)
{
	// Create the table model/component.
	m_overviewTable = std::make_unique<TableModelComponent>();
	m_overviewTable->currentSelectedProcessorChanged = [=](ProcessorId id) { this->onCurrentSelectedProcessorChanged(id); };
	addAndMakeVisible(m_overviewTable.get());

	// Add/Remove Buttons
	m_addInstance = std::make_unique<TextButton>();
	m_addInstance->setClickingTogglesState(false);
	m_addInstance->setButtonText("Add");
	m_addInstance->addListener(this);
	addAndMakeVisible(m_addInstance.get());
	m_removeInstance = std::make_unique<TextButton>();
	m_removeInstance->setClickingTogglesState(false);
	m_removeInstance->setButtonText("Remove");
	m_removeInstance->setEnabled(false);
	m_removeInstance->addListener(this);
	addAndMakeVisible(m_removeInstance.get());

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

	// register this object as config watcher
	auto config = AppConfiguration::getInstance();
	if (config)
		config->addWatcher(this);
}

/**
 * Class destructor.
 */
OverviewTableContainer::~OverviewTableContainer()
{
}

/**
 * Reimplemented to paint background and frame.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void OverviewTableContainer::paint(Graphics& g)
{
	int w = getLocalBounds().getWidth();
	int h = getLocalBounds().getHeight();	

	// Background
	g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId));
	g.fillRect(Rectangle<int>(8, h - 41, w - 16, 34));

	// Frame
	g.setColour(getLookAndFeel().findColour(TableListBox::outlineColourId));
	g.drawRect(Rectangle<int>(8, h - 41, w - 16, 34), 1);
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void OverviewTableContainer::resized()
{
	// flexbox for table and editor as column or row layout depending on aspect ratio
	FlexBox tableAndEditorFlex;
	FlexItem::Margin tableMargin{ 8 };
	FlexItem::Margin editorMargin{ 8 };
	auto isPortrait = getLocalBounds().getHeight() > getLocalBounds().getWidth();
	if (isPortrait)
	{
		tableAndEditorFlex.flexDirection = FlexBox::Direction::column;
		if (m_selectedProcessorInstanceEditor)
		{
			tableMargin = FlexItem::Margin(8, 8, 4, 8);
			editorMargin = FlexItem::Margin(4, 8, 0, 8);
		}
		else
			tableMargin = FlexItem::Margin(8, 8, 0, 8);
	}
	else
	{
		tableAndEditorFlex.flexDirection = FlexBox::Direction::row;
		if (m_selectedProcessorInstanceEditor)
		{
			tableMargin = FlexItem::Margin(8, 4, 0, 8);
			editorMargin = FlexItem::Margin(8, 8, 0, 4);
		}
		else
			tableMargin = FlexItem::Margin(8, 8, 0, 8);
	}

	tableAndEditorFlex.justifyContent = FlexBox::JustifyContent::center;

	if (m_selectedProcessorInstanceEditor)
	{
		tableAndEditorFlex.items.add(FlexItem(*m_overviewTable).withFlex(1).withMargin(tableMargin));
		tableAndEditorFlex.items.add(FlexItem(*m_selectedProcessorInstanceEditor.get()).withFlex(1).withMargin(editorMargin));
	}
	else
		tableAndEditorFlex.items.add(FlexItem(*m_overviewTable).withFlex(1).withMargin(tableMargin));
	
	// flexbox for bottom buttons
	FlexBox bottomBarFlex;
	bottomBarFlex.flexDirection = FlexBox::Direction::row;
	bottomBarFlex.justifyContent = FlexBox::JustifyContent::center;
	bottomBarFlex.alignContent = FlexBox::AlignContent::center;
	bottomBarFlex.items.addArray({
		FlexItem(*m_addInstance.get()).withFlex(1).withMaxWidth(40).withMargin(FlexItem::Margin(2, 2, 2, 4)),
		FlexItem(*m_removeInstance.get()).withFlex(1).withMaxWidth(60).withMargin(FlexItem::Margin(2)),
		FlexItem().withFlex(2).withHeight(30),
		FlexItem(*m_selectLabel.get()).withFlex(1).withMaxWidth(80),
		FlexItem(*m_selectAll.get()).withFlex(1).withMaxWidth(40).withMargin(FlexItem::Margin(2)),
		FlexItem(*m_selectNone.get()).withFlex(1).withMaxWidth(46).withMargin(FlexItem::Margin(2, 4, 2, 2)),
		});

	FlexBox mainFB;
	mainFB.flexDirection = FlexBox::Direction::column;
	mainFB.justifyContent = FlexBox::JustifyContent::center;
	mainFB.items.addArray({
		FlexItem(tableAndEditorFlex).withFlex(4),
		FlexItem(bottomBarFlex).withFlex(1).withMaxHeight(32).withMargin(FlexItem::Margin(0, 8, 8, 8))
		});
	mainFB.performLayout(getLocalBounds().toFloat());
}

/**
 * Reimplemented from Button::Listener, gets called whenever the buttons are clicked.
 * @param button	The button which has been clicked.
 */
void OverviewTableContainer::buttonClicked(Button *button)
{
	if ((button == m_selectAll.get()) || (button == m_selectNone.get()))
	{
		// Send true to select all rows, false to deselect all.
		m_overviewTable->SelectAllRows(button == m_selectAll.get());

		// Un-toggle button.			
		button->setToggleState(false, NotificationType::dontSendNotification);
	}
	else if ((button == m_addInstance.get()) || (button == m_removeInstance.get()))
	{
		auto addInstance = (button == m_addInstance.get());

		CController* ctrl = CController::GetInstance();
		if (ctrl)
		{
			if (addInstance)
			{
				ctrl->createNewProcessor();
			}
			else
			{
				auto const& selectedProcessorIds = m_overviewTable->GetSelectedRows();

				if (ctrl->GetProcessorCount() <= selectedProcessorIds.size())
					onCurrentSelectedProcessorChanged(INVALID_PROCESSOR_ID);
				else
				{
					auto processorCount = ctrl->GetProcessorCount();
					auto currentLastProcessorId = processorCount - 1;
					auto selectedProcessorsToRemoveCount = selectedProcessorIds.size();
					auto nextStillExistingId = static_cast<ProcessorId>(currentLastProcessorId - selectedProcessorsToRemoveCount);
					m_overviewTable->selectedRowsChanged(nextStillExistingId);
				}

				for (auto processorId : selectedProcessorIds)
				{
					if (ctrl->GetProcessorCount() >= 1)
						auto processor = std::unique_ptr<SoundsourceProcessor>(ctrl->GetProcessor(processorId)); // when processor goes out of scope, it is destroyed and the destructor does handle unregistering from ccontroller by itself
				}
			}
		}
	}
}

/**
 * Function to be called from model when the current selection has changed
 */
void OverviewTableContainer::onCurrentSelectedProcessorChanged(ProcessorId selectedProcessorId)
{
	if (selectedProcessorId == INVALID_PROCESSOR_ID)
	{
		if (m_selectedProcessorInstanceEditor)
		{
			removeChildComponent(m_selectedProcessorInstanceEditor.get());
			m_selectedProcessorInstanceEditor.reset();
			resized();
		}

		// since we just removed the editor after the last table row was removed, the remove button must be deactivated as well
		m_removeInstance->setEnabled(false);
	}
	else
	{
		CController* ctrl = CController::GetInstance();
		if (ctrl)
		{
			auto processor = ctrl->GetProcessor(selectedProcessorId);
			auto processorEditor = processor->createEditorIfNeeded();
			auto sspEditor = dynamic_cast<SoundsourceProcessorEditor*>(processorEditor);
			if (sspEditor != m_selectedProcessorInstanceEditor.get())
			{
				removeChildComponent(m_selectedProcessorInstanceEditor.get());
				m_selectedProcessorInstanceEditor.reset();
				m_selectedProcessorInstanceEditor = std::unique_ptr<SoundsourceProcessorEditor>(sspEditor);
				if (m_selectedProcessorInstanceEditor)
				{
					addAndMakeVisible(m_selectedProcessorInstanceEditor.get());
					m_selectedProcessorInstanceEditor->UpdateGui(true);
				}
				resized();

				// since we just added another editor, remove button can be enabled (regardless of if it already was enabled)
				m_removeInstance->setEnabled(true);
			}
		}
	}
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the plugin parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void OverviewTableContainer::UpdateGui(bool init)
{
	CController* ctrl = CController::GetInstance();
	if (ctrl && m_overviewTable)
	{
		if (ctrl->PopParameterChanged(DCS_Overview, DCT_NumProcessors) || init)
		{
			m_overviewTable->RecreateTableRowIds();
			m_overviewTable->UpdateTable();
		}

		else
		{
			// Iterate through all plugin instances and see if anything changed there.
			for (int pIdx = 0; pIdx < ctrl->GetProcessorCount(); pIdx++)
			{
				SoundsourceProcessor* plugin = ctrl->GetProcessor(pIdx);
				if (plugin && plugin->PopParameterChanged(DCS_Overview, DCT_PluginInstanceConfig))
				{
					m_overviewTable->UpdateTable();
				}
			}
		}
	}
}

/**
 * Overridden from AppConfiguration Watcher to be able
 * to live react on config changes and update the table contents.
 */
void OverviewTableContainer::onConfigUpdated()
{
	UpdateGui(false);
}


/*
===============================================================================
 Class CustomTableHeaderComponent
===============================================================================
*/

/**
 * Class constructor.
 */
CustomTableHeaderComponent::CustomTableHeaderComponent()
{
	// Add columns to the table header
	int tableHeaderFlags = (TableHeaderComponent::visible | TableHeaderComponent::sortable);
	addColumn("", TableModelComponent::OC_TrackID, 15, 15, -1, tableHeaderFlags);
	addColumn("Input", TableModelComponent::OC_SourceID, 40, 30, -1, tableHeaderFlags);
	addColumn("Mapping", TableModelComponent::OC_Mapping, 40, 30, -1, tableHeaderFlags);
	addColumn("Mode", TableModelComponent::OC_ComsMode, 40, 30, -1, tableHeaderFlags);
	addColumn("", TableModelComponent::OC_BridgingMute, 40, 30, -1, tableHeaderFlags);
	setSortColumnId(TableModelComponent::OC_SourceID, true); // sort forwards by the Input number column
	setStretchToFitActive(true);

	updateBridgingTitles();
}

/**
 * Class destructor.
 */
CustomTableHeaderComponent::~CustomTableHeaderComponent()
{
}

/**
 * Helper method to update the list of bridging titles by querying
 * data from controller. This should be called on configuration updates
 * that affect bridging protocol active state.
 */
void CustomTableHeaderComponent::updateBridgingTitles()
{
	CController* ctrl = CController::GetInstance();
	if (!ctrl)
		return;

	m_activeBridgingTitles.clear();

	auto activeBridging = ctrl->GetActiveProtocolBridging();
	if ((activeBridging & PBT_DiGiCo) == PBT_DiGiCo)
		m_activeBridgingTitles.push_back("DiGiCo");
	if ((activeBridging & PBT_GenericOSC) == PBT_GenericOSC)
		m_activeBridgingTitles.push_back("Generic OSC");
	if ((activeBridging & PBT_BlacktraxRTTRP) == PBT_BlacktraxRTTRP)
		m_activeBridgingTitles.push_back("Blacktrax");
	if ((activeBridging & PBT_GenericMIDI) == PBT_GenericMIDI)
		m_activeBridgingTitles.push_back("MIDI");
	if ((activeBridging & PBT_YamahaSQ) == PBT_YamahaSQ)
		m_activeBridgingTitles.push_back("Yamaha");
	if ((activeBridging & PBT_HUI) == PBT_HUI)
		m_activeBridgingTitles.push_back("HUI");
}

/**
 * Overridden to handle some special two-lined text arrangement
 * @param g The graphics object for painting
 */
void CustomTableHeaderComponent::paint(Graphics& g)
{
	TableHeaderComponent::paint(g);

	auto bridgingCellRect = getColumnPosition(getNumColumns(true)).reduced(3);

	auto font = g.getCurrentFont();
	font.setBold(true);
	g.setFont(font);
	g.setColour(getLookAndFeel().findColour(TableHeaderComponent::textColourId));

	if (m_activeBridgingTitles.empty())
	{
		g.drawText("Bridging", bridgingCellRect, Justification::centredLeft);
	}
	else
	{
		auto upperHalfCellRect = bridgingCellRect.removeFromTop(bridgingCellRect.getHeight() / 2).reduced(2);
		g.drawText("Bridging", upperHalfCellRect, Justification::centred);

		font.setBold(false);
		auto fh = font.getHeight();
		font.setHeight(fh - 2);
		g.setFont(font);
	
		auto singleTitleWidth = bridgingCellRect.getWidth() / m_activeBridgingTitles.size();

		for (auto title : m_activeBridgingTitles)
		{
			auto titleRect = bridgingCellRect.removeFromLeft(singleTitleWidth).reduced(2);
			g.drawText(title, titleRect, Justification::centredLeft);
		}
	}
}


/*
===============================================================================
 Class TableModelComponent
===============================================================================
*/

/**
 * Class constructor.
 */
TableModelComponent::TableModelComponent()
{
	// This fills m_ids.
	RecreateTableRowIds();

	// Create our table component and add it to this component..
	addAndMakeVisible(m_table);
	m_table.setModel(this);

	m_table.setHeader(std::make_unique<CustomTableHeaderComponent>());

	m_table.setRowHeight(33);
	m_table.setOutlineThickness(1);
	m_table.setClickingTogglesRowSelection(false);
	m_table.setMultipleSelectionEnabled(true);
}

/**
 * Class destructor.
 */
TableModelComponent::~TableModelComponent()
{
}

/**
 * Get the ID of the plugin instance corresponding to the given table row number.
 * @param rowNumber	The desired row number (starts at 0).
 * @return	The ID of the plugin instance at that row number, if any.
 */
ProcessorId TableModelComponent::GetProcessorIdForRow(int rowNumber)
{
	if ((unsigned int)rowNumber > (m_ids.size() - 1))
	{
		jassertfalse; // Unexpected row number!
		return 0;
	}

	return m_ids.at(rowNumber);
}

/**
 * Get the IDs of the plugin instances corresponding to the given table row numbers.
 * @param rowNumbers	A list of desired row numbers.
 * @return	A list of IDs of the plugin instances at those rows.
 */
std::vector<ProcessorId> TableModelComponent::GetProcessorIdsForRows(std::vector<int> rowNumbers)
{
	std::vector<ProcessorId> ids;
	ids.reserve(rowNumbers.size());
	for (std::size_t i = 0; i < rowNumbers.size(); ++i)
		ids.push_back(GetProcessorIdForRow(rowNumbers[i]));

	return ids;
}

/**
 * Get the list of rows which are currently selected on the table.
 * @return	A std::vector containing all selected row numbers.
 */
std::vector<int> TableModelComponent::GetSelectedRows() const
{
	std::vector<int> selectedRows;
	selectedRows.reserve(m_table.getSelectedRows().size());
	for (int i = 0; i < m_table.getSelectedRows().size(); ++i)
		selectedRows.push_back(m_table.getSelectedRows()[i]);

	return selectedRows;
}

/**
 * Select all (or none) of the rows on the table.
 * @param all	True to select all rows. False to de-select all (clear selection).
 */
void TableModelComponent::SelectAllRows(bool all)
{
	if (all)
		m_table.selectRangeOfRows(0, m_table.getNumRows(), true /* Do not scroll */);
	else
		m_table.deselectAllRows();
}

/**
 * Helper sorting function used by std::sort(). This version is used to sort by plugin's SourceId.
 * @param pId1	Id of the first plugin processor.
 * @param pId2	Id of the second plugin processor.
 * @return	True if the first plugin's SourceId is less than the second's.
 */
bool TableModelComponent::LessThanSourceId(ProcessorId pId1, ProcessorId pId2)
{
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		if ((pId1 < (ProcessorId)ctrl->GetProcessorCount()) && (pId2 < (ProcessorId)ctrl->GetProcessorCount()))
			return (ctrl->GetProcessor(pId1)->GetSourceId() < ctrl->GetProcessor(pId2)->GetSourceId());
	}

	jassertfalse; // Index out of range!
	return false;
}

/**
 * Helper sorting function used by std::sort(). This version is used to sort by plugin's MappingId.
 * @param pId1	Id of the first plugin processor.
 * @param pId2	Id of the second plugin processor.
 * @return	True if the first plugin's MappingId is less than the second's.
 */
bool TableModelComponent::LessThanMapping(ProcessorId pId1, ProcessorId pId2)
{
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		if ((pId1 < (ProcessorId)ctrl->GetProcessorCount()) && (pId2 < (ProcessorId)ctrl->GetProcessorCount()))
			return (ctrl->GetProcessor(pId1)->GetMappingId() < ctrl->GetProcessor(pId2)->GetMappingId());
	}

	jassertfalse; // Index out of range!
	return false;
}

/**
 * Helper sorting function used by std::sort(). This version is used to sort by plugin's ComsMode. 
 * @param pId1	Id of the first plugin processor.
 * @param pId2	Id of the second plugin processor.
 * @return	True if the first plugin's ComsMode is less than the second's.
 */
bool TableModelComponent::LessThanComsMode(ProcessorId pId1, ProcessorId pId2)
{
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		if ((pId1 < (ProcessorId)ctrl->GetProcessorCount()) && (pId2 < (ProcessorId)ctrl->GetProcessorCount()))
			return (ctrl->GetProcessor(pId1)->GetComsMode() < ctrl->GetProcessor(pId2)->GetComsMode());
	}

	jassertfalse; // Index out of range!
	return false;
}

/**
 * Helper sorting function used by std::sort(). This version is used to sort by plugin's ComsMode.
 * @param pId1	Id of the first plugin processor.
 * @param pId2	Id of the second plugin processor.
 * @return	True if the first plugin's ComsMode is less than the second's.
 */
bool TableModelComponent::LessThanBridgingMute(ProcessorId pId1, ProcessorId pId2)
{
	ignoreUnused(pId1);
	ignoreUnused(pId2);

	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		return true;
	}

	jassertfalse; // Index out of range!
	return false;
}

/**
 * This clears and re-fills m_ids.
 */
void TableModelComponent::RecreateTableRowIds()
{
	m_ids.clear();
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		m_ids.reserve(ctrl->GetProcessorCount());
		for (int idx = 0; idx < ctrl->GetProcessorCount(); ++idx)
			m_ids.push_back(idx);
	}

	// Clear row selection, since rows may have changed.
	auto currentSelectedRows = m_table.getSelectedRows();
	if (!currentSelectedRows.isEmpty())
	{
		m_table.deselectAllRows();
		m_table.selectRow(currentSelectedRows[currentSelectedRows.size() - 1]);
	}
}

/**
 * This refreshes the table contents.
 */
void TableModelComponent::UpdateTable()
{
	// Re-sort table again depending on the currently selected column.
	sortOrderChanged(m_table.getHeader().getSortColumnId(), m_table.getHeader().isSortedForwards());

	// Refresh table
	m_table.updateContent();

	// Refresh table header
	auto customTableHeader = dynamic_cast<CustomTableHeaderComponent*>(&m_table.getHeader());
	if (customTableHeader)
		customTableHeader->updateBridgingTitles();
}

/**
 * This can be overridden to react to the user double-clicking on a part of the list where there are no rows. 
 * @param event	Contains position and status information about a mouse event.
 */
void TableModelComponent::backgroundClicked(const MouseEvent &event)
{
	// Clear selection
	m_table.deselectAllRows();

	// Base class implementation.
	TableListBoxModel::backgroundClicked(event);
}

/**
 * This is overloaded from TableListBoxModel, and must return the total number of rows in our table.
 * @return	Number of rows on the table, equal to number of plugin instances.
 */
int TableModelComponent::getNumRows()
{
	int ret = 0;

	CController* ctrl = CController::GetInstance();
	if (ctrl)
		ret = ctrl->GetProcessorCount();

	return ret;
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
		g.setColour(getLookAndFeel().findColour(LookAndFeel_V4::ColourScheme::highlightedFill));
	else
		g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId));
	g.fillRect(0, 0, width, height - 1);

	// Line between rows.
	g.setColour(getLookAndFeel().findColour(LookAndFeel_V4::ColourScheme::outline));
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
	std::vector<ProcessorId> selectedPlugins = GetProcessorIdsForRows(GetSelectedRows());
	m_table.deselectAllRows();

	// Use a different helper sorting function depending on which column is selected for sorting.
	switch (newSortColumnId)
	{
	case OC_TrackID:
		std::sort(m_ids.begin(), m_ids.end());
		break;
	case OC_SourceID:
		std::sort(m_ids.begin(), m_ids.end(), TableModelComponent::LessThanSourceId);
		break;
	case OC_Mapping:
		std::sort(m_ids.begin(), m_ids.end(), TableModelComponent::LessThanMapping);
		break;
	case OC_ComsMode:
		std::sort(m_ids.begin(), m_ids.end(), TableModelComponent::LessThanComsMode);
		break;
	case OC_BridgingMute:
		std::sort(m_ids.begin(), m_ids.end(), TableModelComponent::LessThanBridgingMute);
		break;
	default:
		break;
	}

	// If reverse order is selected, reverse the list.
	if (!isForwards)
		std::reverse(m_ids.begin(), m_ids.end());

	m_table.updateContent();

	// Restore row selection after sorting order has been changed, BUT make sure that
	// it is the same Plugins which are selected after the sorting, NOT the same rows.
	for (ProcessorId pId : selectedPlugins)
	{
		int rowNo = static_cast<int>(std::find(m_ids.begin(), m_ids.end(), pId) - m_ids.begin());
		m_table.selectRow(rowNo, true /* don't scroll */, false /* do not deselect other rows*/);
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
		case OC_TrackID:
		{
			EditableLabelContainer* label = static_cast<EditableLabelContainer*> (existingComponentToUpdate);

			// If an existing component is being passed-in for updating, we'll re-use it, but
			// if not, we'll have to create one.
			if (label == nullptr)
				label = new EditableLabelContainer(*this);

			// Ensure that the component knows which row number it is located at.
			label->SetRow(rowNumber);

			// Return a pointer to the component.
			ret = label;
		}
		break;

	case OC_Mapping:
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
	case OC_SourceID:
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

	case OC_ComsMode:
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

	case OC_BridgingMute:
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
	case OC_TrackID:
		return 15;
	case OC_SourceID:
		return 40;
	case OC_Mapping:
		return 40;
	case OC_ComsMode:
		return 40;
	case OC_BridgingMute:
		return 40;
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
	if (currentSelectedProcessorChanged)
	{
		if (m_table.getSelectedRows().isEmpty() || m_table.getSelectedRows().size() > 1)
		{
			currentSelectedProcessorChanged(SoundscapeBridgeApp::INVALID_PROCESSOR_ID);
		}
		else
		{
			currentSelectedProcessorChanged(GetProcessorIdForRow(lastRowSelected));
		}
	}
}

/**
 *  This is overloaded from Component, and will reposition the TableListBox inside it. 
 */
void TableModelComponent::resized()
{
	m_table.setBounds(getLocalBounds());
}


/*
===============================================================================
 Class ComboBoxContainer
===============================================================================
*/

/**
 * Class constructor.
 */
ComboBoxContainer::ComboBoxContainer(TableModelComponent& td)
	: m_owner(td)
{
	// Create and configure actual combo box component inside this container.
	m_comboBox.setEditableText(false);
	m_comboBox.addItem("1", 1);
	m_comboBox.addItem("2", 2);
	m_comboBox.addItem("3", 3);
	m_comboBox.addItem("4", 4);
	m_comboBox.addListener(this);
	m_comboBox.setWantsKeyboardFocus(false);
	addAndMakeVisible(m_comboBox);
}

/**
 * Class destructor.
 */
ComboBoxContainer::~ComboBoxContainer()
{
}

/**
 * Reimplemented from ComboBox::Listener, gets called whenever the selected combo box item is changed.
 * @param comboBox	The comboBox which has been changed.
 */
void ComboBoxContainer::comboBoxChanged(ComboBox *comboBox)
{
	// Get the list of rows which are currently selected on the table.
	std::vector<int> selectedRows = m_owner.GetSelectedRows();
	if ((selectedRows.size() < 2) ||
		(std::find(selectedRows.begin(), selectedRows.end(), m_row) == selectedRows.end()))
	{
		// If this comboBoxes row (m_row) is NOT selected, or if no multi-selection was made 
		// then modify the selectedRows list so that it only contains m_row.
		selectedRows.clear();
		selectedRows.push_back(m_row);
	}

	// Get the IDs of the plugins on the selected rows.
	std::vector<ProcessorId> ProcessorIds = m_owner.GetProcessorIdsForRows(selectedRows);

	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		// New MappingID which should be applied to all plugins in the selected rows.
		auto newMapping = static_cast<MappingId>(comboBox->getSelectedId());
		for (std::size_t i = 0; i < ProcessorIds.size(); ++i)
		{
			// Set the value of the combobox to the current MappingID of the corresponding plugin.
			SoundsourceProcessor* plugin = ctrl->GetProcessor(ProcessorIds[i]);
			if (plugin)
				plugin->SetMappingId(DCS_Overview, newMapping);
		}
	}
}

/**
 * Reimplemented from Component, used to resize the actual combo box component inside.
 */
void ComboBoxContainer::resized()
{
	m_comboBox.setBoundsInset(BorderSize<int>(4, 4, 5, 4));
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updated the combo box's selected item according to that plugin's MappingID.
 * @param newRow	The new row number.
 */
void ComboBoxContainer::SetRow(int newRow)
{
	m_row = newRow;

	// Find the plugin instance corresponding to the given row number.
	ProcessorId ProcessorId = m_owner.GetProcessorIdForRow(newRow);
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		// Set the value of the combobox to the current MappingID of the corresponding plugin.
		const SoundsourceProcessor* plugin = ctrl->GetProcessor(ProcessorId);
		if (plugin)
			m_comboBox.setSelectedId(plugin->GetMappingId(), dontSendNotification);
	}
}


/*
===============================================================================
 Class TextEditorContainer
===============================================================================
*/

/**
 * Class constructor.
 */
TextEditorContainer::TextEditorContainer(TableModelComponent& td)
	: m_owner(td)
{
	// Create and configure actual textEditor component inside this container.
	m_editor.addListener(this);
	addAndMakeVisible(m_editor);
}

/**
 * Class destructor.
 */
TextEditorContainer::~TextEditorContainer()
{
}

/**
 * Reimplemented from TextEditor::Listener, gets called whenever the TextEditor loses keyboard focus.
 * @param textEditor	The textEditor which has been changed.
 */
void TextEditorContainer::textEditorFocusLost(TextEditor& textEditor)
{
	// Get the list of rows which are currently selected on the table.
	std::vector<int> selectedRows = m_owner.GetSelectedRows();
	if ((selectedRows.size() < 2) ||
		(std::find(selectedRows.begin(), selectedRows.end(), m_row) == selectedRows.end()))
	{
		// If this comboBoxes row (m_row) is NOT selected, or if no multi-selection was made 
		// then modify the selectedRows list so that it only contains m_row.
		selectedRows.clear();
		selectedRows.push_back(m_row);
	}

	// Get the IDs of the plugins on the selected rows.
	std::vector<ProcessorId> ProcessorIds = m_owner.GetProcessorIdsForRows(selectedRows);

	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		// New SourceID which should be applied to all plugins in the selected rows.
		int newSourceId;
		newSourceId = textEditor.getText().getIntValue();
		for (std::size_t i = 0; i < ProcessorIds.size(); ++i)
		{
			// Set the value of the combobox to the current MappingID of the corresponding plugin.
			SoundsourceProcessor* plugin = ctrl->GetProcessor(ProcessorIds[i]);
			if (plugin)
				plugin->SetSourceId(DCS_Overview, newSourceId);
		}
	}
}

/**
 * Callback function for Enter key presses on textEditors.
 * @param textEditor	The TextEditor object whose where enter key was pressed.
 */
void TextEditorContainer::textEditorReturnKeyPressed(TextEditor& textEditor)
{
	ignoreUnused(textEditor);

	// Remove keyboard focus from this editor. 
	// Function textEditorFocusLost will then take care of setting values.
	//m_owner.grabKeyboardFocus();

	textEditor.unfocusAllComponents();
	unfocusAllComponents();
}

/**
 * Reimplemented from Component, used to resize the actual component inside.
 */
void TextEditorContainer::resized()
{
	m_editor.setBoundsInset(BorderSize<int>(4, 4, 5, 4));
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updates the text inside the textEditor with the current SourceID
 * @param newRow	The new row number.
 */
void TextEditorContainer::SetRow(int newRow)
{
	m_row = newRow;

	// Find the plugin instance corresponding to the given row number.
	ProcessorId ProcessorId = m_owner.GetProcessorIdForRow(newRow);
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		// Set the value of the textEditor to the current SourceID of the corresponding plugin.
		const SoundsourceProcessor* plugin = ctrl->GetProcessor(ProcessorId);
		if (plugin)
			m_editor.setText(String(plugin->GetSourceId()), false);
	}
}


/*
===============================================================================
 Class RadioButtonContainer
===============================================================================
*/

/**
 * Class constructor.
 */
RadioButtonContainer::RadioButtonContainer(TableModelComponent& td)
	: m_owner(td)
{
	auto blueColour = Colours::blue;
	auto lookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (lookAndFeel)
		blueColour = lookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::ButtonBlueColor);

	// Create and configure button components inside this container.
	m_txButton.setButtonText("Tx");
	m_txButton.setClickingTogglesState(true);
	m_txButton.setColour(TextButton::ColourIds::buttonOnColourId, blueColour.brighter(0.05f));
	m_txButton.setEnabled(true);
	m_txButton.addListener(this);
	addAndMakeVisible(m_txButton);

	m_rxButton.setButtonText("Rx");
	m_rxButton.setClickingTogglesState(true);
	m_rxButton.setColour(TextButton::ColourIds::buttonOnColourId, blueColour.brighter(0.05f));
	m_rxButton.setEnabled(true);
	m_rxButton.addListener(this);
	addAndMakeVisible(m_rxButton);
}

/**
 * Class destructor.
 */
RadioButtonContainer::~RadioButtonContainer()
{
}

/**
 * Reimplemented from Button::Listener, gets called whenever the buttons are clicked.
 * @param button	The button which has been clicked.
 */
void RadioButtonContainer::buttonClicked(Button *button)
{
	CController* ctrl = CController::GetInstance();
	if (ctrl && 
		((button == &m_txButton) || (button == &m_rxButton))) 
	{
		bool newToggleState = button->getToggleState();

		// Get the list of rows which are currently selected on the table.
		std::vector<int> selectedRows = m_owner.GetSelectedRows();
		if ((selectedRows.size() < 2) ||
			(std::find(selectedRows.begin(), selectedRows.end(), m_row) == selectedRows.end()))
		{
			// If this button's row (m_row) is NOT selected, or if no multi-selection was made 
			// then modify the selectedRows list so that it only contains m_row.
			selectedRows.clear();
			selectedRows.push_back(m_row);
		}

		// Get the IDs of the plugins on the selected rows.
		std::vector<ProcessorId> ProcessorIds = m_owner.GetProcessorIdsForRows(selectedRows);

		for (std::size_t i = 0; i < ProcessorIds.size(); ++i)
		{
			SoundsourceProcessor* plugin = ctrl->GetProcessor(ProcessorIds[i]);
			if (plugin)
			{
				ComsMode oldMode = plugin->GetComsMode();
				ComsMode newFlag = (button == &m_txButton) ? CM_Tx : CM_Rx;

				if (newToggleState == true)
					oldMode |= newFlag;
				else
					oldMode &= ~newFlag;

				plugin->SetComsMode(DCS_Overview, oldMode);
			}
		}
	}
}

/**
 * Reimplemented from Component, used to resize the actual component inside.
 */
void RadioButtonContainer::resized()
{
	auto bounds = getLocalBounds();
	bounds.removeFromBottom(1);
	auto singleButtonWidth = 0.5f * bounds.getWidth();

	auto buttonRect = bounds.removeFromLeft(singleButtonWidth).reduced(4);
	m_txButton.setBounds(buttonRect);
	buttonRect = bounds.removeFromLeft(singleButtonWidth).reduced(4);
	m_rxButton.setBounds(buttonRect);
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updates the radio buttons with the current ComsMode.
 * @param newRow	The new row number.
 */
void RadioButtonContainer::SetRow(int newRow)
{
	m_row = newRow;

	// Find the plugin instance corresponding to the given row number.
	ProcessorId ProcessorId = m_owner.GetProcessorIdForRow(newRow);
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		// Toggle the correct radio buttons to the current ComsMode of the corresponding plugin.
		const SoundsourceProcessor* plugin = ctrl->GetProcessor(ProcessorId);
		if (plugin)
		{
			const Array<AudioProcessorParameter*>& params = plugin->getParameters();
			AudioParameterChoice* param = dynamic_cast<AudioParameterChoice*>(params[ParamIdx_DelayMode]);
			if (param)
			{
				ComsMode newMode = plugin->GetComsMode();
				m_txButton.setToggleState(((newMode & CM_Tx) == CM_Tx), dontSendNotification);
				m_rxButton.setToggleState(((newMode & CM_Rx) == CM_Rx), dontSendNotification);
			}
		}
	}
}



/*
===============================================================================
 Class MuteButtonContainer
===============================================================================
*/

/**
 * Class constructor.
 */
MuteButtonContainer::MuteButtonContainer(TableModelComponent& td)
	: m_owner(td)
{
}

/**
 * Class destructor.
 */
MuteButtonContainer::~MuteButtonContainer()
{
}

/**
 * Helper method to update the map of bridging mute buttons by querying
 * data from controller. This should be called on configuration updates
 * that affect bridging protocol active state.
 */
void MuteButtonContainer::updateBridgingMuteButtons()
{
	CController* ctrl = CController::GetInstance();
	if (!ctrl)
		return;

	auto activeBridging = ctrl->GetActiveProtocolBridging();

	auto redColour = Colours::red;
	auto lookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (lookAndFeel)
		redColour = lookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::ButtonRedColor);

	for (auto type : m_knowntypes)
	{
		if (((activeBridging & type) == type) && (m_bridgingMutes.count(type) == 0))
		{
			m_bridgingMutes[type].setButtonText("Mute");
			m_bridgingMutes[type].setClickingTogglesState(true);
			m_bridgingMutes[type].setColour(TextButton::ColourIds::buttonOnColourId, redColour.brighter(0.05f));
			m_bridgingMutes[type].setEnabled(true);
			m_bridgingMutes[type].addListener(this);
			addAndMakeVisible(&m_bridgingMutes.at(type));
		}
		else if (((activeBridging & type) != type) && (m_bridgingMutes.count(type) > 0))
		{
			m_bridgingMutes.erase(type);
		}
	}

	resized();
}

/**
 * Reimplemented from Button::Listener, gets called whenever the buttons are clicked.
 * @param button	The button which has been clicked.
 */
void MuteButtonContainer::buttonClicked(Button* button)
{
	CController* ctrl = CController::GetInstance();
	if (!ctrl)
		return;

	for (auto type : m_knowntypes)
	{
		if ((m_bridgingMutes.count(type) > 0) && (button == &m_bridgingMutes.at(type)))
		{
			bool newToggleState = button->getToggleState();

			// Get the list of rows which are currently selected on the table.
			std::vector<int> selectedRows = m_owner.GetSelectedRows();
			if ((selectedRows.size() < 2) ||
				(std::find(selectedRows.begin(), selectedRows.end(), m_row) == selectedRows.end()))
			{
				// If this button's row (m_row) is NOT selected, or if no multi-selection was made 
				// then modify the selectedRows list so that it only contains m_row.
				selectedRows.clear();
				selectedRows.push_back(m_row);
			}

			// Get the IDs of the plugins on the selected rows.
			std::vector<ProcessorId> ProcessorIds = m_owner.GetProcessorIdsForRows(selectedRows);

			for (auto processorId : ProcessorIds)
			{
				ctrl->SetMuteBridgingSourceId(type, static_cast<juce::int16>(processorId), newToggleState);
			}
		}
	}
}

/**
 * Reimplemented from Component, used to resize the actual component inside.
 */
void MuteButtonContainer::resized()
{
	if (m_bridgingMutes.empty())
		return;

	auto bounds = getLocalBounds();
	bounds.removeFromBottom(1);
	auto singleButtonWidth = bounds.getWidth() / m_bridgingMutes.size();
    
    auto buttonText = ((1.5f * bounds.getHeight()) > singleButtonWidth) ? "M" : "Mute";

	for (auto& buttonKV : m_bridgingMutes)
	{
		auto buttonRect = bounds.removeFromLeft(singleButtonWidth).reduced(4);
		buttonKV.second.setBounds(buttonRect);
        buttonKV.second.setName(buttonText);
	}
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updates the radio buttons with the current mute state.
 * @param newRow	The new row number.
 */
void MuteButtonContainer::SetRow(int newRow)
{
	m_row = newRow;

	// Find the plugin instance corresponding to the given row number.
	ProcessorId processorId = m_owner.GetProcessorIdForRow(newRow);
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		for (auto type : m_knowntypes)
		{
			if (m_bridgingMutes.count(type) > 0)
			{
				m_bridgingMutes.at(type).setToggleState(ctrl->GetMuteBridgingSourceId(type, static_cast<uint16>(processorId)), dontSendNotification);
			}
		}
	}
}


/*
===============================================================================
 Class EditableLabelContainer
===============================================================================
*/

/**
 * Class constructor.
 */
EditableLabelContainer::EditableLabelContainer(TableModelComponent& td) 
	: m_owner(td)
{
	// Here we set the 'editOnDoubleClick' to true, but then override the 
	// mouseDoubleClick() method to prevent editing. This is to prevent the
	// TextEdit components on the OC_SourceID column from getting keyboard 
	// focus automatically when a row is selected.
	setEditable(false, true, false);
}

/**
 * Class destructor.
 */
EditableLabelContainer::~EditableLabelContainer()
{
}

/**
 * Reimplemented from Label, gets called whenever the label is clicked.
 * @param event		The mouse event properties.
 */
void EditableLabelContainer::mouseDown(const MouseEvent& event)
{
	// Emulate R1 behaviour that is not standard for Juce: if multiple rows are selected
	// and one of the selected rows is clicked, only this row should remain selected.
	// So here we clear the selection and further down the clicked row is selected.
	if ((m_owner.GetTable().getNumSelectedRows() > 1) && m_owner.GetTable().isRowSelected(m_row))
		m_owner.GetTable().deselectAllRows();

	// Single click on the label should simply select the row
	m_owner.GetTable().selectRowsBasedOnModifierKeys(m_row, event.mods, false);

	// Base class implementation.
	Label::mouseDown(event);
}

/**
 * Reimplemented from Label to prevent label editing (see setEditable(..)).
 * @param event		The mouse event properties.
 */
void EditableLabelContainer::mouseDoubleClick(const MouseEvent& event)
{
	ignoreUnused(event);

	// Do nothing.
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updates the text to the current plugins name.
 * @param newRow	The new row number.
 */
void EditableLabelContainer::SetRow(int newRow)
{
	m_row = newRow;
	String displayName;

	// Find the plugin instance corresponding to the given row number.
	ProcessorId ProcessorId = m_owner.GetProcessorIdForRow(newRow);
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		// Set the value of the combobox to the current MappingID of the corresponding plugin.
		SoundsourceProcessor* plugin = ctrl->GetProcessor(ProcessorId);
		if (plugin)
		{
			displayName = plugin->getProgramName(0);
			if (displayName.isEmpty())
				displayName = String("Input ") + String(plugin->GetSourceId());

		}
	}

	setText(displayName, dontSendNotification);
}


} // namespace SoundscapeBridgeApp
