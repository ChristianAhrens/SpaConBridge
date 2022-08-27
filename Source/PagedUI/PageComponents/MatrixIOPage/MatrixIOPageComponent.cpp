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

#include "MatrixIOPageComponent.h"

#include "MatrixInputTableComponent.h"
#include "MatrixOutputTableComponent.h"

#include "../../../CustomAudioProcessors/MatrixInputProcessor/MatrixInputProcessor.h"
#include "../../../CustomAudioProcessors/MatrixInputProcessor/MatrixInputProcessorEditor.h"
#include "../../../CustomAudioProcessors/MatrixOutputProcessor/MatrixOutputProcessor.h"

#include "../../PageComponentManager.h"

#include "../../../Controller.h"


namespace SpaConBridge
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
	m_inputsComponent->onCurrentCollapseStateChanged = [=](bool collapsed) {
		ignoreUnused(collapsed);
		resized();
		if (IsPageInitializing())
			return;
		auto config = SpaConBridge::AppConfiguration::getInstance();
		if (config)
			config->triggerConfigurationDump(false);
	};
	m_inputsComponent->onCurrentRowHeightChanged = [=](int rowHeight) {
		ignoreUnused(rowHeight);
		if (IsPageInitializing())
			return;
		auto config = SpaConBridge::AppConfiguration::getInstance();
		if (config)
			config->triggerConfigurationDump(false);
	};
	addAndMakeVisible(m_inputsComponent.get());

	m_outputsComponent = std::make_unique<MatrixOutputTableComponent>();
	m_outputsComponent->onCurrentCollapseStateChanged = [=](bool collapsed) {
		ignoreUnused(collapsed);
		resized();
		if (IsPageInitializing())
			return;
		auto config = SpaConBridge::AppConfiguration::getInstance();
		if (config)
			config->triggerConfigurationDump(false);
	};
	m_outputsComponent->onCurrentRowHeightChanged = [=](int rowHeight) {
		ignoreUnused(rowHeight);
		if (IsPageInitializing())
			return;
		auto config = SpaConBridge::AppConfiguration::getInstance();
		if (config)
			config->triggerConfigurationDump(false);
	};
	addAndMakeVisible(m_outputsComponent.get());

	// trigger lookandfeel update
	lookAndFeelChanged();

	// register this object as config watcher
	auto config = SpaConBridge::AppConfiguration::getInstance();
	if (config)
		config->addWatcher(this, true);
}

/**
 * Class destructor.
 */
MatrixIOPageComponent::~MatrixIOPageComponent()
{
}

/**
 * Setter for the row height of internal matrix inputs table component
 * @param	height  The height value to set.
 */
void MatrixIOPageComponent::SetInputsRowHeight(int height)
{
	if (m_inputsComponent)
		m_inputsComponent->SetRowHeight(height);
}

/**
 * Getter for the current row height of internal matrix inputs table component
 * @return	The current height value.
 */
int MatrixIOPageComponent::GetInputsRowHeight()
{
	if (m_inputsComponent)
		return m_inputsComponent->GetRowHeight();
	else
		return 0;
}

/**
 * Setter for the row height of internal matrix outputs table component
 * @param	height  The height value to set.
 */
void MatrixIOPageComponent::SetOutputsRowHeight(int height)
{
	if (m_outputsComponent)
		m_outputsComponent->SetRowHeight(height);
}

/**
 * Getter for the current row height of internal matrix outputs table component
 * @return	The current height value.
 */
int MatrixIOPageComponent::GetOutputsRowHeight()
{
	if (m_outputsComponent)
		return m_outputsComponent->GetRowHeight();
	else
		return 0;
}

/**
 * Setter for the collapsed state of internal matrix inputs table component
 * @param	height  The collapsed state value to set.
 */
void MatrixIOPageComponent::SetInputsCollapsed(bool collapsed)
{
	if (m_inputsComponent)
		m_inputsComponent->SetCollapsed(collapsed);

	resized();
}

/**
 * Getter for the current collapsed state of internal matrix inputs table component
 * @return	The current collapsed state value.
 */
