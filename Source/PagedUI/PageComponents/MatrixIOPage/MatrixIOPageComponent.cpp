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

#include "MatrixInputsComponent.h"
#include "MatrixOutputsComponent.h"

#include "../../../CustomAudioProcessors/MatrixInputProcessor/MatrixInputProcessor.h"
#include "../../../CustomAudioProcessors/MatrixInputProcessor/MatrixInputProcessorEditor.h"
#include "../../../CustomAudioProcessors/MatrixOutputProcessor/MatrixOutputProcessor.h"
#include "../../../CustomAudioProcessors/MatrixOutputProcessor/MatrixOutputProcessorEditor.h"

#include "../../PageComponentManager.h"

#include "../../../Controller.h"

#include <Image_utils.h>


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
	m_inputsComponent = std::make_unique<MatrixInputsComponent>();
	addAndMakeVisible(m_inputsComponent.get());

	m_outputsComponent = std::make_unique<MatrixOutputsComponent>();
	addAndMakeVisible(m_outputsComponent.get());

	m_addInput = std::make_unique<TextButton>();
	m_addInput->setClickingTogglesState(false);
	m_addInput->setButtonText("Add");
	m_addInput->addListener(this);
	addAndMakeVisible(m_addInput.get());
	m_removeInput = std::make_unique<TextButton>();
	m_removeInput->setClickingTogglesState(false);
	m_removeInput->setButtonText("Remove");
	m_removeInput->setEnabled(false);
	m_removeInput->addListener(this);
	addAndMakeVisible(m_removeInput.get());

	m_addOutput = std::make_unique<TextButton>();
	m_addOutput->setClickingTogglesState(false);
	m_addOutput->setButtonText("Add");
	m_addOutput->addListener(this);
	addAndMakeVisible(m_addOutput.get());
	m_removeOutput = std::make_unique<TextButton>();
	m_removeOutput->setClickingTogglesState(false);
	m_removeOutput->setButtonText("Remove");
	m_removeOutput->setEnabled(false);
	m_removeOutput->addListener(this);
	addAndMakeVisible(m_removeOutput.get());

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
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void MatrixIOPageComponent::paint(Graphics& g)
{
	auto bounds = getLocalBounds();
	auto bottomBarBounds = bounds.reduced(8);

	if (IsPortraitAspectRatio())
		bottomBarBounds = bottomBarBounds.removeFromLeft(33);
	else
		bottomBarBounds = bottomBarBounds.removeFromBottom(33);

	// Background
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(bounds);

	// Bottm bar background
	g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId));
	g.fillRect(bottomBarBounds);

	// Frame
	g.setColour(getLookAndFeel().findColour(TableListBox::outlineColourId));
	g.drawRect(bottomBarBounds, 1);
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void MatrixIOPageComponent::resized()
{
	auto bounds = getLocalBounds().toFloat().reduced(3);
	auto isPortrait = IsPortraitAspectRatio();

	auto margin = 5;

	// The layouting flexbox with parameters
	FlexBox matrixIOFlex;
	if (isPortrait)
		matrixIOFlex.flexDirection = FlexBox::Direction::column;
	else
		matrixIOFlex.flexDirection = FlexBox::Direction::row;

	matrixIOFlex.justifyContent = FlexBox::JustifyContent::center;

	matrixIOFlex.items.add(FlexItem(*m_inputsComponent).withFlex(1).withMargin(FlexItem::Margin(margin, margin, margin, margin)));
	matrixIOFlex.items.add(FlexItem(*m_outputsComponent).withFlex(1).withMargin(FlexItem::Margin(margin, margin, margin, margin)));

	if (isPortrait)
	{
		bounds.removeFromLeft(32);
		auto leftBarBounds = getLocalBounds().reduced(8).removeFromLeft(32);
		auto inputsBarBounds = leftBarBounds.removeFromTop(leftBarBounds.getHeight() / 2).reduced(4);
		inputsBarBounds.removeFromBottom(margin);
		auto outputsBarBounds = leftBarBounds.reduced(4);
		outputsBarBounds.removeFromTop(margin);

		m_addInput->setBounds(inputsBarBounds.removeFromTop(32).toNearestInt());
		inputsBarBounds.removeFromTop(margin);
		m_removeInput->setBounds(inputsBarBounds.removeFromTop(32).toNearestInt());

		m_addOutput->setBounds(outputsBarBounds.removeFromTop(32).toNearestInt());
		outputsBarBounds.removeFromTop(margin);
		m_removeOutput->setBounds(outputsBarBounds.removeFromTop(32).toNearestInt());
	}
	else
	{
		bounds.removeFromBottom(32);
		auto bottomBarBounds = getLocalBounds().reduced(8).removeFromBottom(32);
		auto inputsBarBounds = bottomBarBounds.removeFromLeft(bottomBarBounds.getWidth() / 2).reduced(4);
		inputsBarBounds.removeFromRight(margin);
		auto outputsBarBounds = bottomBarBounds.reduced(4);
		outputsBarBounds.removeFromLeft(margin);

		m_addInput->setBounds(inputsBarBounds.removeFromLeft(40).toNearestInt());
		inputsBarBounds.removeFromLeft(margin);
		m_removeInput->setBounds(inputsBarBounds.removeFromLeft(60).toNearestInt());

		m_addOutput->setBounds(outputsBarBounds.removeFromLeft(40).toNearestInt());
		outputsBarBounds.removeFromLeft(margin);
		m_removeOutput->setBounds(outputsBarBounds.removeFromLeft(60).toNearestInt());
	}

	matrixIOFlex.performLayout(bounds);
}

