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


#include "BridgingAwareTableHeaderComponent.h"


#include "../../Controller.h"
#include "../../LookAndFeel.h"

#include <Image_utils.h>


namespace SpaConBridge
{


/*
===============================================================================
 Class BridgingAwareTableHeaderComponent
===============================================================================
*/

/**
 * Class constructor.
 */
BridgingAwareTableHeaderComponent::BridgingAwareTableHeaderComponent(const std::map<TableColumn, ColumnProperties>& tableColumns, TableColumn sortColumn)
{
	// Add columns to the table header
	for (auto const& columnPropertiesKV : tableColumns)
	{
		addColumn(columnPropertiesKV.second._columnName,
			columnPropertiesKV.first,
			columnPropertiesKV.second._width,
			columnPropertiesKV.second._minimumWidth,
			columnPropertiesKV.second._maximumWidth,
			columnPropertiesKV.second._propertyFlags,
			columnPropertiesKV.second._insertIndex);
	}

	if (sortColumn != TC_None)
		setSortColumnId(sortColumn, true);
}

/**
 * Class destructor.
 */
BridgingAwareTableHeaderComponent::~BridgingAwareTableHeaderComponent()
{
}

/**
 * Helper method to update the list of bridging titles by querying
 * data from controller. This should be called on configuration updates
 * that affect bridging protocol active state.
 */
void BridgingAwareTableHeaderComponent::updateBridgingTitles()
{
	Controller* ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	auto activeBridging = ctrl->GetActiveProtocolBridging();

	for (auto protocolType : ProtocolBridgingTypes)
		m_bridgingProtocolActive[protocolType] = ((activeBridging & protocolType) == protocolType);

	resized();
}

/**
 * Helper method to update the sizing of columns.
 * This takes the overall available width and distributes it to the columns with a given ratio.
 */
void BridgingAwareTableHeaderComponent::updateColumnWidths()
{
	Controller* ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	auto activeBridgingCount = ctrl->GetActiveProtocolBridgingCount();
	auto itemWidth = 55;

	setColumnWidth(TC_BridgingMute, activeBridgingCount * itemWidth);
}

/**
 * Overridden to handle some special two-lined text arrangement
 * @param g The graphics object for painting
 */
void BridgingAwareTableHeaderComponent::paint(Graphics& g)
{
	TableHeaderComponent::paint(g);

	auto bridgingCellRect = getColumnPosition(getNumColumns(true)).reduced(3);

	auto font = g.getCurrentFont();
	font.setBold(true);
	g.setFont(font);
	g.setColour(getLookAndFeel().findColour(TableHeaderComponent::textColourId));

	std::vector<ProtocolBridgingType> activeBridgingProtocols;
	for (auto protocolActiveKV : m_bridgingProtocolActive)
		if (protocolActiveKV.second)
			activeBridgingProtocols.push_back(protocolActiveKV.first);

	if (activeBridgingProtocols.empty())
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
	
		auto singleTitleWidth = static_cast<int>(bridgingCellRect.getWidth() / activeBridgingProtocols.size());

		for (auto protocolActiveKV : m_bridgingProtocolActive)
		{
			if (protocolActiveKV.second)
			{
				auto titleRect = bridgingCellRect.removeFromLeft(singleTitleWidth).reduced(2);
				g.drawText(GetProtocolBridgingShortName(protocolActiveKV.first), titleRect, Justification::centredLeft);
			}
		}
	}
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void BridgingAwareTableHeaderComponent::resized()
{
	TableHeaderComponent::resized();

	updateColumnWidths();
}


} // namespace SpaConBridge
