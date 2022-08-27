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


#include "SoundobjectTablePageComponent.h"

#include "SoundobjectTableComponent.h"

#include "../../PageComponentManager.h"

#include "../../../CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"
#include "../../../CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessorEditor.h"

#include "../../../Controller.h"
#include "../../../LookAndFeel.h"
#include "../../../MultiSoundobjectComponent.h"
#include "../../../SoundobjectSlider.h"

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
	: PageComponentBase(UIPageId::UPI_Soundobjects)
{
	// Create the layouting manger/slider objects
	m_layoutManager = std::make_unique<StretchableLayoutManager>();
	m_layoutManager->setItemLayout(0, -1, -1, -1);
	m_layoutManagerItemCount = 1;

	m_isHorizontalSlider = true;
	m_multiSoundobjectsActive = false;

	// Create the table model/component.
	m_soundobjectsTable = std::make_unique<SoundobjectTableComponent>();
	m_soundobjectsTable->onCurrentSelectedProcessorChanged = [=](SoundobjectProcessorId id) { 
		SetSoundsourceProcessorEditorActive(id);
		if (IsPageInitializing())
			return;
		auto config = SpaConBridge::AppConfiguration::getInstance();
		if (config)
			config->triggerConfigurationDump(false);
	};
	m_soundobjectsTable->onCurrentRowHeightChanged = [=](int rowHeight) { 
		ignoreUnused(rowHeight);
		if (IsPageInitializing())
			return;
		auto config = SpaConBridge::AppConfiguration::getInstance();
		if (config)
			config->triggerConfigurationDump(false);
	};
	m_soundobjectsTable->onMultiProcessorsSelectionChanged = [=](bool multiselected) {
		SetMultiSoundobjectComponentActive(multiselected);
	};
	addAndMakeVisible(m_soundobjectsTable.get());

	// register this object as config watcher
	auto config = SpaConBridge::AppConfiguration::getInstance();
	if (config)
		config->addWatcher(this, true);
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
	if (!IsPageVisible())
		return;

	auto layoutingMargins = 8;
	auto layoutingBounds = getLocalBounds().reduced(layoutingMargins);
	auto layoutOrigX = layoutingMargins;
	auto layoutOrigY = layoutingMargins;
	auto layoutWidth = layoutingBounds.getWidth();
	auto layoutHeight = layoutingBounds.getHeight();

	if (m_selectedProcessorInstanceEditor || m_multiSoundobjectsActive)
	{
		if (m_layoutManagerItemCount != 3)
		{
			m_layoutManager->clearAllItems();
			m_layoutManager->setItemLayout(0, -0.05, -1, -0.5);
			m_layoutManager->setItemLayout(1, 6, 6, 6);
			m_layoutManager->setItemLayout(2, -0.05, -1, -0.5);
			m_layoutManagerItemCount = 3;
		}

		auto isPortrait = IsPortraitAspectRatio();
		if (m_isHorizontalSlider != !isPortrait)
		{
			m_isHorizontalSlider = !isPortrait;
			removeChildComponent(m_layoutResizerBar.get());
			m_layoutResizerBar = std::make_unique<StretchableLayoutResizerBar>(m_layoutManager.get(), 1, m_isHorizontalSlider);
			addAndMakeVisible(m_layoutResizerBar.get());
		}

		if (m_multiSoundobjectsActive)
		{
			auto& multiSoundobjectComponent = PageComponentManager::GetInstance()->GetMultiSoundobjectComponent();
			if (multiSoundobjectComponent)
			{
				Component* comps[] = { m_soundobjectsTable.get(), m_layoutResizerBar.get(), multiSoundobjectComponent.get() };
				m_layoutManager->layOutComponents(comps, 3, layoutOrigX, layoutOrigY, layoutWidth, layoutHeight, isPortrait, true);
			}
		}
		else
		{
			Component* comps[] = { m_soundobjectsTable.get(), m_layoutResizerBar.get(), m_selectedProcessorInstanceEditor.get() };
			m_layoutManager->layOutComponents(comps, 3, layoutOrigX, layoutOrigY, layoutWidth, layoutHeight, isPortrait, true);
		}
	}
	else
	{
		if (m_layoutManagerItemCount != 1)
		{
			m_layoutManager->clearAllItems();
			m_layoutManager->setItemLayout(0, -1, -1, -1);
			m_layoutManagerItemCount = 1;
		}

		Component* comps[] = { m_soundobjectsTable.get() };
		m_layoutManager->layOutComponents(comps, 1, layoutOrigX, layoutOrigY, layoutWidth, layoutHeight, false, true);
	}
}

