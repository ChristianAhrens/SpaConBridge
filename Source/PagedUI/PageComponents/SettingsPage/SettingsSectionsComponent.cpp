/* Copyright (c) 2020-2021, Christian Ahrens
 *
 * This file is part of SoundscapeBridgeApp <https://github.com/ChristianAhrens/SoundscapeBridgeApp>
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


#include "SettingsSectionsComponent.h"

#include "../../../Controller.h"


namespace SoundscapeBridgeApp
{


/*
===============================================================================
 Class SettingsSectionsComponent
===============================================================================
*/

/**
 * Class constructor.
 */
SettingsSectionsComponent::SettingsSectionsComponent()
{

	// TextEditor input filters to be used for different editors
	m_intervalEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(7, "1234567890"); // 7 digits: "9999 ms"
	m_ipAddressEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(15, "1234567890."); // 15 digits: "255.255.255.255"
	m_portEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(5, "1234567890"); // 5 digits: "65535"
	m_mappingEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(1, "1234"); // 1 digit: "4"

	// DS100 settings section
	m_DS100Settings = std::make_unique<HeaderWithElmListComponent>();
	m_DS100Settings->setHeaderText("DS100");
	m_DS100Settings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/DS100.md"));
	m_DS100Settings->setHasActiveToggle(false);
	addAndMakeVisible(m_DS100Settings.get());

	m_DS100IntervalEdit = std::make_unique<TextEditor>();
	m_DS100IntervalEdit->addListener(this);
	m_DS100IntervalEdit->setInputFilter(m_intervalEditFilter.get(), false);
	m_DS100IntervalLabel = std::make_unique<Label>();
	m_DS100IntervalLabel->setJustificationType(Justification::centred);
	m_DS100IntervalLabel->setText("Interval", dontSendNotification);
	m_DS100IntervalLabel->attachToComponent(m_DS100IntervalEdit.get(), true);
	m_DS100Settings->addComponent(m_DS100IntervalLabel.get(), false, false);
	m_DS100Settings->addComponent(m_DS100IntervalEdit.get(), true, false);

	//first DS100 - ch. 1-64
	m_DS100IpAddressEdit = std::make_unique<TextEditor>();
	m_DS100IpAddressEdit->addListener(this);
	m_DS100IpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_DS100IpAddressLabel = std::make_unique<Label>();
	m_DS100IpAddressLabel->setJustificationType(Justification::centred);
	m_DS100IpAddressLabel->setText("IP Address", dontSendNotification);
	m_DS100IpAddressLabel->attachToComponent(m_DS100IpAddressEdit.get(), true);
	m_DS100Settings->addComponent(m_DS100IpAddressLabel.get(), false, false);
	m_DS100Settings->addComponent(m_DS100IpAddressEdit.get(), true, false);

	m_DS100ZeroconfDiscovery = std::make_unique<JUCEAppBasics::ZeroconfDiscoverComponent>(false, false);
	m_DS100ZeroconfDiscovery->onServiceSelected = [=](JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, JUCEAppBasics::ZeroconfDiscoverComponent::ServiceInfo* info) { handleDS100ServiceSelected(type, info); };
	m_DS100ZeroconfDiscovery->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OSC, RX_PORT_DS100_HOST);
	m_DS100Settings->addComponent(m_DS100ZeroconfDiscovery.get(), true, false);

	m_SecondDS100ModeButton = std::make_unique<JUCEAppBasics::SplitButtonComponent>();
	m_SecondDS100ModeButton->addListener(this);
	m_SecondDS100ModeButtonIds[m_SecondDS100Modes[0]] = m_SecondDS100ModeButton->addButton(m_SecondDS100Modes[0]);
	m_SecondDS100ModeButtonIds[m_SecondDS100Modes[1]] = m_SecondDS100ModeButton->addButton(m_SecondDS100Modes[1]);
	m_SecondDS100ModeButtonIds[m_SecondDS100Modes[2]] = m_SecondDS100ModeButton->addButton(m_SecondDS100Modes[2]);
	m_SecondDS100ModeButtonIds[m_SecondDS100Modes[3]] = m_SecondDS100ModeButton->addButton(m_SecondDS100Modes[3]);
	m_SecondDS100ModeButton->setButtonDown(m_SecondDS100ModeButtonIds[m_SecondDS100Modes[0]]);
	m_SecondDS100ModeLabel = std::make_unique<Label>();
	m_SecondDS100ModeLabel->setJustificationType(Justification::centred);
	m_SecondDS100ModeLabel->setText("2nd DS100", dontSendNotification);
	m_SecondDS100ModeLabel->attachToComponent(m_SecondDS100ModeButton.get(), true);
	m_DS100Settings->addComponent(m_SecondDS100ModeLabel.get(), false, false);
	m_DS100Settings->addComponent(m_SecondDS100ModeButton.get(), true, false);

	//second DS100 - ch. 65-128
	m_SecondDS100IpAddressEdit = std::make_unique<TextEditor>();
	m_SecondDS100IpAddressEdit->addListener(this);
	m_SecondDS100IpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_SecondDS100IpAddressLabel = std::make_unique<Label>();
	m_SecondDS100IpAddressLabel->setJustificationType(Justification::centred);
	m_SecondDS100IpAddressLabel->setText("IP Address", dontSendNotification);
	m_SecondDS100IpAddressLabel->attachToComponent(m_SecondDS100IpAddressEdit.get(), true);
	m_SecondDS100ZeroconfDiscovery = std::make_unique<JUCEAppBasics::ZeroconfDiscoverComponent>(false, false);
	m_SecondDS100ZeroconfDiscovery->onServiceSelected = [=](JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, JUCEAppBasics::ZeroconfDiscoverComponent::ServiceInfo* info) { handleSecondDS100ServiceSelected(type, info); };
	m_SecondDS100ZeroconfDiscovery->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OSC, RX_PORT_DS100_HOST);
	m_DS100Settings->addComponent(m_SecondDS100IpAddressLabel.get(), false, false);
	m_DS100Settings->addComponent(m_SecondDS100IpAddressEdit.get(), true, false);
	m_DS100Settings->addComponent(m_SecondDS100ZeroconfDiscovery.get(), true, false);

	m_DS100Settings->resized();


	// DiGiCo settings section
	m_DiGiCoBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_DiGiCoBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_DiGiCo) + " Bridging");
	m_DiGiCoBridgingSettings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/DiGiCoOSC.md"));
	m_DiGiCoBridgingSettings->setHasActiveToggle(true);
	m_DiGiCoBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_DiGiCoBridgingSettings.get());

	m_DiGiCoIpAddressEdit = std::make_unique<TextEditor>();
	m_DiGiCoIpAddressEdit->addListener(this);
	m_DiGiCoIpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_DiGiCoIpAddressLabel = std::make_unique<Label>();
	m_DiGiCoIpAddressLabel->setJustificationType(Justification::centred);
	m_DiGiCoIpAddressLabel->setText("IP Address", dontSendNotification);
	m_DiGiCoIpAddressLabel->attachToComponent(m_DiGiCoIpAddressEdit.get(), true);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoIpAddressLabel.get(), false, false);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoIpAddressEdit.get(), true, false);

	m_DiGiCoListeningPortEdit = std::make_unique<TextEditor>();
	m_DiGiCoListeningPortEdit->addListener(this);
	m_DiGiCoListeningPortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_DiGiCoListeningPortLabel = std::make_unique<Label>();
	m_DiGiCoListeningPortLabel->setJustificationType(Justification::centred);
	m_DiGiCoListeningPortLabel->setText("Listening Port", dontSendNotification);
	m_DiGiCoListeningPortLabel->attachToComponent(m_DiGiCoListeningPortEdit.get(), true);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoListeningPortLabel.get(), false, false);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoListeningPortEdit.get(), true, false);

	m_DiGiCoRemotePortEdit = std::make_unique<TextEditor>();
	m_DiGiCoRemotePortEdit->addListener(this);
	m_DiGiCoRemotePortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_DiGiCoRemotePortLabel = std::make_unique<Label>();
	m_DiGiCoRemotePortLabel->setJustificationType(Justification::centred);
	m_DiGiCoRemotePortLabel->setText("Remote Port", dontSendNotification);
	m_DiGiCoRemotePortLabel->attachToComponent(m_DiGiCoRemotePortEdit.get(), true);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoRemotePortLabel.get(), false, false);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoRemotePortEdit.get(), true, false);

	m_DiGiCoBridgingSettings->resized();


	// BlackTrax RTTrPM settings section
	m_RTTrPMBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_RTTrPMBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_BlacktraxRTTrPM) + " Bridging");
	m_RTTrPMBridgingSettings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/BlacktraxRTTrPM.md"));
	m_RTTrPMBridgingSettings->setHasActiveToggle(true);
	m_RTTrPMBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_RTTrPMBridgingSettings.get());

	m_RTTrPMListeningPortEdit = std::make_unique<TextEditor>();
	m_RTTrPMListeningPortEdit->addListener(this);
	m_RTTrPMListeningPortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_RTTrPMListeningPortLabel = std::make_unique<Label>();
	m_RTTrPMListeningPortLabel->setJustificationType(Justification::centred);
	m_RTTrPMListeningPortLabel->setText("Listening Port", dontSendNotification);
	m_RTTrPMListeningPortLabel->attachToComponent(m_RTTrPMListeningPortEdit.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMListeningPortLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMListeningPortEdit.get(), true, false);

	m_RTTrPMInterpretXYRelativeButton = std::make_unique<JUCEAppBasics::SplitButtonComponent>();
	m_RTTrPMInterpretXYRelativeButton->addListener(this);
	m_RTTrPMInterpretXYRelativeButtonIds[m_RTTrPMInterpretXYRelativeModes[0]] = m_RTTrPMInterpretXYRelativeButton->addButton(m_RTTrPMInterpretXYRelativeModes[0]);
	m_RTTrPMInterpretXYRelativeButtonIds[m_RTTrPMInterpretXYRelativeModes[1]] = m_RTTrPMInterpretXYRelativeButton->addButton(m_RTTrPMInterpretXYRelativeModes[1]);
	m_RTTrPMInterpretXYRelativeButton->setButtonDown(m_RTTrPMInterpretXYRelativeButtonIds[m_RTTrPMInterpretXYRelativeModes[0]]);
	m_RTTrPMInterpretXYRelativeLabel = std::make_unique<Label>();
	m_RTTrPMInterpretXYRelativeLabel->setJustificationType(Justification::centred);
	m_RTTrPMInterpretXYRelativeLabel->setText("XY interpret mode", dontSendNotification);
	m_RTTrPMInterpretXYRelativeLabel->attachToComponent(m_RTTrPMInterpretXYRelativeButton.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMInterpretXYRelativeLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMInterpretXYRelativeButton.get(), true, false);

	m_RTTrPMMappingAreaSelect = std::make_unique<ComboBox>();
	m_RTTrPMMappingAreaSelect->addListener(this);
	m_RTTrPMMappingAreaSelect->addItemList({ "1", "2", "3", "4" }, MAI_First);
	m_RTTrPMMappingAreaLabel = std::make_unique<Label>();
	m_RTTrPMMappingAreaLabel->setJustificationType(Justification::centred);
	m_RTTrPMMappingAreaLabel->setText("Mapping Area", dontSendNotification);
	m_RTTrPMMappingAreaLabel->attachToComponent(m_RTTrPMMappingAreaSelect.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMMappingAreaLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMMappingAreaSelect.get(), true, false);

	m_RTTrPMBridgingSettings->resized();


	// Generic OSC settings section
	m_GenericOSCBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_GenericOSCBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_GenericOSC) + " Bridging");
	m_GenericOSCBridgingSettings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/GenericOSC.md"));
	m_GenericOSCBridgingSettings->setHasActiveToggle(true);
	m_GenericOSCBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_GenericOSCBridgingSettings.get());

	m_GenericOSCIpAddressEdit = std::make_unique<TextEditor>();
	m_GenericOSCIpAddressEdit->addListener(this);
	m_GenericOSCIpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_GenericOSCIpAddressLabel = std::make_unique<Label>();
	m_GenericOSCIpAddressLabel->setJustificationType(Justification::centred);
	m_GenericOSCIpAddressLabel->setText("IP Address", dontSendNotification);
	m_GenericOSCIpAddressLabel->attachToComponent(m_GenericOSCIpAddressEdit.get(), true);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCIpAddressLabel.get(), false, false);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCIpAddressEdit.get(), true, false);

	m_GenericOSCListeningPortEdit = std::make_unique<TextEditor>();
	m_GenericOSCListeningPortEdit->addListener(this);
	m_GenericOSCListeningPortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_GenericOSCListeningPortLabel = std::make_unique<Label>();
	m_GenericOSCListeningPortLabel->setJustificationType(Justification::centred);
	m_GenericOSCListeningPortLabel->setText("Listening Port", dontSendNotification);
	m_GenericOSCListeningPortLabel->attachToComponent(m_GenericOSCListeningPortEdit.get(), true);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCListeningPortLabel.get(), false, false);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCListeningPortEdit.get(), true, false);

	m_GenericOSCRemotePortEdit = std::make_unique<TextEditor>();
	m_GenericOSCRemotePortEdit->addListener(this);
	m_GenericOSCRemotePortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_GenericOSCRemotePortLabel = std::make_unique<Label>();
	m_GenericOSCRemotePortLabel->setJustificationType(Justification::centred);
	m_GenericOSCRemotePortLabel->setText("Remote Port", dontSendNotification);
	m_GenericOSCRemotePortLabel->attachToComponent(m_GenericOSCRemotePortEdit.get(), true);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCRemotePortLabel.get(), false, false);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCRemotePortEdit.get(), true, false);

	m_GenericOSCBridgingSettings->resized();


	// Generic MIDI settings section
	m_GenericMIDIBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_GenericMIDIBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_GenericMIDI) + " Bridging");
	m_GenericMIDIBridgingSettings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/GenericMIDI.md"));
	m_GenericMIDIBridgingSettings->setHasActiveToggle(true);
	m_GenericMIDIBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_GenericMIDIBridgingSettings.get());

	m_GenericMIDIInputDeviceSelect = std::make_unique<ComboBox>();
	m_GenericMIDIInputDeviceSelect->setTextWhenNoChoicesAvailable("No MIDI Inputs Enabled");
	updateAvailableMidiInputDevices();
	m_GenericMIDIInputDeviceSelect->addListener(this);
	m_GenericMIDIInputDeviceSelectLabel = std::make_unique<Label>();
	m_GenericMIDIInputDeviceSelectLabel->setJustificationType(Justification::centred);
	m_GenericMIDIInputDeviceSelectLabel->setText("MIDI Input", dontSendNotification);
	m_GenericMIDIInputDeviceSelectLabel->attachToComponent(m_GenericMIDIInputDeviceSelect.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIInputDeviceSelectLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIInputDeviceSelect.get(), true, false);

	m_GenericMIDIOutputDeviceSelect = std::make_unique<ComboBox>();
	m_GenericMIDIOutputDeviceSelect->setTextWhenNoChoicesAvailable("No MIDI Outputs Enabled");
	updateAvailableMidiOutputDevices();
	m_GenericMIDIOutputDeviceSelect->addListener(this);
	m_GenericMIDIOutputDeviceSelectLabel = std::make_unique<Label>();
	m_GenericMIDIOutputDeviceSelectLabel->setJustificationType(Justification::centred);
	m_GenericMIDIOutputDeviceSelectLabel->setText("MIDI Output", dontSendNotification);
	m_GenericMIDIOutputDeviceSelectLabel->attachToComponent(m_GenericMIDIOutputDeviceSelect.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIOutputDeviceSelectLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIOutputDeviceSelect.get(), true, false);

	m_GenericMIDIMappingAreaSelect = std::make_unique<ComboBox>();
	m_GenericMIDIMappingAreaSelect->addListener(this);
	m_GenericMIDIMappingAreaSelect->addItemList({ "1", "2", "3", "4" }, MAI_First);
	m_GenericMIDIMappingAreaLabel = std::make_unique<Label>();
	m_GenericMIDIMappingAreaLabel->setJustificationType(Justification::centred);
	m_GenericMIDIMappingAreaLabel->setText("Mapping Area", dontSendNotification);
	m_GenericMIDIMappingAreaLabel->attachToComponent(m_GenericMIDIMappingAreaSelect.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMappingAreaLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMappingAreaSelect.get(), true, false);

	m_GenericMIDIMatrixInputSelectLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_MatrixInput_Select), 
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_Trigger | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIMatrixInputSelectLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIMatrixInputSelectLabel = std::make_unique<Label>();
	m_GenericMIDIMatrixInputSelectLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIMatrixInputSelectLabel->setText("Object Select", dontSendNotification);
	m_GenericMIDIMatrixInputSelectLabel->attachToComponent(m_GenericMIDIMatrixInputSelectLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMatrixInputSelectLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMatrixInputSelectLearner.get(), true, false);

	m_GenericMIDIXValueLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_CoordinateMapping_SourcePosition_X),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_ValueRange | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIXValueLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIXValueLabel = std::make_unique<Label>();
	m_GenericMIDIXValueLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIXValueLabel->setText("Relative Pos. X", dontSendNotification);
	m_GenericMIDIXValueLabel->attachToComponent(m_GenericMIDIXValueLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIXValueLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIXValueLearner.get(), true, false);

	m_GenericMIDIYValueLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_CoordinateMapping_SourcePosition_Y),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_ValueRange | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIYValueLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIYValueLabel = std::make_unique<Label>();
	m_GenericMIDIYValueLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIYValueLabel->setText("Relative Pos. Y", dontSendNotification);
	m_GenericMIDIYValueLabel->attachToComponent(m_GenericMIDIYValueLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIYValueLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIYValueLearner.get(), true, false);

	m_GenericMIDIReverbSendGainLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_MatrixInput_ReverbSendGain),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_ValueRange | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIReverbSendGainLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIReverbSendGainLabel = std::make_unique<Label>();
	m_GenericMIDIReverbSendGainLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIReverbSendGainLabel->setText("Reverb Send Gain", dontSendNotification);
	m_GenericMIDIReverbSendGainLabel->attachToComponent(m_GenericMIDIReverbSendGainLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIReverbSendGainLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIReverbSendGainLearner.get(), true, false);

	m_GenericMIDISourceSpreadLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_Positioning_SourceSpread),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_ValueRange | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDISourceSpreadLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDISourceSpreadLabel = std::make_unique<Label>();
	m_GenericMIDISourceSpreadLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDISourceSpreadLabel->setText("Object Spread", dontSendNotification);
	m_GenericMIDISourceSpreadLabel->attachToComponent(m_GenericMIDISourceSpreadLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDISourceSpreadLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDISourceSpreadLearner.get(), true, false);

	m_GenericMIDIDelayModeLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_Positioning_SourceDelayMode),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_ValueRange | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIDelayModeLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIDelayModeLabel = std::make_unique<Label>();
	m_GenericMIDIDelayModeLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIDelayModeLabel->setText("Object DelayMode", dontSendNotification);
	m_GenericMIDIDelayModeLabel->attachToComponent(m_GenericMIDIDelayModeLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIDelayModeLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIDelayModeLearner.get(), true, false);

	m_GenericMIDIBridgingSettings->resized();

	// YamahaOSC settings section
	m_YamahaOSCBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_YamahaOSCBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_YamahaOSC) + " Bridging");
	m_YamahaOSCBridgingSettings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/YamahaOSC.md"));
	m_YamahaOSCBridgingSettings->setHasActiveToggle(true);
	m_YamahaOSCBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_YamahaOSCBridgingSettings.get());

	m_YamahaOSCIpAddressEdit = std::make_unique<TextEditor>();
	m_YamahaOSCIpAddressEdit->addListener(this);
	m_YamahaOSCIpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_YamahaOSCIpAddressLabel = std::make_unique<Label>();
	m_YamahaOSCIpAddressLabel->setJustificationType(Justification::centred);
	m_YamahaOSCIpAddressLabel->setText("IP Address", dontSendNotification);
	m_YamahaOSCIpAddressLabel->attachToComponent(m_YamahaOSCIpAddressEdit.get(), true);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCIpAddressLabel.get(), false, false);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCIpAddressEdit.get(), true, false);

	m_YamahaOSCListeningPortEdit = std::make_unique<TextEditor>();
	m_YamahaOSCListeningPortEdit->addListener(this);
	m_YamahaOSCListeningPortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_YamahaOSCListeningPortLabel = std::make_unique<Label>();
	m_YamahaOSCListeningPortLabel->setJustificationType(Justification::centred);
	m_YamahaOSCListeningPortLabel->setText("Listening Port", dontSendNotification);
	m_YamahaOSCListeningPortLabel->attachToComponent(m_YamahaOSCListeningPortEdit.get(), true);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCListeningPortLabel.get(), false, false);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCListeningPortEdit.get(), true, false);

	m_YamahaOSCRemotePortEdit = std::make_unique<TextEditor>();
	m_YamahaOSCRemotePortEdit->addListener(this);
	m_YamahaOSCRemotePortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_YamahaOSCRemotePortLabel = std::make_unique<Label>();
	m_YamahaOSCRemotePortLabel->setJustificationType(Justification::centred);
	m_YamahaOSCRemotePortLabel->setText("Remote Port", dontSendNotification);
	m_YamahaOSCRemotePortLabel->attachToComponent(m_YamahaOSCRemotePortEdit.get(), true);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCRemotePortLabel.get(), false, false);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCRemotePortEdit.get(), true, false);

	m_YamahaOSCMappingAreaSelect = std::make_unique<ComboBox>();
	m_YamahaOSCMappingAreaSelect->addListener(this);
	m_YamahaOSCMappingAreaSelect->addItemList({ "1", "2", "3", "4" }, MAI_First);
	m_YamahaOSCMappingAreaLabel = std::make_unique<Label>();
	m_YamahaOSCMappingAreaLabel->setJustificationType(Justification::centred);
	m_YamahaOSCMappingAreaLabel->setText("Mapping Area", dontSendNotification);
	m_YamahaOSCMappingAreaLabel->attachToComponent(m_YamahaOSCMappingAreaSelect.get(), true);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCMappingAreaLabel.get(), false, false);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCMappingAreaSelect.get(), true, false);

	m_YamahaOSCBridgingSettings->resized();
}