/**
 * Minimal helper method to determine if aspect ratio of currently
 * available screen realestate suggests we are in portrait or landscape orientation
 * and be able to use the same determination code in multiple places.
 * 
 * @return	True if we are in portrait, false if in landscape aspect ratio.
 */
bool MatrixIOPageComponent::IsPortraitAspectRatio()
{
	// determine the layout direction (we want a ratio of 0.75 to be the switching point)
	auto layoutSwitchAspectRatio = 0.75f;
	auto w = getLocalBounds().getWidth();
	auto h = getLocalBounds().getHeight();
	auto aspectRatio = h / (w != 0.0f ? w : 1.0f);

	return layoutSwitchAspectRatio < aspectRatio;
}

/**
 * Reimplemented from Button::Listener, gets called whenever the buttons are clicked.
 * @param button	The button which has been clicked.
 */
void MatrixIOPageComponent::buttonClicked(Button* button)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	if (button == m_addInput.get())
	{
		ctrl->createNewMatrixInputProcessor();
	}
	else if (button == m_addOutput.get())
	{
		ctrl->createNewMatrixOutputProcessor();
	}
	else if (button == m_removeInput.get())
	{
		auto const& selectedProcessorIds = m_inputsComponent->GetProcessorIdsForRows(m_inputsComponent->GetSelectedRows());
		
		//if (ctrl->GetSoundobjectProcessorCount() <= selectedProcessorIds.size())
		//	onCurrentSelectedProcessorChanged(INVALID_PROCESSOR_ID);
		//else
		//{
		//	auto processorCount = ctrl->GetSoundobjectProcessorCount();
		//	auto currentLastProcessorId = processorCount - 1;
		//	auto selectedProcessorsToRemoveCount = selectedProcessorIds.size();
		//	auto nextStillExistingId = static_cast<SoundobjectProcessorId>(currentLastProcessorId - selectedProcessorsToRemoveCount);
		//	m_pageContainerTable->selectedRowsChanged(nextStillExistingId);
		//}
		
		for (auto processorId : selectedProcessorIds)
		{
			if (ctrl->GetMatrixInputProcessorCount() >= 1)
				auto processor = std::unique_ptr<MatrixInputProcessor>(ctrl->GetMatrixInputProcessor(processorId)); // when processor goes out of scope, it is destroyed and the destructor does handle unregistering from ccontroller by itself
		}
	}
	else if (button == m_removeOutput.get())
	{
		auto const& selectedProcessorIds = m_outputsComponent->GetProcessorIdsForRows(m_outputsComponent->GetSelectedRows());

		//if (ctrl->GetSoundobjectProcessorCount() <= selectedProcessorIds.size())
		//	onCurrentSelectedProcessorChanged(INVALID_PROCESSOR_ID);
		//else
		//{
		//	auto processorCount = ctrl->GetSoundobjectProcessorCount();
		//	auto currentLastProcessorId = processorCount - 1;
		//	auto selectedProcessorsToRemoveCount = selectedProcessorIds.size();
		//	auto nextStillExistingId = static_cast<SoundobjectProcessorId>(currentLastProcessorId - selectedProcessorsToRemoveCount);
		//	m_pageContainerTable->selectedRowsChanged(nextStillExistingId);
		//}

		for (auto processorId : selectedProcessorIds)
		{
			if (ctrl->GetMatrixOutputProcessorCount() >= 1)
				auto processor = std::unique_ptr<MatrixOutputProcessor>(ctrl->GetMatrixOutputProcessor(processorId)); // when processor goes out of scope, it is destroyed and the destructor does handle unregistering from ccontroller by itself
		}
	}
	else
		jassertfalse; // this cannot happen, addInstance+removeInstance are complementary and only exist for readability

}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the procssor parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void MatrixIOPageComponent::UpdateGui(bool init)
{
	ignoreUnused(init);
}

/**
 * Overridden from AppConfiguration Watcher to be able
 * to live react on config changes and update the table contents.
 */
void MatrixIOPageComponent::onConfigUpdated()
{

}


} // namespace SoundscapeBridgeApp