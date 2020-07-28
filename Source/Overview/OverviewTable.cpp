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
#include "../Controller.h"
#include "../SoundsourceProcessor/SurfaceSlider.h"

#include "../submodules/JUCE-AppBasics/Source/Image_utils.hpp"


namespace SoundscapeBridgeApp
{


/*
===============================================================================
 Class COverviewTableContainer
===============================================================================
*/

/**
 * Class constructor.
 */
COverviewTableContainer::COverviewTableContainer()
	: AOverlay(OT_Overview)
{
	// Create the table model/component.
	m_overviewTable = std::make_unique<CTableModelComponent>();
	m_overviewTable->currentSelectedProcessorChanged = [=](ProcessorId id) { this->onCurrentSelectedProcessorChanged(id); };
	addAndMakeVisible(m_overviewTable.get());

	// Add/Remove Buttons
	m_addInstance = std::make_unique<CButton>("Add");
	m_addInstance->setClickingTogglesState(false);
	m_addInstance->addListener(this);
	addAndMakeVisible(m_addInstance.get());
	m_removeInstance = std::make_unique<CButton>("Remove");
	m_removeInstance->setClickingTogglesState(false);
	m_removeInstance->setEnabled(false);
	m_removeInstance->addListener(this);
	addAndMakeVisible(m_removeInstance.get());

	// Create quick selection buttons
	m_selectLabel = std::make_unique<CLabel>("Select:", "Select:");
	addAndMakeVisible(m_selectLabel.get());

	m_selectAll = std::make_unique<CButton>("All");
	m_selectAll->setEnabled(true);
	m_selectAll->addListener(this);
	addAndMakeVisible(m_selectAll.get());

	m_selectNone = std::make_unique<CButton>("None");
	m_selectNone->setEnabled(true);
	m_selectNone->addListener(this);
	addAndMakeVisible(m_selectNone.get());
}

/**
 * Class destructor.
 */
COverviewTableContainer::~COverviewTableContainer()
{
}

/**
 * Reimplemented to paint background and frame.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void COverviewTableContainer::paint(Graphics& g)
{
	int w = getLocalBounds().getWidth();
	int h = getLocalBounds().getHeight();	

	// Background
	g.setColour(CDbStyle::GetDbColor(CDbStyle::MidColor));
	g.fillRect(Rectangle<int>(8, h - 41, w - 16, 34));

	// Frame
	g.setColour(CDbStyle::GetDbColor(CDbStyle::DarkLineColor));
	g.drawRect(Rectangle<int>(8, h - 41, w - 16, 34), 1);
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void COverviewTableContainer::resized()
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
		FlexItem(*m_addInstance.get()).withFlex(1).withMaxWidth(40).withMargin(FlexItem::Margin(2)),
		FlexItem(*m_removeInstance.get()).withFlex(1).withMaxWidth(60).withMargin(FlexItem::Margin(2)),
		FlexItem().withFlex(2).withHeight(30),
		FlexItem(*m_selectLabel.get()).withFlex(1).withMaxWidth(80),
		FlexItem(*m_selectAll.get()).withFlex(1).withMaxWidth(40).withMargin(FlexItem::Margin(2)),
		FlexItem(*m_selectNone.get()).withFlex(1).withMaxWidth(46).withMargin(FlexItem::Margin(2)),
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
void COverviewTableContainer::buttonClicked(Button *button)
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
				auto processor = std::make_unique<SoundscapeBridgeApp::SoundsourceProcessor>();
				processor.release(); // let go of the instance here, we do not want to destroy it, since it lives as member of CCOntroller when constructed
			}
			else
			{
				auto const& processorIds = m_overviewTable->GetSelectedRows();

				if (ctrl->GetProcessorCount() <= processorIds.size())
					onCurrentSelectedProcessorChanged(INVALID_PROCESSOR_ID);
				else
				{
					ProcessorId nextStillExistingId = static_cast<ProcessorId>(ctrl->GetProcessorCount() - processorIds.size());
					m_overviewTable->selectedRowsChanged(nextStillExistingId);
					//onCurrentSelectedProcessorChanged(nextStillExistingId);
				}

				for (auto processorId : processorIds)
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
void COverviewTableContainer::onCurrentSelectedProcessorChanged(ProcessorId selectedProcessorId)
{
	if (selectedProcessorId == INVALID_PROCESSOR_ID)
	{
		if (m_selectedProcessorInstanceEditor)
		{
			removeChildComponent(m_selectedProcessorInstanceEditor.get());
			m_selectedProcessorInstanceEditor.release();
			resized();
		}

		m_removeInstance->setEnabled(false);
	}
	else
	{
		CController* ctrl = CController::GetInstance();
		if (ctrl)
		{
			auto processor = ctrl->GetProcessor(selectedProcessorId);
			m_selectedProcessorInstanceEditor = std::make_unique<SoundsourceProcessorEditor>(*processor);
			addAndMakeVisible(m_selectedProcessorInstanceEditor.get());
			m_selectedProcessorInstanceEditor->UpdateGui(true);
			resized();
		}

		m_removeInstance->setEnabled(true);
	}
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the plugin parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void COverviewTableContainer::UpdateGui(bool init)
{
	CController* ctrl = CController::GetInstance();
	if (ctrl && m_overviewTable)
	{
		if (ctrl->PopParameterChanged(DCS_Overview, DCT_NumPlugins) || init)
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


/*
===============================================================================
 Class CTableModelComponent
===============================================================================
*/

/**
 * Class constructor.
 */
CTableModelComponent::CTableModelComponent()
{
	// This fills m_ids.
	RecreateTableRowIds();

	// Create our table component and add it to this component..
	addAndMakeVisible(m_table);
	m_table.setModel(this);

	// Add columns to the table header
	int tableHeaderFlags = (TableHeaderComponent::visible | TableHeaderComponent::sortable);
	m_table.getHeader().addColumn("Track", OC_TrackID, 50, 30, -1, tableHeaderFlags);
	m_table.getHeader().addColumn("Input", OC_SourceID, 50, 30, -1, tableHeaderFlags);
	m_table.getHeader().addColumn("Mapping", OC_Mapping, 50, 30, -1, tableHeaderFlags);
	m_table.getHeader().addColumn("Mode", OC_ComsMode, 50, 30, -1, tableHeaderFlags);
	m_table.getHeader().setSortColumnId(OC_SourceID, true); // sort forwards by the Input number column
	m_table.getHeader().setStretchToFitActive(true);

	// Header colors
	m_table.getHeader().setColour(TableHeaderComponent::textColourId, CDbStyle::GetDbColor(CDbStyle::TextColor));
	m_table.getHeader().setColour(TableHeaderComponent::backgroundColourId, CDbStyle::GetDbColor(CDbStyle::MidColor));
	m_table.getHeader().setColour(TableHeaderComponent::outlineColourId, CDbStyle::GetDbColor(CDbStyle::DarkLineColor));
	m_table.getHeader().setColour(TableHeaderComponent::highlightColourId, CDbStyle::GetDbColor(CDbStyle::HighlightColor));

	// Scroll bar colors
	m_table.getVerticalScrollBar().setColour(ScrollBar::backgroundColourId, CDbStyle::GetDbColor(CDbStyle::MidColor));
	m_table.getVerticalScrollBar().setColour(ScrollBar::thumbColourId, CDbStyle::GetDbColor(CDbStyle::DarkTextColor));
	m_table.getVerticalScrollBar().setColour(ScrollBar::trackColourId, CDbStyle::GetDbColor(CDbStyle::MidColor));

	// Table colors
	m_table.setColour(TableListBox::backgroundColourId, CDbStyle::GetDbColor(CDbStyle::DarkColor));
	m_table.setColour(TableListBox::outlineColourId, CDbStyle::GetDbColor(CDbStyle::DarkLineColor));
	m_table.setColour(TableListBox::textColourId, CDbStyle::GetDbColor(CDbStyle::TextColor));

	m_table.setRowHeight(33);
	m_table.setOutlineThickness(1);
	m_table.setClickingTogglesRowSelection(false);
	m_table.setMultipleSelectionEnabled(true);
}

/**
 * Class destructor.
 */
CTableModelComponent::~CTableModelComponent()
{
}

/**
 * Get the ID of the plugin instance corresponding to the given table row number.
 * @param rowNumber	The desired row number (starts at 0).
 * @return	The ID of the plugin instance at that row number, if any.
 */
ProcessorId CTableModelComponent::GetProcessorIdForRow(int rowNumber)
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
std::vector<ProcessorId> CTableModelComponent::GetProcessorIdsForRows(std::vector<int> rowNumbers)
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
std::vector<int> CTableModelComponent::GetSelectedRows() const
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
void CTableModelComponent::SelectAllRows(bool all)
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
bool CTableModelComponent::LessThanSourceId(ProcessorId pId1, ProcessorId pId2)
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
bool CTableModelComponent::LessThanMapping(ProcessorId pId1, ProcessorId pId2)
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
bool CTableModelComponent::LessThanComsMode(ProcessorId pId1, ProcessorId pId2)
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
 * This clears and re-fills m_ids.
 */
void CTableModelComponent::RecreateTableRowIds()
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
void CTableModelComponent::UpdateTable()
{
	// Re-sort table again depending on the currently selected column.
	sortOrderChanged(m_table.getHeader().getSortColumnId(), m_table.getHeader().isSortedForwards());

	// Refresh table
	m_table.updateContent();
}

/**
 * This can be overridden to react to the user double-clicking on a part of the list where there are no rows. 
 * @param event	Contains position and status information about a mouse event.
 */
void CTableModelComponent::backgroundClicked(const MouseEvent &event)
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
int CTableModelComponent::getNumRows()
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
void CTableModelComponent::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
	ignoreUnused(rowNumber);

