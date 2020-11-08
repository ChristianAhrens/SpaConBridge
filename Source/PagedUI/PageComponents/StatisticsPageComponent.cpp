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
	m_hRange = PC_HOR_RANGE;
	m_hStepping = PC_HOR_DEFAULTSTEPPING;
	m_vRange = PC_VERT_RANGE;

	for (auto bridgingProtocol : m_plottedBridgingTypes)
	{
		m_plotData[bridgingProtocol].resize(m_hRange / m_hStepping);

		/*fill plotdata with default zero*/
		for (int i = 0; i < (m_hRange / m_hStepping); ++i)
			m_plotData[bridgingProtocol].at(i) = 0;

		float r = float(rand()) / float(RAND_MAX);
		float g = float(rand()) / float(RAND_MAX);
		float b = float(rand()) / float(RAND_MAX);
		float a = 170.0f;
		m_plotColours[bridgingProtocol] = Colour::fromFloatRGBA(r, g, b, a);
	}

	startTimer(m_hStepping);
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
	auto contentBounds = bounds.reduced(10);
	auto legendBounds = contentBounds.removeFromBottom(30);
	auto plotBounds = contentBounds;

	auto w = plotBounds.getWidth();
	auto h = plotBounds.getHeight();

	// Plot background area
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	g.fillRect(plotBounds);

	// Plot grid
	const float dashLengths[2] = { 5.0f, 6.0f };
	const float lineThickness = 1.0f;
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId).brighter(0.15f));
	g.drawDashedLine(Line<float>(plotBounds.getX() + w * 0.25f, plotBounds.getY(), plotBounds.getX() + w * 0.25f, plotBounds.getY() + h), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(plotBounds.getX() + w * 0.50f, plotBounds.getY(), plotBounds.getX() + w * 0.50f, plotBounds.getY() + h), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(plotBounds.getX() + w * 0.75f, plotBounds.getY(), plotBounds.getX() + w * 0.75f, plotBounds.getY() + h), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(plotBounds.getX(), plotBounds.getY() + h * 0.25f, plotBounds.getX() + w, plotBounds.getY() + h * 0.25f), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(plotBounds.getX(), plotBounds.getY() + h * 0.50f, plotBounds.getX() + w, plotBounds.getY() + h * 0.50f), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(plotBounds.getX(), plotBounds.getY() + h * 0.75f, plotBounds.getX() + w, plotBounds.getY() + h * 0.75f), dashLengths, 2, lineThickness);

	auto plotDataCount = m_plotData.begin()->second.size();
	auto plotStepWidthPx = float(plotBounds.getWidth()) / float((plotDataCount > 0 ? plotDataCount : 1) - 1);
	auto newPointX = 0.0f;
	auto newPointY = 0.0f;
	auto vFactor = float(plotBounds.getHeight()) / float(m_vRange > 0 ? m_vRange : 1);
	auto plotOrigX = plotBounds.getBottomLeft().getX();
	auto plotOrigY = plotBounds.getBottomLeft().getY();
	auto legendColWidth = 0.8f * (legendBounds.getWidth() / (m_plotData.empty() ? 1 : m_plotData.size()));

	Path path;
	for (auto const& dataEntryKV : m_plotData)
	{
		// legend text and graph curve colour for individual protocols
		g.setColour(m_plotColours.at(dataEntryKV.first));

		// draw legend
		auto legendColBounds = legendBounds.removeFromLeft(legendColWidth);
		g.drawFittedText(GetProtocolBridgingNiceName(dataEntryKV.first), legendColBounds.removeFromLeft(80).reduced(3).toNearestInt(), Justification::centredRight, 1);
		g.drawHorizontalLine(legendColBounds.getCentreY(), legendColBounds.getTopLeft().getX() + 4, legendColBounds.getTopRight().getX() - 4);

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

	//if (!m_plotData.empty() && !m_plotData.begin()->second.empty())
	//{
	//	auto plotDataCount = m_plotData.begin()->second.size();
	//
	//	// fill margin values to floats to enhance code readability
	//	auto ms = 5.0f;
	//	auto mm = 10.0f;
	//	auto ml = 25.0f;
	//	auto mxl = 30.0f;
	//
	//	float plotStepWidthPx = float(plotBounds.getWidth()) / float((plotDataCount > 0 ? plotDataCount : 1) - 1);
	//
	//	g.setColour(getLookAndFeel().findColour(CodeEditorComponent::ColourIds::defaultTextColourId));
	//	g.drawLine(Line<float>(plotBounds.getX(), plotBounds.getY(), plotBounds.getX(), plotBounds.getY() - plotBounds.getHeight()));
	//	g.drawLine(Line<float>(plotBounds.getX(), plotBounds.getY(), plotBounds.getX() + plotBounds.getWidth(), plotBounds.getY()));
	//
	//	float vUserRange = float(m_vRange) * (float(PC_HOR_USERVISUSTEPPING) / float(m_hStepping));
	//	g.drawText("msg/s", Rectangle<float>(ms, mm, 3 * ml, ml), Justification::topLeft, true);
	//	g.drawText(String(vUserRange), Rectangle<float>(ms * 0.5f, ms + mxl, mxl - ms * 0.5f, mm), Justification::centred, true);
	//	g.drawText(String(vUserRange * 0.5f), Rectangle<float>(ms * 0.5f, ms + plotBounds.getY() - (plotBounds.getHeight() * 0.5f), mxl - ms * 0.5f, mm),
	//		Justification::centred, true);
	//	g.drawText(String(0), Rectangle<float>(ms * 0.5f, ms + plotBounds.getY(), mxl - ms * 0.5f, mm), Justification::centred, true);
	//	g.drawLine(Line<float>(plotBounds.getX() - mm, plotBounds.getY() - plotBounds.getHeight(), plotBounds.getX(), plotBounds.getY() - plotBounds.getHeight()));
	//	g.drawLine(Line<float>(plotBounds.getX() - ms, plotBounds.getY() - (plotBounds.getHeight() * 0.75f), plotBounds.getX(), plotBounds.getY() - (plotBounds.getHeight() * 0.75f)));
	//	g.drawLine(Line<float>(plotBounds.getX() - mm, plotBounds.getY() - (plotBounds.getHeight() * 0.5f), plotBounds.getX(), plotBounds.getY() - (plotBounds.getHeight() * 0.5f)));
	//	g.drawLine(Line<float>(plotBounds.getX() - ms, plotBounds.getY() - (plotBounds.getHeight() * 0.25f), plotBounds.getX(), plotBounds.getY() - (plotBounds.getHeight() * 0.25f)));
	//	g.drawLine(Line<float>(plotBounds.getX() - mm, plotBounds.getY(), plotBounds.getX(), plotBounds.getY()));
	//
	//	int hTime = int(float(plotDataCount) * float(m_hStepping) * 0.001);
	//	g.drawText(String(hTime), Rectangle<float>(ms + plotBounds.getX(), mm + plotBounds.getY(), mxl, mm), Justification::bottomLeft, true);
	//	g.drawText(String(hTime * 0.5f), Rectangle<float>(ms + plotBounds.getX() + (plotBounds.getWidth() * 0.5f), mm + plotBounds.getY(), mxl, mm), Justification::bottomLeft, true);
	//	g.drawText(String(0), Rectangle<float>(ms + plotBounds.getX() + plotBounds.getWidth(), mm + plotBounds.getY(), mxl, mm), Justification::bottomLeft, true);
	//	g.drawText("time (s)", Rectangle<float>(mxl + plotBounds.getWidth() - 2 * mxl, plotBounds.getY(), 2 * ml, ml), Justification::bottomRight, true);
	//	g.drawLine(Line<float>(plotBounds.getX(), plotBounds.getY(), plotBounds.getX(), plotBounds.getY() + mm));
	//	g.drawLine(Line<float>(plotBounds.getX() + (plotBounds.getWidth() * 0.25f), plotBounds.getY(), plotBounds.getX() + (plotBounds.getWidth() * 0.25f), plotBounds.getY() + ms));
	//	g.drawLine(Line<float>(plotBounds.getX() + (plotBounds.getWidth() * 0.5f), plotBounds.getY(), plotBounds.getX() + (plotBounds.getWidth() * 0.5f), plotBounds.getY() + mm));
	//	g.drawLine(Line<float>(plotBounds.getX() + (plotBounds.getWidth() * 0.75f), plotBounds.getY(), plotBounds.getX() + (plotBounds.getWidth() * 0.75f), plotBounds.getY() + ms));
	//	g.drawLine(Line<float>(plotBounds.getX() + plotBounds.getWidth(), plotBounds.getY(), plotBounds.getX() + plotBounds.getWidth(), plotBounds.getY() + mm));
	//
	//	float legendPosX = plotBounds.getX() + mxl;
	//	g.drawText("Total", Rectangle<float>(legendPosX, mm, 2 * ml, mm), Justification::centred, true);
	//	legendPosX += 2 * ml;
	//	g.drawLine(Line<float>(legendPosX, mm + ms, legendPosX + ml, mm + ms));
	//	legendPosX += 3 * ml;
	//	for (const std::pair<int, Colour>& protoCol : m_plotColours)
	//	{
	//		g.setColour(protoCol.second);
	//		g.drawText("PId" + String(protoCol.first), Rectangle<float>(legendPosX, mm, 2 * ml, mm), Justification::centred, true);
	//		legendPosX += 2 * ml;
	//		g.drawLine(Line<float>(legendPosX, mm + ms, legendPosX + ml, mm + ms));
	//		legendPosX += 3 * ml;
	//	}
	//
	//	float newPointX = 0;
	//	float newPointY = 0;
	//	float vFactor = float(plotBounds.getHeight()) / float(m_vRange > 0 ? m_vRange : 1);
	//
	//	Path path;
	//	for (const std::pair<int, std::vector<float>> pd : m_plotData)
	//	{
	//		//Graph curve colour for individual protocols
	//		g.setColour(m_plotColours.at(pd.first));
	//
	//		path.startNewSubPath(Point<float>(plotBounds.getX(), plotBounds.getY() - (m_plotData[pd.first].front()) * vFactor));
	//		for (int i = 1; i < m_plotData[pd.first].size(); ++i)
	//		{
	//			newPointX = plotBounds.getX() + float(i) * plotStepWidthPx;
	//			newPointY = plotBounds.getY() - (m_plotData[pd.first].at(i) * vFactor);
	//
	//			path.lineTo(Point<float>(newPointX, newPointY));
	//		}
	//		g.strokePath(path, PathStrokeType(2));
	//		path.closeSubPath();
	//		path.clear();
	//	}
	//}
	/******************************************************************************************/

	// Plot frame
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));
	g.drawRect(plotBounds, 1.5f);
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
 * Reimplemented from Timer - called every timeout timer
 * We do the processing of count of messages per node during last interval into our plot data for next paint here.
 */
