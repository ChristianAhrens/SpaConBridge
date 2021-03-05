/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file was originally part of the Soundscape VST, AU, and AAX Plug-in and now in a derived version is part of SoundscapeBridgeApp.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY d&b audiotechnik GmbH & Co. KG "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================
*/


#include "SoundobjectTablePageComponent.h"

#include "SoundobjectTableComponent.h"

#include "../../../CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"
#include "../../../CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessorEditor.h"

#include "../../../SurfaceSlider.h"
#include "../../../Controller.h"
#include "../../../LookAndFeel.h"

#include <Image_utils.h>


namespace SoundscapeBridgeApp
{


/*
===============================================================================
 Class SoundobjectTablePageComponent
===============================================================================
*/

/**
 * Class constructor.
 */
SoundobjectTablePageComponent::SoundobjectTablePageComponent()
	: PageComponentBase(PCT_Overview)
{
	// Create the table model/component.
	m_pageContainerTable = std::make_unique<SoundobjectTableComponent>();
	m_pageContainerTable->currentSelectedProcessorChanged = [=](SoundobjectProcessorId id) { this->onCurrentSelectedProcessorChanged(id); };
	addAndMakeVisible(m_pageContainerTable.get());

	// Add/Remove Buttons
	m_addInstance = std::make_unique<DrawableButton>("add", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_addInstance->setClickingTogglesState(false);
	m_addInstance->addListener(this);
	addAndMakeVisible(m_addInstance.get());
	m_removeInstance = std::make_unique<DrawableButton>("remove", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_removeInstance->setClickingTogglesState(false);
	m_removeInstance->setEnabled(false);
	m_removeInstance->addListener(this);
	addAndMakeVisible(m_removeInstance.get());

	// Create quick selection buttons
	m_selectLabel = std::make_unique<Label>("Select:", "Select:");
	m_selectLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(m_selectLabel.get());

	m_selectAll = std::make_unique<TextButton>();
	m_selectAll->setClickingTogglesState(false);
	m_selectAll->setButtonText("All");
	m_selectAll->setEnabled(true);
	m_selectAll->addListener(this);
	addAndMakeVisible(m_selectAll.get());

	m_selectNone = std::make_unique<TextButton>();
	m_selectNone->setClickingTogglesState(false);
	m_selectNone->setButtonText("None");
	m_selectNone->setEnabled(true);
	m_selectNone->addListener(this);
	addAndMakeVisible(m_selectNone.get());

	// trigger lookandfeel update
	lookAndFeelChanged();

	// register this object as config watcher
	auto config = SoundscapeBridgeApp::AppConfiguration::getInstance();
	if (config)
		config->addWatcher(this);
}

/**
 * Class destructor.
 */
SoundobjectTablePageComponent::~SoundobjectTablePageComponent()
{
}

/**
 * Reimplemented to paint background and frame.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void SoundobjectTablePageComponent::paint(Graphics& g)
{
	auto bounds = getLocalBounds();
	auto bottomBarBounds = bounds.reduced(8).removeFromBottom(33);

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
void SoundobjectTablePageComponent::resized()
{
	// flexbox for table and editor as column or row layout depending on aspect ratio
	FlexBox tableAndEditorFlex;
	FlexItem::Margin tableMargin{ 8 };
	FlexItem::Margin editorMargin{ 8 };
	auto isPortrait = getLocalBounds().getHeight() > getLocalBounds().getWidth();
	if (isPortrait)
	{
		tableAndEditorFlex.flexDirection = FlexBox::Direction::column;
		if (m_selectedProcessorInstanceEditor)
		{
			tableMargin = FlexItem::Margin(8, 8, 4, 8);
			editorMargin = FlexItem::Margin(4, 8, 0, 8);
		}
		else
			tableMargin = FlexItem::Margin(8, 8, 0, 8);
	}
	else
	{
		tableAndEditorFlex.flexDirection = FlexBox::Direction::row;
		if (m_selectedProcessorInstanceEditor)
		{
			tableMargin = FlexItem::Margin(8, 4, 0, 8);
			editorMargin = FlexItem::Margin(8, 8, 0, 4);
		}
		else
			tableMargin = FlexItem::Margin(8, 8, 0, 8);
	}

	tableAndEditorFlex.justifyContent = FlexBox::JustifyContent::center;

	if (m_selectedProcessorInstanceEditor)
	{
		tableAndEditorFlex.items.add(FlexItem(*m_pageContainerTable).withFlex(1).withMargin(tableMargin));
		tableAndEditorFlex.items.add(FlexItem(*m_selectedProcessorInstanceEditor.get()).withFlex(1).withMargin(editorMargin));
	}
	else
		tableAndEditorFlex.items.add(FlexItem(*m_pageContainerTable).withFlex(1).withMargin(tableMargin));
	
	// flexbox for bottom buttons
	FlexBox bottomBarFlex;
	bottomBarFlex.flexDirection = FlexBox::Direction::row;
	bottomBarFlex.justifyContent = FlexBox::JustifyContent::center;
	bottomBarFlex.alignContent = FlexBox::AlignContent::center;
	bottomBarFlex.items.addArray({
		FlexItem(*m_addInstance.get()).withFlex(1).withMaxWidth(30).withMargin(FlexItem::Margin(2, 2, 3, 4)),
		FlexItem(*m_removeInstance.get()).withFlex(1).withMaxWidth(30).withMargin(FlexItem::Margin(2, 2, 3, 2)),
		FlexItem().withFlex(2).withHeight(30),
		FlexItem(*m_selectLabel.get()).withFlex(1).withMaxWidth(80),
		FlexItem(*m_selectAll.get()).withFlex(1).withMaxWidth(40).withMargin(FlexItem::Margin(2, 2, 3, 2)),
		FlexItem(*m_selectNone.get()).withFlex(1).withMaxWidth(46).withMargin(FlexItem::Margin(2, 4, 3, 2)),
		});

	FlexBox mainFB;
	mainFB.flexDirection = FlexBox::Direction::column;
	mainFB.justifyContent = FlexBox::JustifyContent::center;
	mainFB.items.addArray({
		FlexItem(tableAndEditorFlex).withFlex(4),
		FlexItem(bottomBarFlex).withFlex(1).withMaxHeight(32).withMargin(FlexItem::Margin(0, 8, 8, 8))
		});
	mainFB.performLayout(getLocalBounds().toFloat());
}

/**
 * Reimplemented from Button::Listener, gets called whenever the buttons are clicked.
 * @param button	The button which has been clicked.
 */
void SoundobjectTablePageComponent::buttonClicked(Button *button)
{
	if ((button == m_selectAll.get()) || (button == m_selectNone.get()))
	{
		// Send true to select all rows, false to deselect all.
		m_pageContainerTable->SelectAllRows(button == m_selectAll.get());

		// Un-toggle button.			
		button->setToggleState(false, NotificationType::dontSendNotification);
	}
	else if ((button == m_addInstance.get()) || (button == m_removeInstance.get()))
	{
		auto addInstance = (button == m_addInstance.get());
		auto removeInstance = (button == m_removeInstance.get());

		auto ctrl = Controller::GetInstance();
		if (ctrl)
		{
			if (addInstance)
			{
				ctrl->createNewSoundobjectProcessor();
			}
			else if (removeInstance)
			{
				auto const& selectedProcessorIds = m_pageContainerTable->GetProcessorIdsForRows(m_pageContainerTable->GetSelectedRows());

				if (ctrl->GetSoundobjectProcessorCount() <= selectedProcessorIds.size())
					onCurrentSelectedProcessorChanged(INVALID_PROCESSOR_ID);
				else
				{
					auto processorCount = ctrl->GetSoundobjectProcessorCount();
					auto currentLastProcessorId = processorCount - 1;
					auto selectedProcessorsToRemoveCount = selectedProcessorIds.size();
					auto nextStillExistingId = static_cast<SoundobjectProcessorId>(currentLastProcessorId - selectedProcessorsToRemoveCount);
					m_pageContainerTable->selectedRowsChanged(nextStillExistingId);
				}

				for (auto processorId : selectedProcessorIds)
				{
					if (ctrl->GetSoundobjectProcessorCount() >= 1)
						auto processor = std::unique_ptr<SoundobjectProcessor>(ctrl->GetSoundobjectProcessor(processorId)); // when processor goes out of scope, it is destroyed and the destructor does handle unregistering from ccontroller by itself
				}
			}
			else
				jassertfalse; // this cannot happen, addInstance+removeInstance are complementary and only exist for readability
		}
	}
}

/**
 * Function to be called from model when the current selection has changed
 */
void SoundobjectTablePageComponent::onCurrentSelectedProcessorChanged(SoundobjectProcessorId selectedProcessorId)
{
	if (selectedProcessorId == INVALID_PROCESSOR_ID)
	{
		if (m_selectedProcessorInstanceEditor)
		{
			removeChildComponent(m_selectedProcessorInstanceEditor.get());
			m_selectedProcessorInstanceEditor.reset();
			resized();
		}

		// since we just removed the editor after the last table row was removed, the remove button must be deactivated as well
		m_removeInstance->setEnabled(false);
	}
	else
	{
		auto ctrl = Controller::GetInstance();
		if (ctrl)
		{
			auto processor = ctrl->GetSoundobjectProcessor(selectedProcessorId);
			if (processor)
			{
				auto processorEditor = processor->createEditorIfNeeded();
				auto sspEditor = dynamic_cast<SoundobjectProcessorEditor*>(processorEditor);
				if (sspEditor != m_selectedProcessorInstanceEditor.get())
				{
					removeChildComponent(m_selectedProcessorInstanceEditor.get());
					m_selectedProcessorInstanceEditor.reset();
					m_selectedProcessorInstanceEditor = std::unique_ptr<SoundobjectProcessorEditor>(sspEditor);
					if (m_selectedProcessorInstanceEditor)
					{
						addAndMakeVisible(m_selectedProcessorInstanceEditor.get());
						m_selectedProcessorInstanceEditor->UpdateGui(true);
					}
					resized();

					// since we just added another editor, remove button can be enabled (regardless of if it already was enabled)
					m_removeInstance->setEnabled(true);
				}
			}
		}
	}
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the procssor parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void SoundobjectTablePageComponent::UpdateGui(bool init)
{
	Controller* ctrl = Controller::GetInstance();
	if (ctrl && m_pageContainerTable)
	{
		if (ctrl->PopParameterChanged(DCS_SoundobjectTable, DCT_NumProcessors) || init)
		{
			m_pageContainerTable->RecreateTableRowIds();
			m_pageContainerTable->UpdateTable();
		}
		else if (ctrl->PopParameterChanged(DCS_Protocol, DCT_ProcessorSelection) ||
			ctrl->PopParameterChanged(DCS_Host, DCT_BridgingConfig))
		{
			m_pageContainerTable->UpdateTable();
		}
		else
		{
			// Iterate through all procssor instances and see if anything changed there.
			for (auto const& processorId : ctrl->GetSoundobjectProcessorIds())
			{
				auto processor = ctrl->GetSoundobjectProcessor(processorId);
				if (processor && processor->GetParameterChanged(DCS_SoundobjectTable, DCT_ProcessorInstanceConfig))
				{
					m_pageContainerTable->UpdateTable();
				}
			}
		}
	}
}

/**
 * Reimplemented method to handle changed look and feel data.
 * This makes shure the add/remove buttons' svg images are colored correctly.
 */
void SoundobjectTablePageComponent::lookAndFeelChanged()
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

		m_addInstance->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());

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

		m_removeInstance->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
	}
}

/**
 * Overridden from AppConfiguration Watcher to be able
 * to live react on config changes and update the table contents.
 */
void SoundobjectTablePageComponent::onConfigUpdated()
{
	UpdateGui(false);
}


} // namespace SoundscapeBridgeApp
