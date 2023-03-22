/* Copyright (c) 2020-2023, Christian Ahrens
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
#include "../../../ProcessorSelectionManager.h"
#include "../../../LookAndFeel.h"
#include "../../../MultiSoundobjectComponent.h"
#include "../../../SoundobjectSlider.h"

#include <Image_utils.h>


namespace SpaConBridge
{

/*
===============================================================================
 Class TriplePointResizerBar
===============================================================================
*/

/**
 * Class constructor.
 */
TriplePointResizerBar::TriplePointResizerBar(StretchableLayoutManager* layoutToUse,
                             int itemIndexInLayout,
                             bool isBarVertical)
    : StretchableLayoutResizerBar(layoutToUse,
                                  itemIndexInLayout,
                                  isBarVertical)
{
}

/**
 * Class destructor.
 */
TriplePointResizerBar::~TriplePointResizerBar()
{
}

/**
 * Reimplemented to paint background and handle indication circles.
 * @param g        Graphics context that must be used to do the drawing operations.
 */
void TriplePointResizerBar::paint(Graphics& g)
{
    StretchableLayoutResizerBar::paint(g);

    auto bounds = getLocalBounds();
    auto ellipse = juce::Rectangle<float>(0, 0, 4, 4);
    ellipse.setCentre(bounds.getCentre().toFloat());
    g.setColour(getLookAndFeel().findColour(Label::textColourId));
    if (bounds.getWidth() < bounds.getHeight())
    {
        g.fillEllipse(ellipse);
        ellipse.setCentre(ellipse.getCentre() + Point<float>(0, 10));
        g.fillEllipse(ellipse);
        ellipse.setCentre(ellipse.getCentre() + Point<float>(0, -20));
        g.fillEllipse(ellipse);
    }
    else
    {
        g.fillEllipse(ellipse);
        ellipse.setCentre(ellipse.getCentre() + Point<float>(10, 0));
        g.fillEllipse(ellipse);
        ellipse.setCentre(ellipse.getCentre() + Point<float>(-20, 0));
        g.fillEllipse(ellipse);
    }
}
/**
 * Reimplemented to notify the world about mouse up events and potentially changed resizer bar position.
 * @param e        Mouse event to react to.
 */
void TriplePointResizerBar::mouseUp(const MouseEvent& e)
{
	if (e.getDistanceFromDragStart() > 0 && onResizeBarMoved)
		onResizeBarMoved();
	StretchableLayoutResizerBar::mouseUp(e);
}


/*
===============================================================================
 Class BlackFrameMultiSoundobjectComponentHelper
===============================================================================
*/

/**
 * Class constructor.
 */
BlackFrameMultiSoundobjectComponentHelper::BlackFrameMultiSoundobjectComponentHelper()
{
    
}

/**
 * Class destructor.
 */
BlackFrameMultiSoundobjectComponentHelper::~BlackFrameMultiSoundobjectComponentHelper()
{
    
}

/**
 * Reimplemented paint method to achieve having a framing black 1px rect around the embedded component.
 * @param g     The graphics object to use for painting
 */
void BlackFrameMultiSoundobjectComponentHelper::paint(Graphics& g)
{
    Component::paint(g);
    auto& multiSoundobjectComponent = PageComponentManager::GetInstance()->GetMultiSoundobjectComponent();
    if (IsInUse() && multiSoundobjectComponent)
    {
        g.setColour(getLookAndFeel().findColour(TextEditor::outlineColourId));
        g.drawRect(getLocalBounds());
    }
}

/**
 * Reimplemented resized method to achieve resizing of the multiSoundobjectComponent to full extent of this component
 */
void BlackFrameMultiSoundobjectComponentHelper::resized()
{
    auto& multiSoundobjectComponent = PageComponentManager::GetInstance()->GetMultiSoundobjectComponent();
	if (IsInUse() && multiSoundobjectComponent)
	{
		multiSoundobjectComponent->setBounds(getLocalBounds().reduced(1));
	}
}

/**
 * Helper to add the multiSoundobjectComponent to this component's UI handling
 */
void BlackFrameMultiSoundobjectComponentHelper::addInternalComponent()
{
    auto& multiSoundobjectComponent = PageComponentManager::GetInstance()->GetMultiSoundobjectComponent();
    if (multiSoundobjectComponent	// multisoundobject component exists
		&& this != multiSoundobjectComponent->getParentComponent() // multisoundobject component is not yet a child of this
		&& !(multiSoundobjectComponent->getParentComponent() && multiSoundobjectComponent->getParentComponent()->isOnDesktop())) // multisoundobject is not shown as windowed page
    {
		multiSoundobjectComponent->SetHandleSelectedOnly(true);
        addAndMakeVisible(multiSoundobjectComponent.get());
		m_inUse = true;
    }
}

