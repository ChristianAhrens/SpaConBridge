/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file was originally part of the Soundscape VST, AU, and AAX Plug-in and now in a derived version is part of SpaConBridge.

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


namespace SpaConBridge
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
	m_soundobjectsTable = std::make_unique<SoundobjectTableComponent>();
	m_soundobjectsTable->onCurrentSelectedProcessorChanged = [=](SoundobjectProcessorId id) { 
		SetSoundsourceProcessorEditorActive(id);
		auto config = SpaConBridge::AppConfiguration::getInstance();
		if (config)
			config->triggerConfigurationDump(false);
	};
	m_soundobjectsTable->onCurrentRowHeightChanged = [=](int rowHeight) { 
		ignoreUnused(rowHeight);
		auto config = SpaConBridge::AppConfiguration::getInstance();
		if (config)
			config->triggerConfigurationDump(false);
	};
	addAndMakeVisible(m_soundobjectsTable.get());

	// register this object as config watcher
	auto config = SpaConBridge::AppConfiguration::getInstance();
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
 * Setter for the row height of internal soundobjects table component
 * @param	height  The height value to set.
 */
void SoundobjectTablePageComponent::SetRowHeight(int height)
{
	if (m_soundobjectsTable)
		m_soundobjectsTable->SetRowHeight(height);
}

/**
 * Getter for the current row height of internal sound objects table component
 * @return	The current height value.
 */
int SoundobjectTablePageComponent::GetRowHeight()
{
	if (m_soundobjectsTable)
		return m_soundobjectsTable->GetRowHeight();
	else
		return 0;
}

/**
 * Reimplemented to paint background and frame.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void SoundobjectTablePageComponent::paint(Graphics& g)
{
	auto bounds = getLocalBounds();

	// Background
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(bounds);
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void SoundobjectTablePageComponent::resized()
{
	auto bounds = getLocalBounds();

	// flexbox for table and editor as column or row layout depending on aspect ratio
	FlexBox tableAndEditorFlex;
	FlexItem::Margin tableMargin{ 8 };
	FlexItem::Margin editorMargin{ 8 };
	auto isPortrait = IsPortraitAspectRatio();
	if (isPortrait)
	{
		tableAndEditorFlex.flexDirection = FlexBox::Direction::column;
		if (m_selectedProcessorInstanceEditor)
		{
			tableMargin = FlexItem::Margin(8, 8, 4, 8);
			editorMargin = FlexItem::Margin(4, 8, 8, 8);
		}
		else
			tableMargin = FlexItem::Margin(8, 8, 8, 8);
	}
	else
	{
		tableAndEditorFlex.flexDirection = FlexBox::Direction::row;
		if (m_selectedProcessorInstanceEditor)
		{
			tableMargin = FlexItem::Margin(8, 4, 8, 8);
			editorMargin = FlexItem::Margin(8, 8, 8, 4);
		}
		else
			tableMargin = FlexItem::Margin(8, 8, 8, 8);
	}

	tableAndEditorFlex.justifyContent = FlexBox::JustifyContent::center;

	if (m_selectedProcessorInstanceEditor)
	{
		tableAndEditorFlex.items.add(FlexItem(*m_soundobjectsTable).withFlex(1).withMargin(tableMargin));
		tableAndEditorFlex.items.add(FlexItem(*m_selectedProcessorInstanceEditor.get()).withFlex(1).withMargin(editorMargin));
	}
	else
		tableAndEditorFlex.items.add(FlexItem(*m_soundobjectsTable).withFlex(1).withMargin(tableMargin));
	
	tableAndEditorFlex.performLayout(bounds.toFloat());
}

/**
 * Function to be called from model when the current selection has changed
 */
void SoundobjectTablePageComponent::SetSoundsourceProcessorEditorActive(SoundobjectProcessorId processorId)
{
	if (processorId == INVALID_PROCESSOR_ID)
	{
		if (m_selectedProcessorInstanceEditor)
		{
			removeChildComponent(m_selectedProcessorInstanceEditor.get());
			m_selectedProcessorInstanceEditor.reset();
			resized();
		}
	}
	else
	{
		auto ctrl = Controller::GetInstance();
		if (ctrl)
		{
			auto processor = ctrl->GetSoundobjectProcessor(processorId);
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
	if (ctrl && m_soundobjectsTable)
	{
		if (ctrl->PopParameterChanged(DCP_SoundobjectTable, DCT_NumProcessors) || init)
		{
			m_soundobjectsTable->RecreateTableRowIds();
			m_soundobjectsTable->UpdateTable();
		}
		else if (ctrl->PopParameterChanged(DCP_Protocol, DCT_ProcessorSelection) ||
			ctrl->PopParameterChanged(DCP_Host, DCT_BridgingConfig))
		{
			m_soundobjectsTable->UpdateTable();
		}
		else
		{
			// Iterate through all procssor instances and see if anything changed there.
			for (auto const& processorId : ctrl->GetSoundobjectProcessorIds())
			{
				auto processor = ctrl->GetSoundobjectProcessor(processorId);
				if (processor && processor->PopParameterChanged(DCP_SoundobjectTable, DCT_SoundobjectProcessorConfig))
				{
					m_soundobjectsTable->UpdateTable();
				}
			}
		}
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


} // namespace SpaConBridge
