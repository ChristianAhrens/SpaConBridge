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


#pragma once

#include "../../../SpaConBridgeCommon.h"


namespace SpaConBridge
{


typedef int PlotConstant;
static constexpr PlotConstant	PC_HOR_RANGE			= 20000;	// 20s on horizontal axis
static constexpr PlotConstant	PC_HOR_DEFAULTSTEPPING	= 400;		// 400ms default refresh resolution
static constexpr PlotConstant	PC_HOR_USERVISUSTEPPING = 1000;		// User is presented with plot legend msg/s to have something more legible than 200ms
static constexpr PlotConstant	PC_VERT_RANGE			= 2;		// 10 msg/s default on vertical axis (2 msg per 200ms interval)


/**
 * StatisticsPlot class provides a rolling plot to show protocol traffic.
 */
class StatisticsPlot :	public Component, 
						private Timer
{
public:
	StatisticsPlot();
	~StatisticsPlot() override;

	//==============================================================================
	void IncreaseCount(ProtocolBridgingType bridgingProtocol);
	void ResetStatisticsPlot();

	//==============================================================================
	void mouseUp(const MouseEvent& e) override;

	//==========================================================================
	std::function<void(bool)>	toggleShowDS100Traffic;

protected:
	//==============================================================================
	void paint(Graphics& g) override;

private:
	//==============================================================================
	void timerCallback() override;

private:
	juce::Array<ProtocolBridgingType> m_plottedBridgingTypes;

	bool m_showDS100Traffic{ false };
	int	m_vertValueRange;	/**< Vertical max plot value (value range). */
	std::map<ProtocolBridgingType, int>					m_currentMsgPerProtocol;	/**< Map to help counting messages per protocol in current interval. This is processed every timer callback to update plot data. */
	std::map<ProtocolBridgingType, std::vector<float>>	m_plotData;					/**< Data for plotting. Primitive vector of floats that represents the msg count per hor. step width. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatisticsPlot)
};


} // namespace SpaConBridge
