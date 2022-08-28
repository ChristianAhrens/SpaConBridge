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


#pragma once

#include "../../SpaConBridgeCommon.h"


namespace SpaConBridge
{


/**
 * Class BridgingAwareTableHeaderComponent acts as a table model and a component at the same time.
 */
class BridgingAwareTableHeaderComponent : public TableHeaderComponent
{
public:
	/**
	 * Enum defininig the table columns available for the channel table derivates.
	 */
	enum TableColumn
	{
		TC_None = 0,		//< Juce column IDs start at 1
		TC_EmptyHandleCellID,
		TC_SoundobjectID,
		TC_InputID,
		TC_OutputID,
		TC_Name,
		TC_InputEditor,
		TC_OutputEditor,
		TC_Mapping,
		TC_ComsMode,
		TC_SoundobjectColourAndSize,
		TC_BridgingMute,
		TC_MAX_COLUMNS
	};

	/**
	 * Structure that collects all properties needed to initialize a table column.
	 */
	struct ColumnProperties
	{
		ColumnProperties() {};
		ColumnProperties(
			String	columnName,
			int		width,
			int		minimumWidth,
			int		maximumWidth,
			int		propertyFlags,
			int		insertIndex = -1)
		{
			_columnName = columnName;
			_width = width;
			_minimumWidth = minimumWidth;
			_maximumWidth = maximumWidth;
			_propertyFlags = propertyFlags;
			_insertIndex = insertIndex;
		};

		String	_columnName;
		int		_width;
		int		_minimumWidth;
		int		_maximumWidth;
		int		_propertyFlags;
		int		_insertIndex;
	};

public:
	BridgingAwareTableHeaderComponent(const std::map<TableColumn, ColumnProperties>& tableColumns, TableColumn sortColumn = TC_None);
	~BridgingAwareTableHeaderComponent() override;

	void paint(Graphics& g) override;
	void resized() override;

	void updateBridgingTitles();
	void updateColumnWidths();

	
private:
	std::map<ProtocolBridgingType, bool>	m_bridgingProtocolActive;
};


} // namespace SpaConBridge
