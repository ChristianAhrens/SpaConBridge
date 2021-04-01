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


#include "MultisurfacePageComponent.h"

#include "../../PageComponentManager.h"

#include "../../../CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"
#include "../../../Controller.h"
#include "../../../SurfaceSlider.h"

#include <Image_utils.h>


namespace SpaConBridge
{


/*
===============================================================================
 Class MultiSurfacePageComponent
===============================================================================
*/

/**
 * Class constructor.
 */
MultiSurfacePageComponent::MultiSurfacePageComponent()
	: PageComponentBase(PCT_MultiSlide)
{
	// Add multi-slider
	m_multiSliderSurface = std::make_unique<SurfaceMultiSlider>();
	addAndMakeVisible(m_multiSliderSurface.get());

	// Mapping selector
	m_mappingAreaSelect = std::make_unique<ComboBox>("Coordinate mapping");
	m_mappingAreaSelect->setEditableText(false);
	m_mappingAreaSelect->addItem("1", 1);
	m_mappingAreaSelect->addItem("2", 2);
	m_mappingAreaSelect->addItem("3", 3);
	m_mappingAreaSelect->addItem("4", 4);
	m_mappingAreaSelect->addListener(this);
	addAndMakeVisible(m_mappingAreaSelect.get());
	// Mapping label
	m_mappingAreaLabel = std::make_unique<Label>("Coordinate mapping label", "View mapping:");
	m_mappingAreaLabel->setJustificationType(Justification::centred);
	m_mappingAreaLabel->attachToComponent(m_mappingAreaSelect.get(), true);
	addAndMakeVisible(m_mappingAreaLabel.get());
}

/**
 * Class destructor.
 */
MultiSurfacePageComponent::~MultiSurfacePageComponent()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void MultiSurfacePageComponent::paint(Graphics& g)
{
	// Paint background to cover the controls behind this overlay.
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(Rectangle<int>(0, 0, getLocalBounds().getWidth(), getLocalBounds().getHeight()));
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void MultiSurfacePageComponent::resized()
{
	auto bounds = getLocalBounds().reduced(5);
	
	// set the bounds for dropdown select by onthefly modifying 'bounds' dimensions - this leaves 'bounds' as rect with 25 removed from bottom
	m_mappingAreaSelect->setBounds(bounds.removeFromBottom(25).removeFromLeft(170).removeFromRight(70));
	
	// set the bounds for the 2D slider area.
	bounds.removeFromBottom(5);
	bounds.reduce(5, 5);
	m_multiSliderSurface->setBounds(bounds);
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the procssor parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void MultiSurfacePageComponent::UpdateGui(bool init)
{
	// Will be set to true if any changes relevant to the multi-slider are found.
	bool update = init;

	// Update the selected mapping area.
	int selectedMapping = 0;
	auto pageMgr = PageComponentManager::GetInstance();
	if (pageMgr)
	{
		selectedMapping = pageMgr->GetSelectedMapping();
		if (selectedMapping != m_mappingAreaSelect->getSelectedId())
		{
			m_mappingAreaSelect->setSelectedId(selectedMapping, dontSendNotification);
			update = true;
		}
	}

	auto ctrl = Controller::GetInstance();
	if (ctrl && m_multiSliderSurface)
	{
		if (ctrl->PopParameterChanged(DCP_MultiSlider, DCT_NumProcessors) || (ctrl->PopParameterChanged(DCP_MultiSlider, DCT_ProcessorSelection)))
			update = true;
		
		// Iterate through all procssor instances and see if anything changed there.
		// At the same time collect all sources positions for updating.
		SurfaceMultiSlider::PositionCache cachedPositions;
		for (auto const& processorId : ctrl->GetSoundobjectProcessorIds())
		{
			auto processor = ctrl->GetSoundobjectProcessor(processorId);
			if (processor)
			{
				auto soundobjectId = processor->GetSoundobjectId();
				if (processor->GetMappingId() == selectedMapping)
				{
					// NOTE: only sources are included, which match the selected viewing mapping.
					Point<float> p(processor->GetParameterValue(SPI_ParamIdx_X), processor->GetParameterValue(SPI_ParamIdx_Y));
					cachedPositions.insert(std::make_pair(processorId, SurfaceMultiSlider::SoundobjectPosition(soundobjectId, p, ctrl->IsSoundobjectIdSelected(soundobjectId))));
				}

				if (processor->PopParameterChanged(DCP_MultiSlider, (DCT_SoundobjectProcessorConfig | DCT_SoundobjectPosition)))
					update = true;
			}
		}

		SurfaceMultiSlider* multiSlider = dynamic_cast<SurfaceMultiSlider*>(m_multiSliderSurface.get());
		if (update && multiSlider)
		{
			// Update all nipple positions on the 2D-Slider.
			multiSlider->UpdatePositions(cachedPositions);
			multiSlider->repaint();
		}
	}
}

/**
 * Called when a ComboBox has its selected item changed. 
 * @param comboBox	The combo box which has changed.
 */
void MultiSurfacePageComponent::comboBoxChanged(ComboBox *comboBox)
{
	auto pageMgr = PageComponentManager::GetInstance();
	if (pageMgr)
	{
		if (pageMgr->GetSelectedMapping() != comboBox->getSelectedId())
		{
			pageMgr->SetSelectedMapping(comboBox->getSelectedId());

			// Trigger an update on the multi-slider, so that only sources with the
			// selected mapping are visible.
			UpdateGui(true);
		}
	}
}


} // namespace SpaConBridge
