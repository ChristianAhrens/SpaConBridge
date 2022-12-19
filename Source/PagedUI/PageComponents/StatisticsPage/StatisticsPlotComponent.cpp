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

#include "StatisticsPlotComponent.h"

#include "../../../Controller.h"


namespace SpaConBridge
{


/*
===============================================================================
	Class StatisticsPlot
===============================================================================
*/

/**
 * Class constructor.
 */
StatisticsPlot::StatisticsPlot()
{
	m_vertValueRange = PC_VERT_RANGE;

	ResetStatisticsPlot();

	startTimer(PC_HOR_DEFAULTSTEPPING);
}

/**
 * Class destructor.
 */
StatisticsPlot::~StatisticsPlot()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void StatisticsPlot::paint(Graphics& g)
{
	auto bounds = getLocalBounds().toFloat();

	// Background of plot area
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(bounds);

	// Frame of plot area
	g.setColour(getLookAndFeel().findColour(TableListBox::outlineColourId));
	g.drawRect(bounds);

	/******************************************************************************************/
	auto contentBounds = bounds.reduced(1);
	auto legendBounds = contentBounds.removeFromBottom(30);
	auto plotBounds = contentBounds;

	// Plot background area
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	g.fillRect(plotBounds);

	// reduce area to not paint over boarders
	plotBounds.reduce(1, 1);

	// Plot grid
	auto w = plotBounds.getWidth();
	auto h = plotBounds.getHeight();
	const float dashLengths[2] = { 5.0f, 6.0f };
	const float gridLineThickness = 1.0f;
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));
	g.drawDashedLine(Line<float>(plotBounds.getX(), plotBounds.getY() + h * 0.25f, plotBounds.getX() + w, plotBounds.getY() + h * 0.25f), dashLengths, 2, gridLineThickness);
	g.drawDashedLine(Line<float>(plotBounds.getX(), plotBounds.getY() + h * 0.50f, plotBounds.getX() + w, plotBounds.getY() + h * 0.50f), dashLengths, 2, gridLineThickness);
	g.drawDashedLine(Line<float>(plotBounds.getX(), plotBounds.getY() + h * 0.75f, plotBounds.getX() + w, plotBounds.getY() + h * 0.75f), dashLengths, 2, gridLineThickness);
	
	// msg rate text
	float msgRate = float(m_vertValueRange) * (float(PC_HOR_USERVISUSTEPPING) / float(PC_HOR_DEFAULTSTEPPING));
	g.drawText(String(msgRate) + " msg/s", plotBounds.reduced(2), Justification::topLeft, true);

	// Plot graph parameters
	auto plotDataCount = (m_plotData.empty() ? 0 : m_plotData.begin()->second.size());
	auto plotStepWidthPx = float(plotBounds.getWidth() - 1) / float((plotDataCount > 0 ? plotDataCount : 1) - 1);
	auto vFactor = float(plotBounds.getHeight() - 1) / float(m_vertValueRange > 0 ? m_vertValueRange : 1);
	auto plotOrigX = plotBounds.getBottomLeft().getX();
	auto plotOrigY = plotBounds.getBottomLeft().getY() - 1;
	auto legendColWidth = std::min(legendBounds.getWidth() / (m_plotData.empty() ? 1 : m_plotData.size()), 90.0f);

	Path path;
	for (auto const& dataEntryKV : m_plotData)
	{
		// draw legend items
		Rectangle<float> legendItemBounds;
		if (dataEntryKV.first != PBT_DS100)
			legendItemBounds = legendBounds.removeFromLeft(legendColWidth).reduced(5);
		else
			legendItemBounds = legendBounds.removeFromRight(legendColWidth).reduced(5);
		auto legendIndicator = legendItemBounds.removeFromLeft(legendItemBounds.getHeight());

		g.setColour(getLookAndFeel().findColour(TableListBox::textColourId));
		g.drawFittedText(GetProtocolBridgingShortName(dataEntryKV.first), legendItemBounds.reduced(3).toNearestInt(), Justification::centredLeft, 1);

		auto colour = GetProtocolBridgingColour(dataEntryKV.first);
		if (colour.isTransparent())
			g.setColour(getLookAndFeel().findColour(TableListBox::textColourId));
		else
			g.setColour(colour);

		if (dataEntryKV.first == PBT_DS100 && !m_showDS100Traffic)
			g.drawRoundedRectangle(legendIndicator.reduced(5), 4.0f, 1);
		else
		{
			g.fillRoundedRectangle(legendIndicator.reduced(5), 4.0f);

			// draw graph
			path.startNewSubPath(Point<float>(plotOrigX, plotOrigY - (m_plotData[dataEntryKV.first].front()) * vFactor));
			for (int i = 1; i < m_plotData[dataEntryKV.first].size(); ++i)
			{
				auto newPointX = plotOrigX + float(i) * plotStepWidthPx;
				auto newPointY = plotOrigY - (m_plotData[dataEntryKV.first].at(i) * vFactor);

				path.lineTo(Point<float>(newPointX, newPointY));
			}
			g.strokePath(path, PathStrokeType(2));
			path.closeSubPath();
			path.clear();
		}
	}

	// Plot legend markings
	const float legendMarkLengths[2] = { 5.0f, 8.0f };
	const float legendLineThickness = 1.5f;
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));
	g.drawLine(Line<float>(plotBounds.getX() + w * 0.25f, plotBounds.getBottom(), plotBounds.getX() + w * 0.25f, plotBounds.getBottom() - legendMarkLengths[0]), legendLineThickness);
	g.drawLine(Line<float>(plotBounds.getX() + w * 0.50f, plotBounds.getBottom(), plotBounds.getX() + w * 0.50f, plotBounds.getBottom() - legendMarkLengths[1]), legendLineThickness);
	g.drawLine(Line<float>(plotBounds.getX() + w * 0.75f, plotBounds.getBottom(), plotBounds.getX() + w * 0.75f, plotBounds.getBottom() - legendMarkLengths[0]), legendLineThickness);
	g.drawLine(Line<float>(plotBounds.getX() + w * 1.00f, plotBounds.getBottom(), plotBounds.getX() + w * 1.00f, plotBounds.getBottom() - legendMarkLengths[1]), legendLineThickness);
	g.drawLine(Line<float>(plotBounds.getX(), plotBounds.getY(), plotBounds.getX() + legendMarkLengths[1], plotBounds.getY()), legendLineThickness);
	g.drawLine(Line<float>(plotBounds.getX(), plotBounds.getY() + h * 0.25f, plotBounds.getX() + legendMarkLengths[0], plotBounds.getY() + h * 0.25f), legendLineThickness);
	g.drawLine(Line<float>(plotBounds.getX(), plotBounds.getY() + h * 0.50f, plotBounds.getX() + legendMarkLengths[1], plotBounds.getY() + h * 0.50f), legendLineThickness);
	g.drawLine(Line<float>(plotBounds.getX(), plotBounds.getY() + h * 0.75f, plotBounds.getX() + legendMarkLengths[0], plotBounds.getY() + h * 0.75f), legendLineThickness);

	// Plot x/y axis
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));
	g.drawLine(Line<float>(plotBounds.getBottomLeft(), plotBounds.getBottomRight()), 1.5f);
	g.drawLine(Line<float>(plotBounds.getBottomLeft(), plotBounds.getTopLeft()), 1.5f);
}

