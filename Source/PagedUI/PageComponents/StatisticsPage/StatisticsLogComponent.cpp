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

#include "StatisticsLogComponent.h"

#include "../../../Controller.h"


namespace SpaConBridge
{


/*
===============================================================================
	Class StatisticsLog
===============================================================================
*/

/**
 * Class constructor.
 */
StatisticsLog::StatisticsLog()
{
	m_table = std::make_unique<TableListBox>();
	m_table->setModel(this);
	m_table->setRowHeight(25);
	m_table->setOutlineThickness(1);
	m_table->setClickingTogglesRowSelection(false);
	m_table->setMultipleSelectionEnabled(true);
	addAndMakeVisible(m_table.get());

	int tableHeaderFlags = (TableHeaderComponent::visible);
	m_table->getHeader().addColumn("", SLC_Number, 60, 60, -1, tableHeaderFlags);
	m_table->getHeader().addColumn("Remote Object", SLC_ObjectName, 120, 120, -1, tableHeaderFlags);
	m_table->getHeader().addColumn("Ch.", SLC_SourceId, 35, 35, -1, tableHeaderFlags);
	m_table->getHeader().addColumn("Value", SLC_Value, 70, 70, -1, tableHeaderFlags);
	m_table->getHeader().addColumn("Origin", SLC_LogSourceName, 60, 60, -1, tableHeaderFlags);

	startTimer(LC_HOR_DEFAULTSTEPPING);
}

/**
	* Class destructor.
	*/
StatisticsLog::~StatisticsLog()
{
}

/**
 * Reimplemented to resize and re-postion controls.
 */
void StatisticsLog::resized()
{
	m_table->setBounds(getLocalBounds());
}

/**
 * Reimplemented from Timer - called every timeout timer
 *
 * Iterates over logging queue and adds all queued messages
 * to end of text area and terminates with a newline
 */
void StatisticsLog::timerCallback()
{
	// check if new data is ready to be visualized
	if (!m_dataChanged)
		return;
	else
	{
		m_dataChanged = false;
		m_table->repaint();
	}
}

/**
 * Method to add the received message data for given receiving bridging type.
 * @param logSourceType	The type of the bridging protocol that received the data
 * @param Id			The remote object id that was received
 * @param msgData		The actual message data that shall be logged
 */
void StatisticsLog::AddMessageData(StatisticsLogSource logSourceType, RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData)
{
	if (!m_showDS100Traffic && (logSourceType == SLS_DS100 || logSourceType == SLS_DS100_2))
		return;

	String valueString;
	if (msgData._payload != 0)
	{
		if (msgData._valType == ROVT_FLOAT)
		{
			float fvalue;
			for (int i = 0; i < msgData._valCount; ++i)
			{
				fvalue = static_cast<float*>(msgData._payload)[i];
				valueString += String(fvalue, 2) + ",";
			}
		}
		else if (msgData._valType == ROVT_INT)
		{
			int ivalue;
			for (int i = 0; i < msgData._valCount; ++i)
			{
				ivalue = static_cast<int*>(msgData._payload)[i];
				valueString += String(ivalue) + ",";
			}
		}
	}

	m_logEntryCounter++;
	// We do not want to modify the entire container for every dataset that is added - 
	// therefor the index into the map is changed regarding where the first entry is expected. 
	// This is of course regarded both where the data is inserted (here) and where it is extracted (::paintCell).
	auto mapIdx = m_logEntryCounter % m_logCount; 
	jassert(mapIdx >= 0);
	m_logEntries[mapIdx][SLC_Number] = String(m_logEntryCounter);
	m_logEntries[mapIdx][SLC_ObjectName] = ProcessingEngineConfig::GetObjectShortDescription(Id);
	m_logEntries[mapIdx][SLC_SourceId] = String(msgData._addrVal._first);
	m_logEntries[mapIdx][SLC_Value] = valueString;
	m_logEntries[mapIdx][SLC_LogSourceName] = GetLogSourceName(logSourceType);
	m_logEntries[mapIdx][SLC_LogSourceType] = String(logSourceType);

	m_dataChanged = true;
}

/**
 * This can be overridden to react to the user double-clicking on a part of the list where there are no rows.
 * @param event	Contains position and status information about a mouse event.
 */
void StatisticsLog::backgroundClicked(const MouseEvent& event)
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
int StatisticsLog::getNumRows()
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
void StatisticsLog::paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
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
void StatisticsLog::paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
	ignoreUnused(rowIsSelected);
	ignoreUnused(rowIsSelected);

	// Reconstruct the index into the map
	auto mapIdx = ((m_logEntryCounter - rowNumber) % m_logCount);
	// sanity check for index into data map - while table is not fully populated, invalid indices must be caught here.
	if (mapIdx < 0)
		return;