/**
 * Class destructor.
 */
SettingsSectionsComponent::~SettingsSectionsComponent()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void SettingsSectionsComponent::paint(Graphics& g)
{
	// Paint background to cover the controls behind this overlay.
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(Rectangle<int>(0, 0, getLocalBounds().getWidth(), getLocalBounds().getHeight()));
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void SettingsSectionsComponent::resized()
{
	auto margin = 3.0f;

	auto minWidth = HeaderWithElmListComponent::m_attachedItemWidth + HeaderWithElmListComponent::m_layoutItemWidth;
	auto minHeight = (m_DS100Settings->getHeight() + (2 * margin))
		+ (m_DiGiCoBridgingSettings->getHeight() + (2 * margin))
		+ (m_RTTrPMBridgingSettings->getHeight() + (2 * margin))
		+ (m_GenericOSCBridgingSettings->getHeight() + (2 * margin))
		+ (m_GenericMIDIBridgingSettings->getHeight() + (2 * margin))
		+ (m_YamahaOSCBridgingSettings->getHeight() + (2 * margin));

	auto bounds = getLocalBounds();
	if (bounds.getWidth() < minWidth || bounds.getHeight() < minHeight)
	{
		if (bounds.getWidth() < minWidth)
			bounds.setWidth(minWidth);
		if (bounds.getHeight() < static_cast<int>(minHeight))
			bounds.setHeight(static_cast<int>(minHeight));

		setBounds(bounds);
	}

	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.justifyContent = FlexBox::JustifyContent::flexStart;
	fb.items.addArray({
		FlexItem(*m_DS100Settings.get())
			.withHeight(static_cast<float>(m_DS100Settings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_DiGiCoBridgingSettings.get())
			.withHeight(static_cast<float>(m_DiGiCoBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_RTTrPMBridgingSettings.get())
			.withHeight(static_cast<float>(m_RTTrPMBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_GenericOSCBridgingSettings.get())
			.withHeight(static_cast<float>(m_GenericOSCBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_GenericMIDIBridgingSettings.get())
			.withHeight(static_cast<float>(m_GenericMIDIBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_YamahaOSCBridgingSettings.get())
			.withHeight(static_cast<float>(m_YamahaOSCBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)) });
	fb.performLayout(bounds);
}

/**
 * Reimplemented from SplitButtonComponent Listener.
 * @param buttonId	The uid of a button element of the splitbutton component
 */
void SettingsSectionsComponent::buttonClicked(JUCEAppBasics::SplitButtonComponent* button, uint64 buttonId)
{
	Controller* ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// DS100 settings section
	if (m_SecondDS100ModeButton && m_SecondDS100ModeButton.get() == button)
	{
		if (m_SecondDS100ModeButtonIds[m_SecondDS100Modes[0]] == buttonId) // Off
		{
			ctrl->SetExtensionMode(DCS_Settings, EM_Off);
		}
		else if (m_SecondDS100ModeButtonIds[m_SecondDS100Modes[1]] == buttonId) // Extend
		{
			ctrl->SetExtensionMode(DCS_Settings, EM_Extend);
		}
		else if (m_SecondDS100ModeButtonIds[m_SecondDS100Modes[2]] == buttonId) // Parallel
		{
			ctrl->SetExtensionMode(DCS_Settings, EM_Parallel);
		}
		else if (m_SecondDS100ModeButtonIds[m_SecondDS100Modes[3]] == buttonId) // Mirror
		{
			ctrl->SetExtensionMode(DCS_Settings, EM_Mirror);
		}
	}

	// RTTrPM settings section
	else if (m_RTTrPMInterpretXYRelativeButton && m_RTTrPMInterpretXYRelativeButton.get() == button)
	{
		// If button is set to absolute, set the mapping area id to -1 (meaning that the RTTrPM data will be handled as absolute, not relative to a mapping area)
		if (m_RTTrPMInterpretXYRelativeButtonIds[m_RTTrPMInterpretXYRelativeModes[0]] == buttonId) // Absolute
		{
			m_previousRTTrPMMappingAreaId = ctrl->GetBridgingMappingArea(PBT_BlacktraxRTTrPM);
			ctrl->SetBridgingMappingArea(PBT_BlacktraxRTTrPM, -1);
		}
		else if(m_RTTrPMInterpretXYRelativeButtonIds[m_RTTrPMInterpretXYRelativeModes[1]] == buttonId) // Relative
		{
			ctrl->SetBridgingMappingArea(PBT_BlacktraxRTTrPM, m_previousRTTrPMMappingAreaId);
		}
	}

	// return without config update trigger if the button was unknown
	else
		return;

	processUpdatedConfig();
}

/**
 * Reimplemented from TextEditor Listener.
 * This just forwards it to private method that handles relevant changes in editor contents in general.
 * @param editor	The editor component that changes were made in
 */
void SettingsSectionsComponent::textEditorReturnKeyPressed(TextEditor& editor)
{
	textEditorUpdated(editor);
}

/**
 * Reimplemented from TextEditor Listener.
 * This just forwards it to private method that handles relevant changes in editor contents in general.
 * @param editor	The editor component that changes were made in
 */
void SettingsSectionsComponent::textEditorFocusLost(TextEditor& editor)
{
	textEditorUpdated(editor);
}

/**
 * Method to handle relevant changes in text editors by processing them and inserting into config through controller interface
 * @param editor	The editor component that changes were made in
 */
void SettingsSectionsComponent::textEditorUpdated(TextEditor& editor)
{
	Controller* ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// DS100 settings section
	if (m_DS100IntervalEdit && m_DS100IntervalEdit.get() == &editor)
		ctrl->SetRate(DCS_Settings, m_DS100IntervalEdit->getText().getIntValue());
	else if (m_DS100IpAddressEdit && m_DS100IpAddressEdit.get() == &editor)
		ctrl->SetDS100IpAddress(DCS_Settings, m_DS100IpAddressEdit->getText());
	else if (m_SecondDS100IpAddressEdit && m_SecondDS100IpAddressEdit.get() == &editor)
		ctrl->SetSecondDS100IpAddress(DCS_Settings, m_SecondDS100IpAddressEdit->getText());

	// DiGiCo settings section
	else if (m_DiGiCoIpAddressEdit && m_DiGiCoIpAddressEdit.get() == &editor)
		ctrl->SetBridgingIpAddress(PBT_DiGiCo, m_DiGiCoIpAddressEdit->getText());
	else if (m_DiGiCoListeningPortEdit && m_DiGiCoListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_DiGiCo, m_DiGiCoListeningPortEdit->getText().getIntValue());
	else if (m_DiGiCoRemotePortEdit && m_DiGiCoRemotePortEdit.get() == &editor)
		ctrl->SetBridgingRemotePort(PBT_DiGiCo, m_DiGiCoRemotePortEdit->getText().getIntValue());

	// RTTrPM settings section
	else if (m_RTTrPMListeningPortEdit && m_RTTrPMListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_BlacktraxRTTrPM, m_RTTrPMListeningPortEdit->getText().getIntValue());

	// Generic OSC settings section
	else if (m_GenericOSCIpAddressEdit && m_GenericOSCIpAddressEdit.get() == &editor)
		ctrl->SetBridgingIpAddress(PBT_GenericOSC, m_GenericOSCIpAddressEdit->getText());
	else if (m_GenericOSCListeningPortEdit && m_GenericOSCListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_GenericOSC, m_GenericOSCListeningPortEdit->getText().getIntValue());
	else if (m_GenericOSCRemotePortEdit && m_GenericOSCRemotePortEdit.get() == &editor)
		ctrl->SetBridgingRemotePort(PBT_GenericOSC, m_GenericOSCRemotePortEdit->getText().getIntValue());

	// Yamaha OSC settings section
	else if (m_YamahaOSCIpAddressEdit && m_YamahaOSCIpAddressEdit.get() == &editor)
		ctrl->SetBridgingIpAddress(PBT_YamahaOSC, m_YamahaOSCIpAddressEdit->getText());
	else if (m_YamahaOSCListeningPortEdit && m_YamahaOSCListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_YamahaOSC, m_YamahaOSCListeningPortEdit->getText().getIntValue());
	else if (m_YamahaOSCRemotePortEdit && m_YamahaOSCRemotePortEdit.get() == &editor)
		ctrl->SetBridgingRemotePort(PBT_YamahaOSC, m_YamahaOSCRemotePortEdit->getText().getIntValue());
	else if (m_YamahaOSCListeningPortEdit && m_YamahaOSCListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_YamahaOSC, m_YamahaOSCListeningPortEdit->getText().getIntValue());

	// return without config update trigger if the editor was unknown
	else
		return;

	processUpdatedConfig();
}

/**
 * Reimplemented method to handle combobox changes
 * @param comboBox	The comboBox component that changes were made in
 */
void SettingsSectionsComponent::comboBoxChanged(ComboBox* comboBox)
{
	Controller* ctrl = Controller::GetInstance();
	if (!ctrl)
		return;
    
    // if the combobox that was changed is disabled, don't handle its change
    if (comboBox && !comboBox->isEnabled())
        return;

	// RTTrPM settings section
	if (m_RTTrPMMappingAreaSelect && m_RTTrPMMappingAreaSelect.get() == comboBox)
	{
		m_previousRTTrPMMappingAreaId = m_RTTrPMMappingAreaSelect->getSelectedId();
		ctrl->SetBridgingMappingArea(PBT_BlacktraxRTTrPM, m_previousRTTrPMMappingAreaId);
	}

	// Generic MIDI settings section
	else if (m_GenericMIDIInputDeviceSelect && m_GenericMIDIInputDeviceSelect.get() == comboBox)
	{
		if (m_midiInputDeviceIdentifiers.count(m_GenericMIDIInputDeviceSelect->getSelectedId()) > 0)
			ctrl->SetBridgingInputDeviceIdentifier(PBT_GenericMIDI, m_midiInputDeviceIdentifiers.at(m_GenericMIDIInputDeviceSelect->getSelectedId()));
		else
			ctrl->SetBridgingInputDeviceIdentifier(PBT_GenericMIDI, String());
	}
	else if (m_GenericMIDIOutputDeviceSelect && m_GenericMIDIOutputDeviceSelect.get() == comboBox)
	{
		if (m_midiOutputDeviceIdentifiers.count(m_GenericMIDIOutputDeviceSelect->getSelectedId()) > 0)
			ctrl->SetBridgingOutputDeviceIdentifier(PBT_GenericMIDI, m_midiOutputDeviceIdentifiers.at(m_GenericMIDIOutputDeviceSelect->getSelectedId()));
		else
			ctrl->SetBridgingOutputDeviceIdentifier(PBT_GenericMIDI, String());
	}
	else if (m_GenericMIDIMappingAreaSelect && m_GenericMIDIMappingAreaSelect.get() == comboBox)
		ctrl->SetBridgingMappingArea(PBT_GenericMIDI, m_GenericMIDIMappingAreaSelect->getSelectedId());

	// Yamaha OSC settings section
	else if (m_YamahaOSCMappingAreaSelect && m_YamahaOSCMappingAreaSelect.get() == comboBox)
		ctrl->SetBridgingMappingArea(PBT_YamahaOSC, m_YamahaOSCMappingAreaSelect->getSelectedId());

	// return without config update trigger if the comboBox was unknown
	else
		return;

	processUpdatedConfig();
}

/**
 * Proxy method to activate a single bridging protocol in controller.
 * @param sectionType	The protocolType to be active from now on.
 */
void SettingsSectionsComponent::setSettingsSectionActiveState(HeaderWithElmListComponent* settingsSection, bool activeState)
{
	Controller* ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	ProtocolBridgingType sectionType = PBT_None;
	if (settingsSection == m_DiGiCoBridgingSettings.get())
		sectionType = PBT_DiGiCo;
	else if (settingsSection == m_RTTrPMBridgingSettings.get())
		sectionType = PBT_BlacktraxRTTrPM;
	else if (settingsSection == m_GenericOSCBridgingSettings.get())
		sectionType = PBT_GenericOSC;
	else if (settingsSection == m_GenericMIDIBridgingSettings.get())
		sectionType = PBT_GenericMIDI;
	else if (settingsSection == m_YamahaOSCBridgingSettings.get())
		sectionType = PBT_YamahaOSC;

	if (activeState)
		ctrl->SetActiveProtocolBridging(ctrl->GetActiveProtocolBridging() | sectionType);
	else
		ctrl->SetActiveProtocolBridging(ctrl->GetActiveProtocolBridging() & ~sectionType);
}

/**
 * Private helper method to update midi input device selection dropdown contents.
 */
void SettingsSectionsComponent::updateAvailableMidiInputDevices()
{
	if (!m_GenericMIDIInputDeviceSelect)
		return;

	m_midiInputDeviceIdentifiers.clear();

	// collect available devices to populate our dropdown
	auto startItemIndex = 1;
	auto itemIndex = startItemIndex;
	auto midiInputs = juce::MidiInput::getAvailableDevices();
	juce::StringArray midiInputNames;
	midiInputNames.add("None");
	m_midiInputDeviceIdentifiers[itemIndex++] = String();
	for (auto input : midiInputs)
	{
		midiInputNames.add(input.name);
		m_midiInputDeviceIdentifiers[itemIndex++] = input.identifier;
	}

	m_GenericMIDIInputDeviceSelect->addItemList(midiInputNames, startItemIndex);
}

/**
 * Private helper method to update midi output device selection dropdown contents.
 */
void SettingsSectionsComponent::updateAvailableMidiOutputDevices()
{
	if (!m_GenericMIDIOutputDeviceSelect)
		return;

	m_midiOutputDeviceIdentifiers.clear();

	// collect available devices to populate our dropdown
	auto startItemIndex = 1;
	auto itemIndex = startItemIndex;
	auto midiOutputs = juce::MidiOutput::getAvailableDevices();
	juce::StringArray midiOutputNames;
	midiOutputNames.add("None");
	m_midiOutputDeviceIdentifiers[itemIndex++] = String();
	for (auto output : midiOutputs)
	{
		midiOutputNames.add(output.name);
		m_midiOutputDeviceIdentifiers[itemIndex++] = output.identifier;
	}

	m_GenericMIDIOutputDeviceSelect->addItemList(midiOutputNames, startItemIndex);
}

/**
 * Method to update the elements on UI when app configuration changed.
 * This is called by parent container component when it receives
 * onConfigUpdated call (it's a config listener and subscribed to changes)
 */
void SettingsSectionsComponent::processUpdatedConfig()
{
	Controller* ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// DS100 settings section
	if (m_DS100IntervalEdit)
		m_DS100IntervalEdit->setText(String(ctrl->GetRate()) + UNIT_MILLISECOND);
	if (m_DS100IpAddressEdit)
		m_DS100IpAddressEdit->setText(ctrl->GetDS100IpAddress());
	if (m_SecondDS100ModeButton)
	{
		auto newActiveButtonId = m_SecondDS100ModeButtonIds[m_SecondDS100Modes[0]];
		if (ctrl->GetExtensionMode() == EM_Extend)
			newActiveButtonId = m_SecondDS100ModeButtonIds[m_SecondDS100Modes[1]];
		else if (ctrl->GetExtensionMode() == EM_Parallel)
			newActiveButtonId = m_SecondDS100ModeButtonIds[m_SecondDS100Modes[2]];
		else if (ctrl->GetExtensionMode() == EM_Mirror)
			newActiveButtonId = m_SecondDS100ModeButtonIds[m_SecondDS100Modes[3]];
		m_SecondDS100ModeButton->setButtonDown(newActiveButtonId);
	}
	if (m_SecondDS100IpAddressEdit)
	{
		m_SecondDS100IpAddressEdit->setText(ctrl->GetSecondDS100IpAddress());
		m_SecondDS100IpAddressEdit->setEnabled(ctrl->GetExtensionMode() != EM_Off);
	}
	if (m_SecondDS100IpAddressLabel)
		m_SecondDS100IpAddressLabel->setEnabled(ctrl->GetExtensionMode() != EM_Off);
	if (m_SecondDS100ZeroconfDiscovery)
		m_SecondDS100ZeroconfDiscovery->setEnabled(ctrl->GetExtensionMode() != EM_Off);

	// DiGiCo settings section
	auto DiGiCoBridgingActive = (ctrl->GetActiveProtocolBridging() & PBT_DiGiCo) == PBT_DiGiCo;
	if (m_DiGiCoBridgingSettings)
		m_DiGiCoBridgingSettings->setToggleActiveState(DiGiCoBridgingActive);
	if (m_DiGiCoIpAddressEdit)
		m_DiGiCoIpAddressEdit->setText(ctrl->GetBridgingIpAddress(PBT_DiGiCo));
	if (m_DiGiCoListeningPortEdit)
		m_DiGiCoListeningPortEdit->setText(String(ctrl->GetBridgingListeningPort(PBT_DiGiCo)), false);
	if (m_DiGiCoRemotePortEdit)
		m_DiGiCoRemotePortEdit->setText(String(ctrl->GetBridgingRemotePort(PBT_DiGiCo)), false);

	// RTTrPM settings section
	auto RTTrPMBridgingActive = (ctrl->GetActiveProtocolBridging() & PBT_BlacktraxRTTrPM) == PBT_BlacktraxRTTrPM;
	if (m_RTTrPMBridgingSettings)
		m_RTTrPMBridgingSettings->setToggleActiveState(RTTrPMBridgingActive);
	if (m_RTTrPMListeningPortEdit)
		m_RTTrPMListeningPortEdit->setText(String(ctrl->GetBridgingListeningPort(PBT_BlacktraxRTTrPM)), false);
    auto RTTrPMMappingAreaId = ctrl->GetBridgingMappingArea(PBT_BlacktraxRTTrPM);
	if (m_RTTrPMInterpretXYRelativeButton)
	{
		auto newActiveButtonId = m_RTTrPMInterpretXYRelativeButtonIds[m_RTTrPMInterpretXYRelativeModes[(RTTrPMMappingAreaId == -1) ? 0 : 1]];
		m_RTTrPMInterpretXYRelativeButton->setButtonDown(newActiveButtonId);
	}
	if (m_RTTrPMMappingAreaSelect)
	{
		m_RTTrPMMappingAreaSelect->setSelectedId(RTTrPMMappingAreaId, sendNotificationAsync);
		m_RTTrPMMappingAreaSelect->setEnabled((RTTrPMMappingAreaId != MAI_Invalid));
	}
	if (m_RTTrPMMappingAreaLabel)
		m_RTTrPMMappingAreaLabel->setEnabled((RTTrPMMappingAreaId != MAI_Invalid));

	// Generic OSC settings section
	auto GenericOSCBridgingActive = (ctrl->GetActiveProtocolBridging() & PBT_GenericOSC) == PBT_GenericOSC;
	if (m_GenericOSCBridgingSettings)
		m_GenericOSCBridgingSettings->setToggleActiveState(GenericOSCBridgingActive);
	if (m_GenericOSCIpAddressEdit)
		m_GenericOSCIpAddressEdit->setText(ctrl->GetBridgingIpAddress(PBT_GenericOSC));
	if (m_GenericOSCListeningPortEdit)
		m_GenericOSCListeningPortEdit->setText(String(ctrl->GetBridgingListeningPort(PBT_GenericOSC)), false);
	if (m_GenericOSCRemotePortEdit)
		m_GenericOSCRemotePortEdit->setText(String(ctrl->GetBridgingRemotePort(PBT_GenericOSC)), false);

	// Generic MIDI settings section
	auto GenericMIDIBridgingActive = (ctrl->GetActiveProtocolBridging() & PBT_GenericMIDI) == PBT_GenericMIDI;
	if (m_GenericMIDIBridgingSettings)
		m_GenericMIDIBridgingSettings->setToggleActiveState(GenericMIDIBridgingActive);
	if (m_GenericMIDIInputDeviceSelect)
	{
		auto idToSelect = -1;
		auto identifierToSelect = ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI);
		for (auto const& selectIdDevIdentKV : m_midiInputDeviceIdentifiers)
		{
			if (selectIdDevIdentKV.second == identifierToSelect)
			{
				idToSelect = selectIdDevIdentKV.first;
				break;
			}
		}
		m_GenericMIDIInputDeviceSelect->setSelectedId(idToSelect);
	}
	if (m_GenericMIDIOutputDeviceSelect)
	{
		auto idToSelect = -1;
		auto identifierToSelect = ctrl->GetBridgingOutputDeviceIdentifier(PBT_GenericMIDI);
		for (auto const& selectIdDevIdentKV : m_midiOutputDeviceIdentifiers)
		{
			if (selectIdDevIdentKV.second == identifierToSelect)
			{
				idToSelect = selectIdDevIdentKV.first;
				break;
			}
		}
		m_GenericMIDIOutputDeviceSelect->setSelectedId(idToSelect);
	}
	if (m_GenericMIDIMappingAreaSelect)
	{
		m_GenericMIDIMappingAreaSelect->setSelectedId(ctrl->GetBridgingMappingArea(PBT_GenericMIDI), sendNotificationAsync);
		m_GenericMIDIMappingAreaSelect->setEnabled((ctrl->GetBridgingMappingArea(PBT_GenericMIDI) != MAI_Invalid));
	}
	if (m_GenericMIDIMappingAreaLabel)
		m_GenericMIDIMappingAreaLabel->setEnabled((ctrl->GetBridgingMappingArea(PBT_GenericMIDI) != MAI_Invalid));
	if (m_GenericMIDIMatrixInputSelectLearner)
	{
		m_GenericMIDIMatrixInputSelectLearner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDIMatrixInputSelectLearner->setCurrentMidiAssi(ctrl->GetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDIMatrixInputSelectLearner->getReferredId())));
	}
    if (m_GenericMIDIXValueLearner)
	{
		m_GenericMIDIXValueLearner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDIXValueLearner->setCurrentMidiAssi(ctrl->GetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDIXValueLearner->getReferredId())));
	}
    if (m_GenericMIDIYValueLearner)
	{
		m_GenericMIDIYValueLearner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDIYValueLearner->setCurrentMidiAssi(ctrl->GetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDIYValueLearner->getReferredId())));
	}
    if (m_GenericMIDIReverbSendGainLearner)
	{
		m_GenericMIDIReverbSendGainLearner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDIReverbSendGainLearner->setCurrentMidiAssi(ctrl->GetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDIReverbSendGainLearner->getReferredId())));
	}
    if (m_GenericMIDISourceSpreadLearner)
	{
		m_GenericMIDISourceSpreadLearner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDISourceSpreadLearner->setCurrentMidiAssi(ctrl->GetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDISourceSpreadLearner->getReferredId())));
	}
    if (m_GenericMIDIDelayModeLearner)
	{
		m_GenericMIDIDelayModeLearner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDIDelayModeLearner->setCurrentMidiAssi(ctrl->GetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDIDelayModeLearner->getReferredId())));
	}
    
	// Yamaha OSC settings section
	auto YamahaOSCBridgingActive = (ctrl->GetActiveProtocolBridging() & PBT_YamahaOSC) == PBT_YamahaOSC;
	if (m_YamahaOSCBridgingSettings)
		m_YamahaOSCBridgingSettings->setToggleActiveState(YamahaOSCBridgingActive);
	if (m_YamahaOSCIpAddressEdit)
		m_YamahaOSCIpAddressEdit->setText(ctrl->GetBridgingIpAddress(PBT_YamahaOSC));
	if (m_YamahaOSCListeningPortEdit)
		m_YamahaOSCListeningPortEdit->setText(String(ctrl->GetBridgingListeningPort(PBT_YamahaOSC)), false);
	if (m_YamahaOSCRemotePortEdit)
		m_YamahaOSCRemotePortEdit->setText(String(ctrl->GetBridgingRemotePort(PBT_YamahaOSC)), false);
	if (m_YamahaOSCMappingAreaSelect)
	{
		m_YamahaOSCMappingAreaSelect->setSelectedId(ctrl->GetBridgingMappingArea(PBT_YamahaOSC), sendNotificationAsync);
		m_YamahaOSCMappingAreaSelect->setEnabled((ctrl->GetBridgingMappingArea(PBT_YamahaOSC) != MAI_Invalid));
	}
	if (m_YamahaOSCMappingAreaLabel)
		m_YamahaOSCMappingAreaLabel->setEnabled((ctrl->GetBridgingMappingArea(PBT_YamahaOSC) != MAI_Invalid));
}

