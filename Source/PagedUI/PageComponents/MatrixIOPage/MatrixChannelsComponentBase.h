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


#pragma once

#include "../../../SoundscapeBridgeAppCommon.h"


namespace SoundscapeBridgeApp
{


/**
 * MatrixChannelsComponentBase class provides a rolling log to show protocol data.
 */
class MatrixChannelsComponentBase :	public Component,
						private Timer,
						public TableListBoxModel
{
public:
	/**
	 * Enum to define where a log entry originates from.
	 * This is used to e.g. differentiate between different DS100 in log,
	 * but show only a single DS100 category in plot.
	 */
	enum MatrixChannelsComponentBaseSource
	{
		SLS_None,
		SLS_DiGiCo,
		SLS_BlacktraxRTTrPM,
		SLS_GenericOSC,
		SLS_GenericMIDI,
		SLS_YamahaSQ,
		SLS_YamahaOSC,
		SLS_HUI,
		SLS_DS100,
		SLS_DS100_2,
	};

	enum MatrixChannelsComponentBaseColumn
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
	MatrixChannelsComponentBase();
	virtual ~MatrixChannelsComponentBase() override;

	//==============================================================================
	void AddMessageData(MatrixChannelsComponentBaseSource logSourceType, RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData);

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
	String GetLogSourceName(MatrixChannelsComponentBaseSource logSourceType);
	const Colour GetLogSourceColour(MatrixChannelsComponentBaseSource logSourceType);

	//==============================================================================
	void timerCallback() override;

private:
	std::unique_ptr<TableListBox>			m_table;				/**< The table component itself. */
	std::map<int, std::map<int, String>>	m_logEntries;			/**< Map of log entry # and map of column and its cell string contents. */
	const int								m_logCount{ 200 };
	int										m_logEntryCounter{ 0 };
	bool									m_dataChanged{ false };
	bool									m_showDS100Traffic{ false };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatrixChannelsComponentBase)
};


} // namespace SoundscapeBridgeApp