bool MatrixIOPageComponent::GetInputsCollapsed()
{
	if (m_inputsComponent)
		return m_inputsComponent->IsCollapsed();
	else
	{
		jassertfalse;
		return false;
	}
}

/**
 * Setter for the collapsed state of internal matrix outputs table component
 * @param	height  The collapsed state value to set.
 */
void MatrixIOPageComponent::SetOutputsCollapsed(bool collapsed)
{
	if (m_outputsComponent)
		m_outputsComponent->SetCollapsed(collapsed);

	resized();
}

/**
 * Getter for the current collapsed state of internal matrix outputs table component
 * @return	The current collapsed state value.
 */
bool MatrixIOPageComponent::GetOutputsCollapsed()
{
	if (m_outputsComponent)
		return m_outputsComponent->IsCollapsed();
	else
	{
		jassertfalse;
		return false;
	}
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
	if (!m_inputsComponent || !m_outputsComponent)
		return;

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

	if (m_inputsComponent->IsCollapsed())
	{
		if (isPortrait)
			matrixIOFlex.items.add(FlexItem(*m_inputsComponent).withHeight(33).withMargin(FlexItem::Margin(fmargin, fmargin, fmargin, fmargin)));
		else
			matrixIOFlex.items.add(FlexItem(*m_inputsComponent).withWidth(33).withMargin(FlexItem::Margin(fmargin, fmargin, fmargin, fmargin)));
	}
	else
		matrixIOFlex.items.add(FlexItem(*m_inputsComponent).withFlex(1).withMargin(FlexItem::Margin(fmargin, fmargin, fmargin, fmargin)));

	if (m_outputsComponent->IsCollapsed())
	{
		if (isPortrait)
			matrixIOFlex.items.add(FlexItem(*m_outputsComponent).withHeight(33).withMargin(FlexItem::Margin(fmargin, fmargin, fmargin, fmargin)));
		else
			matrixIOFlex.items.add(FlexItem(*m_outputsComponent).withWidth(33).withMargin(FlexItem::Margin(fmargin, fmargin, fmargin, fmargin)));
	}
	else
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
		if (ctrl->PopParameterChanged(DCP_MatrixInputTable, DCT_NumProcessors) || init)
		{
			m_inputsComponent->RecreateTableRowIds();
			m_inputsComponent->UpdateTable();
		}
		else if (ctrl->PopParameterChanged(DCP_MatrixInputTable, DCT_ProcessorSelection) ||
			ctrl->PopParameterChanged(DCP_MatrixInputTable, DCT_BridgingConfig))
		{
			m_inputsComponent->UpdateTable();
		}
		else
		{
			// Iterate through all procssor instances and see if anything changed there.
			for (auto const& processorId : ctrl->GetMatrixInputProcessorIds())
			{
				auto processor = ctrl->GetMatrixInputProcessor(processorId);
				if (processor && processor->PopParameterChanged(DCP_MatrixInputTable, DCT_MatrixInputProcessorConfig))
				{
					m_inputsComponent->UpdateTable();
				}
			}
		}
	}

	if (m_outputsComponent)
	{
		if (ctrl->PopParameterChanged(DCP_MatrixOutputTable, DCT_NumProcessors) || init)
		{
			m_outputsComponent->RecreateTableRowIds();
			m_outputsComponent->UpdateTable();
		}
		else if (ctrl->PopParameterChanged(DCP_MatrixOutputTable, DCT_ProcessorSelection) ||
			ctrl->PopParameterChanged(DCP_MatrixOutputTable, DCT_BridgingConfig))
		{
			m_outputsComponent->UpdateTable();
		}
		else
		{
			// Iterate through all procssor instances and see if anything changed there.
			for (auto const& processorId : ctrl->GetMatrixOutputProcessorIds())
			{
				auto processor = ctrl->GetMatrixOutputProcessor(processorId);
				if (processor && processor->PopParameterChanged(DCP_MatrixOutputTable, DCT_MatrixOutputProcessorConfig))
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
	UpdateGui(false);
}


} // namespace SpaConBridge
