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
	m_mappingAreaSelect->addItem("Mapping Area 1", 1);
	m_mappingAreaSelect->addItem("Mapping Area 2", 2);
	m_mappingAreaSelect->addItem("Mapping Area 3", 3);
	m_mappingAreaSelect->addItem("Mapping Area 4", 4);
	m_mappingAreaSelect->addListener(this);
	m_mappingAreaSelect->setTooltip("Show sound objects assigned to selected mapping area");
	addAndMakeVisible(m_mappingAreaSelect.get());

	// reverb send gain enable
	m_reverbEnable = std::make_unique<DrawableButton>("Reverb", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_reverbEnable->addListener(this);
	m_reverbEnable->setTooltip("Show En-Space send gain");
	m_reverbEnable->setClickingTogglesState(true);
	addAndMakeVisible(m_reverbEnable.get());

	// spread factor enable 
	m_spreadEnable = std::make_unique<DrawableButton>("Spread", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_spreadEnable->addListener(this);
	m_spreadEnable->setTooltip("Show Spread factor");
	m_spreadEnable->setClickingTogglesState(true);
	addAndMakeVisible(m_spreadEnable.get());

	// trigger lookandfeel update
	lookAndFeelChanged();
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
	auto margin = 5;
	auto bounds = getLocalBounds().reduced(margin);

	auto controlElementsBounds = bounds.removeFromBottom(25);
	
	// set the bounds for dropdown select by onthefly modifying 'bounds' dimensions - this leaves 'bounds' as rect with 25 removed from bottom
	m_mappingAreaSelect->setBounds(controlElementsBounds.removeFromLeft(140));
	controlElementsBounds.removeFromLeft(margin);
	m_reverbEnable->setBounds(controlElementsBounds.removeFromLeft(controlElementsBounds.getHeight()));
	controlElementsBounds.removeFromLeft(margin);
	m_spreadEnable->setBounds(controlElementsBounds.removeFromLeft(controlElementsBounds.getHeight()));
	
	// set the bounds for the 2D slider area.
	bounds.removeFromBottom(margin);
	bounds.reduce(margin, margin);
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
	if (GetSelectedMapping() != m_mappingAreaSelect->getSelectedId())
	{
		m_mappingAreaSelect->setSelectedId(GetSelectedMapping(), dontSendNotification);
		update = true;
	}

	// Update the reverb enabled state
	if (IsReverbEnabled() != m_reverbEnable->getToggleState())
	{
		m_reverbEnable->setToggleState(IsReverbEnabled(), dontSendNotification);
		update = true;
	}

	// Update the spread enabled state
	if (IsSpreadEnabled() != m_spreadEnable->getToggleState())
	{
		m_spreadEnable->setToggleState(IsSpreadEnabled(), dontSendNotification);
		update = true;
	}

	auto ctrl = Controller::GetInstance();
	if (ctrl && m_multiSliderSurface)
	{
		if (ctrl->PopParameterChanged(DCP_MultiSlider, DCT_NumProcessors) || (ctrl->PopParameterChanged(DCP_MultiSlider, DCT_ProcessorSelection)))
			update = true;
		
		// Iterate through all procssor instances and see if anything changed there.
		// At the same time collect all sources positions for updating.
		SurfaceMultiSlider::ParameterCache cachedParameters;
		for (auto const& processorId : ctrl->GetSoundobjectProcessorIds())
		{
			auto processor = ctrl->GetSoundobjectProcessor(processorId);
			if (processor)
			{
				// NOTE: only soundobjects are used that match the selected viewing mapping.
				if (processor->GetMappingId() == GetSelectedMapping())
				{
					auto soundobjectId	= processor->GetSoundobjectId();
					auto pos			= Point<float>(processor->GetParameterValue(SPI_ParamIdx_X), processor->GetParameterValue(SPI_ParamIdx_Y));
					auto spread			= processor->GetParameterValue(SPI_ParamIdx_ObjectSpread);
					auto reverbSendGain	= processor->GetParameterValue(SPI_ParamIdx_ReverbSendGain);
					auto selected		= ctrl->IsSoundobjectProcessorIdSelected(processorId);

					cachedParameters.insert(std::make_pair(processorId, SurfaceMultiSlider::SoundobjectParameters(soundobjectId, pos, spread, reverbSendGain, selected)));
				}

				if (processor->PopParameterChanged(DCP_MultiSlider, (DCT_SoundobjectProcessorConfig | DCT_SoundobjectParameters)))
					update = true;
			}
		}

		if (update && m_multiSliderSurface)
		{
			// Update all nipple positions on the 2D-Slider.
			m_multiSliderSurface->UpdateParameters(cachedParameters);
			m_multiSliderSurface->repaint();
		}
	}
}

/**
 * Called when a ComboBox has its selected item changed. 
 * @param comboBox	The combo box which has changed.
 */
void MultiSurfacePageComponent::comboBoxChanged(ComboBox *comboBox)
{
	if (GetSelectedMapping() != comboBox->getSelectedId())
	{
		SetSelectedMapping(static_cast<MappingAreaId>(comboBox->getSelectedId()));

		// Trigger an update on the multi-slider, so that only sources with the
		// selected mapping are visible.
		UpdateGui(true);
	}
}

/**
 * Called when a button has been clicked.
 * @param button	The button that was clicked.
 */
void MultiSurfacePageComponent::buttonClicked(Button* button)
{
	if (m_reverbEnable.get() == button)
	{
		SetReverbEnabled(button->getToggleState());
	}
	else if (m_spreadEnable.get() == button)
	{
		SetSpreadEnabled(button->getToggleState());
	}
}

/**
 * Get the currently selected coordinate mapping used for the multi-slider.
 * @return The selected mapping area.
 */
MappingAreaId MultiSurfacePageComponent::GetSelectedMapping() const
{
	return m_selectedMapping;
}

/**
 * Set the currently selected coordinate mapping used for the multi-slider.
 * @param mapping	The new selected mapping area.
 */
void MultiSurfacePageComponent::SetSelectedMapping(MappingAreaId mapping)
{
	m_selectedMapping = mapping;
}

/**
 * Getter for the reverb enabled state
 * @return	The enabled state.
 */
bool MultiSurfacePageComponent::IsReverbEnabled() const
{
	if (m_multiSliderSurface)
		return m_multiSliderSurface->IsReverbSndGainEnabled();
	else
		return false;
}

/**
 * Setter for the reverb enabled state
 * @param enabled	The enabled state to set.
 */
void MultiSurfacePageComponent::SetReverbEnabled(bool enabled)
{
	if (m_multiSliderSurface)
		m_multiSliderSurface->SetReverbSndGainEnabled(enabled);

	// Trigger an update on the multi-slider
	UpdateGui(true);
}

/**
 * Getter for the spread enabled state
 * @return	The enabled state.
 */
bool MultiSurfacePageComponent::IsSpreadEnabled() const
{
	if (m_multiSliderSurface)
		return m_multiSliderSurface->IsSpreadEnabled();
	else
		return false;
}

/**
 * Setter for the spread enabled state
 * @param enabled	The enabled state to set.
 */
void MultiSurfacePageComponent::SetSpreadEnabled(bool enabled)
{
	if (m_multiSliderSurface)
		m_multiSliderSurface->SetSpreadEnabled(enabled);

	// Trigger an update on the multi-slider
	UpdateGui(true);
}

/**
 * Reimplemented method to handle changed look and feel data.
 * This makes shure the add/remove buttons' svg images are colored correctly.
 */
void MultiSurfacePageComponent::lookAndFeelChanged()
{
	// first forward the call to base implementation
	Component::lookAndFeelChanged();

	// create the required button drawable images based on lookandfeel colours
	std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (dblookAndFeel)
	{
		// reverb images
		JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::sensors_black_24dp_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

		m_reverbEnable->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());

		// spread images
		JUCEAppBasics::Image_utils::getDrawableButtonImages(BinaryData::adjust_black_24dp_svg, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

		m_spreadEnable->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
	}
}


} // namespace SpaConBridge
