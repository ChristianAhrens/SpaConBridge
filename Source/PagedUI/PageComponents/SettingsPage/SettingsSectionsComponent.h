/* Copyright (c) 2020-2021, Christian Ahrens
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

#include "../HeaderWithElmListComponent.h"

#include "../../../SpaConBridgeCommon.h"

#include <ZeroconfDiscoverComponent.h>
#include <SplitButtonComponent.h>
#include <TextWithImageButton.h>
#include <MidiLearnerComponent.h>


namespace SpaConBridge
{


/**
 * SettingsSectionsComponent is the component to hold multiple components 
 * that are dedicated to app configuration and itself resides within 
 * a viewport for scrolling functionality.
 */
class SettingsSectionsComponent : 
	public Component, 
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
	void buttonClicked(JUCEAppBasics::SplitButtonComponent* button, uint64 buttonId) override;

	//==========================================================================
	void textEditorReturnKeyPressed(TextEditor&) override;
	void textEditorFocusLost(TextEditor&) override;

	//==========================================================================
	void comboBoxChanged(ComboBox* comboBoxThatHasChanged) override;

	//==========================================================================
	void setSettingsSectionActiveState(HeaderWithElmListComponent* settingsSection, bool activeState);

private:
	void updateAvailableMidiInputDevices();
	void updateAvailableMidiOutputDevices();

	//==========================================================================
	void textEditorUpdated(TextEditor&);

	//==============================================================================
	void handleDS100ServiceSelected(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, JUCEAppBasics::ZeroconfDiscoverComponent::ServiceInfo* info);
	void handleSecondDS100ServiceSelected(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, JUCEAppBasics::ZeroconfDiscoverComponent::ServiceInfo* info);

	//==============================================================================
	void handleMidiAssiSet(Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi);

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

	// DS100 settings section
	std::unique_ptr<HeaderWithElmListComponent>					m_DS100Settings;
	std::unique_ptr<TextEditor>									m_DS100IntervalEdit;
	std::unique_ptr<Label>										m_DS100IntervalLabel;
	std::unique_ptr<TextEditor>									m_DS100IpAddressEdit;
	std::unique_ptr<Label>										m_DS100IpAddressLabel;
	std::unique_ptr<JUCEAppBasics::ZeroconfDiscoverComponent>	m_DS100ZeroconfDiscovery;

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
};


} // namespace SpaConBridge