/**
 * Method to increase the received message counter per current interval for given Node and Protocol.
 * Currently we simply sum up all protocol traffic per node.
 *
 * @param bridgingProtocol	The the bridging type the count shall be increased for
 */
void StatisticsPlot::IncreaseCount(ProtocolBridgingType bridgingProtocol)
{
	m_currentMsgPerProtocol[bridgingProtocol]++;
}

/**
 * Method to reset internal data based on what map of currently plotted bridging types contains.
 */
void StatisticsPlot::ResetStatisticsPlot()
{
	m_plotData.clear();
	m_plottedBridgingTypes.clear();

	auto ctrl = SpaConBridge::Controller::GetInstance();
	if (!ctrl)
		return;

	auto bridgingTypes = ctrl->GetActiveProtocolBridging() | PBT_DS100; // DS100 is not a bridging protocol, even though it is part of the enum PBT. Needs special handling therefor.
	if ((bridgingTypes & PBT_DiGiCo) == PBT_DiGiCo)
		m_plottedBridgingTypes.add(PBT_DiGiCo);
	if ((bridgingTypes & PBT_DAWPlugin) == PBT_DAWPlugin)
		m_plottedBridgingTypes.add(PBT_DAWPlugin);
	if ((bridgingTypes & PBT_BlacktraxRTTrPM) == PBT_BlacktraxRTTrPM)
		m_plottedBridgingTypes.add(PBT_BlacktraxRTTrPM);
	if ((bridgingTypes & PBT_GenericOSC) == PBT_GenericOSC)
		m_plottedBridgingTypes.add(PBT_GenericOSC);
	if ((bridgingTypes & PBT_GenericMIDI) == PBT_GenericMIDI)
		m_plottedBridgingTypes.add(PBT_GenericMIDI);
	if ((bridgingTypes & PBT_DS100) == PBT_DS100)
		m_plottedBridgingTypes.add(PBT_DS100);
	if ((bridgingTypes & PBT_YamahaOSC) == PBT_YamahaOSC)
		m_plottedBridgingTypes.add(PBT_YamahaOSC);
	if ((bridgingTypes & PBT_ADMOSC) == PBT_ADMOSC)
		m_plottedBridgingTypes.add(PBT_ADMOSC);

	for (auto bridgingProtocol : m_plottedBridgingTypes)
	{
		m_plotData[bridgingProtocol].resize(PC_HOR_RANGE / PC_HOR_DEFAULTSTEPPING);

		/*fill plotdata with default zero*/
		for (int i = 0; i < (PC_HOR_RANGE / PC_HOR_DEFAULTSTEPPING); ++i)
			m_plotData[bridgingProtocol].at(i) = 0;
	}
}

