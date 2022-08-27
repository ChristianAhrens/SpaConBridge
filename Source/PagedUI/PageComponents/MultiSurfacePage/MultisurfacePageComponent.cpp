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


#include "MultisurfacePageComponent.h"

#include "../../PageComponentManager.h"

#include "../../../CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"
#include "../../../Controller.h"
#include "../../../MultiSoundobjectSlider.h"

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
	m_multiSliderSurface = std::make_unique<MultiSoundobjectSlider>();
	addAndMakeVisible(m_multiSliderSurface.get());

	// Mapping selector
	m_mappingAreaSelect = std::make_unique<ComboBox>("Coordinate mapping");
	m_mappingAreaSelect->setEditableText(false);
	m_mappingAreaSelect->addItem("Mapping Area 1", 1);
	m_mappingAreaSelect->addItem("Mapping Area 2", 2);
	m_mappingAreaSelect->addItem("Mapping Area 3", 3);
	m_mappingAreaSelect->addItem("Mapping Area 4", 4);
	m_mappingAreaSelect->addListener(this);
	m_mappingAreaSelect->setTooltip("Show sound objects assigned to selected Mapping Area");
	addAndMakeVisible(m_mappingAreaSelect.get());

	// load background image
	m_loadImage = std::make_unique<DrawableButton>("Load Image", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_loadImage->addListener(this);
	m_loadImage->setTooltip("Load background image for selected Mapping Area");
	addAndMakeVisible(m_loadImage.get());

	// remove background image
	m_removeImage = std::make_unique<DrawableButton>("Remove Image", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_removeImage->addListener(this);
	m_removeImage->setTooltip("Remove background image of selected Mapping Area");
	addAndMakeVisible(m_removeImage.get());

	// object names enable
	m_objectNamesEnable = std::make_unique<DrawableButton>("Object Names", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_objectNamesEnable = std::make_unique<DrawableButton>("Reverb", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_objectNamesEnable->addListener(this);
	m_objectNamesEnable->setTooltip("Show Soundobject names");
	m_objectNamesEnable->setClickingTogglesState(true);
	addAndMakeVisible(m_objectNamesEnable.get());

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
	
	m_mappingAreaSelect->setBounds(controlElementsBounds.removeFromLeft(140));
	controlElementsBounds.removeFromLeft(margin);

	m_loadImage->setBounds(controlElementsBounds.removeFromLeft(controlElementsBounds.getHeight()));
	controlElementsBounds.removeFromLeft(margin);
	m_removeImage->setBounds(controlElementsBounds.removeFromLeft(controlElementsBounds.getHeight()));
	
	controlElementsBounds.removeFromRight(margin);
	m_spreadEnable->setBounds(controlElementsBounds.removeFromRight(controlElementsBounds.getHeight()));
	controlElementsBounds.removeFromRight(margin);
	m_reverbEnable->setBounds(controlElementsBounds.removeFromRight(controlElementsBounds.getHeight()));
	controlElementsBounds.removeFromRight(margin);
	m_objectNamesEnable->setBounds(controlElementsBounds.removeFromRight(controlElementsBounds.getHeight()));
	
	// set the bounds for the 2D slider area.
	bounds.removeFromBottom(margin);
	bounds.reduce(margin, margin);

	if (m_multiSliderSurface)
	{
		auto multiSliderBounds = bounds;
		auto multiSliderAspect = multiSliderBounds.toFloat().getAspectRatio();

		auto backgroundImage = m_multiSliderSurface->GetBackgroundImage(GetSelectedMapping());
		if (backgroundImage)
		{
			auto imageBounds = backgroundImage->getBounds().toFloat();
			auto imageAspect = imageBounds.getAspectRatio();
			
			if (imageAspect > multiSliderAspect) // larger aspectratio is wider
			{
				auto aspectAdjustedHeight = static_cast<int>(multiSliderBounds.getWidth() / imageAspect);
				auto yShift = static_cast<int>(0.5f * (multiSliderBounds.getHeight() - aspectAdjustedHeight));
				multiSliderBounds.setY(multiSliderBounds.getY() + yShift);
				multiSliderBounds.setHeight(aspectAdjustedHeight);
			}
			else if (imageAspect < multiSliderAspect)
			{
				auto aspectAdjustedWidth = static_cast<int>(multiSliderBounds.getHeight() * imageAspect);
				auto xShift = static_cast<int>(0.5f * (multiSliderBounds.getWidth() - aspectAdjustedWidth));
				multiSliderBounds.setX(multiSliderBounds.getX() + xShift);
				multiSliderBounds.setWidth(aspectAdjustedWidth);
			}
		}

		m_multiSliderSurface->setBounds(multiSliderBounds);
	}
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the procssor parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void MultiSurfacePageComponent::UpdateGui(bool init)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

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

	// Update the objectnames enabled state
	if (ctrl->IsStaticRemoteObjectsPollingEnabled() != m_objectNamesEnable->getToggleState())
	{
		m_objectNamesEnable->setToggleState(ctrl->IsStaticRemoteObjectsPollingEnabled(), dontSendNotification);
		if (m_multiSliderSurface)
			m_multiSliderSurface->SetSoundobjectNamesEnabled(ctrl->IsStaticRemoteObjectsPollingEnabled());
		update = true;
	}

	if (m_multiSliderSurface)
	{
		if (	ctrl->PopParameterChanged(DCP_MultiSlider, DCT_NumProcessors) 
			|| (ctrl->PopParameterChanged(DCP_MultiSlider, DCT_ProcessorSelection)) 
			|| (ctrl->PopParameterChanged(DCP_MultiSlider, DCT_SoundobjectColourAndSize)) 
			|| (ctrl->PopParameterChanged(DCP_MultiSlider, DCT_RefreshInterval)))
			update = true;
		
		// Iterate through all procssor instances and see if anything changed there.
		// At the same time collect all sources positions for updating.
		MultiSoundobjectSlider::ParameterCache cachedParameters;
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
					auto colour			= processor->GetSoundobjectColour();
					auto size			= processor->GetSoundobjectSize();
					auto objectName		= processor->getProgramName(processor->getCurrentProgram());

					cachedParameters.insert(std::make_pair(processorId, MultiSoundobjectSlider::SoundobjectParameters(soundobjectId, pos, spread, reverbSendGain, selected, colour, size, objectName)));
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

		// finally trigger refreshing the config file
		auto config = SpaConBridge::AppConfiguration::getInstance();
		if (config)
			config->triggerConfigurationDump(false);
	}
}

/**
 * Called when a button has been clicked.
 * @param button	The button that was clicked.
 */
void MultiSurfacePageComponent::buttonClicked(Button* button)
{
	if (m_loadImage.get() == button)
	{
		// create the file chooser dialog
		auto chooser = std::make_unique<FileChooser>("Select a background image for Mapping Area " + String(GetSelectedMapping()) + "...",
			File::getSpecialLocation(File::userDocumentsDirectory), "*.jpg;*.png", true, false, this); // all filepatterns are allowed for loading (currently seems to not work on iOS and not be regarded on macOS at all)
		// and trigger opening it
		chooser->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [this](const FileChooser& chooser)
			{
				auto file = chooser.getResult();

				// verify that the result is valid (ok clicked)
				if (!file.getFullPathName().isEmpty())
					PageComponentManager::GetInstance()->LoadImageForMappingFromFile(GetSelectedMapping(), file);

				delete static_cast<const FileChooser*>(&chooser);
			});
		chooser.release();
	}
	else if (m_removeImage.get() == button)
	{
		PageComponentManager::GetInstance()->RemoveImageForMapping(GetSelectedMapping());
	}
	else if (m_reverbEnable.get() == button)
	{
		if (IsReverbEnabled() != button->getToggleState())
		{
			SetReverbEnabled(button->getToggleState());

			// finally trigger refreshing the config file
			auto config = SpaConBridge::AppConfiguration::getInstance();
			if (config)
				config->triggerConfigurationDump(false);
		}
	}
	else if (m_spreadEnable.get() == button)
	{
		if (IsSpreadEnabled() != button->getToggleState())
		{
			SetSpreadEnabled(button->getToggleState());

			// finally trigger refreshing the config file
			auto config = SpaConBridge::AppConfiguration::getInstance();
			if (config)
				config->triggerConfigurationDump(false);
		}
	}
	else if (m_objectNamesEnable.get() == button)
	{
		auto ctrl = Controller::GetInstance();
		if (ctrl && ctrl->IsStaticRemoteObjectsPollingEnabled() != button->getToggleState())
		{
			ctrl->SetStaticRemoteObjectsPollingEnabled(DCP_MultiSlider, button->getToggleState());
			
			if (m_multiSliderSurface)
				m_multiSliderSurface->SetSoundobjectNamesEnabled(button->getToggleState());
			
			// Trigger an update on the multi-slider
			UpdateGui(true);
		}
	}
}

/**
 * Get the currently selected coordinate mapping used for the multi-slider.
 * @return The selected mapping area.
 */
MappingAreaId MultiSurfacePageComponent::GetSelectedMapping() const
{
	if (m_multiSliderSurface)
		return m_multiSliderSurface->GetSelectedMapping();
	else
		return MappingAreaId::MAI_First;
}

/**
 * Set the currently selected coordinate mapping used for the multi-slider.
 * @param mapping	The new selected mapping area.
 */
bool MultiSurfacePageComponent::SetSelectedMapping(MappingAreaId mapping)
{
	if (m_multiSliderSurface)
	{
		m_multiSliderSurface->SetSelectedMapping(mapping);

		resized();

		// Trigger an update on the multi-slider
		UpdateGui(true);

		return true;
	}
	else
		return false;
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
 * Getter for the background image for given mapping area.
 * @param	mappingAreaId	The id of the mapping area to get the currently used background image for
 * @return	A pointer to the currently used image, nullptr if none is set.
 */
const juce::Image* MultiSurfacePageComponent::GetBackgroundImage(MappingAreaId mappingAreaId)
{
	if (m_multiSliderSurface)
		return m_multiSliderSurface->GetBackgroundImage(mappingAreaId);
	else
		return nullptr;
}

/**
 * Setter for the background image for given mapping area.
 * @param	mappingAreaId	The id of the mapping area to set the background image for
 * @param	backgroundImage	The image to set as background
 */
void MultiSurfacePageComponent::SetBackgroundImage(MappingAreaId mappingAreaId, const juce::Image& backgroundImage)
{
	if (m_multiSliderSurface)
	{
		m_multiSliderSurface->SetBackgroundImage(mappingAreaId, backgroundImage);

		resized();

		// Trigger an update on the multi-slider
		UpdateGui(true);
	}
}

/**
 * Helper method to remove the background image for given mapping area.
 * @param	mappingAreaId	The id of the mapping area to remove the background image of
 */
void MultiSurfacePageComponent::RemoveBackgroundImage(MappingAreaId mappingAreaId)
{
	if (m_multiSliderSurface)
	{
		m_multiSliderSurface->RemoveBackgroundImage(mappingAreaId);

		resized();

		// Trigger an update on the multi-slider
		UpdateGui(true);
	}
}

/**
 * Reimplemented method to handle changed look and feel data.
 * This makes shure the add/remove buttons' svg images are colored correctly.
 */
void MultiSurfacePageComponent::lookAndFeelChanged()
{
	// first forward the call to base implementation
	Component::lookAndFeelChanged();

	// Update drawable button images with updated lookAndFeel colours
	UpdateDrawableButtonImages(m_loadImage, BinaryData::image_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_removeImage, BinaryData::hide_image_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_reverbEnable, BinaryData::sensors_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_spreadEnable, BinaryData::adjust_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_objectNamesEnable, BinaryData::text_fields_black_24dp_svg, &getLookAndFeel());
}


} // namespace SpaConBridge
