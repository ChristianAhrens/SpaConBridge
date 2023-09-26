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

#pragma once

#include "../../../SpaConBridgeCommon.h"

#include <SplitButtonComponent.h>
#include <ZeroconfDiscoverComponent.h>


#define ZEROCONF_SUPPORTED


/**
 * Fwd. decls.
 */
namespace JUCEAppBasics {
	class FixedFontTextEditor;
	class TextWithImageButton;
	class MidiLearnerComponent;
	class MidiCommandRangeAssignment;
}

namespace SpaConBridge
{

/**
 * Fwd decls.
 */
class HeaderWithElmListComponent;
class HorizontalLayouterComponent;
class SceneIndexToMidiAssignerComponent;
class RemoteObjectToOscAssignerComponent;
class IndexToChannelAssignerComponent;
class IPAddressDisplay;


/**
 * SettingsSectionsComponent is the component to hold multiple components 
 * that are dedicated to app configuration and itself resides within 
 * a viewport for scrolling functionality.
 */
class SettingsSectionsComponent : 
	public Component,
	public Button::Listener,
	public TextEditor::Listener,
	public ComboBox::Listener,
	public JUCEAppBasics::SplitButtonComponent::Listener
{
public:
	SettingsSectionsComponent();
	~SettingsSectionsComponent() override;

	//==============================================================================
	void processUpdatedConfig();

	//==========================================================================
	void paint(Graphics&) override;
	void resized() override;

	//==========================================================================
	void lookAndFeelChanged() override;

	//==========================================================================
	void buttonClicked(Button* button) override;

	//==========================================================================
	void buttonClicked(JUCEAppBasics::SplitButtonComponent* button, std::uint64_t buttonId) override;

	//==========================================================================
	void textEditorReturnKeyPressed(TextEditor&) override;
	void textEditorFocusLost(TextEditor&) override;

	//==========================================================================
	void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

	//==========================================================================
	void setSettingsSectionActiveState(HeaderWithElmListComponent* settingsSection, bool activeState);

	//==============================================================================
	std::function<void()>								onContentSizesChangedCallback;
	std::function<void(const juce::Rectangle<int>&)>	onContentMinRequiredSizeChangedCallback;

private:
	void updateAvailableMidiInputDevices();
	void updateAvailableMidiOutputDevices();

	//==========================================================================
	void textEditorUpdated(TextEditor&);

	//==============================================================================
	void handleDS100ServiceSelected(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, ZeroconfSearcher::ZeroconfSearcher::ServiceInfo* info);
	void handleSecondDS100ServiceSelected(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, ZeroconfSearcher::ZeroconfSearcher::ServiceInfo* info);

	//==============================================================================
	void handleMidiAssiSet(Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi);
	void handleScenesToMidiAssiSet(Component* sender, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& scenesToMidiAssi);

	//==============================================================================
	void handleRemapOscAssisSet(Component* sender, const std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>& roiToCustomOscAssis);

	//==============================================================================
	void handleRTTrPMBeaconIdxAssisSet(Component* sender, const std::map<int, ChannelId>& idxToChAssis);

	//==============================================================================
	void processUpdatedGeneralConfig();
	void processUpdatedDS100Config();
	void processUpdatedDiGiCoConfig();
	void processUpdatedDAWPluginConfig();
	void processUpdatedRTTrPMConfig();
	void processUpdatedGenericOSCConfig();
	void processUpdatedGenericMIDIConfig();
	void processUpdatedADMOSCConfig();
	void processUpdatedYamahaOSCConfig();
	void processUpdatedRemapOSCConfig();

	//==============================================================================
	void createGeneralSettingsSection();
	void createDS100SettingsSection();
	void createDiGiCoSettingsSection();
	void createDAWPluginSettingsSection();
	void createRTTrPMSettingsSection();
	void createGenericOSCSettingsSection();
	void createGenericMIDISettingsSection();
	void createADMOSCSettingsSection();
	void createYamahaOSCSettingsSection();
	void createRemapOSCSettingsSection();

	// input filters for texteditors
	std::unique_ptr<TextEditor::LengthAndCharacterRestriction>	m_intervalEditFilter;
	std::unique_ptr<TextEditor::LengthAndCharacterRestriction>	m_ipAddressEditFilter;
	std::unique_ptr<TextEditor::LengthAndCharacterRestriction>	m_portEditFilter;
	std::unique_ptr<TextEditor::LengthAndCharacterRestriction>	m_mappingEditFilter;

	// General settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_GeneralSettings;
	std::unique_ptr<HorizontalLayouterComponent>				m_PageEnableButtonContainer;
	std::unique_ptr<DrawableButton>								m_SoundObjectPageButton;
	std::unique_ptr<DrawableButton>								m_MultisurfacePageButton;
	std::unique_ptr<DrawableButton>								m_MatrixIOPageButton;
	std::unique_ptr<DrawableButton>								m_ScenesPageButton;
	std::unique_ptr<DrawableButton>								m_EnSpacePageButton;
	std::unique_ptr<DrawableButton>								m_StatisticsPageButton;
	std::unique_ptr<Label>										m_EnabledPagesLabel;
	std::unique_ptr<ComboBox>									m_LookAndFeelSelect;
	std::unique_ptr<Label>										m_LookAndFeelLabel;
#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>			m_ToggleFullscreenButton;
#endif
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>			m_StaticObjectsPollingButton;
	std::unique_ptr<IPAddressDisplay>							m_SystemIpInfoEdit;
	std::unique_ptr<Label>										m_SystemIpInfoLabel;

	// DS100 settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_DS100Settings;
	std::unique_ptr<JUCEAppBasics::SplitButtonComponent>		m_DS100ProtocolSelectButton;
	std::unique_ptr<Label>										m_DS100ProtocolSelectLabel;
	const std::vector<std::pair<std::string, std::float_t>>		m_DS100ProtocolSelectTexts{ { "OSC", 1.0f}, { "AES70/OCP1", 1.0f }, { "None", 0.6f } };
	std::map<std::string, uint64>								m_DS100ProtocolSelectButtonIds;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_DS100IntervalEdit;
	std::unique_ptr<Label>										m_DS100IntervalLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_DS100IpAndPortEdit;
	std::unique_ptr<HorizontalLayouterComponent>				m_DS100ConnectionElmsContainer;
	std::unique_ptr<Label>										m_DS100IpAndPortLabel;
#ifdef ZEROCONF_SUPPORTED
	std::unique_ptr<JUCEAppBasics::ZeroconfDiscoverComponent>	m_DS100ZeroconfDiscovery;
#endif

	std::unique_ptr<JUCEAppBasics::SplitButtonComponent>		m_SecondDS100ModeButton;
	std::unique_ptr<Label>										m_SecondDS100ModeLabel;
	const std::vector<std::string>								m_SecondDS100Modes{ "Off", "Extend", "Parallel", "Mirror" };
	std::map<std::string, uint64>								m_SecondDS100ModeButtonIds;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_SecondDS100IpAndPortEdit;
	std::unique_ptr<HorizontalLayouterComponent>				m_SecondDS100ConnectionElmsContainer;
	std::unique_ptr<Label>										m_SecondDS100IpAndPortLabel;
#ifdef ZEROCONF_SUPPORTED
	std::unique_ptr<JUCEAppBasics::ZeroconfDiscoverComponent>	m_SecondDS100ZeroconfDiscovery;
#endif

	std::unique_ptr<JUCEAppBasics::SplitButtonComponent>		m_SecondDS100ParallelModeButton;
	std::unique_ptr<Label>										m_SecondDS100ParallelModeLabel;
	const std::vector<std::string>								m_SecondDS100ParallelModes{ "1st", "2nd" };
	std::map<std::string, uint64>								m_SecondDS100ParallelModeButtonIds;

	// DiGiCo settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_DiGiCoBridgingSettings;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_DiGiCoIpAddressEdit;
	std::unique_ptr<Label>										m_DiGiCoIpAddressLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_DiGiCoListeningPortEdit;
	std::unique_ptr<Label>										m_DiGiCoListeningPortLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_DiGiCoRemotePortEdit;
	std::unique_ptr<Label>										m_DiGiCoRemotePortLabel;

	// Soundscape DAW Plugin settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_DAWPluginBridgingSettings;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_DAWPluginIpAddressEdit;
	std::unique_ptr<Label>										m_DAWPluginIpAddressLabel;
	std::unique_ptr<Label>										m_DAWPluginDifferentHostInfoLabel;

	// RTTrPM settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_RTTrPMBridgingSettings;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_RTTrPMListeningPortEdit;
	std::unique_ptr<Label>										m_RTTrPMListeningPortLabel;
	std::unique_ptr<JUCEAppBasics::SplitButtonComponent>		m_RTTrPMInterpretXYRelativeButton;
	std::unique_ptr<Label>										m_RTTrPMInterpretXYRelativeLabel;
	const std::vector<std::string>								m_RTTrPMInterpretXYRelativeModes{ "Absolute", "Relative" };
	std::map<std::string, uint64>								m_RTTrPMInterpretXYRelativeButtonIds;
	std::unique_ptr<HorizontalLayouterComponent>				m_RTTrPMCoordSysModContainer;
	std::unique_ptr<Label>										m_RTTrPMCoordSysModLabel;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>			m_RTTrPMXYSwapButton;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>			m_RTTrPMInvertXButton;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>			m_RTTrPMInvertYButton;
	std::unique_ptr<Label>										m_RTTrPMAbsoluteOriginLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_RTTrPMAbsoluteOriginXEdit;
	std::unique_ptr<Label>										m_RTTrPMAbsoluteOriginXLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_RTTrPMAbsoluteOriginYEdit;
	std::unique_ptr<Label>										m_RTTrPMAbsoluteOriginYLabel;
	std::unique_ptr<HorizontalLayouterComponent>				m_RTTrPMAbsoluteOriginElmsContainer;
	std::unique_ptr<ComboBox>									m_RTTrPMMappingAreaSelect;
	std::unique_ptr<Label>										m_RTTrPMMappingAreaLabel;
	std::unique_ptr<Label>										m_RTTrPMMappingPoint1Label;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_RTTrPMMappingPoint1XEdit;
	std::unique_ptr<Label>										m_RTTrPMMappingPoint1XLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_RTTrPMMappingPoint1YEdit;
	std::unique_ptr<Label>										m_RTTrPMMappingPoint1YLabel;
	std::unique_ptr<HorizontalLayouterComponent>				m_RTTrPMMappingPoint1ElmsContainer;
	std::unique_ptr<Label>										m_RTTrPMMappingPoint2Label;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_RTTrPMMappingPoint2XEdit;
	std::unique_ptr<Label>										m_RTTrPMMappingPoint2XLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_RTTrPMMappingPoint2YEdit;
	std::unique_ptr<Label>										m_RTTrPMMappingPoint2YLabel;
	std::unique_ptr<HorizontalLayouterComponent>				m_RTTrPMMappingPoint2ElmsContainer;
	std::unique_ptr<IndexToChannelAssignerComponent>			m_RTTrPMBeaconIdxAssignmentsEditor;
	std::unique_ptr<Label>										m_RTTrPMBeaconIdxAssignmentsLabel;
	std::unique_ptr<ComboBox>									m_RTTrPMDataTypeSelect;
	const std::map<juce::String, juce::String>					m_RTTrPMDataTypes{ { "Centroid Position", { "CentroidPosition;CentroidAccelerationAndVelocity" } }, { "LED Position", { "TrackedPointPosition;TrackedPointAccelerationAndVelocity" } } };
	std::unique_ptr<Label>										m_RTTrPMDataTypeLabel;
	int															m_previousRTTrPMMappingAreaId{ 1 };

	// Generic OSC settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_GenericOSCBridgingSettings;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_GenericOSCIpAddressEdit;
	std::unique_ptr<Label>										m_GenericOSCIpAddressLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_GenericOSCListeningPortEdit;
	std::unique_ptr<Label>										m_GenericOSCListeningPortLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_GenericOSCRemotePortEdit;
	std::unique_ptr<Label>										m_GenericOSCRemotePortLabel; 
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>			m_GenericOSCDisableSendingButton;

	// Generic MIDI settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_GenericMIDIBridgingSettings;
	std::unique_ptr<ComboBox>									m_GenericMIDIInputDeviceSelect;
	std::map<int, juce::String>									m_midiOutputDeviceIdentifiers;
	std::unique_ptr<Label>										m_GenericMIDIInputDeviceSelectLabel;
	std::unique_ptr<ComboBox>									m_GenericMIDIOutputDeviceSelect;
	std::map<int, juce::String>									m_midiInputDeviceIdentifiers;
	std::unique_ptr<Label>										m_GenericMIDIOutputDeviceSelectLabel;
	std::unique_ptr<ComboBox>									m_GenericMIDIMappingAreaSelect;
	std::unique_ptr<Label>										m_GenericMIDIMappingAreaLabel;
	std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>		m_GenericMIDIMatrixInputSelectLearner;
	std::unique_ptr<Label>										m_GenericMIDIMatrixInputSelectLabel;
	std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>		m_GenericMIDISelectionSelectLearner;
	std::unique_ptr<Label>										m_GenericMIDISelectionSelectLabel;	
	std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>		m_GenericMIDIXValueLearner;
	std::unique_ptr<Label>										m_GenericMIDIXValueLabel;
	std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>		m_GenericMIDIYValueLearner;
	std::unique_ptr<Label>										m_GenericMIDIYValueLabel;
	std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>		m_GenericMIDIReverbSendGainLearner;
	std::unique_ptr<Label>										m_GenericMIDIReverbSendGainLabel;
	std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>		m_GenericMIDISourceSpreadLearner;
	std::unique_ptr<Label>										m_GenericMIDISourceSpreadLabel;
	std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>		m_GenericMIDIDelayModeLearner;
	std::unique_ptr<Label>										m_GenericMIDIDelayModeLabel;
	std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>		m_GenericMIDIMatrixInputGainLearner;
	std::unique_ptr<Label>										m_GenericMIDIMatrixInputGainLabel;
	std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>		m_GenericMIDIMatrixInputMuteLearner;
	std::unique_ptr<Label>										m_GenericMIDIMatrixInputMuteLabel;
	std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>		m_GenericMIDIMatrixOutputGainLearner;
	std::unique_ptr<Label>										m_GenericMIDIMatrixOutputGainLabel;
	std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>		m_GenericMIDIMatrixOutputMuteLearner;
	std::unique_ptr<Label>										m_GenericMIDIMatrixOutputMuteLabel;
	std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>		m_GenericMIDINextSceneLearner;
	std::unique_ptr<Label>										m_GenericMIDINextSceneLabel;
	std::unique_ptr<JUCEAppBasics::MidiLearnerComponent>		m_GenericMIDIPrevSceneLearner;
	std::unique_ptr<Label>										m_GenericMIDIPrevSceneLabel;
	std::unique_ptr<SceneIndexToMidiAssignerComponent>			m_GenericMIDIRecallSceneAssigner;
	std::unique_ptr<Label>										m_GenericMIDIRecallSceneLabel;

	// ADM OSC settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_ADMOSCBridgingSettings;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_ADMOSCIpAddressEdit;
	std::unique_ptr<Label>										m_ADMOSCIpAddressLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_ADMOSCListeningPortEdit;
	std::unique_ptr<Label>										m_ADMOSCListeningPortLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_ADMOSCRemotePortEdit;
	std::unique_ptr<Label>										m_ADMOSCRemotePortLabel;
	std::unique_ptr<ComboBox>									m_ADMOSCMappingAreaSelect;
	std::unique_ptr<Label>										m_ADMOSCMappingAreaLabel;
	std::unique_ptr<HorizontalLayouterComponent>				m_ADMOSCCoordSysModContainer;
	std::unique_ptr<Label>										m_ADMOSCCoordSysModLabel;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>			m_ADMOSCInvertXButton;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>			m_ADMOSCInvertYButton;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>			m_ADMOSCSwapXYButton;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>			m_ADMOSCDisableSendingButton;
	std::unique_ptr<JUCEAppBasics::SplitButtonComponent>		m_ADMOSCxyMsgSndModeButton;
	std::unique_ptr<Label>										m_ADMOSCxyMsgSndLabel;
	const std::vector<std::string>								m_ADMOSCxyMsgSndModes{ "Separate x, y messages", "Combined xy message" };
	std::map<std::string, uint64>								m_ADMOSCxyMsgSndModeButtonIds;

	// Yamaha OSC settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_YamahaOSCBridgingSettings;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_YamahaOSCIpAddressEdit;
	std::unique_ptr<Label>										m_YamahaOSCIpAddressLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_YamahaOSCListeningPortEdit;
	std::unique_ptr<Label>										m_YamahaOSCListeningPortLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_YamahaOSCRemotePortEdit;
	std::unique_ptr<Label>										m_YamahaOSCRemotePortLabel;
	std::unique_ptr<ComboBox>									m_YamahaOSCMappingAreaSelect;
	std::unique_ptr<Label>										m_YamahaOSCMappingAreaLabel;

	// Remap OSC settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_RemapOSCBridgingSettings;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_RemapOSCIpAddressEdit;
	std::unique_ptr<Label>										m_RemapOSCIpAddressLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_RemapOSCListeningPortEdit;
	std::unique_ptr<Label>										m_RemapOSCListeningPortLabel;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>			m_RemapOSCRemotePortEdit;
	std::unique_ptr<Label>										m_RemapOSCRemotePortLabel;
	std::unique_ptr<RemoteObjectToOscAssignerComponent>			m_RemapOSCAssignmentsEditor;
	std::unique_ptr<Label>										m_RemapOSCAssignmentsLabel;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>			m_RemapOSCDisableSendingButton;
};


} // namespace SpaConBridge