/**
 * Reimplemented from Timer - called every timeout timer
 * We do the processing of count of messages per node during last interval into our plot data for next paint here.
 */
void StatisticsPlot::timerCallback()
{
	// accumulate all protocol msgs as well as handle individual protocol msg counts
	auto maxCurrentValueOfProtocols = static_cast<float>(PC_VERT_RANGE);
	for (auto const& msgCountKV : m_currentMsgPerProtocol)
	{
		if (!m_plottedBridgingTypes.contains(msgCountKV.first))
			continue;

		if (msgCountKV.first == PBT_DS100 && !m_showDS100Traffic)
		{
			m_currentMsgPerProtocol[msgCountKV.first] = 0;
			continue;
		}

		std::vector<float> shiftedVector(m_plotData[msgCountKV.first].begin() + 1, m_plotData[msgCountKV.first].end());
		m_plotData[msgCountKV.first].swap(shiftedVector);
		m_plotData[msgCountKV.first].push_back(float(msgCountKV.second));

		m_currentMsgPerProtocol[msgCountKV.first] = 0;

		// Adjust our vertical plotting range to have better visu when large peaks would get out of scope
		auto maxCurrentValueOfThisProtocol = *std::max_element(m_plotData[msgCountKV.first].begin(), m_plotData[msgCountKV.first].end());
		maxCurrentValueOfProtocols = std::max(maxCurrentValueOfProtocols, maxCurrentValueOfThisProtocol);
	}

	m_vertValueRange = static_cast<int>(round(maxCurrentValueOfProtocols));

	if (isVisible())
		repaint();
}

/**
 * Called when the mouse button is released.
 * Reimplemented just to call EndGuiGesture() to inform the host.
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred
 */
void StatisticsPlot::mouseUp(const MouseEvent& e)
{
	auto clickPos = e.getMouseDownPosition();

	auto contentBounds = getLocalBounds().reduced(1);
	auto legendBounds = contentBounds.removeFromBottom(30);
	auto legendColWidth = std::min(int(legendBounds.getWidth() / (m_plotData.empty() ? 1 : m_plotData.size())), 90);

	for (auto const& dataEntryKV : m_plotData)
	{
		Rectangle<int> legendItemBounds;
		if (dataEntryKV.first != PBT_DS100)
			legendItemBounds = legendBounds.removeFromLeft(legendColWidth).reduced(5);
		else
			legendItemBounds = legendBounds.removeFromRight(legendColWidth).reduced(5);

		if (dataEntryKV.first == PBT_DS100 && legendItemBounds.contains(clickPos))
		{
			m_showDS100Traffic = !m_showDS100Traffic;

			if (toggleShowDS100Traffic)
				toggleShowDS100Traffic(m_showDS100Traffic);
		}
	}
}


} // namespace SpaConBridge