	auto cellRect = Rectangle<int>(width, height);

	if (columnId == SLC_Number)
	{
		auto logSource = static_cast<StatisticsLogSource>(m_logEntries[mapIdx][SLC_LogSourceType].getIntValue());
		auto colour = GetLogSourceColour(logSource);
		if (colour.isTransparent())
			g.setColour(getLookAndFeel().findColour(TableListBox::textColourId));
		else
			g.setColour(colour);
		cellRect.removeFromRight(5);
		g.drawFittedText(m_logEntries[mapIdx][columnId], cellRect, Justification::centredRight, 1);
	}
	else
	{
		g.setColour(getLookAndFeel().findColour(TableListBox::textColourId));
		g.drawFittedText(m_logEntries[mapIdx][columnId], cellRect, Justification::centred, 1);
	}
}

/**
 * This is overloaded from TableListBoxModel, and should choose the best width for the specified column.
 * @param columnId	Desired column ID.
 * @return	Width to be used for the desired column.
 */
int StatisticsLog::getColumnAutoSizeWidth(int columnId)
{
	switch (columnId)
	{
	case SLC_Number:
		return 60;
	case SLC_ObjectName:
		return 120;
	case SLC_SourceId:
		return 40;
	case SLC_Value:
		return 60;
	case SLC_LogSourceName:
		return 60;
	default:
		break;
	}

	return 0;
}

/**
 * Setter for 'showDS100Traffic' member variable.
 * @param show	The value to set to member variable.
 */
void StatisticsLog::SetShowDS100Traffic(bool show)
{
	m_showDS100Traffic = show;
}

/**
 * Helper method to get a user displayable/readable string representation for the log source.
 * This gets the generic protocol names and applies some logic regarding what log source is what protocol.
 * @param logSourceType	The type to get a string representation for.
 * @retun	The requested string representation.
 */
String StatisticsLog::GetLogSourceName(StatisticsLogSource logSourceType)
{
	switch (logSourceType)
	{
	case SLS_DiGiCo:
		return GetProtocolBridgingShortName(PBT_DiGiCo);
	case SLS_BlacktraxRTTrPM:
		return GetProtocolBridgingShortName(PBT_BlacktraxRTTrPM);
	case SLS_GenericOSC:
		return GetProtocolBridgingShortName(PBT_GenericOSC);
	case SLS_GenericMIDI:
		return GetProtocolBridgingShortName(PBT_GenericMIDI);
	case SLS_YamahaSQ:
		return GetProtocolBridgingShortName(PBT_YamahaSQ);
	case SLS_YamahaOSC:
		return GetProtocolBridgingShortName(PBT_YamahaOSC);
	case SLS_ADMOSC:
		return GetProtocolBridgingShortName(PBT_ADMOSC);
	case SLS_HUI:
		return GetProtocolBridgingShortName(PBT_HUI);
	case SLS_DS100:
		return GetProtocolBridgingShortName(PBT_DS100);
	case SLS_DS100_2:
		return GetProtocolBridgingShortName(PBT_DS100) + "(2nd)";
	default:
		return GetProtocolBridgingShortName(PBT_None);
	}
}

/**
 * Helper method to get a colour representation for the log source.
 * This uses the generic colour getter for a ProtocolBridgingType.
 * @param logSourceType	The type to get a colour representation for.
 * @retun	The requested colour.
 */
const Colour StatisticsLog::GetLogSourceColour(StatisticsLogSource logSourceType)
{
	switch (logSourceType)
	{
	case SLS_DiGiCo:
		return GetProtocolBridgingColour(PBT_DiGiCo);
	case SLS_BlacktraxRTTrPM:
		return GetProtocolBridgingColour(PBT_BlacktraxRTTrPM);
	case SLS_GenericOSC:
		return GetProtocolBridgingColour(PBT_GenericOSC);
	case SLS_GenericMIDI:
		return GetProtocolBridgingColour(PBT_GenericMIDI);
	case SLS_YamahaSQ:
		return GetProtocolBridgingColour(PBT_YamahaSQ);
	case SLS_YamahaOSC:
		return GetProtocolBridgingColour(PBT_YamahaOSC);
	case SLS_ADMOSC:
		return GetProtocolBridgingColour(PBT_ADMOSC);
	case SLS_HUI:
		return GetProtocolBridgingColour(PBT_HUI);
	case SLS_DS100:
	case SLS_DS100_2:
		return GetProtocolBridgingColour(PBT_DS100);
	default:
		return GetProtocolBridgingColour(PBT_None);
	}
}


} // namespace SpaConBridge
