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
						public TableListBoxModel
{
	enum MatrixChannelsComponentBaseColumn
	{
		MCC_None = 0,		//< Juce column IDs start at 1
		MCC_SourceID,
		MCC_InputEditor,
		MCC_ComsMode,
		MCC_BridgingMute,
		MCC_MAX_COLUMNS
	};
public:
	MatrixChannelsComponentBase();
	virtual ~MatrixChannelsComponentBase() override;

	//==========================================================================
	void backgroundClicked(const MouseEvent&) override;
	int getNumRows() override;
	void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
	void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
	int getColumnAutoSizeWidth(int columnId) override;

protected:
	//==============================================================================
	void resized() override;

private:
	std::unique_ptr<TableListBox>			m_table;				/**< The table component itself. */
	const int								m_logCount{ 200 };
	bool									m_dataChanged{ false };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatrixChannelsComponentBase)
};


} // namespace SoundscapeBridgeApp
