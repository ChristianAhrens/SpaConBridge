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


#include "MultiSoundobjectComponent.h"

#include "PagedUI/PageComponentManager.h"

#include "CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"
#include "Controller.h"
#include "ProcessorSelectionManager.h"
#include "MultiSoundobjectSlider.h"
#include "SelectGroupSelector.h"

#include <Image_utils.h>


namespace SpaConBridge
{


/*
===============================================================================
 Class MultiSoundobjectComponent
===============================================================================
*/

/**
 * Class constructor.
 */
MultiSoundobjectComponent::MultiSoundobjectComponent()
	: StandalonePollingBase()
{
	// Add multi-slider
	m_multiSoundobjectSlider = std::make_unique<MultiSoundobjectSlider>();
	addAndMakeVisible(m_multiSoundobjectSlider.get());

	// Mapping selector
	m_mappingAreaSelect = std::make_unique<ComboBox>("Coordinate mapping");
	m_mappingAreaSelect->setEditableText(false);
	m_mappingAreaSelect->addItem("All", MAI_Invalid);
	m_mappingAreaSelect->addItem("Mapping Area 1", MAI_First);
	m_mappingAreaSelect->addItem("Mapping Area 2", MAI_Second);
	m_mappingAreaSelect->addItem("Mapping Area 3", MAI_Third);
	m_mappingAreaSelect->addItem("Mapping Area 4", MAI_Fourth);
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

	// extended multiselection interaction enable
	m_muselvisuEnable = std::make_unique<DrawableButton>("MuselVisuInteraction", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_muselvisuEnable->addListener(this);
	m_muselvisuEnable->setTooltip("Enable extended multiselection interaction.\nOnly available when specific Mapping Area is selected.");
	m_muselvisuEnable->setClickingTogglesState(true);
	addAndMakeVisible(m_muselvisuEnable.get());

	// select a selection group or add a new one
	m_selectionGroupSelect = std::make_unique<SelectGroupSelector>("selectgroups");
	m_selectionGroupSelect->SetMode(SelectGroupSelector::SoundobjectSelections);
	addAndMakeVisible(m_selectionGroupSelect.get());

	// object names enable
	m_objectNamesEnable = std::make_unique<DrawableButton>("Object Names", DrawableButton::ButtonStyle::ImageOnButtonBackground);
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

	// add the coordinatemapping settings objects to low-freq local polling
	for (auto i = int(MAI_First); i <= int(MAI_Fourth); i++)
	{
		AddStandalonePollingObject(ROI_CoordinateMappingSettings_P1real, RemoteObjectAddressing(i, INVALID_ADDRESS_VALUE));
		AddStandalonePollingObject(ROI_CoordinateMappingSettings_P2real, RemoteObjectAddressing(i, INVALID_ADDRESS_VALUE));
		AddStandalonePollingObject(ROI_CoordinateMappingSettings_P3real, RemoteObjectAddressing(i, INVALID_ADDRESS_VALUE));
		AddStandalonePollingObject(ROI_CoordinateMappingSettings_P4real, RemoteObjectAddressing(i, INVALID_ADDRESS_VALUE));
		AddStandalonePollingObject(ROI_CoordinateMappingSettings_P1virtual, RemoteObjectAddressing(i, INVALID_ADDRESS_VALUE));
		AddStandalonePollingObject(ROI_CoordinateMappingSettings_P3virtual, RemoteObjectAddressing(i, INVALID_ADDRESS_VALUE));
		AddStandalonePollingObject(ROI_CoordinateMappingSettings_Flip, RemoteObjectAddressing(i, INVALID_ADDRESS_VALUE));
		AddStandalonePollingObject(ROI_CoordinateMappingSettings_Name, RemoteObjectAddressing(i, INVALID_ADDRESS_VALUE));
	}

	// add the speaker position objects to low-freq local polling
	for (auto i = 1; i <= DS100_CHANNELCOUNT; i++)
	{
		AddStandalonePollingObject(ROI_Positioning_SpeakerPosition, RemoteObjectAddressing(i, INVALID_ADDRESS_VALUE));
	}

	// start the object refresh timer now
	setRefreshRateMs(3000);
	triggerPollOnce();
}

/**
 * Class destructor.
 */
MultiSoundobjectComponent::~MultiSoundobjectComponent()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void MultiSoundobjectComponent::paint(Graphics& g)
{
	// Paint background to cover the controls behind this overlay.
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(Rectangle<int>(0, 0, getLocalBounds().getWidth(), getLocalBounds().getHeight()));
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void MultiSoundobjectComponent::resized()
{
	auto margin = 5;
	auto bounds = getLocalBounds().reduced(margin);

	auto controlElementsBounds = bounds.removeFromBottom(25);
	
	m_mappingAreaSelect->setBounds(controlElementsBounds.removeFromLeft(140));
	controlElementsBounds.removeFromLeft(margin);

	m_loadImage->setBounds(controlElementsBounds.removeFromLeft(controlElementsBounds.getHeight()));
	controlElementsBounds.removeFromLeft(margin);
	m_removeImage->setBounds(controlElementsBounds.removeFromLeft(controlElementsBounds.getHeight()));
	controlElementsBounds.removeFromLeft(margin);
    
    if (controlElementsBounds.getWidth() < 4.8f * controlElementsBounds.getHeight())
    {
        m_muselvisuEnable->setVisible(false);
    }
    else
    {
        m_muselvisuEnable->setVisible(true);
        m_muselvisuEnable->setBounds(controlElementsBounds.removeFromLeft(controlElementsBounds.getHeight()));
    }

	controlElementsBounds.removeFromRight(margin);
	m_spreadEnable->setBounds(controlElementsBounds.removeFromRight(controlElementsBounds.getHeight()));
	controlElementsBounds.removeFromRight(margin);
	m_reverbEnable->setBounds(controlElementsBounds.removeFromRight(controlElementsBounds.getHeight()));
	controlElementsBounds.removeFromRight(margin);
	m_objectNamesEnable->setBounds(controlElementsBounds.removeFromRight(controlElementsBounds.getHeight()));

	auto selGrComboWidth = (controlElementsBounds.getWidth()) > (140 + 2 * margin) ? 140 : controlElementsBounds.getWidth() - 2 * margin;
	controlElementsBounds.removeFromLeft(margin);
	controlElementsBounds.removeFromRight(margin);
	if (controlElementsBounds.getWidth() < 2.0f * controlElementsBounds.getHeight())
	{
		m_selectionGroupSelect->setVisible(false);
	}
	else
	{
		m_selectionGroupSelect->setVisible(true);

		auto blankSpace = controlElementsBounds.getWidth() - selGrComboWidth;
		controlElementsBounds.removeFromLeft(static_cast<int>(0.5f * blankSpace));
		m_selectionGroupSelect->setBounds(controlElementsBounds.removeFromLeft(selGrComboWidth));
	}
	
	// set the bounds for the 2D slider area.
	bounds.removeFromBottom(margin);
	bounds.reduce(margin, margin);

	if (m_multiSoundobjectSlider)
	{
		auto multiSliderBounds = bounds;
		auto multiSliderAspect = multiSliderBounds.toFloat().getAspectRatio();

		if (m_multiSoundobjectSlider->HasBackgroundImage(GetSelectedMapping()))
		{
			auto imageBounds = m_multiSoundobjectSlider->GetBackgroundImage(GetSelectedMapping()).getBounds().toFloat();
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

		m_multiSoundobjectSlider->setBounds(multiSliderBounds);
	}
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the procssor parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void MultiSoundobjectComponent::UpdateGui(bool init)
{
	auto const ctrl = Controller::GetInstance();
	auto const selMgr = ProcessorSelectionManager::GetInstance();
	if (!ctrl || !selMgr)
		return;

	// Will be set to true if any changes relevant to the multi-slider are found.
	auto update = init;

	// Update the selected mapping area.
	if (GetSelectedMapping() != m_mappingAreaSelect->getSelectedId())
	{
		m_mappingAreaSelect->setSelectedId(GetSelectedMapping(), dontSendNotification);
		update = true;
	}

	// Update the reverb enabled state
	if (IsReverbVisuEnabled() != m_reverbEnable->getToggleState())
	{
		m_reverbEnable->setToggleState(IsReverbVisuEnabled(), dontSendNotification);
		update = true;
	}

	// Update the spread enabled state
	if (IsSpreadVisuEnabled() != m_spreadEnable->getToggleState())
	{
		m_spreadEnable->setToggleState(IsSpreadVisuEnabled(), dontSendNotification);
		update = true;
	}

	// Update the muselvisu enabled state
	if (IsMuSelVisuEnabled() != m_muselvisuEnable->getToggleState())
	{
		m_muselvisuEnable->setToggleState(IsMuSelVisuEnabled(), dontSendNotification);
		update = true;
	}

	// Update the objectnames enabled state
	if (ctrl->IsStaticRemoteObjectsPollingEnabled() != m_objectNamesEnable->getToggleState())
	{
		m_objectNamesEnable->setToggleState(ctrl->IsStaticRemoteObjectsPollingEnabled(), dontSendNotification);
		if (m_multiSoundobjectSlider)
			m_multiSoundobjectSlider->SetSoundobjectNamesEnabled(ctrl->IsStaticRemoteObjectsPollingEnabled());
		update = true;
	}

	if (m_multiSoundobjectSlider)
	{
#ifdef UNDEF//DEBUG
		if (ctrl->PopParameterChanged(DCP_MultiSlider, DCT_NumProcessors))
		{
			DBG(String(__FUNCTION__) + String(" ctrl update DCT_NumProcessors"));
			update = true;
		}
		if (ctrl->PopParameterChanged(DCP_MultiSlider, DCT_ProcessorSelection))
		{
			DBG(String(__FUNCTION__) + String(" ctrl update DCT_ProcessorSelection"));
			update = true;
		}
		if (ctrl->PopParameterChanged(DCP_MultiSlider, DCT_SoundobjectColourAndSize))
		{
			DBG(String(__FUNCTION__) + String(" ctrl update DCT_SoundobjectColourAndSize"));
			update = true;
		}
		if (ctrl->PopParameterChanged(DCP_MultiSlider, DCT_RefreshInterval))
		{
			DBG(String(__FUNCTION__) + String(" ctrl update DCT_RefreshInterval"));
			update = true;
		}
		if (ctrl->PopParameterChanged(DCP_MultiSlider, (DCT_SpeakerPositionData | DCT_CoordinateMappingSettingsData | DCT_ProtocolType)))
		{
			DBG(String(__FUNCTION__) + String(" ctrl update DCT_SpeakerPositionData | DCT_CoordinateMappingSettingsData"));
			if (m_multiSoundobjectSlider->IsSpeakerPositionDataReady())
				m_multiSoundobjectSlider->SetSpeakerPositionDataReady(false);
			if (m_multiSoundobjectSlider->IsCoordinateMappingsSettingsDataReady())
				m_multiSoundobjectSlider->SetCoordinateMappingSettingsDataReady(false);
			update = true;
		}
#else
		if (ctrl->PopParameterChanged(DCP_MultiSlider, (DCT_NumProcessors | DCT_ProcessorSelection | DCT_SoundobjectColourAndSize | DCT_RefreshInterval)))
		{
			update = true;
		}
		if (ctrl->PopParameterChanged(DCP_MultiSlider, (DCT_SpeakerPositionData | DCT_CoordinateMappingSettingsData | DCT_ProtocolType)))
		{
			if (m_multiSoundobjectSlider->IsSpeakerPositionDataReady())
				m_multiSoundobjectSlider->SetSpeakerPositionDataReady(false);
			if (m_multiSoundobjectSlider->IsCoordinateMappingsSettingsDataReady())
				m_multiSoundobjectSlider->SetCoordinateMappingSettingsDataReady(false);
			update = true;
		}
#endif

		// Iterate through all procssor instances and see if anything changed there.
		// At the same time collect all sources positions for updating.
		MultiSoundobjectSlider::ParameterCache cachedParameters;
		auto& soundobjectParameterMap = std::get<0>(cachedParameters);

		// special helper flag to indicate if a change was received from external source
		auto externalChangeOrigin = false;

		auto selectedSOs = 0;
		for (auto const& processorId : ctrl->GetSoundobjectProcessorIds())
		{
			auto processor = ctrl->GetSoundobjectProcessor(processorId);
			if (processor)
			{
				auto soundobjectId	= processor->GetSoundobjectId();
				auto pos			= Point<float>(processor->GetParameterValue(SPI_ParamIdx_X), processor->GetParameterValue(SPI_ParamIdx_Y));
				auto spread			= processor->GetParameterValue(SPI_ParamIdx_ObjectSpread);
				auto reverbSendGain	= processor->GetParameterValue(SPI_ParamIdx_ReverbSendGain);
				auto selected		= selMgr->IsSoundobjectProcessorIdSelected(processorId);
				auto colour			= processor->GetSoundobjectColour();
				auto size			= processor->GetSoundobjectSize();
				auto objectName		= processor->getProgramName(processor->getCurrentProgram());

				jassert(processor->GetMappingId() > MAI_Invalid && processor->GetMappingId() <= MAI_Fourth);
				soundobjectParameterMap[static_cast<MappingAreaId>(processor->GetMappingId())].insert(std::make_pair(processorId, MultiSoundobjectSlider::SoundobjectParameters(soundobjectId, pos, spread, reverbSendGain, selected, colour, size, objectName)));

				if (selected)
					selectedSOs++;

#ifdef UNDEF//DEBUG
				if (processor->PopParameterChanged(DCP_MultiSlider, DCT_SoundobjectProcessorConfig))
				{
					DBG(String(__FUNCTION__) + String(" processor update DCT_SoundobjectProcessorConfig"));
					update = true;
				}
				if (processor->PopParameterChanged(DCP_MultiSlider, DCT_SoundobjectParameters))
				{
					DBG(String(__FUNCTION__) + String(" processor update DCT_SoundobjectParameters"));
					update = true;
				}
				if (processor->PopParameterChanged(DCP_MultiSlider, DCT_ProcessorSelection))
				{
					DBG(String(__FUNCTION__) + String(" processor update DCT_ProcessorSelection"));
					update = true;
				}

				externalChangeOrigin = DataChangeParticipant::DCP_Protocol == processor->GetParameterChangeSource(DCT_SoundobjectParameters);
#else
				if (processor->PopParameterChanged(DCP_MultiSlider, (DCT_SoundobjectProcessorConfig | DCT_SoundobjectParameters | DCT_ProcessorSelection)))
				{
					update = true;

					externalChangeOrigin = DataChangeParticipant::DCP_Protocol == processor->GetParameterChangeSource(DCT_SoundobjectParameters);
				}
#endif
			}
		}

		// flag a multiselelction if present
		if (selectedSOs > 1)
			std::get<1>(cachedParameters) |= MultiSoundobjectSlider::CacheFlag::MultiSelection;

		if (update)
		{
			// Update all nipple positions on the 2D-Slider.
			m_multiSoundobjectSlider->UpdateParameters(cachedParameters, externalChangeOrigin);
			m_multiSoundobjectSlider->repaint();
		}
	}
}

/**
 * Called when a ComboBox has its selected item changed. 
 * @param comboBox	The combo box which has changed.
 */
void MultiSoundobjectComponent::comboBoxChanged(ComboBox *comboBox)
{
	if (comboBox == m_mappingAreaSelect.get())
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
}

/**
 * Called when a button has been clicked.
 * @param button	The button that was clicked.
 */
void MultiSoundobjectComponent::buttonClicked(Button* button)
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
	else if (m_muselvisuEnable.get() == button)
	{
		if (IsMuSelVisuEnabled() != button->getToggleState())
		{
			SetMuSelVisuEnabled(button->getToggleState());

			// finally trigger refreshing the config file
			auto config = SpaConBridge::AppConfiguration::getInstance();
			if (config)
				config->triggerConfigurationDump(false);
		}
	}
	else if (m_reverbEnable.get() == button)
	{
		if (IsReverbVisuEnabled() != button->getToggleState())
		{
			SetReverbVisuEnabled(button->getToggleState());

			// finally trigger refreshing the config file
			auto config = SpaConBridge::AppConfiguration::getInstance();
			if (config)
				config->triggerConfigurationDump(false);
		}
	}
	else if (m_spreadEnable.get() == button)
	{
		if (IsSpreadVisuEnabled() != button->getToggleState())
		{
			SetSpreadVisuEnabled(button->getToggleState());

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
			
			if (m_multiSoundobjectSlider)
				m_multiSoundobjectSlider->SetSoundobjectNamesEnabled(button->getToggleState());
			
			// Trigger an update on the multi-slider
			UpdateGui(true);
		}
	}
}

/**
 * Get the currently selected coordinate mapping used for the multi-slider.
 * @return The selected mapping area.
 */
MappingAreaId MultiSoundobjectComponent::GetSelectedMapping() const
{
	if (m_multiSoundobjectSlider)
		return m_multiSoundobjectSlider->GetSelectedMapping();
	else
		return MappingAreaId::MAI_First;
}

/**
 * Set the currently selected coordinate mapping used for the multi-slider.
 * @param mapping	The new selected mapping area.
 */
bool MultiSoundobjectComponent::SetSelectedMapping(MappingAreaId mapping)
{
	if (m_multiSoundobjectSlider && m_loadImage && m_removeImage && m_muselvisuEnable)
	{
		if (mapping == MAI_Invalid)
		{
			if (m_multiSoundobjectSlider->GetSelectedMapping() != MAI_Invalid)
			{
				m_multiSoundobjectSlider->SetCoordinateMappingSettingsDataReady(false);
				m_multiSoundobjectSlider->SetSpeakerPositionDataReady(false);

				restartTimer();
				triggerPollOnce();
			}
            
            m_loadImage->setEnabled(false);
            m_removeImage->setEnabled(false);

			m_muselvisuEnable->setToggleState(false, juce::dontSendNotification);
			m_muselvisuEnable->setEnabled(false);
			SetMuSelVisuEnabled(false);
		}
		else
        {
            m_loadImage->setEnabled(true);
            m_removeImage->setEnabled(true);
            m_muselvisuEnable->setEnabled(true);
        }
		m_multiSoundobjectSlider->SetSelectedMapping(mapping);

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
bool MultiSoundobjectComponent::IsReverbVisuEnabled() const
{
	if (m_multiSoundobjectSlider)
		return m_multiSoundobjectSlider->IsReverbSndGainEnabled();
	else
		return false;
}

/**
 * Setter for the reverb enabled state
 * @param enabled	The enabled state to set.
 */
void MultiSoundobjectComponent::SetReverbVisuEnabled(bool enabled)
{
	if (m_multiSoundobjectSlider)
		m_multiSoundobjectSlider->SetReverbSndGainEnabled(enabled);

	// Trigger an update on the multi-slider
	UpdateGui(true);
}

/**
 * Getter for the multiselection visu enabled state
 * @return	The enabled state.
 */
bool MultiSoundobjectComponent::IsMuSelVisuEnabled() const
{
	if (m_multiSoundobjectSlider)
		return m_multiSoundobjectSlider->IsMuSelVisuEnabled();
	else
		return false;
}

/**
 * Setter for the multiselection visu enabled state
 * @param enabled	The enabled state to set.
 */
void MultiSoundobjectComponent::SetMuSelVisuEnabled(bool enabled)
{
	if (m_multiSoundobjectSlider)
		m_multiSoundobjectSlider->SetMuSelVisuEnabled(enabled);

	// Trigger an update on the multi-slider
	UpdateGui(true);
}

/**
 * Getter for the spread enabled state
 * @return	The enabled state.
 */
bool MultiSoundobjectComponent::IsSpreadVisuEnabled() const
{
	if (m_multiSoundobjectSlider)
		return m_multiSoundobjectSlider->IsSpreadEnabled();
	else
		return false;
}

/**
 * Setter for the spread enabled state
 * @param enabled	The enabled state to set.
 */
void MultiSoundobjectComponent::SetSpreadVisuEnabled(bool enabled)
{
	if (m_multiSoundobjectSlider)
		m_multiSoundobjectSlider->SetSpreadEnabled(enabled);

	// Trigger an update on the multi-slider
	UpdateGui(true);
}

/**
 * Getter for the background image for given mapping area.
 * @param	mappingAreaId	The id of the mapping area to get the currently used background image for
 * @return	A pointer to the currently used image, nullptr if none is set.
 */
const juce::Image* MultiSoundobjectComponent::GetBackgroundImage(MappingAreaId mappingAreaId)
{
	if (m_multiSoundobjectSlider)
		return &m_multiSoundobjectSlider->GetBackgroundImage(mappingAreaId);
	else
		return nullptr;
}

/**
 * Setter for the background image for given mapping area.
 * @param	mappingAreaId	The id of the mapping area to set the background image for
 * @param	backgroundImage	The image to set as background
 */
void MultiSoundobjectComponent::SetBackgroundImage(MappingAreaId mappingAreaId, const juce::Image& backgroundImage)
{
	if (m_multiSoundobjectSlider)
	{
		m_multiSoundobjectSlider->SetBackgroundImage(mappingAreaId, backgroundImage);

		resized();

		// Trigger an update on the multi-slider
		UpdateGui(true);
	}
}

/**
 * Helper method to remove the background image for given mapping area.
 * @param	mappingAreaId	The id of the mapping area to remove the background image of
 */
void MultiSoundobjectComponent::RemoveBackgroundImage(MappingAreaId mappingAreaId)
{
	if (m_multiSoundobjectSlider)
	{
		m_multiSoundobjectSlider->RemoveBackgroundImage(mappingAreaId);

		resized();

		// Trigger an update on the multi-slider
		UpdateGui(true);
	}
}

/**
 * Getter for the show selected only state
 * @return	The show selected only state.
 */
bool MultiSoundobjectComponent::IsHandlingSelectedOnly() const
{
	if (m_multiSoundobjectSlider)
		return m_multiSoundobjectSlider->IsHandlingSelectedSoundobjectsOnly();
	else
		return false;
}

/**
 * Setter for the show selected only state
 * @param selectedOnly	The show selected only state to set.
 */
void MultiSoundobjectComponent::SetHandleSelectedOnly(bool selectedOnly)
{
	if (m_multiSoundobjectSlider)
		m_multiSoundobjectSlider->SetHandleSelectedSoundobjectsOnly(selectedOnly);

	// Trigger an update on the multi-slider
	UpdateGui(true);
}

/**
 * Reimplemented method to handle changed look and feel data.
 * This makes shure the add/remove buttons' svg images are colored correctly.
 */
void MultiSoundobjectComponent::lookAndFeelChanged()
{
	// first forward the call to base implementation
	Component::lookAndFeelChanged();

	if (m_multiSoundobjectSlider)
		m_multiSoundobjectSlider->lookAndFeelChanged();

	// Update drawable button images with updated lookAndFeel colours
	UpdateDrawableButtonImages(m_loadImage, BinaryData::image_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_removeImage, BinaryData::hide_image_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_muselvisuEnable, BinaryData::dual_handle24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_reverbEnable, BinaryData::sensors_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_spreadEnable, BinaryData::adjust_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_objectNamesEnable, BinaryData::text_fields_black_24dp_svg, &getLookAndFeel());
}

/**
 * Reimplemented from StandalonePollingBase to handle the incoming values from polling that were requested
 * and insert them to internal member data maps
 * @param	roi			The roi that was received. In this method the CoordinateMappingSettings objects are handled
 * @param	msgData		The object message value data.
 */
void MultiSoundobjectComponent::HandleObjectDataInternal(const RemoteObjectIdentifier& roi, const RemoteObjectMessageData& msgData)
{
	// all in here relies on existance of m_multiSoundobjectSlider, so we can abort if it doesnt exist
	if (!m_multiSoundobjectSlider)
		return;

	auto channel = msgData._addrVal._first;
	auto mappingAreaId = static_cast<MappingAreaId>(channel);

	switch (roi)
	{
	case ROI_CoordinateMappingSettings_P1real:
		if (msgData._payload != nullptr && msgData._payloadSize == 3 * sizeof(float) && msgData._valCount == 3 && msgData._valType == ROVT_FLOAT)
		{
			auto floatPtr = static_cast<float*>(msgData._payload);
			auto mappingCornerReal = juce::Vector3D<float>{ floatPtr[0], floatPtr[1], floatPtr[2] };
			m_multiSoundobjectSlider->SetMappingCornerReal(mappingAreaId, 0, mappingCornerReal);
		}
		break;
	case ROI_CoordinateMappingSettings_P2real:
		if (msgData._payload != nullptr && msgData._payloadSize == 3 * sizeof(float) && msgData._valCount == 3 && msgData._valType == ROVT_FLOAT)
		{
			auto floatPtr = static_cast<float*>(msgData._payload);
			auto mappingCornerReal = juce::Vector3D<float>{ floatPtr[0], floatPtr[1], floatPtr[2] };
			m_multiSoundobjectSlider->SetMappingCornerReal(mappingAreaId, 1, mappingCornerReal);
		}
		break;
	case ROI_CoordinateMappingSettings_P3real:
		if (msgData._payload != nullptr && msgData._payloadSize == 3 * sizeof(float) && msgData._valCount == 3 && msgData._valType == ROVT_FLOAT)
		{
			auto floatPtr = static_cast<float*>(msgData._payload);
			auto mappingCornerReal = juce::Vector3D<float>{ floatPtr[0], floatPtr[1], floatPtr[2] };
			m_multiSoundobjectSlider->SetMappingCornerReal(mappingAreaId, 2, mappingCornerReal);
		}
		break;
	case ROI_CoordinateMappingSettings_P4real:
		if (msgData._payload != nullptr && msgData._payloadSize == 3 * sizeof(float) && msgData._valCount == 3 && msgData._valType == ROVT_FLOAT)
		{
			auto floatPtr = static_cast<float*>(msgData._payload);
			auto mappingCornerReal = juce::Vector3D<float>{ floatPtr[0], floatPtr[1], floatPtr[2] };
			m_multiSoundobjectSlider->SetMappingCornerReal(mappingAreaId, 3, mappingCornerReal);
		}
		break;
	case ROI_CoordinateMappingSettings_P1virtual:
		if (msgData._payload != nullptr && msgData._payloadSize == 3 * sizeof(float) && msgData._valCount == 3 && msgData._valType == ROVT_FLOAT)
		{
			auto floatPtr = static_cast<float*>(msgData._payload);
			auto mappingCornerVirtual = juce::Vector3D<float>{ floatPtr[0], floatPtr[1], floatPtr[2] };
			m_multiSoundobjectSlider->SetMappingCornerVirtual(mappingAreaId, 0, mappingCornerVirtual);
		}
		break;
	case ROI_CoordinateMappingSettings_P3virtual:
		if (msgData._payload != nullptr && msgData._payloadSize == 3 * sizeof(float) && msgData._valCount == 3 && msgData._valType == ROVT_FLOAT)
		{
			auto floatPtr = static_cast<float*>(msgData._payload);
			auto mappingCornerVirtual = juce::Vector3D<float>{ floatPtr[0], floatPtr[1], floatPtr[2] };
			m_multiSoundobjectSlider->SetMappingCornerVirtual(mappingAreaId, 1, mappingCornerVirtual);
		}
		break;
	case ROI_CoordinateMappingSettings_Flip:
		if (msgData._payload != nullptr && msgData._payloadSize == sizeof(int) && msgData._valCount == 1 && msgData._valType == ROVT_INT)
		{
			auto flip = (1 == *static_cast<int*>(msgData._payload));
			m_multiSoundobjectSlider->SetMappingFlip(mappingAreaId, flip);
		}
		break;
	case ROI_CoordinateMappingSettings_Name:
		if (msgData._payload != nullptr && msgData._payloadSize == msgData._valCount * sizeof(char) && msgData._valType == ROVT_STRING)
		{
			auto name = juce::String(static_cast<char*>(msgData._payload), msgData._valCount);
			m_multiSoundobjectSlider->SetMappingName(mappingAreaId, name);
		}
		break;
	case ROI_Positioning_SpeakerPosition:
		if (msgData._payload != nullptr && msgData._payloadSize == 6 * sizeof(float) && msgData._valCount == 6 && msgData._valType == ROVT_FLOAT)
		{
			auto floatPtr = static_cast<float*>(msgData._payload);
			auto pos = juce::Vector3D(floatPtr[0], floatPtr[1], floatPtr[2]);
			auto rot = juce::Vector3D(floatPtr[3], floatPtr[4], floatPtr[5]);
			m_multiSoundobjectSlider->SetSpeakerPosition(channel, std::make_pair(pos, rot));
		}
		break;
	default:
		break;
	}

	if (m_multiSoundobjectSlider->CheckCoordinateMappingSettingsDataCompleteness() && !m_multiSoundobjectSlider->IsCoordinateMappingsSettingsDataReady())
	{
		m_multiSoundobjectSlider->SetCoordinateMappingSettingsDataReady(true);
		DBG(juce::String(__FUNCTION__) + " we now have the required CoordinateMappingSettings data at hand to do fancy stuff.");
	}

	if (m_multiSoundobjectSlider->CheckSpeakerPositionDataCompleteness() && !m_multiSoundobjectSlider->IsSpeakerPositionDataReady())
	{
		m_multiSoundobjectSlider->SetSpeakerPositionDataReady(true);
		DBG(juce::String(__FUNCTION__) + " we now have the required SpeakerPosition data at hand to do fancy stuff.");
	}

	if (m_multiSoundobjectSlider->IsCoordinateMappingsSettingsDataReady() && m_multiSoundobjectSlider->IsSpeakerPositionDataReady())
	{
		stopTimer();
		DBG(juce::String(__FUNCTION__) + " we now have the required CoordinateMappingSettings and SpeakerPosition data at hand to do fancy stuff.");
	}
}


} // namespace SpaConBridge