/**
 * Callback method to be registered with ZeroconfDiscoveryComponent to handle user input regarding service selection.
 * @param type	The service type that was selected
 * @param info	The detailed info on the service that was selected
 */
void SettingsSectionsComponent::handleDS100ServiceSelected(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, JUCEAppBasics::ZeroconfDiscoverComponent::ServiceInfo* info)
{
	ignoreUnused(type);

	if (info)
	{
		m_DS100IpAddressEdit->setText(info->ip, true);
        
        Controller* ctrl = Controller::GetInstance();
        if (ctrl)
            ctrl->SetDS100IpAddress(DCS_Settings, info->ip);
	}
}

/**
 * Callback method to be registered with ZeroconfDiscoveryComponent to handle user input regarding service selection.
 * @param type	The service type that was selected
 * @param info	The detailed info on the service that was selected
 */
void SettingsSectionsComponent::handleSecondDS100ServiceSelected(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, JUCEAppBasics::ZeroconfDiscoverComponent::ServiceInfo* info)
{
	ignoreUnused(type);

	if (info)
	{
		m_SecondDS100IpAddressEdit->setText(info->ip, true);

		Controller* ctrl = Controller::GetInstance();
		if (ctrl)
			ctrl->SetSecondDS100IpAddress(DCS_Settings, info->ip);
	}
}

/**
 * Callback method to be registered with MidiLearnerComponent to handle midi assignment selection.
 * @param sender	The MidiLearnerComponent that sent the assignment.
 * @param midiAssi	The sent assignment that was chosen by user
 */
void SettingsSectionsComponent::handleMidiAssiSet(Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi)
{
	auto learnerComponent = dynamic_cast<JUCEAppBasics::MidiLearnerComponent*>(sender);
	if (learnerComponent)
	{
		// No need to set the assignment to learner here, 
		// as is done in other handle methods with other editors,
		// since it is already done by learners internally!

		Controller* ctrl = Controller::GetInstance();
		if (ctrl)
			ctrl->SetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(learnerComponent->getReferredId()), midiAssi);
	}
}


} // namespace SoundscapeBridgeApp