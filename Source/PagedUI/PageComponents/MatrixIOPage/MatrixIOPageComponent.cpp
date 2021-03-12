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

#include "MatrixIOPageComponent.h"

#include "MatrixInputTableComponent.h"
#include "MatrixOutputTableComponent.h"

#include "../../../CustomAudioProcessors/MatrixInputProcessor/MatrixInputProcessor.h"
#include "../../../CustomAudioProcessors/MatrixInputProcessor/MatrixInputProcessorEditor.h"
#include "../../../CustomAudioProcessors/MatrixOutputProcessor/MatrixOutputProcessor.h"
#include "../../../CustomAudioProcessors/MatrixOutputProcessor/MatrixOutputProcessorEditor.h"

#include "../../PageComponentManager.h"

#include "../../../Controller.h"


namespace SoundscapeBridgeApp
{


/*
===============================================================================
	Class MatrixIOPageComponent
===============================================================================
*/

/**
 * Class constructor.
 */
MatrixIOPageComponent::MatrixIOPageComponent()
	: PageComponentBase(PCT_MatrixIOs)
{
	m_inputsComponent = std::make_unique<MatrixInputTableComponent>();
	addAndMakeVisible(m_inputsComponent.get());

	m_outputsComponent = std::make_unique<MatrixOutputTableComponent>();
	addAndMakeVisible(m_outputsComponent.get());

	// trigger lookandfeel update
	lookAndFeelChanged();

	auto config = SoundscapeBridgeApp::AppConfiguration::getInstance();
	if (config)
		config->addWatcher(this);
}

/**
 * Class destructor.
 */
MatrixIOPageComponent::~MatrixIOPageComponent()
{
}

/**
 * Reimplemented to paint background and frame.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void MatrixIOPageComponent::paint(Graphics& g)
{
	auto bounds = getLocalBounds();

	// Background
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(bounds);
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void MatrixIOPageComponent::resized()
{
	auto bounds = getLocalBounds().toFloat().reduced(3);
	auto isPortrait = IsPortraitAspectRatio();

	auto fmargin = 5.0f;

	// The layouting flexbox with parameters
	FlexBox matrixIOFlex;
	if (isPortrait)
		matrixIOFlex.flexDirection = FlexBox::Direction::column;
	else
		matrixIOFlex.flexDirection = FlexBox::Direction::row;

	matrixIOFlex.justifyContent = FlexBox::JustifyContent::center;

	matrixIOFlex.items.add(FlexItem(*m_inputsComponent).withFlex(1).withMargin(FlexItem::Margin(fmargin, fmargin, fmargin, fmargin)));
	matrixIOFlex.items.add(FlexItem(*m_outputsComponent).withFlex(1).withMargin(FlexItem::Margin(fmargin, fmargin, fmargin, fmargin)));

	if (isPortrait)
	{
		m_inputsComponent->SetControlBarPosition(TableModelComponent::ControlBarPosition::CBP_Bottom);
		m_outputsComponent->SetControlBarPosition(TableModelComponent::ControlBarPosition::CBP_Bottom);
	}
	else
	{
		m_inputsComponent->SetControlBarPosition(TableModelComponent::ControlBarPosition::CBP_Left);
		m_outputsComponent->SetControlBarPosition(TableModelComponent::ControlBarPosition::CBP_Left);
	}

	matrixIOFlex.performLayout(bounds);
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the procssor parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void MatrixIOPageComponent::UpdateGui(bool init)
{
	Controller* ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	if (m_inputsComponent)
	{
		if (ctrl->PopParameterChanged(DCS_MatrixInputTable, DCT_NumProcessors) || init)
		{
			m_inputsComponent->RecreateTableRowIds();
			m_inputsComponent->UpdateTable();
		}
		else if (ctrl->PopParameterChanged(DCS_Protocol, DCT_ProcessorSelection) ||
			ctrl->PopParameterChanged(DCS_Host, DCT_BridgingConfig))
		{
			m_inputsComponent->UpdateTable();
		}
		else
		{
			// Iterate through all procssor instances and see if anything changed there.
			for (auto const& processorId : ctrl->GetMatrixInputProcessorIds())
			{
				auto processor = ctrl->GetMatrixInputProcessor(processorId);
				if (processor && processor->GetParameterChanged(DCS_MatrixInputTable, DCT_ProcessorInstanceConfig))
				{
					m_inputsComponent->UpdateTable();
				}
			}
		}
	}

	if (m_outputsComponent)
	{
		if (ctrl->PopParameterChanged(DCS_MatrixOutputTable, DCT_NumProcessors) || init)
		{
			m_outputsComponent->RecreateTableRowIds();
			m_outputsComponent->UpdateTable();
		}
		else if (ctrl->PopParameterChanged(DCS_Protocol, DCT_ProcessorSelection) ||
			ctrl->PopParameterChanged(DCS_Host, DCT_BridgingConfig))
		{
			m_outputsComponent->UpdateTable();
		}
		else
		{
			// Iterate through all procssor instances and see if anything changed there.
			for (auto const& processorId : ctrl->GetMatrixOutputProcessorIds())
			{
				auto processor = ctrl->GetMatrixOutputProcessor(processorId);
				if (processor && processor->GetParameterChanged(DCS_MatrixOutputTable, DCT_ProcessorInstanceConfig))
				{
					m_outputsComponent->UpdateTable();
				}
			}
		}
	}
}

/**
 * Overridden from AppConfiguration Watcher to be able
 * to live react on config changes and update the table contents.
 */
void MatrixIOPageComponent::onConfigUpdated()
{

}


} // namespace SoundscapeBridgeApp
