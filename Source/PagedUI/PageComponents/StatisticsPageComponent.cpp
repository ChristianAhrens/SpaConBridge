/*
  ==============================================================================

    StatisticsPageComponent.cpp
    Created: 8 Nov 2020 10:19:43am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "StatisticsPageComponent.h"

#include "../PageComponentManager.h"

#include "../../SoundsourceProcessor/SoundsourceProcessor.h"
#include "../../Controller.h"
#include "../../SoundsourceProcessor/SurfaceSlider.h"

#include <Image_utils.hpp>


namespace SoundscapeBridgeApp
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
	auto newPointX = 0.0f;
	auto newPointY = 0.0f;
	auto vFactor = float(plotBounds.getHeight() - 1) / float(m_vertValueRange > 0 ? m_vertValueRange : 1);
	auto plotOrigX = plotBounds.getBottomLeft().getX();
	auto plotOrigY = plotBounds.getBottomLeft().getY() - 1;
	auto legendColWidth = std::min(legendBounds.getWidth() / (m_plotData.empty() ? 1 : m_plotData.size()), 90.0f);

	Path path;
	for (auto const& dataEntryKV : m_plotData)
	{
		// draw legend items
		auto legendItemBounds = legendBounds.removeFromLeft(legendColWidth).reduced(5);
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
				newPointX = plotOrigX + float(i) * plotStepWidthPx;
				newPointY = plotOrigY - (m_plotData[dataEntryKV.first].at(i) * vFactor);

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

	auto ctrl = SoundscapeBridgeApp::CController::GetInstance();
	if (!ctrl)
		return;

	auto bridgingTypes = ctrl->GetActiveProtocolBridging() | PBT_DS100; // DS100 is not a bridging protocol, even though it is part of the enum PBT. Needs special handling therefor.
	if ((bridgingTypes & PBT_DiGiCo) == PBT_DiGiCo)
		m_plottedBridgingTypes.add(PBT_DiGiCo);
	if ((bridgingTypes & PBT_BlacktraxRTTrPM) == PBT_BlacktraxRTTrPM)
		m_plottedBridgingTypes.add(PBT_BlacktraxRTTrPM);
	if ((bridgingTypes & PBT_GenericOSC) == PBT_GenericOSC)
		m_plottedBridgingTypes.add(PBT_GenericOSC);
	if ((bridgingTypes & PBT_DS100) == PBT_DS100)
		m_plottedBridgingTypes.add(PBT_DS100);

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
	int msgCount = 0;
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

		msgCount += msgCountKV.second;

		m_currentMsgPerProtocol[msgCountKV.first] = 0;

		// Adjust our vertical plotting range to have better visu when large peaks would get out of scope
		m_vertValueRange = static_cast<int>(round(std::max(float(PC_VERT_RANGE), *std::max_element(m_plotData[msgCountKV.first].begin(), m_plotData[msgCountKV.first].end()))));
	}

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

	auto bounds = getLocalBounds();
	auto contentBounds = getLocalBounds().reduced(1);
	auto legendBounds = contentBounds.removeFromBottom(30);
	auto legendColWidth = std::min(int(legendBounds.getWidth() / (m_plotData.empty() ? 1 : m_plotData.size())), 90);

	for (auto const& dataEntryKV : m_plotData)
	{
		auto legendItemBounds = legendBounds.removeFromLeft(legendColWidth).reduced(5);

		if (dataEntryKV.first == PBT_DS100 && legendItemBounds.contains(clickPos))
		{
			m_showDS100Traffic = !m_showDS100Traffic;

			if (toggleShowDS100Traffic)
				toggleShowDS100Traffic(m_showDS100Traffic);
		}
	}
}


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
	m_table->getHeader().addColumn("Type", SLC_BridgingName, 60, 60, -1, tableHeaderFlags);

	startTimer(PC_HOR_DEFAULTSTEPPING);
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
 * @param bridgingType	The type of the bridging protocol that received the data
 * @param Id			The remote object id that was received
 * @param msgData		The actual message data that shall be logged
 */
