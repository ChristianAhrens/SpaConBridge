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


#pragma once

#include "PageComponentBase.h"

#include "../../SoundscapeBridgeAppCommon.h"
#include "../../AppConfiguration.h"
#include "../../LookAndFeel.h"

#include "../../../submodules/JUCE-AppBasics/Source/ZeroconfDiscoverComponent.h"
#include "../../../submodules/JUCE-AppBasics/Source/SplitButtonComponent.h"
#include "../../../submodules/JUCE-AppBasics/Source/TextWithImageButton.h"
#include "../../../submodules/JUCE-AppBasics/Source/MidiLearnerComponent.h"


namespace SoundscapeBridgeApp
{


/**
 * HeaderWithElmListComponent is a component to hold a header component with multiple other components in a specific layout.
 */
class HeaderWithElmListComponent : public Component
{
public:
	HeaderWithElmListComponent(const String& componentName = String());
	~HeaderWithElmListComponent() override;

	//==============================================================================
	void setHasActiveToggle(bool hasActiveToggle);
	void setHeaderText(String headerText);
	void addComponent(Component* compo, bool includeInLayout = true, bool takeOwnership = true);
	void setToggleActiveState(bool toggleState);
	
	void onToggleActive();

	//==============================================================================
	void setHelpUrl(const URL& helpUrl);

	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

	//==============================================================================
	void lookAndFeelChanged() override;

	//==============================================================================
	std::function<void(HeaderWithElmListComponent*, bool)>	toggleIsActiveCallback;

	//==============================================================================
	static constexpr std::int32_t	m_attachedItemWidth{ 150 };
	static constexpr std::int32_t	m_layoutItemWidth{ 205 };

protected:
	//==============================================================================
	void setElementsActiveState(bool toggleState);

private:
	//==============================================================================
	bool																		m_hasActiveToggle{ false };
	bool																		m_toggleState{ true };
	std::unique_ptr<ToggleButton>												m_activeToggle;
	std::unique_ptr<Label>														m_activeToggleLabel;
	std::unique_ptr<Label>														m_headerLabel;
	std::unique_ptr<DrawableButton>												m_helpButton;
	std::unique_ptr<URL>														m_helpUrl;
	std::vector<std::pair<std::unique_ptr<Component>, std::pair<bool, bool>>>	m_components;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderWithElmListComponent)
};

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

/**
 * SettingsPageComponent is a component to hold multiple components that are dedicated to app configuration.
 */
class SettingsPageComponent :	public PageComponentBase, 
								public AppConfiguration::Watcher // for raw text editing
{
public:
	SettingsPageComponent();
	~SettingsPageComponent() override;

	//==========================================================================
	void onApplyClicked();
	void onLoadConfigClicked();
	void onSaveConfigClicked();
	void onToggleRawConfigVisible();
	void onSelectedLookAndFeelChanged();

	//==========================================================================
	void SetSelectedLookAndFeelType(DbLookAndFeelBase::LookAndFeelType lookAndFeelType);
	DbLookAndFeelBase::LookAndFeelType GetSelectedLookAndFeelType();

	//==========================================================================
	void lookAndFeelChanged() override;

	//==========================================================================
	void UpdateGui(bool init) override;

	//==========================================================================
	void onConfigUpdated() override;

	//==========================================================================
	void paint(Graphics&) override;
	void resized() override;

private:
	//==============================================================================
	std::unique_ptr<SettingsSectionsComponent>	m_settingsComponent;
	std::unique_ptr<Viewport>					m_settingsViewport;

	std::unique_ptr<TextButton>		m_settingsRawApplyButton;
	std::unique_ptr<TextEditor>		m_settingsRawEditor;
	std::unique_ptr<ComboBox>		m_lookAndFeelSelect;
	std::unique_ptr<Label>			m_lookAndFeelLabel;

	std::unique_ptr<JUCEAppBasics::TextWithImageButton>	m_loadConfigButton;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>	m_saveConfigButton;
	std::unique_ptr<TextButton>							m_useRawConfigButton;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPageComponent)
};


} // namespace SoundscapeBridgeApp