/**
 * Function to be called from model when the current selection has changed
 */
void SoundobjectTablePageComponent::SetSoundsourceProcessorEditorActive(SoundobjectProcessorId processorId)
{
	if (processorId == INVALID_PROCESSOR_ID)
	{
		// remove processoreditor from layout and clean up instances
		if (m_selectedProcessorInstanceEditor)
		{
			removeChildComponent(m_selectedProcessorInstanceEditor.get());
			m_selectedProcessorInstanceEditor.reset();

			resized();
		}
	}
	else
	{
		// create slider and processoreditor instances and add them to layouting
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

					m_isHorizontalSlider = !IsPortraitAspectRatio();
					removeChildComponent(m_layoutResizerBar.get());
					m_layoutResizerBar.reset();
					m_layoutResizerBar = std::make_unique<StretchableLayoutResizerBar>(m_layoutManager.get(), 1, m_isHorizontalSlider);
					addAndMakeVisible(m_layoutResizerBar.get());

					resized();
				}
			}
		}
	}
}

/**
 * Function to be called from model when the current selection
 * has changed in a way that the currently displayed multisurface must be hidden
 * or the currently not displayed multisurface must be shown
 * @param active	True if the multisurface shall be shown, false if hidden.
 */
void SoundobjectTablePageComponent::SetMultiSoundobjectComponentActive(bool active)
{
	m_multiSoundobjectsActive = active;

	if (m_multiSoundobjectsActive)
	{
		if (!m_layoutResizerBar)
		{
			m_layoutResizerBar = std::make_unique<StretchableLayoutResizerBar>(m_layoutManager.get(), 1, m_isHorizontalSlider);
			addAndMakeVisible(m_layoutResizerBar.get());
		}

		auto& multiSoundobjectComponent = PageComponentManager::GetInstance()->GetMultiSoundobjectComponent();
		if (multiSoundobjectComponent && this != multiSoundobjectComponent->getParentComponent())
		{
			addAndMakeVisible(multiSoundobjectComponent.get());
		}
	}
	else if (!m_multiSoundobjectsActive)
	{
		auto& multiSoundobjectComponent = PageComponentManager::GetInstance()->GetMultiSoundobjectComponent();
		if (multiSoundobjectComponent && this == multiSoundobjectComponent->getParentComponent())
		{
			removeChildComponent(multiSoundobjectComponent.get());
		}
	}

	resized();
}

/**
 * Reimplemented from PageComponentBase to add or remove the multiSoundobject component to this page's layouting
 * depending on visibility. 
 * Call is forwarded to baseimplementation afterwards.
 * @param	initializing	The visible state to set.
 */
void SoundobjectTablePageComponent::SetPageIsVisible(bool visible)
{
	auto& multiSoundobjectComponent = PageComponentManager::GetInstance()->GetMultiSoundobjectComponent();
	if (!visible && multiSoundobjectComponent && this == multiSoundobjectComponent->getParentComponent())
	{
		removeChildComponent(multiSoundobjectComponent.get());
	}
	else if(visible && multiSoundobjectComponent && this != multiSoundobjectComponent->getParentComponent())
	{
		addAndMakeVisible(multiSoundobjectComponent.get());
	}

	PageComponentBase::SetPageIsVisible(visible);

	resized();
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

	if (m_multiSoundobjectsActive)
	{
		auto& multiSoundobjectComponent = PageComponentManager::GetInstance()->GetMultiSoundobjectComponent();
		if (multiSoundobjectComponent)
		{
			multiSoundobjectComponent->UpdateGui(false);
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