void StatisticsLog::AddMessageData(ProtocolBridgingType bridgingType, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData)
{
	if (!m_showDS100Traffic && bridgingType == PBT_DS100)
		return;

	String valueString;
	if (msgData.payload != 0)
	{
		if (msgData.valType == ROVT_FLOAT)
		{
			float fvalue;
			for (int i = 0; i < msgData.valCount; ++i)
			{
				fvalue = ((float*)msgData.payload)[i];
				valueString += String(fvalue, 2) + ",";
			}
		}
		else if (msgData.valType == ROVT_INT)
		{
			int ivalue;
			for (int i = 0; i < msgData.valCount; ++i)
			{
				ivalue = ((int*)msgData.payload)[i];
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
	m_logEntries[mapIdx][SLC_SourceId] = String(msgData.addrVal.first);
	m_logEntries[mapIdx][SLC_Value] = valueString;
	m_logEntries[mapIdx][SLC_BridgingName] = GetProtocolBridgingShortName(bridgingType);
	m_logEntries[mapIdx][SLC_BridgingType] = String(bridgingType);

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
 * @return	Number of rows on the table, equal to number of plugin instances.
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
		auto bridgingProtocol = static_cast<ProtocolBridgingType>(m_logEntries[mapIdx][SLC_BridgingType].getIntValue());
		auto colour = GetProtocolBridgingColour(bridgingProtocol);
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
	case SLC_BridgingName:
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


/*
===============================================================================
	Class StatisticsPageComponent
===============================================================================
*/

/**
 * Class constructor.
 */
StatisticsPageComponent::StatisticsPageComponent()
	: PageComponentBase(PCT_Statistics)
{
	m_plotComponent = std::make_unique<StatisticsPlot>();
	addAndMakeVisible(m_plotComponent.get());

	m_logComponent = std::make_unique<StatisticsLog>();
	addAndMakeVisible(m_logComponent.get());

	m_plotComponent->toggleShowDS100Traffic = [=](bool show) { m_logComponent->SetShowDS100Traffic(show); };

	auto ctrl = SoundscapeBridgeApp::CController::GetInstance();
	if (ctrl)
		ctrl->AddProtocolBridgingWrapperListener(this);

	auto config = SoundscapeBridgeApp::AppConfiguration::getInstance();
	if (config)
		config->addWatcher(this);
}

/**
 * Class destructor.
 */
StatisticsPageComponent::~StatisticsPageComponent()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void StatisticsPageComponent::paint(Graphics& g)
{
	// Paint background to cover the controls behind this overlay.
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(Rectangle<int>(0, 0, getLocalBounds().getWidth(), getLocalBounds().getHeight()));
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void StatisticsPageComponent::resized()
{
	auto bounds = getLocalBounds().toFloat().reduced(5);

	// determine the layout direction (we want a ratio of 0.75 to be the switching point)
	auto layoutSwitchAspectRatio = 0.75f;
	auto w = bounds.getWidth();
	auto h = bounds.getHeight();
	auto aspectRatio = h / (w != 0.0f ? w : 1.0f);
	auto isPortrait = layoutSwitchAspectRatio < aspectRatio;

	// The layouting flexbox with parameters
	FlexBox plotAndLogFlex;
	if (isPortrait)
		plotAndLogFlex.flexDirection = FlexBox::Direction::column;
	else
		plotAndLogFlex.flexDirection = FlexBox::Direction::row;
	plotAndLogFlex.justifyContent = FlexBox::JustifyContent::center;

	plotAndLogFlex.items.add(FlexItem(*m_plotComponent).withFlex(2).withMargin(FlexItem::Margin(5, 5, 5, 5)));
	plotAndLogFlex.items.add(FlexItem(*m_logComponent.get()).withFlex(1).withMargin(FlexItem::Margin(5, 5, 5, 5)));

	plotAndLogFlex.performLayout(bounds);
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the plugin parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void StatisticsPageComponent::UpdateGui(bool init)
{
	ignoreUnused(init);
}

/**
 * Overridden from AppConfiguration Watcher to be able
 * to live react on config changes and update the table contents.
 */
void StatisticsPageComponent::onConfigUpdated()
{
	m_plotComponent->ResetStatisticsPlot();
}

/**
 * Reimplemented callback for bridging wrapper callback to process incoming protocol data.
 * It forwards the message to all registered Processor objects.
 * @param nodeId	The bridging node that the message data was received on (only a single default id node supported currently).
 * @param senderProtocolId	The protocol that the message data was received on and was sent to controller from.
 * @param objectId	The remote object id of the object that was received
 * @param msgData	The actual message data that was received
 */
void StatisticsPageComponent::HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData)
{
	if (nodeId != DEFAULT_PROCNODE_ID)
		return;

	// derive the bridging protocol type from given protocol that received the data
	auto bridgingProtocol = PBT_None;
	switch (senderProtocolId)
	{
	case DIGICO_PROCESSINGPROTOCOL_ID:
		bridgingProtocol = PBT_DiGiCo;
		break;
	case RTTRPM_PROCESSINGPROTOCOL_ID:
		bridgingProtocol = PBT_BlacktraxRTTrPM;
		break;
	case GENERICOSC_PROCESSINGPROTOCOL_ID:
		bridgingProtocol = PBT_GenericOSC;
		break;
	case DS100_PROCESSINGPROTOCOL_ID:
		bridgingProtocol = PBT_DS100;
		break;
	default:
		return;
	}

	// increase message counter in plotting component for the given bridging type
	m_plotComponent->IncreaseCount(bridgingProtocol);
	
	// add message data to logging component
	m_logComponent->AddMessageData(bridgingProtocol, Id, msgData);
}


} // namespace SoundscapeBridgeApp