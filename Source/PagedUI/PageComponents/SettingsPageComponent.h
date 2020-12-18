/*
  ==============================================================================

	SettingsPageComponent.h
	Created: 28 July 2020 17:48:55pm
	Author:  Christian Ahrens

  ==============================================================================
*/


#pragma once

#include "PageComponentBase.h"

#include "../../SoundscapeBridgeAppCommon.h"
#include "../../AppConfiguration.h"
#include "../../LookAndFeel.h"

#include "../../../submodules/JUCE-AppBasics/Source/ZeroconfDiscoverComponent.h"
#include "../../../submodules/JUCE-AppBasics/Source/SplitButtonComponent.h"
#include "../../../submodules/JUCE-AppBasics/Source/TextWithImageButton.h"


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

	//==========================================================================
	void paint(Graphics&) override;
	void resized() override;

	//==========================================================================
	std::function<void(HeaderWithElmListComponent*, bool)>	toggleIsActiveCallback;

protected:
	//==========================================================================
	void setElementsActiveState(bool toggleState);

private:
	//==========================================================================
	bool																		m_hasActiveToggle{ false };
	bool																		m_toggleState{ true };
	std::unique_ptr<ToggleButton>												m_activeToggle;
	std::unique_ptr<Label>														m_activeToggleLabel;
	std::unique_ptr<Label>														m_headerLabel;
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

	//==========================================================================
	void textEditorUpdated(TextEditor&);

	//==============================================================================
	void handleDS100ServiceSelected(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, JUCEAppBasics::ZeroconfDiscoverComponent::ServiceInfo* info);
	void handleSecondDS100ServiceSelected(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, JUCEAppBasics::ZeroconfDiscoverComponent::ServiceInfo* info);

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
	const std::vector<std::string>								m_SecondDS100Modes{ "Off", "Extend", "Mirror" };
	std::map<std::string, uint64>								m_SecondDS100ModeButtonIds;

	std::unique_ptr<TextEditor>									m_SecondDS100IpAddressEdit;
	std::unique_ptr<Label>										m_SecondDS100IpAddressLabel;
	std::unique_ptr<JUCEAppBasics::ZeroconfDiscoverComponent>	m_SecondDS100ZeroconfDiscovery;

	// DiGiCo settings section
	std::unique_ptr<HeaderWithElmListComponent>	m_DiGiCoBridgingSettings;
	std::unique_ptr<TextEditor>					m_DiGiCoIpAddressEdit;
	std::unique_ptr<Label>						m_DiGiCoIpAddressLabel;
	std::unique_ptr<TextEditor>					m_DiGiCoListeningPortEdit;
	std::unique_ptr<Label>						m_DiGiCoListeningPortLabel;
	std::unique_ptr<TextEditor>					m_DiGiCoRemotePortEdit;
	std::unique_ptr<Label>						m_DiGiCoRemotePortLabel;

	// RTTrPM settings section
	std::unique_ptr<HeaderWithElmListComponent>				m_RTTrPMBridgingSettings;
	std::unique_ptr<TextEditor>								m_RTTrPMListeningPortEdit;
	std::unique_ptr<Label>									m_RTTrPMListeningPortLabel;
	std::unique_ptr<JUCEAppBasics::SplitButtonComponent>	m_RTTrPMInterpretXYRelativeButton;
	std::unique_ptr<Label>									m_RTTrPMInterpretXYRelativeLabel;
	const std::vector<std::string>							m_RTTrPMInterpretXYRelativeModes{ "Absolute", "Relative" };
	std::map<std::string, uint64>							m_RTTrPMInterpretXYRelativeButtonIds;
	std::unique_ptr<TextEditor>								m_RTTrPMMappingAreaEdit;
	std::unique_ptr<Label>									m_RTTrPMMappingAreaLabel;
	int														m_previousRTTrPMMappingAreaId{ 1 };

	// Generic OSC settings section
	std::unique_ptr<HeaderWithElmListComponent>	m_GenericOSCBridgingSettings;
	std::unique_ptr<TextEditor>					m_GenericOSCIpAddressEdit;
	std::unique_ptr<Label>						m_GenericOSCIpAddressLabel;
	std::unique_ptr<TextEditor>					m_GenericOSCListeningPortEdit;
	std::unique_ptr<Label>						m_GenericOSCListeningPortLabel;
	std::unique_ptr<TextEditor>					m_GenericOSCRemotePortEdit;
	std::unique_ptr<Label>						m_GenericOSCRemotePortLabel;

	// Generic MIDI settings section
	std::unique_ptr<HeaderWithElmListComponent>	m_GenericMIDIBridgingSettings;
	std::unique_ptr<ComboBox>					m_GenericMIDIInputDeviceSelect;
	std::unique_ptr<Label>						m_GenericMIDIInputDeviceSelectLabel;
	std::unique_ptr<Label>						m_GenericMIDIHardcodedWarningLabel;
	std::unique_ptr<Label>						m_GenericMIDIHardcodedMatrixInputSelectLabel;
	std::unique_ptr<Label>						m_GenericMIDIHardcodedXValueLabel;
	std::unique_ptr<Label>						m_GenericMIDIHardcodedYValueLabel;
	std::unique_ptr<Label>						m_GenericMIDIHardcodedReverbSendGainLabel;
	std::unique_ptr<Label>						m_GenericMIDIHardcodedSourceSpreadLabel;
	std::unique_ptr<Label>						m_GenericMIDIHardcodedDelayModeLabel;

	// Yamaha OSC settings section
	std::unique_ptr<HeaderWithElmListComponent>	m_YamahaOSCBridgingSettings;
	std::unique_ptr<TextEditor>					m_YamahaOSCIpAddressEdit;
	std::unique_ptr<Label>						m_YamahaOSCIpAddressLabel;
	std::unique_ptr<TextEditor>					m_YamahaOSCListeningPortEdit;
	std::unique_ptr<Label>						m_YamahaOSCListeningPortLabel;
	std::unique_ptr<TextEditor>					m_YamahaOSCRemotePortEdit;
	std::unique_ptr<Label>						m_YamahaOSCRemotePortLabel;
	std::unique_ptr<TextEditor>					m_YamahaOSCMappingAreaEdit;
	std::unique_ptr<Label>						m_YamahaOSCMappingAreaLabel;
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