	// Selected rows have a different background color.
	if (rowIsSelected)
		g.setColour(CDbStyle::GetDbColor(CDbStyle::HighlightColor));
	else
		g.setColour(CDbStyle::GetDbColor(CDbStyle::MidColor));
	g.fillRect(0, 0, width, height - 1);

	// Line between rows.
	g.setColour(CDbStyle::GetDbColor(CDbStyle::DarkLineColor));
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
void CTableModelComponent::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
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
void CTableModelComponent::sortOrderChanged(int newSortColumnId, bool isForwards)
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
		std::sort(m_ids.begin(), m_ids.end(), CTableModelComponent::LessThanSourceId);
		break;
	case OC_Mapping:
		std::sort(m_ids.begin(), m_ids.end(), CTableModelComponent::LessThanMapping);
		break;
	case OC_ComsMode:
		std::sort(m_ids.begin(), m_ids.end(), CTableModelComponent::LessThanComsMode);
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
Component* CTableModelComponent::refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, Component* existingComponentToUpdate)
{
	ignoreUnused(isRowSelected);

	Component* ret = nullptr;

	switch (columnId)
	{
		case OC_TrackID:
		{
			CEditableLabelContainer* label = static_cast<CEditableLabelContainer*> (existingComponentToUpdate);

			// If an existing component is being passed-in for updating, we'll re-use it, but
			// if not, we'll have to create one.
			if (label == nullptr)
				label = new CEditableLabelContainer(*this);

			// Ensure that the component knows which row number it is located at.
			label->SetRow(rowNumber);

			// Return a pointer to the component.
			ret = label;
		}
		break;

	case OC_Mapping:
		{
			CComboBoxContainer* comboBox = static_cast<CComboBoxContainer*> (existingComponentToUpdate);

			// If an existing component is being passed-in for updating, we'll re-use it, but
			// if not, we'll have to create one.
			if (comboBox == nullptr)
				comboBox = new CComboBoxContainer(*this);

			// Ensure that the comboBox knows which row number it is located at.
			comboBox->SetRow(rowNumber);

			// Return a pointer to the comboBox.
			ret = comboBox;
		}
		break;
	case OC_SourceID:
		{
			CTextEditorContainer* textEdit = static_cast<CTextEditorContainer*> (existingComponentToUpdate);

			// If an existing component is being passed-in for updating, we'll re-use it, but
			// if not, we'll have to create one.
			if (textEdit == nullptr)
				textEdit = new CTextEditorContainer(*this);

			// Ensure that the component knows which row number it is located at.
			textEdit->SetRow(rowNumber);

			// Return a pointer to the component.
			ret = textEdit;
		}
		break;

	case OC_ComsMode:
		{
			CRadioButtonContainer* radioButton = static_cast<CRadioButtonContainer*> (existingComponentToUpdate);

			// If an existing component is being passed-in for updating, we'll re-use it, but
			// if not, we'll have to create one.
			if (radioButton == nullptr)
				radioButton = new CRadioButtonContainer(*this);

			// Ensure that the component knows which row number it is located at.
			radioButton->SetRow(rowNumber);

			// Return a pointer to the component.
			ret = radioButton;
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
int CTableModelComponent::getColumnAutoSizeWidth(int columnId)
{
	switch (columnId)
	{
	case OC_TrackID:
		return 50;
	case OC_SourceID:
		return 50;
	case OC_Mapping:
		return 100;
	case OC_ComsMode:
		return 100;
	default:
		break;
	}

	return 0;
}

/**
 * This is overloaded from TableListBoxModel, and tells us that the row selection has changed.
 * @param lastRowSelected	The last of the now selected rows.
 */
void CTableModelComponent::selectedRowsChanged(int lastRowSelected)
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
void CTableModelComponent::resized()
{
	m_table.setBounds(getLocalBounds());
}


/*
===============================================================================
 Class CComboBoxContainer
===============================================================================
*/

/**
 * Class constructor.
 */
CComboBoxContainer::CComboBoxContainer(CTableModelComponent& td)
	: m_owner(td)
{
	// Create and configure actual combo box component inside this container.
	m_comboBox.setEditableText(false);
	m_comboBox.addItem("1", 1);
	m_comboBox.addItem("2", 2);
	m_comboBox.addItem("3", 3);
	m_comboBox.addItem("4", 4);
	m_comboBox.addListener(this);
	m_comboBox.setColour(ComboBox::backgroundColourId, CDbStyle::GetDbColor(CDbStyle::DarkColor));
	m_comboBox.setColour(ComboBox::textColourId, CDbStyle::GetDbColor(CDbStyle::TextColor));
	m_comboBox.setColour(ComboBox::outlineColourId, CDbStyle::GetDbColor(CDbStyle::WindowColor));
	m_comboBox.setColour(ComboBox::buttonColourId, CDbStyle::GetDbColor(CDbStyle::MidColor));
	m_comboBox.setColour(ComboBox::arrowColourId, CDbStyle::GetDbColor(CDbStyle::TextColor));
	m_comboBox.setWantsKeyboardFocus(false);
	addAndMakeVisible(m_comboBox);
}

/**
 * Class destructor.
 */
CComboBoxContainer::~CComboBoxContainer()
{
}

/**
 * Reimplemented from ComboBox::Listener, gets called whenever the selected combo box item is changed.
 * @param comboBox	The comboBox which has been changed.
 */
void CComboBoxContainer::comboBoxChanged(ComboBox *comboBox)
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
		int newMapping = comboBox->getSelectedId();
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
void CComboBoxContainer::resized()
{
	m_comboBox.setBoundsInset(BorderSize<int>(4));
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updated the combo box's selected item according to that plugin's MappingID.
 * @param newRow	The new row number.
 */
void CComboBoxContainer::SetRow(int newRow)
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
 Class CTextEditorContainer
===============================================================================
*/

/**
 * Class constructor.
 */
CTextEditorContainer::CTextEditorContainer(CTableModelComponent& td)
	: m_owner(td)
{
	// Create and configure actual textEditor component inside this container.
	m_editor.addListener(this);
	addAndMakeVisible(m_editor);
}

/**
 * Class destructor.
 */
CTextEditorContainer::~CTextEditorContainer()
{
}

/**
 * Reimplemented from TextEditor::Listener, gets called whenever the TextEditor loses keyboard focus.
 * @param textEditor	The textEditor which has been changed.
 */
void CTextEditorContainer::textEditorFocusLost(TextEditor& textEditor)
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
	CTextEditor *myEditor = static_cast<CTextEditor*>(&textEditor);
	if (myEditor && ctrl)
	{
		// New SourceID which should be applied to all plugins in the selected rows.
		int newSourceId;
		newSourceId = myEditor->getText().getIntValue();
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
void CTextEditorContainer::textEditorReturnKeyPressed(TextEditor& textEditor)
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
void CTextEditorContainer::resized()
{
	m_editor.setBoundsInset(BorderSize<int>(4));
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updates the text inside the textEditor with the current SourceID
 * @param newRow	The new row number.
 */
void CTextEditorContainer::SetRow(int newRow)
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
 Class CRadioButtonContainer
===============================================================================
*/

/**
 * Class constructor.
 */
CRadioButtonContainer::CRadioButtonContainer(CTableModelComponent& td)
	: m_owner(td)
{
	// Create and configure button components inside this container.
	m_txButton.setName("Tx");
	m_txButton.setEnabled(true);
	m_txButton.addListener(this);
	addAndMakeVisible(m_txButton);

	m_rxButton.setName("Rx");
	m_rxButton.setEnabled(true);
	m_rxButton.addListener(this);
	addAndMakeVisible(m_rxButton);
}

/**
 * Class destructor.
 */
CRadioButtonContainer::~CRadioButtonContainer()
{
}

/**
 * Reimplemented from Button::Listener, gets called whenever the buttons are clicked.
 * @param button	The button which has been clicked.
 */
void CRadioButtonContainer::buttonClicked(Button *button)
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
void CRadioButtonContainer::resized()
{
	int w = getLocalBounds().getWidth();
	int h = getLocalBounds().getHeight();
	m_txButton.setBounds(2, 2, (w / 2) - 3, h - 5);
	m_rxButton.setBounds(w / 2, 2, (w / 2) - 3, h - 5);
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updates the radio buttons with the current ComsMode.
 * @param newRow	The new row number.
 */
void CRadioButtonContainer::SetRow(int newRow)
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
 Class CEditableLabelContainer
===============================================================================
*/

/**
 * Class constructor.
 */
CEditableLabelContainer::CEditableLabelContainer(CTableModelComponent& td) 
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
CEditableLabelContainer::~CEditableLabelContainer()
{
}

/**
 * Reimplemented from Label, gets called whenever the label is clicked.
 * @param event		The mouse event properties.
 */
void CEditableLabelContainer::mouseDown(const MouseEvent& event)
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
void CEditableLabelContainer::mouseDoubleClick(const MouseEvent& event)
{
	ignoreUnused(event);

	// Do nothing.
}

/**
 * Saves the row number where this component is located inside the overview table.
 * It also updates the text to the current plugins name.
 * @param newRow	The new row number.
 */
void CEditableLabelContainer::SetRow(int newRow)
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
