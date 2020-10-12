/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file is part of the Soundscape VST, AU, and AAX Plug-in.

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


#include "OverviewMultisurface.h"

#include "OverviewManager.h"

#include "../SoundsourceProcessor/SoundsourceProcessor.h"
#include "../Controller.h"
#include "../SoundsourceProcessor/SurfaceSlider.h"

#include <Image_utils.hpp>


namespace SoundscapeBridgeApp
{


/*
===============================================================================
 Class COverviewMultiSurface
===============================================================================
*/

/**
 * Class constructor.
 */
COverviewMultiSurface::COverviewMultiSurface()
	: OverlayBase(OT_MultiSlide)
{
	// Add multi-slider
	m_multiSlider = std::make_unique<CSurfaceMultiSlider>();
	addAndMakeVisible(m_multiSlider.get());

	// Add mapping label
	m_posAreaLabel = std::make_unique<Label>("Coordinate mapping label", "View mapping:");
	m_posAreaLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(m_posAreaLabel.get());

	// Add mapping selector
	m_areaSelector = std::make_unique<ComboBox>("Coordinate mapping");
	m_areaSelector->setEditableText(false);
	m_areaSelector->addItem("1", 1);
	m_areaSelector->addItem("2", 2);
	m_areaSelector->addItem("3", 3);
	m_areaSelector->addItem("4", 4);
	m_areaSelector->addListener(this);
	//m_areaSelector->setColour(ComboBox::backgroundColourId, CDbStyle::GetDbColor(CDbStyle::DarkColor));
	//m_areaSelector->setColour(ComboBox::textColourId, CDbStyle::GetDbColor(CDbStyle::TextColor));
	//m_areaSelector->setColour(ComboBox::outlineColourId, CDbStyle::GetDbColor(CDbStyle::WindowColor));
	//m_areaSelector->setColour(ComboBox::buttonColourId, CDbStyle::GetDbColor(CDbStyle::MidColor));
	//m_areaSelector->setColour(ComboBox::arrowColourId, CDbStyle::GetDbColor(CDbStyle::TextColor));
	addAndMakeVisible(m_areaSelector.get());
}

/**
 * Class destructor.
 */
COverviewMultiSurface::~COverviewMultiSurface()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void COverviewMultiSurface::paint(Graphics& g)
{
	// Paint background to cover the controls behind this overlay.
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker()/*CDbStyle::GetDbColor(CDbStyle::DarkColor)*/);
	g.fillRect(Rectangle<int>(0, 0, getLocalBounds().getWidth(), getLocalBounds().getHeight()));
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void COverviewMultiSurface::resized()
{
	// Resize multi-slider.
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		m_multiSlider->setBounds(Rectangle<int>(20, 10, getLocalBounds().getWidth() - 40, getLocalBounds().getHeight() - 52));
	}

	// Mapping selector
	m_posAreaLabel->setBounds(Rectangle<int>(70, getLocalBounds().getHeight() - 32, 100, 25));
	m_areaSelector->setBounds(Rectangle<int>(170, getLocalBounds().getHeight() - 32, 50, 25));
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the plugin parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void COverviewMultiSurface::UpdateGui(bool init)
{
	// Will be set to true if any changes relevant to the multi-slider are found.
	bool update = init;

	// Update the selected mapping area.
	int selectedMapping = 0;
	COverviewManager* ovrMgr = COverviewManager::GetInstance();
	if (ovrMgr)
	{
		selectedMapping = ovrMgr->GetSelectedMapping();
		if (selectedMapping != m_areaSelector->getSelectedId())
		{
			m_areaSelector->setSelectedId(selectedMapping, dontSendNotification);
			update = true;
		}
	}

	CController* ctrl = CController::GetInstance();
	if (ctrl && m_multiSlider)
	{
		if (ctrl->PopParameterChanged(DCS_Overview, DCT_NumProcessors))
			update = true;
		
		// Iterate through all plugin instances and see if anything changed there.
		// At the same time collect all sources positions for updating.
		CSurfaceMultiSlider::PositionCache cachedPositions;
		for (int pIdx = 0; pIdx < ctrl->GetProcessorCount(); pIdx++)
		{
			SoundsourceProcessor* plugin = ctrl->GetProcessor(pIdx);
			if (plugin)
			{
				if (plugin->GetMappingId() == selectedMapping)
				{
					// NOTE: only sources are included, which match the selected viewing mapping.
					Point<float> p(plugin->GetParameterValue(ParamIdx_X), plugin->GetParameterValue(ParamIdx_Y));
					cachedPositions.insert(std::make_pair(pIdx, std::make_pair(plugin->GetSourceId(), p)));
				}

				if (plugin->PopParameterChanged(DCS_Overview, (DCT_PluginInstanceConfig | DCT_SourcePosition)))
					update = true;
			}
		}

		CSurfaceMultiSlider* multiSlider = dynamic_cast<CSurfaceMultiSlider*>(m_multiSlider.get());
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
void COverviewMultiSurface::comboBoxChanged(ComboBox *comboBox)
{
	COverviewManager* ovrMgr = COverviewManager::GetInstance();
	if (ovrMgr)
	{
		if (ovrMgr->GetSelectedMapping() != comboBox->getSelectedId())
		{
			ovrMgr->SetSelectedMapping(comboBox->getSelectedId());

			// Trigger an update on the multi-slider, so that only sources with the
			// selected mapping are visible.
			UpdateGui(true);
		}
	}
}


} // namespace SoundscapeBridgeApp