/**
 * Helper to remove the multiSoundobjectComponent from this component's UI handling
 */
void BlackFrameMultiSoundobjectComponentHelper::removeInternalComponent()
{
    auto& multiSoundobjectComponent = PageComponentManager::GetInstance()->GetMultiSoundobjectComponent();
    if (multiSoundobjectComponent && this == multiSoundobjectComponent->getParentComponent())
    {
        removeChildComponent(multiSoundobjectComponent.get());
    }
	m_inUse = false;
}

/**
 * Getter for the internal 'in use' state
 * @return	The 'in use' state (member)
 */
bool BlackFrameMultiSoundobjectComponentHelper::IsInUse()
{
	return m_inUse;
}


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
    
    m_multiSoundobjectComponentContainer = std::make_unique<BlackFrameMultiSoundobjectComponentHelper>();
    addAndMakeVisible(m_multiSoundobjectComponentContainer.get());

	// Create the table model/component.
	m_soundobjectsTable = std::make_unique<SoundobjectTableComponent>();
	m_soundobjectsTable->onCurrentSelectedProcessorChanged = [=](SoundobjectProcessorId id) { 
		SetSoundsourceProcessorEditorActive(id);
	};
	m_soundobjectsTable->onCurrentRowHeightChanged = [=](int rowHeight) {
		ignoreUnused(rowHeight);
		if (IsPageInitializing())
			return;
		auto config = SpaConBridge::AppConfiguration::getInstance();
		if (config)
			config->triggerConfigurationDump(false);
	};
	m_soundobjectsTable->onCurrentSingleSelectionOnlyStateChanged = [=](bool singleSelectionOnly) {
		SetMultiSoundobjectComponentActive(!singleSelectionOnly);
		if (IsPageInitializing())
			return;
		auto config = SpaConBridge::AppConfiguration::getInstance();
		if (config)
			config->triggerConfigurationDump(false);
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
 * Setter for the resizer bar ratio in internal layout
 * @param	float  The position ratio to set.
 */
void SoundobjectTablePageComponent::SetResizeBarRatio(float ratio)
{
	m_resizeBarRatio = ratio;
    m_resizeBarRatioUpdatePending = true;

    UpdateLayoutRatio();
}

/**
 * Helper method to update layout ratio/ resizebar using value of internal resizeBarRatio member
 */
void SoundobjectTablePageComponent::UpdateLayoutRatio()
{
    auto size = IsPortraitAspectRatio() ? getHeight() : getWidth();
    if (size == 0 || !IsResizeBarRatioUpdatePending())
        return;
    
    m_resizeBarRatioUpdatePending = false;
    
    auto resultingNewPosition = static_cast<float>(size) * m_resizeBarRatio;

    if (m_layoutManager && m_layoutManager->getItemCurrentPosition(1) != resultingNewPosition)
    {
        m_layoutManager->setItemPosition(1, static_cast<int>(resultingNewPosition));

        if (m_layoutResizeBar)
            m_layoutResizeBar->hasBeenMoved();
    }
}

/**
 * Getter for the resizer bar ratio in internal layout
 * @return	The current position ratio.
 */
float SoundobjectTablePageComponent::GetResizeBarRatio()
{
	return m_resizeBarRatio;
}

/**
 * Getter for the resizer bar ratio pending boolean indicator
 * @return    The current position ratio update pending flag value.
 */
bool SoundobjectTablePageComponent::IsResizeBarRatioUpdatePending()
{
    return m_resizeBarRatioUpdatePending;
}

/**
 * Setter for the single selection only flag in sound objects table.
 * @param singleSelectionOnly	The single selection only flag.
 */
void SoundobjectTablePageComponent::SetSingleSelectionOnly(bool singleSelectionOnly)
{
	if (m_soundobjectsTable)
		m_soundobjectsTable->SetSingleSelectionOnly(singleSelectionOnly);

	SetMultiSoundobjectComponentActive(!singleSelectionOnly);
}

/**
 * Getter for the single selection only flag in sound objects table.
 * @return	The single selection only flag.
 */
bool SoundobjectTablePageComponent::GetSingleSelectionOnly()
{
	if (m_soundobjectsTable)
		return m_soundobjectsTable->IsSingleSelectionOnly();
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

	if (m_selectedProcessorInstanceEditor || (m_multiSoundobjectComponentContainer && m_multiSoundobjectComponentContainer->IsInUse()))
	{
		activateStretchableSplitLayout();

		if (m_multiSoundobjectComponentContainer && m_multiSoundobjectComponentContainer->IsInUse())
		{
            Component* comps[] = { m_soundobjectsTable.get(), m_layoutResizeBar.get(), m_multiSoundobjectComponentContainer.get()};
            m_layoutManager->layOutComponents(comps, 3, layoutOrigX, layoutOrigY, layoutWidth, layoutHeight, IsPortraitAspectRatio(), true);
			// unclear why but another explicit resize on the multicomponent is required for correct resize behaviour
			m_multiSoundobjectComponentContainer->resized();
		}
		else
		{
			Component* comps[] = { m_soundobjectsTable.get(), m_layoutResizeBar.get(), m_selectedProcessorInstanceEditor.get() };
			m_layoutManager->layOutComponents(comps, 3, layoutOrigX, layoutOrigY, layoutWidth, layoutHeight, IsPortraitAspectRatio(), true);
		}
	}
	else
	{
		deactivateStretchableSplitLayout();

		Component* comps[] = { m_soundobjectsTable.get() };
		m_layoutManager->layOutComponents(comps, 1, layoutOrigX, layoutOrigY, layoutWidth, layoutHeight, false, true);
	}
    
    UpdateLayoutRatio();
}

void SoundobjectTablePageComponent::activateStretchableSplitLayout()
{
	if (m_layoutManagerItemCount != 3)
	{
		m_layoutManager->clearAllItems();
		m_layoutManager->setItemLayout(0, -0.05, -1, -0.5);
#if JUCE_IOS || JUCE_ANDROID
		m_layoutManager->setItemLayout(1, 16, 16, 16);
#else
		m_layoutManager->setItemLayout(1, 8, 8, 8);
#endif
		m_layoutManager->setItemLayout(2, -0.05, -1, -0.5);
		m_layoutManagerItemCount = 3;
	}

	auto isPortrait = IsPortraitAspectRatio();
	if (m_isHorizontalSlider != !isPortrait || !m_layoutResizeBar)
	{
		m_isHorizontalSlider = !isPortrait;
		removeChildComponent(m_layoutResizeBar.get());
		m_layoutResizeBar = std::make_unique<TriplePointResizerBar>(m_layoutManager.get(), 1, m_isHorizontalSlider);
		m_layoutResizeBar->onResizeBarMoved = [this]() {
			auto size = IsPortraitAspectRatio() ? getHeight() : getWidth();
			if (m_layoutManager && size != 0)
			{
                SetResizeBarRatio(m_layoutManager->getItemCurrentPosition(1) / static_cast<float>(size));

				auto config = SpaConBridge::AppConfiguration::getInstance();
				if (config)
					config->triggerConfigurationDump(false);
			}
		};
		addAndMakeVisible(m_layoutResizeBar.get());
		
        UpdateLayoutRatio();
	}
}

void SoundobjectTablePageComponent::deactivateStretchableSplitLayout()
{
	if (m_layoutManagerItemCount != 1)
	{
		m_layoutManager->clearAllItems();
		m_layoutManager->setItemLayout(0, -1, -1, -1);
		m_layoutManagerItemCount = 1;
	}

	removeChildComponent(m_layoutResizeBar.get());
	m_layoutResizeBar.reset();
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
	else if (!m_multiSoundobjectsActive)
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

	if (m_multiSoundobjectsActive && IsPageVisible())
	{
		SetSoundsourceProcessorEditorActive(INVALID_PROCESSOR_ID);

		m_multiSoundobjectComponentContainer->addInternalComponent();
	}
	else
	{
        m_multiSoundobjectComponentContainer->removeInternalComponent();
		
		auto const selMgr = ProcessorSelectionManager::GetInstance();
		if (selMgr)
		{
			auto selectedProcessorIds = selMgr->GetSelectedSoundobjectProcessorIds();
			if (selectedProcessorIds.size() == 1)
				SetSoundsourceProcessorEditorActive(selectedProcessorIds.at(0));
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
    if (!visible)
    {
        m_multiSoundobjectComponentContainer->removeInternalComponent();
    }
    else if (m_multiSoundobjectsActive)
    {
		m_multiSoundobjectComponentContainer->addInternalComponent();
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

/**
 * Reimplemented PageComponentBase method to handle changed MultiSoundobjects windowing
 * to ensure the surfaceslider is not grabbed by this page to show alongside table view.
 * @param	pageId		The id of the page that was windowed/tabbed
 * @param	windowed	Bool indication if the given page was moved to separate window or docked back into the tabbar
 */
void SoundobjectTablePageComponent::NotifyPageWasWindowed(UIPageId pageId, bool windowed)
{
	switch (pageId)
	{
	case UPI_MultiSoundobjects:
		break;
	default:
		break;
	}
}


} // namespace SpaConBridge
