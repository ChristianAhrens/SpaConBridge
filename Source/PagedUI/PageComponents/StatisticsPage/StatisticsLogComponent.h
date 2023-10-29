/* Copyright (c) 2020-2023, Christian Ahrens
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

#include "../../../SpaConBridgeCommon.h"


namespace SpaConBridge
{


typedef int LogConstant;
static constexpr LogConstant	LC_HOR_DEFAULTSTEPPING = 400;		// 400ms default refresh resolution


/**
 * StatisticsLog class provides a rolling log to show protocol data.
 */
class StatisticsLog :	public Component,
						private Timer,
						public TableListBoxModel
{
public:
	/**
	 * Enum to define where a log entry originates from.
	 * This is used to e.g. differentiate between different DS100 in log,
	 * but show only a single DS100 category in plot.
	 */
	enum StatisticsLogSource
	{
		SLS_None,
		SLS_DiGiCo,
		SLS_DAWPlugin,
		SLS_BlacktraxRTTrPM,
		SLS_GenericOSC,
		SLS_GenericMIDI,
		SLS_YamahaOSC,
		SLS_ADMOSC,
		SLS_DS100,
		SLS_DS100_2,
		SLS_RemapOSC,
	};

	enum StatisticsLogColumn
	{
		SLC_None = 0,		//< Juce column IDs start at 1
		SLC_Number,
		SLC_LogSourceName,
		SLC_ObjectName,
		SLC_SourceId,
		SLC_Value,
		SLC_LogSourceType,
		SLC_MAX_COLUMNS
	};

public:
	StatisticsLog();
	~StatisticsLog() override;

	//==============================================================================
	void AddMessageData(StatisticsLogSource logSourceType, const RemoteObjectIdentifier roi, const RemoteObjectMessageData& msgData);

	//==========================================================================
	void backgroundClicked(const MouseEvent&) override;
	int getNumRows() override;
	void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
	void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
	int getColumnAutoSizeWidth(int columnId) override;

	//==========================================================================
	void SetShowDS100Traffic(bool show);

protected:
	//==============================================================================
	void resized() override;

private:
	String GetLogSourceName(StatisticsLogSource logSourceType);
	const Colour GetLogSourceColour(StatisticsLogSource logSourceType);

	//==============================================================================
	void timerCallback() override;

private:
	std::unique_ptr<TableListBox>			m_table;				/**< The table component itself. */
	std::map<int, std::map<int, String>>	m_logEntries;			/**< Map of log entry # and map of column and its cell string contents. */
	const int								m_logCount{ 200 };
	int										m_logEntryCounter{ 0 };
	bool									m_dataChanged{ false };
	bool									m_showDS100Traffic{ false };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatisticsLog)
};


} // namespace SpaConBridge
