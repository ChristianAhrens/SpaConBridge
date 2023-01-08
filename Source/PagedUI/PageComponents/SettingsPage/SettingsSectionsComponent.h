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

#include <MidiLearnerComponent.h>
#include <SplitButtonComponent.h>
#include <TextWithImageButton.h>
#include <ZeroconfDiscoverComponent.h>

#define ZEROCONF_SUPPORTED

namespace SpaConBridge
{

/**
 * Fwd decls.
 */
class HeaderWithElmListComponent;
class HorizontalLayouterComponent;
class SceneIndexToMidiAssignerComponent;
class RemoteObjectToOscAssignerComponent;

/** 
 *	Custom reimplementation of a Texteditor that simply shows
 *	the systems current IP and if this is not unique,
 *	a popup with all alternative IPs the host system uses. 
 */
class IPAddressDisplay : public TextEditor
{
public:
	IPAddressDisplay();

	void addPopupMenuItems(PopupMenu& menuToAddTo, const MouseEvent* mouseClickEvent) override;
    
    const std::vector<juce::IPAddress> getRelevantIPs();
    
protected:
    void mouseDown(const MouseEvent& e) override;

private:
	bool IsMultiCast(const juce::IPAddress& address);
	bool IsUPnPDiscoverAddress(const juce::IPAddress& address);
    bool IsLoopbackAddress(const juce::IPAddress& address);
    bool IsBroadcastAddress(const juce::IPAddress& address);
};


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
	std::function<void()>	onContentSizesChangedCallback;

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
	std::unique_ptr<TextEditor>									m_SystemIpInfoEdit;
	std::unique_ptr<Label>										m_SystemIpInfoLabel;

	// DS100 settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_DS100Settings;
	std::unique_ptr<TextEditor>									m_DS100IntervalEdit;
	std::unique_ptr<Label>										m_DS100IntervalLabel;
	std::unique_ptr<TextEditor>									m_DS100IpAddressEdit;
	std::unique_ptr<Label>										m_DS100IpAddressLabel;

#ifdef ZEROCONF_SUPPORTED
	std::unique_ptr<JUCEAppBasics::ZeroconfDiscoverComponent>	m_DS100ZeroconfDiscovery;
#endif

	std::unique_ptr<JUCEAppBasics::SplitButtonComponent>		m_SecondDS100ModeButton;
	std::unique_ptr<Label>										m_SecondDS100ModeLabel;
	const std::vector<std::string>								m_SecondDS100Modes{ "Off", "Extend", "Parallel", "Mirror" };
	std::map<std::string, uint64>								m_SecondDS100ModeButtonIds;
	std::unique_ptr<TextEditor>									m_SecondDS100IpAddressEdit;
	std::unique_ptr<Label>										m_SecondDS100IpAddressLabel;
	std::unique_ptr<JUCEAppBasics::ZeroconfDiscoverComponent>	m_SecondDS100ZeroconfDiscovery;

	std::unique_ptr<JUCEAppBasics::SplitButtonComponent>		m_SecondDS100ParallelModeButton;
	std::unique_ptr<Label>										m_SecondDS100ParallelModeLabel;
	const std::vector<std::string>								m_SecondDS100ParallelModes{ "1st", "2nd" };
	std::map<std::string, uint64>								m_SecondDS100ParallelModeButtonIds;

	// DiGiCo settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_DiGiCoBridgingSettings;
	std::unique_ptr<TextEditor>									m_DiGiCoIpAddressEdit;
	std::unique_ptr<Label>										m_DiGiCoIpAddressLabel;
	std::unique_ptr<TextEditor>									m_DiGiCoListeningPortEdit;
	std::unique_ptr<Label>										m_DiGiCoListeningPortLabel;
	std::unique_ptr<TextEditor>									m_DiGiCoRemotePortEdit;
	std::unique_ptr<Label>										m_DiGiCoRemotePortLabel;

	// Soundscape DAW Plugin settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_DAWPluginBridgingSettings;
	std::unique_ptr<TextEditor>									m_DAWPluginIpAddressEdit;
	std::unique_ptr<Label>										m_DAWPluginIpAddressLabel;
	std::unique_ptr<Label>										m_DAWPluginDifferentHostInfoLabel;

	// RTTrPM settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_RTTrPMBridgingSettings;
	std::unique_ptr<TextEditor>									m_RTTrPMListeningPortEdit;
	std::unique_ptr<Label>										m_RTTrPMListeningPortLabel;
	std::unique_ptr<JUCEAppBasics::SplitButtonComponent>		m_RTTrPMInterpretXYRelativeButton;
	std::unique_ptr<Label>										m_RTTrPMInterpretXYRelativeLabel;
	const std::vector<std::string>								m_RTTrPMInterpretXYRelativeModes{ "Absolute", "Relative" };
	std::map<std::string, uint64>								m_RTTrPMInterpretXYRelativeButtonIds;
	std::unique_ptr<ComboBox>									m_RTTrPMMappingAreaSelect;
	std::unique_ptr<Label>										m_RTTrPMMappingAreaLabel;
	int															m_previousRTTrPMMappingAreaId{ 1 };

	// Generic OSC settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_GenericOSCBridgingSettings;
	std::unique_ptr<TextEditor>									m_GenericOSCIpAddressEdit;
	std::unique_ptr<Label>										m_GenericOSCIpAddressLabel;
	std::unique_ptr<TextEditor>									m_GenericOSCListeningPortEdit;
	std::unique_ptr<Label>										m_GenericOSCListeningPortLabel;
	std::unique_ptr<TextEditor>									m_GenericOSCRemotePortEdit;
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
	std::unique_ptr<TextEditor>									m_ADMOSCIpAddressEdit;
	std::unique_ptr<Label>										m_ADMOSCIpAddressLabel;
	std::unique_ptr<TextEditor>									m_ADMOSCListeningPortEdit;
	std::unique_ptr<Label>										m_ADMOSCListeningPortLabel;
	std::unique_ptr<TextEditor>									m_ADMOSCRemotePortEdit;
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
	std::unique_ptr<TextEditor>									m_YamahaOSCIpAddressEdit;
	std::unique_ptr<Label>										m_YamahaOSCIpAddressLabel;
	std::unique_ptr<TextEditor>									m_YamahaOSCListeningPortEdit;
	std::unique_ptr<Label>										m_YamahaOSCListeningPortLabel;
	std::unique_ptr<TextEditor>									m_YamahaOSCRemotePortEdit;
	std::unique_ptr<Label>										m_YamahaOSCRemotePortLabel;
	std::unique_ptr<ComboBox>									m_YamahaOSCMappingAreaSelect;
	std::unique_ptr<Label>										m_YamahaOSCMappingAreaLabel;

	// Remap OSC settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_RemapOSCBridgingSettings;
	std::unique_ptr<TextEditor>									m_RemapOSCIpAddressEdit;
	std::unique_ptr<Label>										m_RemapOSCIpAddressLabel;
	std::unique_ptr<TextEditor>									m_RemapOSCListeningPortEdit;
	std::unique_ptr<Label>										m_RemapOSCListeningPortLabel;
	std::unique_ptr<TextEditor>									m_RemapOSCRemotePortEdit;
	std::unique_ptr<Label>										m_RemapOSCRemotePortLabel;
	std::unique_ptr<RemoteObjectToOscAssignerComponent>			m_RemapOSCAssignmentsEditor;
	std::unique_ptr<Label>										m_RemapOSCAssignmentsLabel;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>			m_RemapOSCDisableSendingButton;
};


} // namespace SpaConBridge
