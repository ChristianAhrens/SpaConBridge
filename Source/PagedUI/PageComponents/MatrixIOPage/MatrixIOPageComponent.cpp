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
	m_inputsComponent = std::make_unique<MatrixInputTableComponent>();
	addAndMakeVisible(m_inputsComponent.get());

	m_outputsComponent = std::make_unique<MatrixOutputTableComponent>();
	addAndMakeVisible(m_outputsComponent.get());

	m_addInput = std::make_unique<DrawableButton>("addInput", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_addInput->setClickingTogglesState(false);
	m_addInput->addListener(this);
	addAndMakeVisible(m_addInput.get());
	m_removeInput = std::make_unique<DrawableButton>("removeInput", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_removeInput->setClickingTogglesState(false);
	m_removeInput->setEnabled(false);
	m_removeInput->addListener(this);
	addAndMakeVisible(m_removeInput.get());

	m_addOutput = std::make_unique<DrawableButton>("addOutput", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_addOutput->setClickingTogglesState(false);
	m_addOutput->addListener(this);
	addAndMakeVisible(m_addOutput.get());
	m_removeOutput = std::make_unique<DrawableButton>("removeOutput", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_removeOutput->setClickingTogglesState(false);
	m_removeOutput->setEnabled(false);
	m_removeOutput->addListener(this);
	addAndMakeVisible(m_removeOutput.get());

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

		m_addInput->setBounds(inputsBarBounds.removeFromLeft(30).toNearestInt());
		inputsBarBounds.removeFromLeft(margin);
		m_removeInput->setBounds(inputsBarBounds.removeFromLeft(30).toNearestInt());

		m_addOutput->setBounds(outputsBarBounds.removeFromLeft(30).toNearestInt());
		outputsBarBounds.removeFromLeft(margin);
		m_removeOutput->setBounds(outputsBarBounds.removeFromLeft(30).toNearestInt());
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
 * Reimplemented method to handle changed look and feel data.
 * This makes shure the add/remove buttons' svg images are colored correctly.
 */
void MatrixIOPageComponent::lookAndFeelChanged()
{
	// first forward the call to base implementation
	Component::lookAndFeelChanged();

	// create the required button drawable images based on lookandfeel colours
	String addImageName = BinaryData::add24px_svg;
	String removeImageName = BinaryData::remove24px_svg;
	std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (dblookAndFeel)
	{
		// add images
		JUCEAppBasics::Image_utils::getDrawableButtonImages(addImageName, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

		m_addInput->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
		m_addOutput->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());

		// remove images
		JUCEAppBasics::Image_utils::getDrawableButtonImages(removeImageName, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

		m_removeInput->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
		m_removeOutput->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
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