void StatisticsPlot::timerCallback()
{
	// accumulate all protocol msgs as well as handle individual protocol msg counts
	m_vRange = 0;
	int msgCount = 0;
	for (auto const& msgCountKV : m_currentMsgPerProtocol)
	{
		if (!m_plottedBridgingTypes.contains(msgCountKV.first))
			continue;

		std::vector<float> shiftedVector(m_plotData[msgCountKV.first].begin() + 1, m_plotData[msgCountKV.first].end());
		m_plotData[msgCountKV.first].swap(shiftedVector);
		m_plotData[msgCountKV.first].push_back(float(msgCountKV.second));

		msgCount += msgCountKV.second;

		m_currentMsgPerProtocol[msgCountKV.first] = 0;

		// Adjust our vertical plotting range to have better visu when large peaks would get out of scope
		m_vRange = std::max(m_vRange, int(round(std::max(float(PC_VERT_RANGE), *std::max_element(m_plotData[msgCountKV.first].begin(), m_plotData[msgCountKV.first].end())))));
	}

	if (isVisible())
		repaint();
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
	m_textBox = std::make_unique<CodeEditorComponent>(m_doc, nullptr);
	addAndMakeVisible(m_textBox.get());

	startTimer(PC_HOR_DEFAULTSTEPPING);
}

