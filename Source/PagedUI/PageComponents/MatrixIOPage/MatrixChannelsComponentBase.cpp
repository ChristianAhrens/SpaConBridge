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

#include "MatrixChannelsComponentBase.h"

#include "../../../Controller.h"


namespace SoundscapeBridgeApp
{


/*
===============================================================================
	Class MatrixChannelsComponentBase
===============================================================================
*/

/**
 * Class constructor.
 */
MatrixChannelsComponentBase::MatrixChannelsComponentBase()
{
	m_table = std::make_unique<TableListBox>();
	m_table->setModel(this);
	m_table->setRowHeight(50);
	m_table->setOutlineThickness(1);
	m_table->setClickingTogglesRowSelection(false);
	m_table->setMultipleSelectionEnabled(true);
	addAndMakeVisible(m_table.get());

	int tableHeaderFlags = (TableHeaderComponent::visible);
	m_table->getHeader().addColumn("Input #",		MCC_SourceID,		60, 60, -1, tableHeaderFlags);
	m_table->getHeader().addColumn("Remote Object", MCC_InputEditor,	140, 140, -1, tableHeaderFlags);
	m_table->getHeader().addColumn("Mode",			MCC_ComsMode,		90, 90, -1, tableHeaderFlags);
	m_table->getHeader().addColumn("B",				MCC_BridgingMute,	90, 90, -1, tableHeaderFlags);
	m_table->getHeader().setSortColumnId(MCC_SourceID, true); // sort forwards by the Input number column
}

/**
	* Class destructor.
	*/
MatrixChannelsComponentBase::~MatrixChannelsComponentBase()
{
}

/**
 * Reimplemented to resize and re-postion controls.
 */
void MatrixChannelsComponentBase::resized()
{
	m_table->setBounds(getLocalBounds());
}

/**
 * This can be overridden to react to the user double-clicking on a part of the list where there are no rows.
 * @param event	Contains position and status information about a mouse event.
 */
void MatrixChannelsComponentBase::backgroundClicked(const MouseEvent& event)
{
	// Clear selection
	m_table->deselectAllRows();

	// Base class implementation.
	TableListBoxModel::backgroundClicked(event);
}

/**
 * This is overloaded from TableListBoxModel, and must return the total number of rows in our table.
 * @return	Number of rows on the table, equal to number of procssor instances.
 */
int MatrixChannelsComponentBase::getNumRows()
{
	return m_logCount;
}

/**
 * This is overloaded from TableListBoxModel, and should fill in the background of the whole row.
 * @param g					Graphics context that must be used to do the drawing operations.
 * @param rowNumber			Number of row to paint.
 * @param width				Width of area to paint.
 * @param height			Height of area to paint.
 * @param rowIsSelected		True if row is currently selected.
 */
void MatrixChannelsComponentBase::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
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
void MatrixChannelsComponentBase::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	ignoreUnused(g);
	ignoreUnused(rowNumber);
	ignoreUnused(columnId);
	ignoreUnused(width);
	ignoreUnused(height);
	ignoreUnused(rowIsSelected);
}

/**
 * This is overloaded from TableListBoxModel, and should choose the best width for the specified column.
 * @param columnId	Desired column ID.
 * @return	Width to be used for the desired column.
 */
int MatrixChannelsComponentBase::getColumnAutoSizeWidth(int columnId)
{
	switch (columnId)
	{
	case MCC_SourceID:
		return 60;
	case MCC_InputEditor:
		return 140;
	case MCC_ComsMode:
		return 90;
	case MCC_BridgingMute:
		return 90;
	default:
		break;
	}

	return 0;
}


} // namespace SoundscapeBridgeApp
