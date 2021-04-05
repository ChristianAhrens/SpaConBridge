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

#include "StatisticsPageComponent.h"

#include "StatisticsPlotComponent.h"
#include "StatisticsLogComponent.h"

#include "../../PageComponentManager.h"

#include "../../../Controller.h"


namespace SpaConBridge
{


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

	auto ctrl = SpaConBridge::Controller::GetInstance();
	if (ctrl)
		ctrl->AddProtocolBridgingWrapperListener(this);

	auto config = SpaConBridge::AppConfiguration::getInstance();
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
	auto bounds = getLocalBounds().toFloat().reduced(3);

	// determine the layout direction (we want a ratio of 0.75 to be the switching point)
	auto isPortrait = IsPortraitAspectRatio();

	// The layouting flexbox with parameters
	FlexBox plotAndLogFlex;
	if (isPortrait)
		plotAndLogFlex.flexDirection = FlexBox::Direction::column;
	else
		plotAndLogFlex.flexDirection = FlexBox::Direction::row;
	plotAndLogFlex.justifyContent = FlexBox::JustifyContent::center;

	plotAndLogFlex.items.add(FlexItem(*m_plotComponent).withFlex(2).withMargin(FlexItem::Margin(5, 5, 5, 5)));
	plotAndLogFlex.items.add(FlexItem(*m_logComponent).withFlex(1).withMargin(FlexItem::Margin(5, 5, 5, 5)));

	plotAndLogFlex.performLayout(bounds);
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the procssor parameters
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
void StatisticsPageComponent::HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData)
{
	if (nodeId != DEFAULT_PROCNODE_ID)
		return;

	// derive the bridging protocol type from given protocol that received the data
	auto bridgingProtocol = PBT_None;
	auto logSource = StatisticsLog::StatisticsLogSource::SLS_None;
	switch (senderProtocolId)
	{
	case DIGICO_PROCESSINGPROTOCOL_ID:
		bridgingProtocol = PBT_DiGiCo;
		logSource = StatisticsLog::StatisticsLogSource::SLS_DiGiCo;
		break;
	case RTTRPM_PROCESSINGPROTOCOL_ID:
		bridgingProtocol = PBT_BlacktraxRTTrPM;
		logSource = StatisticsLog::StatisticsLogSource::SLS_BlacktraxRTTrPM;
		break;
	case GENERICOSC_PROCESSINGPROTOCOL_ID:
		bridgingProtocol = PBT_GenericOSC;
		logSource = StatisticsLog::StatisticsLogSource::SLS_GenericOSC;
		break;
	case DS100_1_PROCESSINGPROTOCOL_ID:
		bridgingProtocol = PBT_DS100;
		logSource = StatisticsLog::StatisticsLogSource::SLS_DS100;
		break;
	case DS100_2_PROCESSINGPROTOCOL_ID:
		bridgingProtocol = PBT_DS100;
		logSource = StatisticsLog::StatisticsLogSource::SLS_DS100_2;
		break;
	case GENERICMIDI_PROCESSINGPROTOCOL_ID:
		bridgingProtocol = PBT_GenericMIDI;
		logSource = StatisticsLog::StatisticsLogSource::SLS_GenericMIDI;
		break;
	case YAMAHAOSC_PROCESSINGPROTOCOL_ID:
		bridgingProtocol = PBT_YamahaOSC;
		logSource = StatisticsLog::StatisticsLogSource::SLS_YamahaOSC;
		break;
	default:
		return;
	}

	// increase message counter in plotting component for the given bridging type
	m_plotComponent->IncreaseCount(bridgingProtocol);
	
	// add message data to logging component
	m_logComponent->AddMessageData(logSource, Id, msgData);
}


} // namespace SpaConBridge