/**
	* Class destructor.
	*/
StatisticsLog::~StatisticsLog()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void StatisticsLog::paint(Graphics& g)
{
	auto bounds = getLocalBounds().toFloat();
	auto w = bounds.getWidth();
	auto h = bounds.getHeight();

	// Background of logging area
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(bounds);

	// Frame of logging area
	g.setColour(getLookAndFeel().findColour(TableListBox::outlineColourId));
	g.drawRect(bounds);
}

/**
 * Reimplemented to resize and re-postion controls.
 */
void StatisticsLog::resized()
{
	auto bounds = getLocalBounds().reduced(1);

	m_textBox->setBounds(bounds);
}

/**
 * Reimplemented from Timer - called every timeout timer
 *
 * Iterates over logging queue and adds all queued messages
 * to end of text area and terminates with a newline
 */
void StatisticsLog::timerCallback()
{
	m_textBox->moveCaretToEnd(false);
	for (auto const& message : m_loggingQueue)
	{
		m_textBox->insertTextAtCaret(message);
		m_textBox->insertTextAtCaret("\n");
	}

	m_loggingQueue.clear();
}

/**
 * Method to add the received message data for given receiving bridging type.
 * @param bridgingType	The type of the bridging protocol that received the data
 * @param Id			The remote object id that was received
 * @param msgData		The actual message data that shall be logged
 */
void StatisticsLog::AddMessageData(ProtocolBridgingType bridgingType, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData)
{
	String messageString =
		GetProtocolBridgingShortName(bridgingType)
		+ String(" | ") + ProcessingEngineConfig::GetObjectDescription(Id).paddedRight(' ', 31)
		+ String(" | ") + String(msgData.addrVal.first);

	if (msgData.payload != 0)
	{
		messageString += " |";

		if (msgData.valType == ROVT_FLOAT)
		{
			float fvalue;
			for (int i = 0; i < msgData.valCount; ++i)
			{
				fvalue = ((float*)msgData.payload)[i];
				messageString += String::formatted(" %f", fvalue);
			}
		}
		else if (msgData.valType == ROVT_INT)
		{
			int ivalue;
			for (int i = 0; i < msgData.valCount; ++i)
			{
				ivalue = ((int*)msgData.payload)[i];
				messageString += String::formatted(" %d", ivalue);
			}
		}
	}

	m_loggingQueue.push_back(messageString);
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

	auto ctrl = SoundscapeBridgeApp::CController::GetInstance();
	if (ctrl)
		ctrl->AddProtocolBridgingWrapperListener(this);
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
	auto bounds = getLocalBounds().reduced(5);

	// determine the layout direction (we want a ratio of 0.75 to be the switching point)
	auto layoutSwitchAspectRatio = 0.75f;
	float w = bounds.getWidth();
	float h = bounds.getHeight();
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

	plotAndLogFlex.performLayout(bounds.toFloat());
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the plugin parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void StatisticsPageComponent::UpdateGui(bool init)
{
	// Will be set to true if any changes relevant to the multi-slider are found.
	bool update = init;
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
	default:
		return;
	}

	// increase message counter in plotting component for the given bridging type
	m_plotComponent->IncreaseCount(bridgingProtocol);
	
	// add message data to logging component
	m_logComponent->AddMessageData(bridgingProtocol, Id, msgData);
}


} // namespace SoundscapeBridgeApp