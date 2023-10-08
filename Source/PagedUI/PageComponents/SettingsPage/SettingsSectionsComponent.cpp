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


#include "SettingsSectionsComponent.h"

#include "../HeaderWithElmListComponent.h"
#include "../../PageComponentManager.h"
#include "../../../Controller.h"
#include "../../../LookAndFeel.h"
#include "../../../IPAddressDisplay.h"

#include "SceneIndexToMidiAssignerComponent.h"
#include "RemoteObjectToOscAssignerComponent.h"
#include "IndexToChannelAssignerComponent.h"
#include "ProjectDummyDataLoaderComponent.h"

#include <Image_utils.h>
#include <MidiLearnerComponent.h>
#include <TextWithImageButton.h>
#include <FixedFontTextEditor.h>


namespace SpaConBridge
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
	m_ipAddressEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(21, "1234567890.:"); // 21 digits: "255.255.255.255:65535"
	m_portEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(5, "1234567890"); // 5 digits: "65535"
	m_mappingEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(1, "1234"); // 1 digit: "4"

	createGeneralSettingsSection();
	createDS100SettingsSection();
	createDiGiCoSettingsSection();
	createDAWPluginSettingsSection();
	createRTTrPMSettingsSection();
	createGenericOSCSettingsSection();
	createGenericMIDISettingsSection();
	createADMOSCSettingsSection();
	createYamahaOSCSettingsSection();
	createRemapOSCSettingsSection();
}

/**
 * Helper method to create and setup objects for general settings section
 */
void SettingsSectionsComponent::createGeneralSettingsSection()
{
	// General settings section
	m_GeneralSettings = std::make_unique<HeaderWithElmListComponent>();
	m_GeneralSettings->setHeaderText("General Settings");
	m_GeneralSettings->setHasActiveToggle(false);
	addAndMakeVisible(m_GeneralSettings.get());

	m_PageEnableButtonContainer = std::make_unique<HorizontalLayouterComponent>();
	m_PageEnableButtonContainer->SetSpacing(5);
	m_SoundObjectPageButton = std::make_unique<DrawableButton>("SoundObjectPage", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_SoundObjectPageButton->setClickingTogglesState(true);
	m_SoundObjectPageButton->setTooltip("Enable " + GetPageNameFromId(UPI_Soundobjects) + " Page");
	m_SoundObjectPageButton->addListener(this);
	m_PageEnableButtonContainer->AddComponent(m_SoundObjectPageButton.get());
	m_MultisurfacePageButton = std::make_unique<DrawableButton>("MultisurfacePage", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_MultisurfacePageButton->setClickingTogglesState(true);
	m_MultisurfacePageButton->setTooltip("Enable " + GetPageNameFromId(UPI_MultiSoundobjects) + " Page");
	m_MultisurfacePageButton->addListener(this);
	m_PageEnableButtonContainer->AddComponent(m_MultisurfacePageButton.get());
	m_MatrixIOPageButton = std::make_unique<DrawableButton>("MatrixIOPage", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_MatrixIOPageButton->setClickingTogglesState(true);
	m_MatrixIOPageButton->setTooltip("Enable " + GetPageNameFromId(UPI_MatrixIOs) + " Page");
	m_MatrixIOPageButton->addListener(this);
	m_PageEnableButtonContainer->AddComponent(m_MatrixIOPageButton.get());
	m_ScenesPageButton = std::make_unique<DrawableButton>("ScenesPage", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_ScenesPageButton->setClickingTogglesState(true);
	m_ScenesPageButton->setTooltip("Enable " + GetPageNameFromId(UPI_Scenes) + " Page");
	m_ScenesPageButton->addListener(this);
	m_PageEnableButtonContainer->AddComponent(m_ScenesPageButton.get());
	m_EnSpacePageButton = std::make_unique<DrawableButton>("EnSpacePage", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_EnSpacePageButton->setClickingTogglesState(true);
	m_EnSpacePageButton->setTooltip("Enable " + GetPageNameFromId(UPI_EnSpace) + " Page");
	m_EnSpacePageButton->addListener(this);
	m_PageEnableButtonContainer->AddComponent(m_EnSpacePageButton.get());
	m_StatisticsPageButton = std::make_unique<DrawableButton>("StatisticsPage", DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_StatisticsPageButton->setClickingTogglesState(true);
	m_StatisticsPageButton->setTooltip("Enable " + GetPageNameFromId(UPI_Statistics) + " Page");
	m_StatisticsPageButton->addListener(this);
	m_PageEnableButtonContainer->AddComponent(m_StatisticsPageButton.get());
	m_EnabledPagesLabel = std::make_unique<Label>("PageEnableButton", "Enabled Pages");
	m_EnabledPagesLabel->setJustificationType(Justification::centred);
	m_EnabledPagesLabel->attachToComponent(m_PageEnableButtonContainer.get(), true);
	m_GeneralSettings->addComponent(m_EnabledPagesLabel.get(), false, false);
	m_GeneralSettings->addComponent(m_PageEnableButtonContainer.get(), true, false);

	m_StaticObjectsPollingButton = std::make_unique<JUCEAppBasics::TextWithImageButton>("Show Soundobject names");
	m_StaticObjectsPollingButton->setClickingTogglesState(true);
	m_StaticObjectsPollingButton->setTooltip("Show object names in " + GetPageNameFromId(UPI_Soundobjects) + " and " + GetPageNameFromId(UPI_MultiSoundobjects) + " Page.");
	m_StaticObjectsPollingButton->setImagePosition(Justification::centredLeft);
	m_StaticObjectsPollingButton->addListener(this);
	m_GeneralSettings->addComponent(m_StaticObjectsPollingButton.get(), true, false);

	m_LookAndFeelSelect = std::make_unique<ComboBox>();
	m_LookAndFeelSelect->addItem(DbLookAndFeelBase::getLookAndFeelName(DbLookAndFeelBase::LAFT_Dark), DbLookAndFeelBase::LAFT_Dark);
	m_LookAndFeelSelect->addItem(DbLookAndFeelBase::getLookAndFeelName(DbLookAndFeelBase::LAFT_Light), DbLookAndFeelBase::LAFT_Light);
	m_LookAndFeelSelect->addListener(this);
	m_LookAndFeelLabel = std::make_unique<Label>("LookAndFeelSelect", "Look and feel");
	m_LookAndFeelLabel->setJustificationType(Justification::centred);
	m_LookAndFeelLabel->attachToComponent(m_LookAndFeelSelect.get(), true);
	m_GeneralSettings->addComponent(m_LookAndFeelLabel.get(), false, false);
	m_GeneralSettings->addComponent(m_LookAndFeelSelect.get(), true, false);

#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
	m_ToggleFullscreenButton = std::make_unique<JUCEAppBasics::TextWithImageButton>("Fullscreen window mode");
	m_ToggleFullscreenButton->setTooltip("Toggle fullscreen window mode.");
	m_ToggleFullscreenButton->setImagePosition(Justification::centredLeft);
	m_ToggleFullscreenButton->setClickingTogglesState(true);
	m_ToggleFullscreenButton->addListener(this);
	m_GeneralSettings->addComponent(m_ToggleFullscreenButton.get(), true, false);
#endif

	m_SystemIpInfoEdit = std::make_unique<IPAddressDisplay>();
	m_SystemIpInfoLabel = std::make_unique<Label>("SystemIpInfoLabel", JUCEApplication::getInstance()->getApplicationName() + " IP");
	m_SystemIpInfoLabel->setJustificationType(Justification::centred);
	m_SystemIpInfoLabel->attachToComponent(m_SystemIpInfoEdit.get(), true);
	m_GeneralSettings->addComponent(m_SystemIpInfoLabel.get(), false, false);
	m_GeneralSettings->addComponent(m_SystemIpInfoEdit.get(), true, false);
    m_SystemIpInfoEdit->setEnabled(false);

	m_GeneralSettings->resized();

	// trigger lookAndFeelChanged once to initially setup drawablebuttons
	lookAndFeelChanged();
}

/**
 * Helper method to create and setup objects for DS100 settings section
 */
void SettingsSectionsComponent::createDS100SettingsSection()
{
	// DS100 settings section
	m_DS100Settings = std::make_unique<HeaderWithElmListComponent>();
	m_DS100Settings->setHeaderText("DS100 Settings");
	m_DS100Settings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/DS100.md"));
	m_DS100Settings->setHasActiveToggle(false);
	addAndMakeVisible(m_DS100Settings.get());

	m_DS100ProtocolSelectButton = std::make_unique<JUCEAppBasics::SplitButtonComponent>();
	m_DS100ProtocolSelectButton->addListener(this);
	auto& t = m_DS100ProtocolSelectTexts;
	m_DS100ProtocolSelectButtonIds[t[0].first] = m_DS100ProtocolSelectButton->addButton(t[0].first, t[0].second);
	m_DS100ProtocolSelectButtonIds[t[1].first] = m_DS100ProtocolSelectButton->addButton(t[1].first, t[1].second);
	m_DS100ProtocolSelectButtonIds[t[2].first] = m_DS100ProtocolSelectButton->addButton(t[2].first, t[2].second);
	m_DS100ProtocolSelectButton->setButtonDown(m_DS100ProtocolSelectButtonIds[m_SecondDS100Modes[0]]);
	m_DS100ProtocolSelectLabel = std::make_unique<Label>("DS100ProtocolSelectButton", "DS100 Protocol");
	m_DS100ProtocolSelectLabel->setJustificationType(Justification::centred);
	m_DS100ProtocolSelectLabel->attachToComponent(m_DS100ProtocolSelectButton.get(), true);
	m_DS100Settings->addComponent(m_DS100ProtocolSelectLabel.get(), false, false);
	m_DS100Settings->addComponent(m_DS100ProtocolSelectButton.get(), true, false);

	m_DS100IntervalEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_DS100IntervalEdit->addListener(this);
	m_DS100IntervalEdit->setInputFilter(m_intervalEditFilter.get(), false);
	m_DS100IntervalLabel = std::make_unique<Label>("DS100IntervalEdit", "Interval");
	m_DS100IntervalLabel->setJustificationType(Justification::centred);
	m_DS100IntervalLabel->attachToComponent(m_DS100IntervalEdit.get(), true);
	m_DS100Settings->addComponent(m_DS100IntervalLabel.get(), false, false);
	m_DS100Settings->addComponent(m_DS100IntervalEdit.get(), true, false);

	//first DS100 - ch. 1-64
	m_DS100ConnectionElmsContainer = std::make_unique<HorizontalLayouterComponent>();
	m_DS100ConnectionElmsContainer->SetSpacing(5);
	m_DS100IpAndPortEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_DS100IpAndPortEdit->addListener(this);
	m_DS100IpAndPortEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_DS100ConnectionElmsContainer->AddComponent(m_DS100IpAndPortEdit.get(), 5);
	m_DS100IpAndPortLabel = std::make_unique<Label>("DS100IpAndPortEdit", "IP Address");
	m_DS100IpAndPortLabel->setJustificationType(Justification::centred);
	m_DS100IpAndPortLabel->attachToComponent(m_DS100ConnectionElmsContainer.get(), true);
#ifdef ZEROCONF_SUPPORTED
	m_DS100ZeroconfDiscovery = std::make_unique<JUCEAppBasics::ZeroconfDiscoverComponent>("ZDC1");
	m_DS100ZeroconfDiscovery->onServiceSelected = [=](JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, ZeroconfSearcher::ZeroconfSearcher::ServiceInfo* info) { handleDS100ServiceSelected(type, info); };
	m_DS100ZeroconfDiscovery->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OSC);
	m_DS100ZeroconfDiscovery->addPopupCategory("d&b DS100 devices", std::make_pair(
		std::make_pair(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceCategoryType::ZSCT_MetaInfo, "NAME"),
		std::make_pair(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceCategoryMatch::ZSCM_Contain, "DS100")));
	m_DS100ConnectionElmsContainer->AddComponent(m_DS100ZeroconfDiscovery.get(), 1);
#endif
	m_DS100Settings->addComponent(m_DS100IpAndPortLabel.get(), false, false);
	m_DS100Settings->addComponent(m_DS100ConnectionElmsContainer.get(), true, false);

	m_SecondDS100ModeButton = std::make_unique<JUCEAppBasics::SplitButtonComponent>();
	m_SecondDS100ModeButton->addListener(this);
	m_SecondDS100ModeButtonIds[m_SecondDS100Modes[0]] = m_SecondDS100ModeButton->addButton(m_SecondDS100Modes[0]);
	m_SecondDS100ModeButtonIds[m_SecondDS100Modes[1]] = m_SecondDS100ModeButton->addButton(m_SecondDS100Modes[1]);
	m_SecondDS100ModeButtonIds[m_SecondDS100Modes[2]] = m_SecondDS100ModeButton->addButton(m_SecondDS100Modes[2]);
	m_SecondDS100ModeButtonIds[m_SecondDS100Modes[3]] = m_SecondDS100ModeButton->addButton(m_SecondDS100Modes[3]);
	m_SecondDS100ModeButton->setButtonDown(m_SecondDS100ModeButtonIds[m_SecondDS100Modes[0]]);
	m_SecondDS100ModeLabel = std::make_unique<Label>("SecondDS100ModeButton", "2nd DS100");
	m_SecondDS100ModeLabel->setJustificationType(Justification::centred);
	m_SecondDS100ModeLabel->attachToComponent(m_SecondDS100ModeButton.get(), true);
	m_DS100Settings->addComponent(m_SecondDS100ModeLabel.get(), false, false);
	m_DS100Settings->addComponent(m_SecondDS100ModeButton.get(), true, false);

	m_SecondDS100ParallelModeButton = std::make_unique<JUCEAppBasics::SplitButtonComponent>();
	m_SecondDS100ParallelModeButton->addListener(this);
	m_SecondDS100ParallelModeButtonIds[m_SecondDS100ParallelModes[0]] = m_SecondDS100ParallelModeButton->addButton(m_SecondDS100ParallelModes[0]);
	m_SecondDS100ParallelModeButtonIds[m_SecondDS100ParallelModes[1]] = m_SecondDS100ParallelModeButton->addButton(m_SecondDS100ParallelModes[1]);
	m_SecondDS100ParallelModeButton->setButtonDown(m_SecondDS100ParallelModeButtonIds[m_SecondDS100ParallelModes[0]]);
	m_SecondDS100ParallelModeLabel = std::make_unique<Label>("SecondDS100ParallelModeButton", "Active DS100");
	m_SecondDS100ParallelModeLabel->setJustificationType(Justification::centred);
	m_SecondDS100ParallelModeLabel->attachToComponent(m_SecondDS100ParallelModeButton.get(), true);
	m_DS100Settings->addComponent(m_SecondDS100ParallelModeLabel.get(), false, false);
	m_DS100Settings->addComponent(m_SecondDS100ParallelModeButton.get(), true, false);

	//second DS100 - ch. 65-128
	m_SecondDS100ConnectionElmsContainer = std::make_unique<HorizontalLayouterComponent>();
	m_SecondDS100ConnectionElmsContainer->SetSpacing(5);
	m_SecondDS100IpAndPortEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_SecondDS100IpAndPortEdit->addListener(this);
	m_SecondDS100IpAndPortEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_SecondDS100ConnectionElmsContainer->AddComponent(m_SecondDS100IpAndPortEdit.get(), 5);
	m_SecondDS100IpAndPortLabel = std::make_unique<Label>("SecondDS100IpAndPortEdit", "IP Address");
	m_SecondDS100IpAndPortLabel->setJustificationType(Justification::centred);
	m_SecondDS100IpAndPortLabel->attachToComponent(m_SecondDS100ConnectionElmsContainer.get(), true);
#ifdef ZEROCONF_SUPPORTED
	m_SecondDS100ZeroconfDiscovery = std::make_unique<JUCEAppBasics::ZeroconfDiscoverComponent>("ZDC2");
	m_SecondDS100ZeroconfDiscovery->onServiceSelected = [=](JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, ZeroconfSearcher::ZeroconfSearcher::ServiceInfo* info) { handleSecondDS100ServiceSelected(type, info); };
	m_SecondDS100ZeroconfDiscovery->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OSC);
	m_SecondDS100ZeroconfDiscovery->addPopupCategory("d&b DS100 devices", std::make_pair(
		std::make_pair(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceCategoryType::ZSCT_MetaInfo, "NAME"),
		std::make_pair(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceCategoryMatch::ZSCM_Contain, "DS100")));
	m_SecondDS100ConnectionElmsContainer->AddComponent(m_SecondDS100ZeroconfDiscovery.get(), 1);
#endif
	m_DS100Settings->addComponent(m_SecondDS100IpAndPortLabel.get(), false, false);
	m_DS100Settings->addComponent(m_SecondDS100ConnectionElmsContainer.get(), true, false);

	//dummy DS100 projectdata loading elements
	m_DS100ProjectDummyDataLoader = std::make_unique<ProjectDummyDataLoaderComponent>();
	m_DS100ProjectDummyDataLoader->onProjectDummyDataLoaded = [=](const juce::String& projectDummyData) { handleDS100dbprData(projectDummyData); };
	m_DS100ProjectDummyDataLabel = std::make_unique<Label>("ProjectDummyDataLoaderComponent", "Project Dummy Data");
	m_DS100ProjectDummyDataLabel->setJustificationType(Justification::centredLeft);
	m_DS100ProjectDummyDataLabel->attachToComponent(m_DS100ProjectDummyDataLoader.get(), true);
	m_DS100Settings->addComponent(m_DS100ProjectDummyDataLabel.get(), false, false);
	m_DS100Settings->addComponent(m_DS100ProjectDummyDataLoader.get(), true, false);

	m_DS100Settings->resized();
}

/**
* Helper method to create and setup objects for DiGiCo settings section
 */
void SettingsSectionsComponent::createDiGiCoSettingsSection()
{
	// DiGiCo settings section
	m_DiGiCoBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_DiGiCoBridgingSettings->setActiveToggleText("Use " + GetProtocolBridgingNiceName(PBT_DiGiCo) + " Bridging");
	m_DiGiCoBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_DiGiCo) + " Bridging Settings");
	m_DiGiCoBridgingSettings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/DiGiCoOSC.md"));
	m_DiGiCoBridgingSettings->setHasActiveToggle(true);
	m_DiGiCoBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_DiGiCoBridgingSettings.get());

	m_DiGiCoIpAddressEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_DiGiCoIpAddressEdit->addListener(this);
	m_DiGiCoIpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_DiGiCoIpAddressLabel = std::make_unique<Label>("DiGiCoIpAddressEdit", "IP Address");
	m_DiGiCoIpAddressLabel->setJustificationType(Justification::centred);
	m_DiGiCoIpAddressLabel->attachToComponent(m_DiGiCoIpAddressEdit.get(), true);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoIpAddressLabel.get(), false, false);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoIpAddressEdit.get(), true, false);

	m_DiGiCoListeningPortEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_DiGiCoListeningPortEdit->addListener(this);
	m_DiGiCoListeningPortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_DiGiCoListeningPortLabel = std::make_unique<Label>("DiGiCoListeningPortEdit", "Listening Port");
	m_DiGiCoListeningPortLabel->setJustificationType(Justification::centred);
	m_DiGiCoListeningPortLabel->attachToComponent(m_DiGiCoListeningPortEdit.get(), true);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoListeningPortLabel.get(), false, false);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoListeningPortEdit.get(), true, false);

	m_DiGiCoRemotePortEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_DiGiCoRemotePortEdit->addListener(this);
	m_DiGiCoRemotePortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_DiGiCoRemotePortLabel = std::make_unique<Label>("DiGiCoRemotePortEdit", "Remote Port");
	m_DiGiCoRemotePortLabel->setJustificationType(Justification::centred);
	m_DiGiCoRemotePortLabel->attachToComponent(m_DiGiCoRemotePortEdit.get(), true);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoRemotePortLabel.get(), false, false);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoRemotePortEdit.get(), true, false);

	m_DiGiCoBridgingSettings->resized();
}

/**
* Helper method to create and setup objects for DiGiCo settings section
 */
void SettingsSectionsComponent::createDAWPluginSettingsSection()
{
	// DAWPlugin settings section
	m_DAWPluginBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_DAWPluginBridgingSettings->setActiveToggleText("Use " + GetProtocolBridgingNiceName(PBT_DAWPlugin) + " Bridging");
	m_DAWPluginBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_DAWPlugin) + " Bridging Settings");
	m_DAWPluginBridgingSettings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/DAWPlugin.md"));
	m_DAWPluginBridgingSettings->setHasActiveToggle(true);
	m_DAWPluginBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_DAWPluginBridgingSettings.get());

	m_DAWPluginIpAddressEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_DAWPluginIpAddressEdit->addListener(this);
	m_DAWPluginIpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_DAWPluginIpAddressLabel = std::make_unique<Label>("DAWPluginIpAddressEdit", "IP Address");
	m_DAWPluginIpAddressLabel->setJustificationType(Justification::centred);
	m_DAWPluginIpAddressLabel->attachToComponent(m_DAWPluginIpAddressEdit.get(), true);
	m_DAWPluginBridgingSettings->addComponent(m_DAWPluginIpAddressLabel.get(), false, false);
	m_DAWPluginBridgingSettings->addComponent(m_DAWPluginIpAddressEdit.get(), true, false);

	m_DAWPluginDifferentHostInfoLabel = std::make_unique<Label>("DAWPluginDifferentHostInfo1Label", "Note: " + GetProtocolBridgingNiceName(PBT_DAWPlugin) + " cannot run on same host as " + JUCEApplicationBase::getInstance()->getApplicationName());
	m_DAWPluginBridgingSettings->addComponent(m_DAWPluginDifferentHostInfoLabel.get(), true, false, 2);

	m_DAWPluginBridgingSettings->resized();
}

/**
 * Helper method to create and setup objects for Blacktrax RTTrPM settings section
 */
void SettingsSectionsComponent::createRTTrPMSettingsSection()
{
	// BlackTrax RTTrPM settings section
	m_RTTrPMBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_RTTrPMBridgingSettings->setActiveToggleText("Use " + GetProtocolBridgingNiceName(PBT_BlacktraxRTTrPM) + " Bridging");
	m_RTTrPMBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_BlacktraxRTTrPM) + " Bridging Settings");
	m_RTTrPMBridgingSettings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/BlacktraxRTTrPM.md"));
	m_RTTrPMBridgingSettings->setHasActiveToggle(true);
	m_RTTrPMBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_RTTrPMBridgingSettings.get());

	m_RTTrPMListeningPortEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_RTTrPMListeningPortEdit->addListener(this);
	m_RTTrPMListeningPortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_RTTrPMListeningPortLabel = std::make_unique<Label>("RTTrPMListeningPortEdit", "Listening Port");
	m_RTTrPMListeningPortLabel->setJustificationType(Justification::centred);
	m_RTTrPMListeningPortLabel->attachToComponent(m_RTTrPMListeningPortEdit.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMListeningPortLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMListeningPortEdit.get(), true, false);

	m_RTTrPMBeaconIdxAssignmentsEditor = std::make_unique<IndexToChannelAssignerComponent>();
	m_RTTrPMBeaconIdxAssignmentsEditor->onAssignmentsSet = [=](Component* sender, const std::map<int, ChannelId>& idxToChAssis) { ignoreUnused(sender); handleRTTrPMBeaconIdxAssisSet(idxToChAssis); };
	m_RTTrPMBeaconIdxAssignmentsLabel = std::make_unique<Label>("RTTrPMBeaconIdxAssignmentsEditor", "Beacon Indices");
	m_RTTrPMBeaconIdxAssignmentsLabel->setJustificationType(Justification::centredLeft);
	m_RTTrPMBeaconIdxAssignmentsLabel->attachToComponent(m_RTTrPMBeaconIdxAssignmentsEditor.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMBeaconIdxAssignmentsLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMBeaconIdxAssignmentsEditor.get(), true, false);

	m_RTTrPMDataTypeSelect = std::make_unique<ComboBox>();
	m_RTTrPMDataTypeSelect->addListener(this);
	m_RTTrPMDataTypeSelect->addItemList(juce::StringArray{ "Centroid Position", "LED Position" }, 1);
	m_RTTrPMDataTypeLabel = std::make_unique<Label>("RTTrPMDataTypeSelect", "Data Type");
	m_RTTrPMDataTypeLabel->setJustificationType(Justification::centred);
	m_RTTrPMDataTypeLabel->attachToComponent(m_RTTrPMDataTypeSelect.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMDataTypeLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMDataTypeSelect.get(), true, false);

	m_RTTrPMInterpretXYRelativeButton = std::make_unique<JUCEAppBasics::SplitButtonComponent>();
	m_RTTrPMInterpretXYRelativeButton->addListener(this);
	m_RTTrPMInterpretXYRelativeButtonIds[m_RTTrPMInterpretXYRelativeModes[0]] = m_RTTrPMInterpretXYRelativeButton->addButton(m_RTTrPMInterpretXYRelativeModes[0]);
	m_RTTrPMInterpretXYRelativeButtonIds[m_RTTrPMInterpretXYRelativeModes[1]] = m_RTTrPMInterpretXYRelativeButton->addButton(m_RTTrPMInterpretXYRelativeModes[1]);
	m_RTTrPMInterpretXYRelativeButton->setButtonDown(m_RTTrPMInterpretXYRelativeButtonIds[m_RTTrPMInterpretXYRelativeModes[0]]);
	m_RTTrPMInterpretXYRelativeLabel = std::make_unique<Label>("RTTrPMInterpretXYRelativeButton", "Soundscape Coords.");
	m_RTTrPMInterpretXYRelativeLabel->setJustificationType(Justification::centred);
	m_RTTrPMInterpretXYRelativeLabel->attachToComponent(m_RTTrPMInterpretXYRelativeButton.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMInterpretXYRelativeLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMInterpretXYRelativeButton.get(), true, false);
	
	m_RTTrPMCoordSysModContainer = std::make_unique<HorizontalLayouterComponent>();
	m_RTTrPMCoordSysModContainer->SetSpacing(5);
	m_RTTrPMXYSwapButton = std::make_unique<JUCEAppBasics::TextWithImageButton>("swap XY");
	m_RTTrPMXYSwapButton->setTooltip("Swap X/Y coordinates.");
	m_RTTrPMXYSwapButton->setImagePosition(Justification::centredLeft);
	m_RTTrPMXYSwapButton->setClickingTogglesState(true);
	m_RTTrPMXYSwapButton->addListener(this);
	m_RTTrPMCoordSysModContainer->AddComponent(m_RTTrPMXYSwapButton.get());
	m_RTTrPMInvertXButton = std::make_unique<JUCEAppBasics::TextWithImageButton>("inv. X");
	m_RTTrPMInvertXButton->setTooltip("Invert X coordinates.");
	m_RTTrPMInvertXButton->setImagePosition(Justification::centredLeft);
	m_RTTrPMInvertXButton->setClickingTogglesState(true);
	m_RTTrPMInvertXButton->addListener(this);
	m_RTTrPMCoordSysModContainer->AddComponent(m_RTTrPMInvertXButton.get());
	m_RTTrPMInvertYButton = std::make_unique<JUCEAppBasics::TextWithImageButton>("inv. Y");
	m_RTTrPMInvertYButton->setTooltip("Invert Y coordinates.");
	m_RTTrPMInvertYButton->setImagePosition(Justification::centredLeft);
	m_RTTrPMInvertYButton->setClickingTogglesState(true);
	m_RTTrPMInvertYButton->addListener(this);
	m_RTTrPMCoordSysModContainer->AddComponent(m_RTTrPMInvertYButton.get());
	m_RTTrPMCoordSysModLabel = std::make_unique<Label>("RTTrPMXYSwapLabel", "XY Coord. Processing");
	m_RTTrPMCoordSysModLabel->setJustificationType(Justification::centred);
	m_RTTrPMCoordSysModLabel->attachToComponent(m_RTTrPMCoordSysModContainer.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMCoordSysModLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMCoordSysModContainer.get(), true, false);

	m_RTTrPMAbsoluteOriginElmsContainer = std::make_unique<HorizontalLayouterComponent>();
	m_RTTrPMAbsoluteOriginElmsContainer->SetSpacing(5);
	m_RTTrPMAbsoluteOriginXLabel = std::make_unique<Label>("RTTrPMAbsolutOriginXEdit", "x");
	m_RTTrPMAbsoluteOriginElmsContainer->AddComponent(m_RTTrPMAbsoluteOriginXLabel.get(), 0.25f);
	m_RTTrPMAbsoluteOriginXEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_RTTrPMAbsoluteOriginXEdit->addListener(this);
	m_RTTrPMAbsoluteOriginXEdit->setInputFilter(std::make_unique<TextEditor::LengthAndCharacterRestriction>(7, "1234567890.,-").release(), true);
	m_RTTrPMAbsoluteOriginElmsContainer->AddComponent(m_RTTrPMAbsoluteOriginXEdit.get(), 0.75f);
	m_RTTrPMAbsoluteOriginYLabel = std::make_unique<Label>("RTTrPMAbsolutOriginYEdit", "y");
	m_RTTrPMAbsoluteOriginElmsContainer->AddComponent(m_RTTrPMAbsoluteOriginYLabel.get(), 0.25f);
	m_RTTrPMAbsoluteOriginYEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_RTTrPMAbsoluteOriginYEdit->addListener(this);
	m_RTTrPMAbsoluteOriginYEdit->setInputFilter(std::make_unique<TextEditor::LengthAndCharacterRestriction>(7, "1234567890.,-").release(), true);
	m_RTTrPMAbsoluteOriginElmsContainer->AddComponent(m_RTTrPMAbsoluteOriginYEdit.get(), 0.75f);
	m_RTTrPMAbsoluteOriginLabel = std::make_unique<Label>("RTTrPMAbsolutOrigin", "Origin Offset");
	m_RTTrPMAbsoluteOriginLabel->setTooltip("Only available when using '" + m_RTTrPMInterpretXYRelativeModes[0] + "' Soundscape coordinates.");
	m_RTTrPMAbsoluteOriginLabel->attachToComponent(m_RTTrPMAbsoluteOriginElmsContainer.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMAbsoluteOriginLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMAbsoluteOriginElmsContainer.get(), true, false);

	m_RTTrPMMappingAreaSelect = std::make_unique<ComboBox>();
	m_RTTrPMMappingAreaSelect->addListener(this);
	m_RTTrPMMappingAreaSelect->addItemList({ "1", "2", "3", "4" }, MAI_First);
	m_RTTrPMMappingAreaLabel = std::make_unique<Label>("RTTrPMMappingAreaSelect", "Mapping Area");
	m_RTTrPMMappingAreaLabel->setJustificationType(Justification::centred);
	m_RTTrPMMappingAreaLabel->setTooltip("Only available when using '" + m_RTTrPMInterpretXYRelativeModes[1] + "' Soundscape coordinates.");
	m_RTTrPMMappingAreaLabel->attachToComponent(m_RTTrPMMappingAreaSelect.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMMappingAreaLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMMappingAreaSelect.get(), true, false);

	m_RTTrPMMappingPoint1ElmsContainer = std::make_unique<HorizontalLayouterComponent>();
	m_RTTrPMMappingPoint1ElmsContainer->SetSpacing(5);
	m_RTTrPMMappingPoint1XLabel = std::make_unique<Label>("RTTrPMMappingPoint1XEdit", "x");
	m_RTTrPMMappingPoint1ElmsContainer->AddComponent(m_RTTrPMMappingPoint1XLabel.get(), 0.25f);
	m_RTTrPMMappingPoint1XEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_RTTrPMMappingPoint1XEdit->addListener(this);
	m_RTTrPMMappingPoint1XEdit->setInputFilter(std::make_unique<TextEditor::LengthAndCharacterRestriction>(7, "1234567890.,-").release(), true);
	m_RTTrPMMappingPoint1ElmsContainer->AddComponent(m_RTTrPMMappingPoint1XEdit.get(), 0.75f);
	m_RTTrPMMappingPoint1YLabel = std::make_unique<Label>("RTTrPMMappingPoint1YEdit", "y");
	m_RTTrPMMappingPoint1ElmsContainer->AddComponent(m_RTTrPMMappingPoint1YLabel.get(), 0.25f);
	m_RTTrPMMappingPoint1YEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_RTTrPMMappingPoint1YEdit->addListener(this);
	m_RTTrPMMappingPoint1YEdit->setInputFilter(std::make_unique<TextEditor::LengthAndCharacterRestriction>(7, "1234567890.,-").release(), true);
	m_RTTrPMMappingPoint1ElmsContainer->AddComponent(m_RTTrPMMappingPoint1YEdit.get(), 0.75f);
	m_RTTrPMMappingPoint1Label = std::make_unique<Label>("RTTrPMMappingPoint1", "Mapping Min");
	m_RTTrPMMappingPoint1Label->setTooltip("Only available when using '" + m_RTTrPMInterpretXYRelativeModes[1] + "' Soundscape coordinates.");
	m_RTTrPMMappingPoint1Label->attachToComponent(m_RTTrPMMappingPoint1ElmsContainer.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMMappingPoint1Label.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMMappingPoint1ElmsContainer.get(), true, false);

	m_RTTrPMMappingPoint2ElmsContainer = std::make_unique<HorizontalLayouterComponent>();
	m_RTTrPMMappingPoint2ElmsContainer->SetSpacing(5);
	m_RTTrPMMappingPoint2XLabel = std::make_unique<Label>("RTTrPMMappingPoint2XEdit", "x");
	m_RTTrPMMappingPoint2ElmsContainer->AddComponent(m_RTTrPMMappingPoint2XLabel.get(), 0.25f);
	m_RTTrPMMappingPoint2XEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_RTTrPMMappingPoint2XEdit->addListener(this);
	m_RTTrPMMappingPoint2XEdit->setInputFilter(std::make_unique<TextEditor::LengthAndCharacterRestriction>(7, "1234567890.,-").release(), true);
	m_RTTrPMMappingPoint2ElmsContainer->AddComponent(m_RTTrPMMappingPoint2XEdit.get(), 0.75f);
	m_RTTrPMMappingPoint2YLabel = std::make_unique<Label>("RTTrPMMappingPoint2YEdit", "y");
	m_RTTrPMMappingPoint2ElmsContainer->AddComponent(m_RTTrPMMappingPoint2YLabel.get(), 0.25f);
	m_RTTrPMMappingPoint2YEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_RTTrPMMappingPoint2YEdit->addListener(this);
	m_RTTrPMMappingPoint2YEdit->setInputFilter(std::make_unique<TextEditor::LengthAndCharacterRestriction>(7, "1234567890.,-").release(), true);
	m_RTTrPMMappingPoint2ElmsContainer->AddComponent(m_RTTrPMMappingPoint2YEdit.get(), 0.75f);
	m_RTTrPMMappingPoint2Label = std::make_unique<Label>("RTTrPMMappingPoint2", "Mapping Max");
	m_RTTrPMMappingPoint2Label->setTooltip("Only available when using '" + m_RTTrPMInterpretXYRelativeModes[1] + "' Soundscape coordinates.");
	m_RTTrPMMappingPoint2Label->attachToComponent(m_RTTrPMMappingPoint2ElmsContainer.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMMappingPoint2Label.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMMappingPoint2ElmsContainer.get(), true, false);

	m_RTTrPMBridgingSettings->resized();
}

/**
 * Helper method to create and setup objects for d&b Generic OSC settings section
 */
void SettingsSectionsComponent::createGenericOSCSettingsSection()
{
	// Generic OSC settings section
	m_GenericOSCBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_GenericOSCBridgingSettings->setActiveToggleText("Use " + GetProtocolBridgingNiceName(PBT_GenericOSC) + " Bridging");
	m_GenericOSCBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_GenericOSC) + " Bridging Settings");
	m_GenericOSCBridgingSettings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/GenericOSC.md"));
	m_GenericOSCBridgingSettings->setHasActiveToggle(true);
	m_GenericOSCBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_GenericOSCBridgingSettings.get());

	m_GenericOSCIpAddressEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_GenericOSCIpAddressEdit->addListener(this);
	m_GenericOSCIpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_GenericOSCIpAddressLabel = std::make_unique<Label>("GenericOSCIpAddressEdit", "IP Address");
	m_GenericOSCIpAddressLabel->setJustificationType(Justification::centred);
	m_GenericOSCIpAddressLabel->attachToComponent(m_GenericOSCIpAddressEdit.get(), true);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCIpAddressLabel.get(), false, false);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCIpAddressEdit.get(), true, false);

	m_GenericOSCListeningPortEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_GenericOSCListeningPortEdit->addListener(this);
	m_GenericOSCListeningPortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_GenericOSCListeningPortLabel = std::make_unique<Label>("GenericOSCListeningPortEdit", "Listening Port");
	m_GenericOSCListeningPortLabel->setJustificationType(Justification::centred);
	m_GenericOSCListeningPortLabel->attachToComponent(m_GenericOSCListeningPortEdit.get(), true);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCListeningPortLabel.get(), false, false);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCListeningPortEdit.get(), true, false);

	m_GenericOSCRemotePortEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_GenericOSCRemotePortEdit->addListener(this);
	m_GenericOSCRemotePortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_GenericOSCRemotePortLabel = std::make_unique<Label>("GenericOSCRemotePortEdit", "Remote Port");
	m_GenericOSCRemotePortLabel->setJustificationType(Justification::centred);
	m_GenericOSCRemotePortLabel->attachToComponent(m_GenericOSCRemotePortEdit.get(), true);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCRemotePortLabel.get(), false, false);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCRemotePortEdit.get(), true, false);

	m_GenericOSCDisableSendingButton = std::make_unique<JUCEAppBasics::TextWithImageButton>("Disable OSC return channel");
	m_GenericOSCDisableSendingButton->setTooltip("Disable sending of value changes to Generic OSC input devices.");
	m_GenericOSCDisableSendingButton->setImagePosition(Justification::centredLeft);
	m_GenericOSCDisableSendingButton->setClickingTogglesState(true);
	m_GenericOSCDisableSendingButton->addListener(this);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCDisableSendingButton.get(), true, false);

	m_GenericOSCBridgingSettings->resized();
}

/**
 * Helper method to create and setup objects for Generic MIDI settings section
 */
void SettingsSectionsComponent::createGenericMIDISettingsSection()
{
	// Generic MIDI settings section
	m_GenericMIDIBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_GenericMIDIBridgingSettings->setBackgroundDecorationText("Beta");
	m_GenericMIDIBridgingSettings->setActiveToggleText("Use " + GetProtocolBridgingNiceName(PBT_GenericMIDI) + " Bridging");
	m_GenericMIDIBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_GenericMIDI) + " Bridging Settings");
	m_GenericMIDIBridgingSettings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/GenericMIDI.md"));
	m_GenericMIDIBridgingSettings->setHasActiveToggle(true);
	m_GenericMIDIBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_GenericMIDIBridgingSettings.get());

	m_GenericMIDIInputDeviceSelect = std::make_unique<ComboBox>();
	m_GenericMIDIInputDeviceSelect->setTextWhenNoChoicesAvailable("No MIDI Inputs Enabled");
	updateAvailableMidiInputDevices();
	m_GenericMIDIInputDeviceSelect->addListener(this);
	m_GenericMIDIInputDeviceSelectLabel = std::make_unique<Label>("GenericMIDIInputDeviceSelect", "MIDI Input");
	m_GenericMIDIInputDeviceSelectLabel->setJustificationType(Justification::centred);
	m_GenericMIDIInputDeviceSelectLabel->attachToComponent(m_GenericMIDIInputDeviceSelect.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIInputDeviceSelectLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIInputDeviceSelect.get(), true, false);

	m_GenericMIDIOutputDeviceSelect = std::make_unique<ComboBox>();
	m_GenericMIDIOutputDeviceSelect->setTextWhenNoChoicesAvailable("No MIDI Outputs Enabled");
	updateAvailableMidiOutputDevices();
	m_GenericMIDIOutputDeviceSelect->addListener(this);
	m_GenericMIDIOutputDeviceSelectLabel = std::make_unique<Label>("GenericMIDIOutputDeviceSelect", "MIDI Output");
	m_GenericMIDIOutputDeviceSelectLabel->setJustificationType(Justification::centred);
	m_GenericMIDIOutputDeviceSelectLabel->attachToComponent(m_GenericMIDIOutputDeviceSelect.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIOutputDeviceSelectLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIOutputDeviceSelect.get(), true, false);

	m_GenericMIDIMappingAreaSelect = std::make_unique<ComboBox>();
	m_GenericMIDIMappingAreaSelect->addListener(this);
	m_GenericMIDIMappingAreaSelect->addItemList({ "1", "2", "3", "4" }, MAI_First);
	m_GenericMIDIMappingAreaLabel = std::make_unique<Label>("GenericMIDIMappingAreaSelect", "Mapping Area");
	m_GenericMIDIMappingAreaLabel->setJustificationType(Justification::centred);
	m_GenericMIDIMappingAreaLabel->attachToComponent(m_GenericMIDIMappingAreaSelect.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMappingAreaLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMappingAreaSelect.get(), true, false);

	m_GenericMIDIMatrixInputSelectLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_RemoteProtocolBridge_SoundObjectSelect),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_Trigger | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIMatrixInputSelectLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIMatrixInputSelectLabel = std::make_unique<Label>("GenericMIDIMatrixInputSelectLearner", "Object Select");
	m_GenericMIDIMatrixInputSelectLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIMatrixInputSelectLabel->attachToComponent(m_GenericMIDIMatrixInputSelectLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMatrixInputSelectLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMatrixInputSelectLearner.get(), true, false);

	m_GenericMIDISelectionSelectLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_RemoteProtocolBridge_SoundObjectGroupSelect),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_Trigger | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDISelectionSelectLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDISelectionSelectLabel = std::make_unique<Label>("GenericMIDISelectSelectLearner", "Selection Select");
	m_GenericMIDISelectionSelectLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDISelectionSelectLabel->attachToComponent(m_GenericMIDISelectionSelectLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDISelectionSelectLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDISelectionSelectLearner.get(), true, false);

	m_GenericMIDIXValueLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_CoordinateMapping_SourcePosition_X),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_ValueRange | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIXValueLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIXValueLabel = std::make_unique<Label>("GenericMIDIXValueLearner", "Relative Pos. X");
	m_GenericMIDIXValueLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIXValueLabel->attachToComponent(m_GenericMIDIXValueLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIXValueLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIXValueLearner.get(), true, false);

	m_GenericMIDIYValueLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_CoordinateMapping_SourcePosition_Y),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_ValueRange | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIYValueLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIYValueLabel = std::make_unique<Label>("GenericMIDIYValueLearner", "Relative Pos. Y");
	m_GenericMIDIYValueLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIYValueLabel->attachToComponent(m_GenericMIDIYValueLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIYValueLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIYValueLearner.get(), true, false);

	m_GenericMIDIReverbSendGainLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_MatrixInput_ReverbSendGain),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_ValueRange | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIReverbSendGainLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIReverbSendGainLabel = std::make_unique<Label>("GenericMIDIReverbSendGainLearner", "Reverb Send Gain");
	m_GenericMIDIReverbSendGainLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIReverbSendGainLabel->attachToComponent(m_GenericMIDIReverbSendGainLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIReverbSendGainLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIReverbSendGainLearner.get(), true, false);

	m_GenericMIDISourceSpreadLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_Positioning_SourceSpread),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_ValueRange | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDISourceSpreadLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDISourceSpreadLabel = std::make_unique<Label>("GenericMIDISourceSpreadLearner", "Object Spread");
	m_GenericMIDISourceSpreadLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDISourceSpreadLabel->attachToComponent(m_GenericMIDISourceSpreadLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDISourceSpreadLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDISourceSpreadLearner.get(), true, false);

	m_GenericMIDIDelayModeLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_Positioning_SourceDelayMode),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_ValueRange | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIDelayModeLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIDelayModeLabel = std::make_unique<Label>("GenericMIDIDelayModeLearner", "Object DelayMode");
	m_GenericMIDIDelayModeLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIDelayModeLabel->attachToComponent(m_GenericMIDIDelayModeLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIDelayModeLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIDelayModeLearner.get(), true, false);

	m_GenericMIDIMatrixInputGainLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_MatrixInput_Gain),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_ValueRange | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIMatrixInputGainLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIMatrixInputGainLabel = std::make_unique<Label>("GenericMIDIMatrixInputGainLearner", "MatrixInput Gain");
	m_GenericMIDIMatrixInputGainLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIMatrixInputGainLabel->attachToComponent(m_GenericMIDIMatrixInputGainLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMatrixInputGainLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMatrixInputGainLearner.get(), true, false);

	m_GenericMIDIMatrixInputMuteLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_MatrixInput_Mute),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_Trigger | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIMatrixInputMuteLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIMatrixInputMuteLabel = std::make_unique<Label>("GenericMIDIMatrixInputMuteLearner", "MatrixInput Mute");
	m_GenericMIDIMatrixInputMuteLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIMatrixInputMuteLabel->attachToComponent(m_GenericMIDIMatrixInputMuteLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMatrixInputMuteLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMatrixInputMuteLearner.get(), true, false);

	m_GenericMIDIMatrixOutputGainLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_MatrixOutput_Gain),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_ValueRange | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIMatrixOutputGainLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIMatrixOutputGainLabel = std::make_unique<Label>("GenericMIDIMatrixOutputGainLearner", "MatrixOutput Gain");
	m_GenericMIDIMatrixOutputGainLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIMatrixOutputGainLabel->attachToComponent(m_GenericMIDIMatrixOutputGainLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMatrixOutputGainLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMatrixOutputGainLearner.get(), true, false);

	m_GenericMIDIMatrixOutputMuteLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_MatrixOutput_Mute),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_Trigger | JUCEAppBasics::MidiLearnerComponent::AT_CommandRange));
	m_GenericMIDIMatrixOutputMuteLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIMatrixOutputMuteLabel = std::make_unique<Label>("GenericMIDIMatrixOutputMuteLearner", "MatrixOutput Mute");
	m_GenericMIDIMatrixOutputMuteLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIMatrixOutputMuteLabel->attachToComponent(m_GenericMIDIMatrixOutputMuteLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMatrixOutputMuteLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIMatrixOutputMuteLearner.get(), true, false);

	m_GenericMIDINextSceneLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_Scene_Next),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_Trigger));
	m_GenericMIDINextSceneLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDINextSceneLabel = std::make_unique<Label>("GenericMIDINextSceneLearner", "Next Scene");
	m_GenericMIDINextSceneLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDINextSceneLabel->attachToComponent(m_GenericMIDINextSceneLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDINextSceneLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDINextSceneLearner.get(), true, false);

	m_GenericMIDIPrevSceneLearner = std::make_unique<JUCEAppBasics::MidiLearnerComponent>(
		static_cast<std::int16_t>(ROI_Scene_Previous),
		static_cast<JUCEAppBasics::MidiLearnerComponent::AssignmentType>(JUCEAppBasics::MidiLearnerComponent::AT_Trigger));
	m_GenericMIDIPrevSceneLearner->onMidiAssiSet = [=](Component* sender, const JUCEAppBasics::MidiCommandRangeAssignment& midiAssi) { handleMidiAssiSet(sender, midiAssi); };
	m_GenericMIDIPrevSceneLabel = std::make_unique<Label>("GenericMIDIPrevSceneLearner", "Previous Scene");
	m_GenericMIDIPrevSceneLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIPrevSceneLabel->attachToComponent(m_GenericMIDIPrevSceneLearner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIPrevSceneLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIPrevSceneLearner.get(), true, false);

	m_GenericMIDIRecallSceneAssigner = std::make_unique<SceneIndexToMidiAssignerComponent>(
		static_cast<std::int16_t>(ROI_Scene_Recall));
	m_GenericMIDIRecallSceneAssigner->onAssignmentsSet = [=](Component* sender, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& scenesToMidiAssi) { handleScenesToMidiAssiSet(sender, scenesToMidiAssi); };
	m_GenericMIDIRecallSceneLabel = std::make_unique<Label>("GenericMIDIRecallSceneAssigner", "Recall Scene");
	m_GenericMIDIRecallSceneLabel->setJustificationType(Justification::centredLeft);
	m_GenericMIDIRecallSceneLabel->attachToComponent(m_GenericMIDIRecallSceneAssigner.get(), true);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIRecallSceneLabel.get(), false, false);
	m_GenericMIDIBridgingSettings->addComponent(m_GenericMIDIRecallSceneAssigner.get(), true, false);

	m_GenericMIDIBridgingSettings->resized();

}

/**
 * Helper method to create and setup objects for Yamaha OSC settings section
 */
void SettingsSectionsComponent::createYamahaOSCSettingsSection()
{
	// YamahaOSC settings section
	m_YamahaOSCBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_YamahaOSCBridgingSettings->setBackgroundDecorationText("Alpha");
	m_YamahaOSCBridgingSettings->setActiveToggleText("Use " + GetProtocolBridgingNiceName(PBT_YamahaOSC) + " Bridging");
	m_YamahaOSCBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_YamahaOSC) + " Bridging Settings");
	m_YamahaOSCBridgingSettings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/YamahaOSC.md"));
	m_YamahaOSCBridgingSettings->setHasActiveToggle(true);
	m_YamahaOSCBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_YamahaOSCBridgingSettings.get());

	m_YamahaOSCIpAddressEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_YamahaOSCIpAddressEdit->addListener(this);
	m_YamahaOSCIpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_YamahaOSCIpAddressLabel = std::make_unique<Label>("YamahaOSCIpAddressEdit", "IP Address");
	m_YamahaOSCIpAddressLabel->setJustificationType(Justification::centred);
	m_YamahaOSCIpAddressLabel->attachToComponent(m_YamahaOSCIpAddressEdit.get(), true);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCIpAddressLabel.get(), false, false);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCIpAddressEdit.get(), true, false);

	m_YamahaOSCListeningPortEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_YamahaOSCListeningPortEdit->addListener(this);
	m_YamahaOSCListeningPortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_YamahaOSCListeningPortLabel = std::make_unique<Label>("YamahaOSCListeningPortEdit", "Listening Port");
	m_YamahaOSCListeningPortLabel->setJustificationType(Justification::centred);
	m_YamahaOSCListeningPortLabel->attachToComponent(m_YamahaOSCListeningPortEdit.get(), true);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCListeningPortLabel.get(), false, false);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCListeningPortEdit.get(), true, false);

	m_YamahaOSCRemotePortEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_YamahaOSCRemotePortEdit->addListener(this);
	m_YamahaOSCRemotePortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_YamahaOSCRemotePortLabel = std::make_unique<Label>("YamahaOSCRemotePortEdit", "Remote Port");
	m_YamahaOSCRemotePortLabel->setJustificationType(Justification::centred);
	m_YamahaOSCRemotePortLabel->attachToComponent(m_YamahaOSCRemotePortEdit.get(), true);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCRemotePortLabel.get(), false, false);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCRemotePortEdit.get(), true, false);

	m_YamahaOSCMappingAreaSelect = std::make_unique<ComboBox>();
	m_YamahaOSCMappingAreaSelect->addListener(this);
	m_YamahaOSCMappingAreaSelect->addItemList({ "1", "2", "3", "4" }, MAI_First);
	m_YamahaOSCMappingAreaLabel = std::make_unique<Label>("YamahaOSCMappingAreaSelect", "Mapping Area");
	m_YamahaOSCMappingAreaLabel->setJustificationType(Justification::centred);
	m_YamahaOSCMappingAreaLabel->attachToComponent(m_YamahaOSCMappingAreaSelect.get(), true);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCMappingAreaLabel.get(), false, false);
	m_YamahaOSCBridgingSettings->addComponent(m_YamahaOSCMappingAreaSelect.get(), true, false);

	m_YamahaOSCBridgingSettings->resized();
}

/**
 * Helper method to create and setup objects for ADM OSC settings section
 */
void SettingsSectionsComponent::createADMOSCSettingsSection()
{
	// ADM-OSC settings section
	m_ADMOSCBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_ADMOSCBridgingSettings->setBackgroundDecorationText("Beta");
	m_ADMOSCBridgingSettings->setActiveToggleText("Use " + GetProtocolBridgingNiceName(PBT_ADMOSC) + " Bridging");
	m_ADMOSCBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_ADMOSC) + " Bridging Settings");
	m_ADMOSCBridgingSettings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/ADMOSC.md"));
	m_ADMOSCBridgingSettings->setHasActiveToggle(true);
	m_ADMOSCBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_ADMOSCBridgingSettings.get());

	m_ADMOSCIpAddressEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_ADMOSCIpAddressEdit->addListener(this);
	m_ADMOSCIpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_ADMOSCIpAddressLabel = std::make_unique<Label>("ADMOSCIpAddressEdit", "IP Address");
	m_ADMOSCIpAddressLabel->setJustificationType(Justification::centred);
	m_ADMOSCIpAddressLabel->attachToComponent(m_ADMOSCIpAddressEdit.get(), true);
	m_ADMOSCBridgingSettings->addComponent(m_ADMOSCIpAddressLabel.get(), false, false);
	m_ADMOSCBridgingSettings->addComponent(m_ADMOSCIpAddressEdit.get(), true, false);

	m_ADMOSCListeningPortEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_ADMOSCListeningPortEdit->addListener(this);
	m_ADMOSCListeningPortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_ADMOSCListeningPortLabel = std::make_unique<Label>("ADMOSCListeningPortEdit", "Listening Port");
	m_ADMOSCListeningPortLabel->setJustificationType(Justification::centred);
	m_ADMOSCListeningPortLabel->attachToComponent(m_ADMOSCListeningPortEdit.get(), true);
	m_ADMOSCBridgingSettings->addComponent(m_ADMOSCListeningPortLabel.get(), false, false);
	m_ADMOSCBridgingSettings->addComponent(m_ADMOSCListeningPortEdit.get(), true, false);

	m_ADMOSCRemotePortEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_ADMOSCRemotePortEdit->addListener(this);
	m_ADMOSCRemotePortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_ADMOSCRemotePortLabel = std::make_unique<Label>("ADMOSCRemotePortEdit", "Remote Port");
	m_ADMOSCRemotePortLabel->setJustificationType(Justification::centred);
	m_ADMOSCRemotePortLabel->attachToComponent(m_ADMOSCRemotePortEdit.get(), true);
	m_ADMOSCBridgingSettings->addComponent(m_ADMOSCRemotePortLabel.get(), false, false);
	m_ADMOSCBridgingSettings->addComponent(m_ADMOSCRemotePortEdit.get(), true, false);

	m_ADMOSCMappingAreaSelect = std::make_unique<ComboBox>();
	m_ADMOSCMappingAreaSelect->addListener(this);
	m_ADMOSCMappingAreaSelect->addItemList({ "1", "2", "3", "4" }, MAI_First);
	m_ADMOSCMappingAreaLabel = std::make_unique<Label>("ADMOSCMappingAreaSelect", "Mapping Area");
	m_ADMOSCMappingAreaLabel->setJustificationType(Justification::centred);
	m_ADMOSCMappingAreaLabel->attachToComponent(m_ADMOSCMappingAreaSelect.get(), true);
	m_ADMOSCBridgingSettings->addComponent(m_ADMOSCMappingAreaLabel.get(), false, false);
	m_ADMOSCBridgingSettings->addComponent(m_ADMOSCMappingAreaSelect.get(), true, false);

	m_ADMOSCCoordSysModContainer = std::make_unique<HorizontalLayouterComponent>();
	m_ADMOSCCoordSysModContainer->SetSpacing(5);
	m_ADMOSCSwapXYButton = std::make_unique<JUCEAppBasics::TextWithImageButton>("swap XY");
	m_ADMOSCSwapXYButton->setTooltip("Swap X/Y coordinates.");
	m_ADMOSCSwapXYButton->setImagePosition(Justification::centredLeft);
	m_ADMOSCSwapXYButton->setClickingTogglesState(true);
	m_ADMOSCSwapXYButton->addListener(this);
	m_ADMOSCCoordSysModContainer->AddComponent(m_ADMOSCSwapXYButton.get());
	m_ADMOSCInvertXButton = std::make_unique<JUCEAppBasics::TextWithImageButton>("inv. X");
	m_ADMOSCInvertXButton->setTooltip("Invert X coordinates.");
	m_ADMOSCInvertXButton->setImagePosition(Justification::centredLeft);
	m_ADMOSCInvertXButton->setClickingTogglesState(true);
	m_ADMOSCInvertXButton->addListener(this);
	m_ADMOSCCoordSysModContainer->AddComponent(m_ADMOSCInvertXButton.get());
	m_ADMOSCInvertYButton = std::make_unique<JUCEAppBasics::TextWithImageButton>("inv. Y");
	m_ADMOSCInvertYButton->setTooltip("Invert Y coordinates.");
	m_ADMOSCInvertYButton->setImagePosition(Justification::centredLeft);
	m_ADMOSCInvertYButton->setClickingTogglesState(true);
	m_ADMOSCInvertYButton->addListener(this);
	m_ADMOSCCoordSysModContainer->AddComponent(m_ADMOSCInvertYButton.get());
	m_ADMOSCCoordSysModLabel = std::make_unique<Label>("ADMOSCCoordSysModLabel", "XY coord. processing");
	m_ADMOSCCoordSysModLabel->setJustificationType(Justification::centred);
	m_ADMOSCCoordSysModLabel->attachToComponent(m_ADMOSCCoordSysModContainer.get(), true);
	m_ADMOSCBridgingSettings->addComponent(m_ADMOSCCoordSysModLabel.get(), false, false);
	m_ADMOSCBridgingSettings->addComponent(m_ADMOSCCoordSysModContainer.get(), true, false);

	m_ADMOSCxyMsgSndModeButton = std::make_unique<JUCEAppBasics::SplitButtonComponent>();
	m_ADMOSCxyMsgSndModeButton->addListener(this);
	m_ADMOSCxyMsgSndModeButtonIds[m_ADMOSCxyMsgSndModes[0]] = m_ADMOSCxyMsgSndModeButton->addButton(m_ADMOSCxyMsgSndModes[0]);
	m_ADMOSCxyMsgSndModeButtonIds[m_ADMOSCxyMsgSndModes[1]] = m_ADMOSCxyMsgSndModeButton->addButton(m_ADMOSCxyMsgSndModes[1]);
	m_ADMOSCxyMsgSndModeButton->setButtonDown(m_ADMOSCxyMsgSndModeButtonIds[m_ADMOSCxyMsgSndModes[0]]);
	m_ADMOSCxyMsgSndLabel = std::make_unique<Label>("ADMxyMessageModeButton", "XY msg. mode");
	m_ADMOSCxyMsgSndLabel->setJustificationType(Justification::centred);
	m_ADMOSCxyMsgSndLabel->attachToComponent(m_ADMOSCxyMsgSndModeButton.get(), true);
	m_ADMOSCBridgingSettings->addComponent(m_ADMOSCxyMsgSndLabel.get(), false, false);
	m_ADMOSCBridgingSettings->addComponent(m_ADMOSCxyMsgSndModeButton.get(), true, false);

	m_ADMOSCDisableSendingButton = std::make_unique<JUCEAppBasics::TextWithImageButton>("Disable return channel");
	m_ADMOSCDisableSendingButton->setTooltip("Disable sending of value changes to " + GetProtocolBridgingNiceName(PBT_ADMOSC) + " input devices.");
	m_ADMOSCDisableSendingButton->setImagePosition(Justification::centredLeft);
	m_ADMOSCDisableSendingButton->setClickingTogglesState(true);
	m_ADMOSCDisableSendingButton->addListener(this);
	m_ADMOSCBridgingSettings->addComponent(m_ADMOSCDisableSendingButton.get(), true, false);

	m_ADMOSCBridgingSettings->resized();
}

/**
 * Helper method to create and setup objects for Remap OSC settings section
 */
void SettingsSectionsComponent::createRemapOSCSettingsSection()
{
	// RemapOSC settings section
	m_RemapOSCBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_RemapOSCBridgingSettings->setBackgroundDecorationText("Alpha");
	m_RemapOSCBridgingSettings->setActiveToggleText("Use " + GetProtocolBridgingNiceName(PBT_RemapOSC) + " Bridging");
	m_RemapOSCBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_RemapOSC) + " Bridging Settings");
	m_RemapOSCBridgingSettings->setHelpUrl(URL(GetDocumentationBaseWebUrl() + "BridgingProtocols/RemapOSC.md"));
	m_RemapOSCBridgingSettings->setHasActiveToggle(true);
	m_RemapOSCBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_RemapOSCBridgingSettings.get());

	m_RemapOSCIpAddressEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_RemapOSCIpAddressEdit->addListener(this);
	m_RemapOSCIpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_RemapOSCIpAddressLabel = std::make_unique<Label>("RemapOSCIpAddressEdit", "IP Address");
	m_RemapOSCIpAddressLabel->setJustificationType(Justification::centred);
	m_RemapOSCIpAddressLabel->attachToComponent(m_RemapOSCIpAddressEdit.get(), true);
	m_RemapOSCBridgingSettings->addComponent(m_RemapOSCIpAddressLabel.get(), false, false);
	m_RemapOSCBridgingSettings->addComponent(m_RemapOSCIpAddressEdit.get(), true, false);

	m_RemapOSCListeningPortEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_RemapOSCListeningPortEdit->addListener(this);
	m_RemapOSCListeningPortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_RemapOSCListeningPortLabel = std::make_unique<Label>("RemapOSCListeningPortEdit", "Listening Port");
	m_RemapOSCListeningPortLabel->setJustificationType(Justification::centred);
	m_RemapOSCListeningPortLabel->attachToComponent(m_RemapOSCListeningPortEdit.get(), true);
	m_RemapOSCBridgingSettings->addComponent(m_RemapOSCListeningPortLabel.get(), false, false);
	m_RemapOSCBridgingSettings->addComponent(m_RemapOSCListeningPortEdit.get(), true, false);

	m_RemapOSCRemotePortEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_RemapOSCRemotePortEdit->addListener(this);
	m_RemapOSCRemotePortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_RemapOSCRemotePortLabel = std::make_unique<Label>("RemapOSCRemotePortEdit", "Remote Port");
	m_RemapOSCRemotePortLabel->setJustificationType(Justification::centred);
	m_RemapOSCRemotePortLabel->attachToComponent(m_RemapOSCRemotePortEdit.get(), true);
	m_RemapOSCBridgingSettings->addComponent(m_RemapOSCRemotePortLabel.get(), false, false);
	m_RemapOSCBridgingSettings->addComponent(m_RemapOSCRemotePortEdit.get(), true, false);

	m_RemapOSCAssignmentsEditor = std::make_unique<RemoteObjectToOscAssignerComponent>();
	m_RemapOSCAssignmentsEditor->onAssignmentsSet = [=](Component* sender, const std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>& roiToCustomOscAssis) { ignoreUnused(sender); handleRemapOscAssisSet(roiToCustomOscAssis); };
	m_RemapOSCAssignmentsLabel = std::make_unique<Label>("RemapOSCAssignmentsEditor", GetProtocolBridgingNiceName(PBT_RemapOSC));
	m_RemapOSCAssignmentsLabel->setJustificationType(Justification::centredLeft);
	m_RemapOSCAssignmentsLabel->attachToComponent(m_RemapOSCAssignmentsEditor.get(), true);
	m_RemapOSCBridgingSettings->addComponent(m_RemapOSCAssignmentsLabel.get(), false, false);
	m_RemapOSCBridgingSettings->addComponent(m_RemapOSCAssignmentsEditor.get(), true, false);

	m_RemapOSCDisableSendingButton = std::make_unique<JUCEAppBasics::TextWithImageButton>("Disable return channel");
	m_RemapOSCDisableSendingButton->setTooltip("Disable sending of value changes to " + GetProtocolBridgingNiceName(PBT_RemapOSC) + " input devices.");
	m_RemapOSCDisableSendingButton->setImagePosition(Justification::centredLeft);
	m_RemapOSCDisableSendingButton->setClickingTogglesState(true);
	m_RemapOSCDisableSendingButton->addListener(this);
	m_RemapOSCBridgingSettings->addComponent(m_RemapOSCDisableSendingButton.get(), true, false);

	m_RemapOSCBridgingSettings->resized();
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
	g.fillRect(juce::Rectangle<int>(0, 0, getLocalBounds().getWidth(), getLocalBounds().getHeight()));
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void SettingsSectionsComponent::resized()
{
	auto margin = 3.0f;

	auto minWidth = HeaderWithElmListComponent::m_attachedItemWidth + HeaderWithElmListComponent::m_layoutItemWidth;
	auto minHeight = 
		(m_GeneralSettings->getHeight() + (2 * margin))
		+ (m_DS100Settings->getHeight() + (2 * margin))
		+ (m_GenericOSCBridgingSettings->getHeight() + (2 * margin))
		+ (m_DAWPluginBridgingSettings->getHeight() + (2 * margin))
		+ (m_DiGiCoBridgingSettings->getHeight() + (2 * margin))
		+ (m_RTTrPMBridgingSettings->getHeight() + (2 * margin))
		+ (m_GenericMIDIBridgingSettings->getHeight() + (2 * margin))
		+ (m_ADMOSCBridgingSettings->getHeight() + (2 * margin))
		+ (m_YamahaOSCBridgingSettings->getHeight() + (2 * margin))
		+ (m_RemapOSCBridgingSettings->getHeight() + (2 * margin));

	auto bounds = getLocalBounds();
	if (bounds.getWidth() < minWidth || bounds.getHeight() < minHeight)
	{
		if (bounds.getWidth() < minWidth)
			bounds.setWidth(minWidth);
		if (bounds.getHeight() < static_cast<int>(minHeight))
			bounds.setHeight(static_cast<int>(minHeight));

		if (onContentMinRequiredSizeChangedCallback)
			onContentMinRequiredSizeChangedCallback(bounds);

		setBounds(bounds);
	}

	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.justifyContent = FlexBox::JustifyContent::flexStart;
	fb.items.addArray({
		FlexItem(*m_GeneralSettings.get())
			.withHeight(static_cast<float>(m_GeneralSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_DS100Settings.get())
			.withHeight(static_cast<float>(m_DS100Settings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_GenericOSCBridgingSettings.get())
			.withHeight(static_cast<float>(m_GenericOSCBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_DAWPluginBridgingSettings.get())
			.withHeight(static_cast<float>(m_DAWPluginBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_DiGiCoBridgingSettings.get())
			.withHeight(static_cast<float>(m_DiGiCoBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_RTTrPMBridgingSettings.get())
			.withHeight(static_cast<float>(m_RTTrPMBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_GenericMIDIBridgingSettings.get())
			.withHeight(static_cast<float>(m_GenericMIDIBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_ADMOSCBridgingSettings.get())
			.withHeight(static_cast<float>(m_ADMOSCBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_YamahaOSCBridgingSettings.get())
			.withHeight(static_cast<float>(m_YamahaOSCBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)),
		FlexItem(*m_RemapOSCBridgingSettings.get())
			.withHeight(static_cast<float>(m_RemapOSCBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)) });
	fb.performLayout(bounds);
}

/**
 * Reimplemented from component to change drawablebutton icon data.
 */
void SettingsSectionsComponent::lookAndFeelChanged()
{
	Component::lookAndFeelChanged();

	// Update drawable button images with updated lookAndFeel colours
	UpdateDrawableButtonImages(m_SoundObjectPageButton, BinaryData::vertical_split24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_MultisurfacePageButton, BinaryData::grain24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_MatrixIOPageButton, BinaryData::tune24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_ScenesPageButton, BinaryData::slideshow_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_EnSpacePageButton, BinaryData::sensors_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_StaticObjectsPollingButton, BinaryData::text_fields_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_StatisticsPageButton, BinaryData::show_chart24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_ADMOSCInvertXButton, BinaryData::flip_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_ADMOSCInvertYButton, BinaryData::flip_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_ADMOSCSwapXYButton, BinaryData::compare_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_ADMOSCDisableSendingButton, BinaryData::mobiledata_off24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_GenericOSCDisableSendingButton, BinaryData::mobiledata_off24px_svg, &getLookAndFeel());
#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
	UpdateDrawableButtonImages(m_ToggleFullscreenButton, BinaryData::open_in_full24px_svg, &getLookAndFeel());
#endif
	UpdateDrawableButtonImages(m_RemapOSCDisableSendingButton, BinaryData::mobiledata_off24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_RTTrPMXYSwapButton, BinaryData::compare_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_RTTrPMInvertXButton, BinaryData::flip_black_24dp_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_RTTrPMInvertYButton, BinaryData::flip_black_24dp_svg, &getLookAndFeel());
}

/**
 * Reimplemented from Button Listener.
 * @param button	The button object instance that was clicked
 */
void SettingsSectionsComponent::buttonClicked(Button* button)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	auto pageMgr = PageComponentManager::GetInstance();
	if (!pageMgr)
		return;

	// if the button that was changed is disabled, don't handle its change whatsoever
	if (button && !button->isEnabled())
		return;

	// General Settings section
	if (m_SoundObjectPageButton.get() == button || m_MultisurfacePageButton.get() == button || m_MatrixIOPageButton.get() == button || m_ScenesPageButton.get() == button || m_EnSpacePageButton.get() == button || m_StatisticsPageButton.get() == button)
	{
		auto enabledPages = std::vector<UIPageId>();
		if (m_SoundObjectPageButton && m_SoundObjectPageButton->getToggleState())
			enabledPages.push_back(UPI_Soundobjects);
		if (m_MultisurfacePageButton && m_MultisurfacePageButton->getToggleState())
			enabledPages.push_back(UPI_MultiSoundobjects);
		if (m_MatrixIOPageButton && m_MatrixIOPageButton->getToggleState())
			enabledPages.push_back(UPI_MatrixIOs);
		if (m_ScenesPageButton && m_ScenesPageButton->getToggleState())
			enabledPages.push_back(UPI_Scenes);
		if (m_EnSpacePageButton && m_EnSpacePageButton->getToggleState())
			enabledPages.push_back(UPI_EnSpace);
		if (m_StatisticsPageButton && m_StatisticsPageButton->getToggleState())
			enabledPages.push_back(UPI_Statistics);
		pageMgr->SetEnabledPages(enabledPages, false);
	}
	if (m_StaticObjectsPollingButton.get() == button)
		ctrl->SetStaticRemoteObjectsPollingEnabled(DCP_Settings, m_StaticObjectsPollingButton->getToggleState());
#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
	if (m_ToggleFullscreenButton.get() == button)
		pageMgr->SetFullscreenWindowMode(m_ToggleFullscreenButton->getToggleState(), false);
#endif

	// RTTrPM settings section
	else if (m_RTTrPMXYSwapButton && m_RTTrPMXYSwapButton.get() == button)
	{
		ctrl->SetBridgingXYAxisSwapped(PBT_BlacktraxRTTrPM, m_RTTrPMXYSwapButton->getToggleState() ? 1 : 0, juce::dontSendNotification);
	}
	else if (m_RTTrPMInvertXButton.get() == button)
	{
		ctrl->SetBridgingXAxisInverted(PBT_BlacktraxRTTrPM, m_RTTrPMInvertXButton->getToggleState() ? 1 : 0);
	}
	else if (m_RTTrPMInvertYButton.get() == button)
	{
		ctrl->SetBridgingYAxisInverted(PBT_BlacktraxRTTrPM, m_RTTrPMInvertYButton->getToggleState() ? 1 : 0);
	}

	// ADM-OSC Settings section
	else if (m_ADMOSCInvertXButton.get() == button)
	{
		ctrl->SetBridgingXAxisInverted(PBT_ADMOSC, m_ADMOSCInvertXButton->getToggleState() ? 1 : 0);
	}
	else if (m_ADMOSCInvertYButton.get() == button)
	{
		ctrl->SetBridgingYAxisInverted(PBT_ADMOSC, m_ADMOSCInvertYButton->getToggleState() ? 1 : 0);
	}
	else if (m_ADMOSCSwapXYButton.get() == button)
	{
		ctrl->SetBridgingXYAxisSwapped(PBT_ADMOSC, m_ADMOSCSwapXYButton->getToggleState() ? 1 : 0);
	}
	else if (m_ADMOSCDisableSendingButton.get() == button)
	{
		ctrl->SetBridgingDataSendingDisabled(PBT_ADMOSC, m_ADMOSCDisableSendingButton->getToggleState() ? 1 : 0);
	}

	// Generic OSC Settings section
	else if (m_GenericOSCDisableSendingButton.get() == button)
	{
		ctrl->SetBridgingDataSendingDisabled(PBT_GenericOSC, m_GenericOSCDisableSendingButton->getToggleState() ? 1 : 0);
	}

	// Remapped OSC Settings section
	else if (m_RemapOSCDisableSendingButton.get() == button)
	{
		ctrl->SetBridgingDataSendingDisabled(PBT_RemapOSC, m_RemapOSCDisableSendingButton->getToggleState() ? 1 : 0);
	}
}

/**
 * Reimplemented from SplitButtonComponent Listener.
 * @param buttonId	The uid of a button element of the splitbutton component
 */
void SettingsSectionsComponent::buttonClicked(JUCEAppBasics::SplitButtonComponent* button, std::uint64_t buttonId)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// if the button that was changed is disabled, don't handle its change whatsoever
	if (button && !button->isEnabled())
		return;

	// DS100 protocol settings section
	if (m_DS100ProtocolSelectButton && m_DS100ProtocolSelectButton.get() == button)
	{
		if (m_DS100ProtocolSelectButtonIds[m_DS100ProtocolSelectTexts[0].first] == buttonId) // OSC
		{
			ctrl->SetDS100ProtocolType(DCP_Settings, PT_OSCProtocol);
		}
		else if (m_DS100ProtocolSelectButtonIds[m_DS100ProtocolSelectTexts[1].first] == buttonId) // OCP1
		{
			ctrl->SetDS100ProtocolType(DCP_Settings, PT_OCP1Protocol);
		}
		else if (m_DS100ProtocolSelectButtonIds[m_DS100ProtocolSelectTexts[2].first] == buttonId) // NoProtocol
		{
			ctrl->SetDS100ProtocolType(DCP_Settings, PT_NoProtocol);
		}
	}

	// DS100 mode settings section
	else if (m_SecondDS100ModeButton && m_SecondDS100ModeButton.get() == button)
	{
		if (m_SecondDS100ModeButtonIds[m_SecondDS100Modes[0]] == buttonId) // Off
		{
			ctrl->SetExtensionMode(DCP_Settings, EM_Off);
		}
		else if (m_SecondDS100ModeButtonIds[m_SecondDS100Modes[1]] == buttonId) // Extend
		{
			ctrl->SetExtensionMode(DCP_Settings, EM_Extend);
		}
		else if (m_SecondDS100ModeButtonIds[m_SecondDS100Modes[2]] == buttonId) // Parallel
		{
			ctrl->SetExtensionMode(DCP_Settings, EM_Parallel);
		}
		else if (m_SecondDS100ModeButtonIds[m_SecondDS100Modes[3]] == buttonId) // Mirror
		{
			ctrl->SetExtensionMode(DCP_Settings, EM_Mirror);
		}
	}

	// DS100 parallel mode active DS100 1st/2nd selection
	else if (m_SecondDS100ParallelModeButton && m_SecondDS100ParallelModeButton.get() == button)
	{
		if (m_SecondDS100ParallelModeButtonIds[m_SecondDS100ParallelModes[0]] == buttonId) // 1st DS100 active
		{
			ctrl->SetActiveParallelModeDS100(DCP_Settings, APM_1st);
		}
		else if (m_SecondDS100ParallelModeButtonIds[m_SecondDS100ParallelModes[1]] == buttonId) // 2nd DS100 active
		{
			ctrl->SetActiveParallelModeDS100(DCP_Settings, APM_2nd);
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

	// ADM OSC settings section
	else if (m_ADMOSCxyMsgSndModeButton && m_ADMOSCxyMsgSndModeButton.get() == button)
	{
		if (m_ADMOSCxyMsgSndModeButtonIds[m_ADMOSCxyMsgSndModes[0]] == buttonId) // separate x and y messages
		{
			ctrl->SetBridgingXYMessageCombined(PBT_ADMOSC, false);
		}
		else if (m_ADMOSCxyMsgSndModeButtonIds[m_ADMOSCxyMsgSndModes[1]] == buttonId) // combined xy message
		{
			ctrl->SetBridgingXYMessageCombined(PBT_ADMOSC, true);
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
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// DS100 settings section
	if (m_DS100IntervalEdit && m_DS100IntervalEdit.get() == &editor)
		ctrl->SetRefreshInterval(DCP_Settings, m_DS100IntervalEdit->getText().getIntValue());
	else if (m_DS100IpAndPortEdit && m_DS100IpAndPortEdit.get() == &editor)
	{
		ctrl->SetDS100IpAndPort(DCP_Settings,
			juce::IPAddress(m_DS100IpAndPortEdit->getText().upToFirstOccurrenceOf(":", false, true)),
			m_DS100IpAndPortEdit->getText().fromFirstOccurrenceOf(":", false, true).getIntValue() % 0xffff);
	}
	else if (m_SecondDS100IpAndPortEdit && m_SecondDS100IpAndPortEdit.get() == &editor)
	{
		ctrl->SetSecondDS100IpAndPort(DCP_Settings,
			juce::IPAddress(m_SecondDS100IpAndPortEdit->getText().upToFirstOccurrenceOf(":", false, true)),
			m_SecondDS100IpAndPortEdit->getText().fromFirstOccurrenceOf(":", false, true).getIntValue() % 0xffff);
	}

	// DiGiCo settings section
	else if (m_DiGiCoIpAddressEdit && m_DiGiCoIpAddressEdit.get() == &editor)
		ctrl->SetBridgingIpAddress(PBT_DiGiCo, juce::IPAddress(m_DiGiCoIpAddressEdit->getText()));
	else if (m_DiGiCoListeningPortEdit && m_DiGiCoListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_DiGiCo, m_DiGiCoListeningPortEdit->getText().getIntValue() % 0xffff);
	else if (m_DiGiCoRemotePortEdit && m_DiGiCoRemotePortEdit.get() == &editor)
		ctrl->SetBridgingRemotePort(PBT_DiGiCo, m_DiGiCoRemotePortEdit->getText().getIntValue() % 0xffff);

	// DAWPlugin settings section
	else if (m_DAWPluginIpAddressEdit && m_DAWPluginIpAddressEdit.get() == &editor)
		ctrl->SetBridgingIpAddress(PBT_DAWPlugin, juce::IPAddress(m_DAWPluginIpAddressEdit->getText()));

	// RTTrPM settings section
	else if (m_RTTrPMListeningPortEdit && m_RTTrPMListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_BlacktraxRTTrPM, m_RTTrPMListeningPortEdit->getText().getIntValue() % 0xffff);
	else if (m_RTTrPMAbsoluteOriginXEdit && m_RTTrPMAbsoluteOriginYEdit && (m_RTTrPMAbsoluteOriginXEdit.get() == &editor || m_RTTrPMAbsoluteOriginYEdit.get() == &editor))
		ctrl->SetBridgingOriginOffset(PBT_BlacktraxRTTrPM, juce::Point<float>(m_RTTrPMAbsoluteOriginXEdit->getText().getFloatValue(), m_RTTrPMAbsoluteOriginYEdit->getText().getFloatValue()));
	else if (m_RTTrPMMappingPoint1XEdit && m_RTTrPMMappingPoint1YEdit && m_RTTrPMMappingPoint2XEdit && m_RTTrPMMappingPoint2YEdit
		&& (m_RTTrPMMappingPoint1XEdit.get() == &editor || m_RTTrPMMappingPoint1YEdit.get() == &editor || m_RTTrPMMappingPoint2XEdit.get() == &editor || m_RTTrPMMappingPoint2YEdit.get() == &editor))
	{
		// Bridging uses a range for x and y to map the RTTrPM incoming absolute coords to d&b relative.
		// For better usability, this is reflected by a min and a max point on the ui that has to be translated here
		ctrl->SetBridgingMappingRange(PBT_BlacktraxRTTrPM, std::make_pair(
			juce::Range<float>(m_RTTrPMMappingPoint1XEdit->getText().getFloatValue(), m_RTTrPMMappingPoint2XEdit->getText().getFloatValue()),
			juce::Range<float>(m_RTTrPMMappingPoint1YEdit->getText().getFloatValue(), m_RTTrPMMappingPoint2YEdit->getText().getFloatValue())));
	}

	// Generic OSC settings section
	else if (m_GenericOSCIpAddressEdit && m_GenericOSCIpAddressEdit.get() == &editor)
		ctrl->SetBridgingIpAddress(PBT_GenericOSC, juce::IPAddress(m_GenericOSCIpAddressEdit->getText()));
	else if (m_GenericOSCListeningPortEdit && m_GenericOSCListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_GenericOSC, m_GenericOSCListeningPortEdit->getText().getIntValue() % 0xffff);
	else if (m_GenericOSCRemotePortEdit && m_GenericOSCRemotePortEdit.get() == &editor)
		ctrl->SetBridgingRemotePort(PBT_GenericOSC, m_GenericOSCRemotePortEdit->getText().getIntValue() % 0xffff);

	// ADM OSC settings section
	else if (m_ADMOSCIpAddressEdit && m_ADMOSCIpAddressEdit.get() == &editor)
		ctrl->SetBridgingIpAddress(PBT_ADMOSC, juce::IPAddress(m_ADMOSCIpAddressEdit->getText()));
	else if (m_ADMOSCListeningPortEdit && m_ADMOSCListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_ADMOSC, m_ADMOSCListeningPortEdit->getText().getIntValue() % 0xffff);
	else if (m_ADMOSCRemotePortEdit && m_ADMOSCRemotePortEdit.get() == &editor)
		ctrl->SetBridgingRemotePort(PBT_ADMOSC, m_ADMOSCRemotePortEdit->getText().getIntValue() % 0xffff);
	else if (m_ADMOSCListeningPortEdit && m_ADMOSCListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_ADMOSC, m_ADMOSCListeningPortEdit->getText().getIntValue() % 0xffff);

	// Yamaha OSC settings section
	else if (m_YamahaOSCIpAddressEdit && m_YamahaOSCIpAddressEdit.get() == &editor)
		ctrl->SetBridgingIpAddress(PBT_YamahaOSC, juce::IPAddress(m_YamahaOSCIpAddressEdit->getText()));
	else if (m_YamahaOSCListeningPortEdit && m_YamahaOSCListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_YamahaOSC, m_YamahaOSCListeningPortEdit->getText().getIntValue() % 0xffff);
	else if (m_YamahaOSCRemotePortEdit && m_YamahaOSCRemotePortEdit.get() == &editor)
		ctrl->SetBridgingRemotePort(PBT_YamahaOSC, m_YamahaOSCRemotePortEdit->getText().getIntValue() % 0xffff);
	else if (m_YamahaOSCListeningPortEdit && m_YamahaOSCListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_YamahaOSC, m_YamahaOSCListeningPortEdit->getText().getIntValue() % 0xffff);

	// Remap OSC settings section
	else if (m_RemapOSCIpAddressEdit && m_RemapOSCIpAddressEdit.get() == &editor)
		ctrl->SetBridgingIpAddress(PBT_RemapOSC, juce::IPAddress(m_RemapOSCIpAddressEdit->getText()));
	else if (m_RemapOSCListeningPortEdit && m_RemapOSCListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_RemapOSC, m_RemapOSCListeningPortEdit->getText().getIntValue() % 0xffff);
	else if (m_RemapOSCRemotePortEdit && m_RemapOSCRemotePortEdit.get() == &editor)
		ctrl->SetBridgingRemotePort(PBT_RemapOSC, m_RemapOSCRemotePortEdit->getText().getIntValue() % 0xffff);
	else if (m_RemapOSCListeningPortEdit && m_RemapOSCListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_RemapOSC, m_RemapOSCListeningPortEdit->getText().getIntValue() % 0xffff);

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
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	auto pageMgr = PageComponentManager::GetInstance();
	if (!pageMgr)
		return;
    
    // if the combobox that was changed is disabled, don't handle its change whatsoever
    if (comboBox && !comboBox->isEnabled())
        return;

	// General settings section
	if (m_LookAndFeelSelect && m_LookAndFeelSelect.get() == comboBox)
	{
		auto lookAndFeelType = static_cast<DbLookAndFeelBase::LookAndFeelType>(m_LookAndFeelSelect->getSelectedId());
		jassert(lookAndFeelType > DbLookAndFeelBase::LookAndFeelType::LAFT_InvalidFirst && lookAndFeelType < DbLookAndFeelBase::LookAndFeelType::LAFT_InvalidLast);
		pageMgr->SetLookAndFeelType(lookAndFeelType, false);
	}

	// RTTrPM settings section
	else if (m_RTTrPMMappingAreaSelect && m_RTTrPMMappingAreaSelect.get() == comboBox)
	{
		m_previousRTTrPMMappingAreaId = m_RTTrPMMappingAreaSelect->getSelectedId();
		ctrl->SetBridgingMappingArea(PBT_BlacktraxRTTrPM, m_previousRTTrPMMappingAreaId);
	}
	else if (m_RTTrPMDataTypeSelect && m_RTTrPMDataTypeSelect.get() == comboBox)
	{
		auto selectedDataType = m_RTTrPMDataTypeSelect->getItemText(m_RTTrPMDataTypeSelect->getSelectedId() - 1);
	 	ctrl->SetBridgingModuleTypeIdentifier(PBT_BlacktraxRTTrPM, m_RTTrPMDataTypes.at(selectedDataType), dontSendNotification);
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

	// ADM OSC settings section
	else if (m_ADMOSCMappingAreaSelect && m_ADMOSCMappingAreaSelect.get() == comboBox)
		ctrl->SetBridgingMappingArea(PBT_ADMOSC, m_ADMOSCMappingAreaSelect->getSelectedId());

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
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	ProtocolBridgingType sectionType = PBT_None;
	if (settingsSection == m_DiGiCoBridgingSettings.get())
		sectionType = PBT_DiGiCo;
	if (settingsSection == m_DAWPluginBridgingSettings.get())
		sectionType = PBT_DAWPlugin;
	else if (settingsSection == m_RTTrPMBridgingSettings.get())
		sectionType = PBT_BlacktraxRTTrPM;
	else if (settingsSection == m_GenericOSCBridgingSettings.get())
		sectionType = PBT_GenericOSC;
	else if (settingsSection == m_GenericMIDIBridgingSettings.get())
		sectionType = PBT_GenericMIDI;
	else if (settingsSection == m_ADMOSCBridgingSettings.get())
		sectionType = PBT_ADMOSC;
	else if (settingsSection == m_YamahaOSCBridgingSettings.get())
		sectionType = PBT_YamahaOSC;
	else if (settingsSection == m_RemapOSCBridgingSettings.get())
		sectionType = PBT_RemapOSC;

	if (activeState)
		ctrl->SetActiveProtocolBridging(ctrl->GetActiveProtocolBridging() | sectionType);
	else
		ctrl->SetActiveProtocolBridging(ctrl->GetActiveProtocolBridging() & ~sectionType);

	resized();

	if (onContentSizesChangedCallback)
		onContentSizesChangedCallback();
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
	processUpdatedGeneralConfig();
	processUpdatedDS100Config();
	processUpdatedDiGiCoConfig();
	processUpdatedDAWPluginConfig();
	processUpdatedRTTrPMConfig();
	processUpdatedGenericOSCConfig();
	processUpdatedGenericMIDIConfig();
	processUpdatedADMOSCConfig();
	processUpdatedYamahaOSCConfig();
	processUpdatedRemapOSCConfig();

	resized();

	if (onContentSizesChangedCallback)
		onContentSizesChangedCallback();
}

/**
 * Helper method to update objects for general settings section with updated config
 */
void SettingsSectionsComponent::processUpdatedGeneralConfig()
{
	auto pageMgr = PageComponentManager::GetInstance();
	if (!pageMgr)
		return;

	// General settings section
	if (m_SoundObjectPageButton)
		m_SoundObjectPageButton->setToggleState(std::find(pageMgr->GetEnabledPages().begin(), pageMgr->GetEnabledPages().end(), UPI_Soundobjects) != pageMgr->GetEnabledPages().end(), dontSendNotification);
	if (m_MultisurfacePageButton)
		m_MultisurfacePageButton->setToggleState(std::find(pageMgr->GetEnabledPages().begin(), pageMgr->GetEnabledPages().end(), UPI_MultiSoundobjects) != pageMgr->GetEnabledPages().end(), dontSendNotification);
	if (m_MatrixIOPageButton)
		m_MatrixIOPageButton->setToggleState(std::find(pageMgr->GetEnabledPages().begin(), pageMgr->GetEnabledPages().end(), UPI_MatrixIOs) != pageMgr->GetEnabledPages().end(), dontSendNotification);
	if (m_ScenesPageButton)
		m_ScenesPageButton->setToggleState(std::find(pageMgr->GetEnabledPages().begin(), pageMgr->GetEnabledPages().end(), UPI_Scenes) != pageMgr->GetEnabledPages().end(), dontSendNotification);
	if (m_EnSpacePageButton)
		m_EnSpacePageButton->setToggleState(std::find(pageMgr->GetEnabledPages().begin(), pageMgr->GetEnabledPages().end(), UPI_EnSpace) != pageMgr->GetEnabledPages().end(), dontSendNotification);
	if (m_StatisticsPageButton)
		m_StatisticsPageButton->setToggleState(std::find(pageMgr->GetEnabledPages().begin(), pageMgr->GetEnabledPages().end(), UPI_Statistics) != pageMgr->GetEnabledPages().end(), dontSendNotification);
	if (m_LookAndFeelSelect)
		m_LookAndFeelSelect->setSelectedId(pageMgr->GetLookAndFeelType(), dontSendNotification);
#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
	if (m_ToggleFullscreenButton)
		m_ToggleFullscreenButton->setToggleState(pageMgr->IsFullscreenWindowMode(), dontSendNotification);
#endif

	auto ctrl = Controller::GetInstance();
	if (ctrl && m_StaticObjectsPollingButton)
		m_StaticObjectsPollingButton->setToggleState(ctrl->IsStaticRemoteObjectsPollingEnabled(), dontSendNotification);
}

/**
 * Helper method to update objects for DS100 settings section with updated config
 * This includes enabling/disabling elements depending on the mode config for a second DS100
 * and what protocol type is selected to be used to communicate with DS100(s).
 */
void SettingsSectionsComponent::processUpdatedDS100Config()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// DS100 settings section
	if (m_DS100ProtocolSelectButton)
	{
		auto newActiveButtonId = m_DS100ProtocolSelectButtonIds[m_DS100ProtocolSelectTexts[0].first];
		if (ctrl->GetDS100ProtocolType() == PT_OCP1Protocol)
			newActiveButtonId = m_DS100ProtocolSelectButtonIds[m_DS100ProtocolSelectTexts[1].first];
		else if (ctrl->GetDS100ProtocolType() == PT_NoProtocol)
			newActiveButtonId = m_DS100ProtocolSelectButtonIds[m_DS100ProtocolSelectTexts[2].first];
		m_DS100ProtocolSelectButton->setButtonDown(newActiveButtonId);
	}
	if (m_DS100IntervalEdit)
	{
		m_DS100IntervalEdit->setText((ctrl->GetDS100ProtocolType() == PT_OSCProtocol) ? (juce::String(ctrl->GetRefreshInterval()) + UNIT_MILLISECOND) : "");
		m_DS100IntervalEdit->setEnabled(ctrl->GetDS100ProtocolType() == PT_OSCProtocol);
	}
	if (m_DS100IntervalLabel)
		m_DS100IntervalLabel->setEnabled(ctrl->GetDS100ProtocolType() == PT_OSCProtocol);
	if (m_DS100IpAndPortEdit)
	{
		auto ipAndPort = ctrl->GetDS100IpAndPort();
		m_DS100IpAndPortEdit->setText(ipAndPort.first.toString() + ":" + juce::String(ipAndPort.second));
		m_DS100IpAndPortEdit->setEnabled(ctrl->GetDS100ProtocolType() != PT_NoProtocol);
	}
	if (m_DS100IpAndPortLabel)
		m_DS100IpAndPortLabel->setEnabled(ctrl->GetDS100ProtocolType() != PT_NoProtocol);
	if (m_DS100ZeroconfDiscovery)
	{
		m_DS100ZeroconfDiscovery->setEnabled(ctrl->GetDS100ProtocolType() != PT_NoProtocol);
		if (ctrl->GetDS100ProtocolType() == PT_OCP1Protocol)
		{
			m_DS100ZeroconfDiscovery->removeDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OSC);
			m_DS100ZeroconfDiscovery->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OCA);
		}
		else
		{
			m_DS100ZeroconfDiscovery->removeDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OCA);
			m_DS100ZeroconfDiscovery->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OSC);
		}
		m_DS100ZeroconfDiscovery->resized();
	}
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
		m_SecondDS100ModeButton->setEnabled(ctrl->GetDS100ProtocolType() != PT_NoProtocol);
	}
	if (m_SecondDS100ModeLabel)
		m_SecondDS100ModeLabel->setEnabled(ctrl->GetDS100ProtocolType() != PT_NoProtocol);
	if (m_SecondDS100ParallelModeButton)
	{
		m_SecondDS100ParallelModeButton->setEnabled(ctrl->GetExtensionMode() == EM_Parallel && ctrl->GetDS100ProtocolType() != PT_NoProtocol);

		auto newActiveButtonId = m_SecondDS100ParallelModeButtonIds[m_SecondDS100ParallelModes[0]];
		if (ctrl->GetActiveParallelModeDS100() == APM_2nd)
			newActiveButtonId = m_SecondDS100ParallelModeButtonIds[m_SecondDS100ParallelModes[1]];
		m_SecondDS100ParallelModeButton->setButtonDown(newActiveButtonId);
	}
	if (m_SecondDS100ParallelModeLabel)
		m_SecondDS100ParallelModeLabel->setEnabled(ctrl->GetExtensionMode() == EM_Parallel && ctrl->GetDS100ProtocolType() != PT_NoProtocol);
	if (m_SecondDS100IpAndPortEdit)
	{
		auto ipAndPort = ctrl->GetSecondDS100IpAndPort();
		m_SecondDS100IpAndPortEdit->setText(ctrl->GetExtensionMode() != EM_Off ? (ipAndPort.first.toString() + ":" + juce::String(ipAndPort.second)) : "");
		m_SecondDS100IpAndPortEdit->setEnabled(ctrl->GetExtensionMode() != EM_Off && ctrl->GetDS100ProtocolType() != PT_NoProtocol);
	}
	if (m_SecondDS100IpAndPortLabel)
		m_SecondDS100IpAndPortLabel->setEnabled(ctrl->GetExtensionMode() != EM_Off && ctrl->GetDS100ProtocolType() != PT_NoProtocol);
	if (m_SecondDS100ZeroconfDiscovery)
	{
		m_SecondDS100ZeroconfDiscovery->setEnabled(ctrl->GetExtensionMode() != EM_Off && ctrl->GetDS100ProtocolType() != PT_NoProtocol);
		if (ctrl->GetDS100ProtocolType() == PT_OCP1Protocol)
		{
			m_SecondDS100ZeroconfDiscovery->removeDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OSC);
			m_SecondDS100ZeroconfDiscovery->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OCA);
		}
		else
		{
			m_SecondDS100ZeroconfDiscovery->removeDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OCA);
			m_SecondDS100ZeroconfDiscovery->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OSC);
		}
		m_SecondDS100ZeroconfDiscovery->resized();
	}
	if (m_DS100ProjectDummyDataLoader)
		m_DS100ProjectDummyDataLoader->setEnabled(ctrl->GetDS100ProtocolType() == PT_NoProtocol);
	if (m_DS100ProjectDummyDataLabel)
		m_DS100ProjectDummyDataLabel->setEnabled(ctrl->GetDS100ProtocolType() == PT_NoProtocol);
}

/**
 * Helper method to update objects for DiGiCo settings section with updated config
 */
void SettingsSectionsComponent::processUpdatedDiGiCoConfig()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// DiGiCo settings section
	auto DiGiCoBridgingActive = (ctrl->GetActiveProtocolBridging() & PBT_DiGiCo) == PBT_DiGiCo;
	if (m_DiGiCoBridgingSettings)
		m_DiGiCoBridgingSettings->setToggleActiveState(DiGiCoBridgingActive);
	if (m_DiGiCoIpAddressEdit)
		m_DiGiCoIpAddressEdit->setText(ctrl->GetBridgingIpAddress(PBT_DiGiCo).toString());
	if (m_DiGiCoListeningPortEdit)
		m_DiGiCoListeningPortEdit->setText(String(ctrl->GetBridgingListeningPort(PBT_DiGiCo)), false);
	if (m_DiGiCoRemotePortEdit)
		m_DiGiCoRemotePortEdit->setText(String(ctrl->GetBridgingRemotePort(PBT_DiGiCo)), false);
}

/**
 * Helper method to update objects for DAWPlugin settings section with updated config
 */
void SettingsSectionsComponent::processUpdatedDAWPluginConfig()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// DAWPlugin settings section
	auto DAWPluginBridgingActive = (ctrl->GetActiveProtocolBridging() & PBT_DAWPlugin) == PBT_DAWPlugin;
	if (m_DAWPluginBridgingSettings)
		m_DAWPluginBridgingSettings->setToggleActiveState(DAWPluginBridgingActive);
	if (m_DAWPluginIpAddressEdit)
		m_DAWPluginIpAddressEdit->setText(ctrl->GetBridgingIpAddress(PBT_DAWPlugin).toString());
}

/**
 * Helper method to update objects for RTTrPM settings section with updated config
 */
void SettingsSectionsComponent::processUpdatedRTTrPMConfig()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

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

	if (m_RTTrPMXYSwapButton)
		m_RTTrPMXYSwapButton->setToggleState(ctrl->GetBridgingXYAxisSwapped(PBT_BlacktraxRTTrPM), juce::dontSendNotification);
	if (m_RTTrPMInvertXButton)
		m_RTTrPMInvertXButton->setToggleState(1 == ctrl->GetBridgingXAxisInverted(PBT_BlacktraxRTTrPM), dontSendNotification);
	if (m_RTTrPMInvertYButton)
		m_RTTrPMInvertYButton->setToggleState(1 == ctrl->GetBridgingYAxisInverted(PBT_BlacktraxRTTrPM), dontSendNotification);

	auto RTTrPMAbsoluteOrigin = ctrl->GetBridgingOriginOffset(PBT_BlacktraxRTTrPM);
	if (m_RTTrPMAbsoluteOriginLabel)
		m_RTTrPMAbsoluteOriginLabel->setEnabled(RTTrPMMappingAreaId == MAI_Invalid);
	if (m_RTTrPMAbsoluteOriginXEdit)
	{
		m_RTTrPMAbsoluteOriginXEdit->setText(juce::String(RTTrPMAbsoluteOrigin.getX()) + " m");
		m_RTTrPMAbsoluteOriginXEdit->setEnabled(RTTrPMMappingAreaId == MAI_Invalid);
	}
	if (m_RTTrPMAbsoluteOriginXLabel)
		m_RTTrPMAbsoluteOriginXLabel->setEnabled(RTTrPMMappingAreaId == MAI_Invalid);
	if (m_RTTrPMAbsoluteOriginYEdit)
	{
		m_RTTrPMAbsoluteOriginYEdit->setText(juce::String(RTTrPMAbsoluteOrigin.getY()) + " m");
		m_RTTrPMAbsoluteOriginYEdit->setEnabled(RTTrPMMappingAreaId == MAI_Invalid);
	}
	if (m_RTTrPMAbsoluteOriginYLabel)
		m_RTTrPMAbsoluteOriginYLabel->setEnabled(RTTrPMMappingAreaId == MAI_Invalid);
	if (m_RTTrPMMappingAreaSelect)
	{
		m_RTTrPMMappingAreaSelect->setSelectedId(RTTrPMMappingAreaId, sendNotificationAsync);
		m_RTTrPMMappingAreaSelect->setEnabled(RTTrPMMappingAreaId != MAI_Invalid);
	}
	if (m_RTTrPMMappingAreaLabel)
		m_RTTrPMMappingAreaLabel->setEnabled(RTTrPMMappingAreaId != MAI_Invalid);

	auto RTTrPMMappingRange = ctrl->GetBridgingMappingRange(PBT_BlacktraxRTTrPM);
	if (m_RTTrPMMappingPoint1XEdit && m_RTTrPMMappingPoint1YEdit)
	{
		// Bridging uses a range for y to map the RTTrPM incoming absolute coords to d&b relative.
		// For better usability, this is reflected by a min point on the ui that has to be translated here
		m_RTTrPMMappingPoint1XEdit->setText(juce::String(RTTrPMMappingRange.first.getStart()) + " m");
		m_RTTrPMMappingPoint1XEdit->setEnabled(RTTrPMMappingAreaId != MAI_Invalid);
		m_RTTrPMMappingPoint1YEdit->setText(juce::String(RTTrPMMappingRange.second.getStart()) + " m");
		m_RTTrPMMappingPoint1YEdit->setEnabled(RTTrPMMappingAreaId != MAI_Invalid);
	}
	if (m_RTTrPMMappingPoint1Label)
		m_RTTrPMMappingPoint1Label->setEnabled(RTTrPMMappingAreaId != MAI_Invalid);
	if (m_RTTrPMMappingPoint2XEdit && m_RTTrPMMappingPoint2YEdit)
	{
		// Bridging uses a range for x to map the RTTrPM incoming absolute coords to d&b relative.
		// For better usability, this is reflected by a max point on the ui that has to be translated here
		m_RTTrPMMappingPoint2XEdit->setText(juce::String(RTTrPMMappingRange.first.getEnd()) + " m");
		m_RTTrPMMappingPoint2XEdit->setEnabled(RTTrPMMappingAreaId != MAI_Invalid);
		m_RTTrPMMappingPoint2YEdit->setText(juce::String(RTTrPMMappingRange.second.getEnd()) + " m");
		m_RTTrPMMappingPoint2YEdit->setEnabled(RTTrPMMappingAreaId != MAI_Invalid);
	}
	if (m_RTTrPMMappingPoint2Label)
		m_RTTrPMMappingPoint2Label->setEnabled(RTTrPMMappingAreaId != MAI_Invalid);

	if (m_RTTrPMBeaconIdxAssignmentsEditor)
	{
		auto assignments = ctrl->GetBridgingChannelRemapAssignments(PBT_BlacktraxRTTrPM);
		m_RTTrPMBeaconIdxAssignmentsEditor->setCurrentIndexToChannelAssignments(assignments);
	}
	
	auto RTTrPMModuleTypeIdentifier = ctrl->GetBridgingModuleTypeIdentifier(PBT_BlacktraxRTTrPM);
	if (m_RTTrPMDataTypeSelect)
	{
		if (m_RTTrPMDataTypes.at("Centroid Position") == RTTrPMModuleTypeIdentifier)
			m_RTTrPMDataTypeSelect->setSelectedItemIndex(0);
		else
			m_RTTrPMDataTypeSelect->setSelectedItemIndex(1);
	}
}

/**
 *  Helper method to update objects for Generic OSC settings section with updated config
 */
void SettingsSectionsComponent::processUpdatedGenericOSCConfig()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// Generic OSC settings section
	auto GenericOSCBridgingActive = (ctrl->GetActiveProtocolBridging() & PBT_GenericOSC) == PBT_GenericOSC;
	if (m_GenericOSCBridgingSettings)
		m_GenericOSCBridgingSettings->setToggleActiveState(GenericOSCBridgingActive);
	if (m_GenericOSCIpAddressEdit)
		m_GenericOSCIpAddressEdit->setText(ctrl->GetBridgingIpAddress(PBT_GenericOSC).toString());
	if (m_GenericOSCListeningPortEdit)
		m_GenericOSCListeningPortEdit->setText(String(ctrl->GetBridgingListeningPort(PBT_GenericOSC)), false);
	if (m_GenericOSCRemotePortEdit)
		m_GenericOSCRemotePortEdit->setText(String(ctrl->GetBridgingRemotePort(PBT_GenericOSC)), false);
	if (m_GenericOSCDisableSendingButton)
		m_GenericOSCDisableSendingButton->setToggleState(1 == ctrl->GetBridgingDataSendingDisabled(PBT_GenericOSC), dontSendNotification);
}

/**
 * Helper method to update objects for Generic MIDI settings section with updated config
 */
void SettingsSectionsComponent::processUpdatedGenericMIDIConfig()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

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
	if (m_GenericMIDISelectionSelectLearner)
	{
		m_GenericMIDISelectionSelectLearner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDISelectionSelectLearner->setCurrentMidiAssi(ctrl->GetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDISelectionSelectLearner->getReferredId())));
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
	if (m_GenericMIDIMatrixInputGainLearner)
	{
		m_GenericMIDIMatrixInputGainLearner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDIMatrixInputGainLearner->setCurrentMidiAssi(ctrl->GetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDIMatrixInputGainLearner->getReferredId())));
	}
	if (m_GenericMIDIMatrixInputMuteLearner)
	{
		m_GenericMIDIMatrixInputMuteLearner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDIMatrixInputMuteLearner->setCurrentMidiAssi(ctrl->GetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDIMatrixInputMuteLearner->getReferredId())));
	}
	if (m_GenericMIDIMatrixOutputGainLearner)
	{
		m_GenericMIDIMatrixOutputGainLearner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDIMatrixOutputGainLearner->setCurrentMidiAssi(ctrl->GetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDIMatrixOutputGainLearner->getReferredId())));
	}
	if (m_GenericMIDIMatrixOutputMuteLearner)
	{
		m_GenericMIDIMatrixOutputMuteLearner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDIMatrixOutputMuteLearner->setCurrentMidiAssi(ctrl->GetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDIMatrixOutputMuteLearner->getReferredId())));
	}
	if (m_GenericMIDINextSceneLearner)
	{
		m_GenericMIDINextSceneLearner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDINextSceneLearner->setCurrentMidiAssi(ctrl->GetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDINextSceneLearner->getReferredId())));
	}
	if (m_GenericMIDIPrevSceneLearner)
	{
		m_GenericMIDIPrevSceneLearner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDIPrevSceneLearner->setCurrentMidiAssi(ctrl->GetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDIPrevSceneLearner->getReferredId())));
	}
	if (m_GenericMIDIRecallSceneAssigner)
	{
		m_GenericMIDIRecallSceneAssigner->setSelectedDeviceIdentifier(ctrl->GetBridgingInputDeviceIdentifier(PBT_GenericMIDI));
		m_GenericMIDIRecallSceneAssigner->setCurrentScenesToMidiAssignments(ctrl->GetBridgingScenesToMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDIRecallSceneAssigner->getReferredId())));
	}
}

/**
 * Helper method to update objects for Yamaha OSC settings section with updated config
 */
void SettingsSectionsComponent::processUpdatedYamahaOSCConfig()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// Yamaha OSC settings section
	auto YamahaOSCBridgingActive = (ctrl->GetActiveProtocolBridging() & PBT_YamahaOSC) == PBT_YamahaOSC;
	if (m_YamahaOSCBridgingSettings)
		m_YamahaOSCBridgingSettings->setToggleActiveState(YamahaOSCBridgingActive);
	if (m_YamahaOSCIpAddressEdit)
		m_YamahaOSCIpAddressEdit->setText(ctrl->GetBridgingIpAddress(PBT_YamahaOSC).toString());
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
 * Helper method to update objects for ADM OSC settings section with updated config
 */
void SettingsSectionsComponent::processUpdatedADMOSCConfig()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// ADM OSC settings section
	auto ADMOSCBridgingActive = (ctrl->GetActiveProtocolBridging() & PBT_ADMOSC) == PBT_ADMOSC;
	if (m_ADMOSCBridgingSettings)
		m_ADMOSCBridgingSettings->setToggleActiveState(ADMOSCBridgingActive);
	if (m_ADMOSCIpAddressEdit)
		m_ADMOSCIpAddressEdit->setText(ctrl->GetBridgingIpAddress(PBT_ADMOSC).toString());
	if (m_ADMOSCListeningPortEdit)
		m_ADMOSCListeningPortEdit->setText(String(ctrl->GetBridgingListeningPort(PBT_ADMOSC)), false);
	if (m_ADMOSCRemotePortEdit)
		m_ADMOSCRemotePortEdit->setText(String(ctrl->GetBridgingRemotePort(PBT_ADMOSC)), false);
	if (m_ADMOSCMappingAreaSelect)
	{
		m_ADMOSCMappingAreaSelect->setSelectedId(ctrl->GetBridgingMappingArea(PBT_ADMOSC), sendNotificationAsync);
		m_ADMOSCMappingAreaSelect->setEnabled((ctrl->GetBridgingMappingArea(PBT_ADMOSC) != MAI_Invalid));
	}
	if (m_ADMOSCMappingAreaLabel)
		m_ADMOSCMappingAreaLabel->setEnabled((ctrl->GetBridgingMappingArea(PBT_ADMOSC) != MAI_Invalid));

	if (m_ADMOSCInvertXButton)
		m_ADMOSCInvertXButton->setToggleState(1 == ctrl->GetBridgingXAxisInverted(PBT_ADMOSC), dontSendNotification);
	if (m_ADMOSCInvertYButton)
		m_ADMOSCInvertYButton->setToggleState(1 == ctrl->GetBridgingYAxisInverted(PBT_ADMOSC), dontSendNotification);
	if (m_ADMOSCSwapXYButton)
		m_ADMOSCSwapXYButton->setToggleState(1 == ctrl->GetBridgingXYAxisSwapped(PBT_ADMOSC), dontSendNotification);
	if (m_ADMOSCDisableSendingButton)
		m_ADMOSCDisableSendingButton->setToggleState(1 == ctrl->GetBridgingDataSendingDisabled(PBT_ADMOSC), dontSendNotification);

	if (m_ADMOSCxyMsgSndModeButton)
	{
		auto xyMessageCombined = ctrl->GetBridgingXYMessageCombined(PBT_ADMOSC);

		auto newActiveButtonId = m_ADMOSCxyMsgSndModeButtonIds[m_ADMOSCxyMsgSndModes[0]];
		if (xyMessageCombined)
			newActiveButtonId = m_ADMOSCxyMsgSndModeButtonIds[m_ADMOSCxyMsgSndModes[1]];

		m_ADMOSCxyMsgSndModeButton->setButtonDown(newActiveButtonId);
	}
}

/**
 * Helper method to update objects for Remap OSC settings section with updated config
 */
void SettingsSectionsComponent::processUpdatedRemapOSCConfig()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// Remap OSC settings section
	auto remapOSCBridgingActive = (ctrl->GetActiveProtocolBridging() & PBT_RemapOSC) == PBT_RemapOSC;
	if (m_RemapOSCBridgingSettings)
		m_RemapOSCBridgingSettings->setToggleActiveState(remapOSCBridgingActive);
	if (m_RemapOSCIpAddressEdit)
		m_RemapOSCIpAddressEdit->setText(ctrl->GetBridgingIpAddress(PBT_RemapOSC).toString());
	if (m_RemapOSCListeningPortEdit)
		m_RemapOSCListeningPortEdit->setText(String(ctrl->GetBridgingListeningPort(PBT_RemapOSC)), false);
	if (m_RemapOSCRemotePortEdit)
		m_RemapOSCRemotePortEdit->setText(String(ctrl->GetBridgingRemotePort(PBT_RemapOSC)), false);

	if (m_RemapOSCAssignmentsEditor)
	{
		auto assignments = ctrl->GetBridgingOscRemapAssignments(PBT_RemapOSC);
		m_RemapOSCAssignmentsEditor->setCurrentRemoteObjecToOscAssignments(assignments);
	}

	if (m_RemapOSCDisableSendingButton)
		m_RemapOSCDisableSendingButton->setToggleState(1 == ctrl->GetBridgingDataSendingDisabled(PBT_RemapOSC), dontSendNotification);

}

/**
 * Callback method to be registered with ZeroconfDiscoveryComponent to handle user input regarding service selection.
 * @param type	The service type that was selected
 * @param info	The detailed info on the service that was selected
 */
void SettingsSectionsComponent::handleDS100ServiceSelected(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, ZeroconfSearcher::ZeroconfSearcher::ServiceInfo* info)
{
	ignoreUnused(type);

	if (info)
	{
		m_DS100IpAndPortEdit->setText(juce::String(info->ip) + ":" + juce::String(info->port), true);
        
        auto ctrl = Controller::GetInstance();
        if (ctrl)
			ctrl->SetDS100IpAndPort(DCP_Settings, juce::IPAddress(info->ip), info->port);
	}
}

/**
 * Callback method to be registered with ZeroconfDiscoveryComponent to handle user input regarding service selection.
 * @param type	The service type that was selected
 * @param info	The detailed info on the service that was selected
 */
void SettingsSectionsComponent::handleSecondDS100ServiceSelected(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, ZeroconfSearcher::ZeroconfSearcher::ServiceInfo* info)
{
	ignoreUnused(type);

	if (info)
	{
		m_SecondDS100IpAndPortEdit->setText(juce::String(info->ip) + ":" + juce::String(info->port), true);

		auto ctrl = Controller::GetInstance();
		if (ctrl)
			ctrl->SetSecondDS100IpAndPort(DCP_Settings, juce::IPAddress(info->ip), info->port);
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

		auto ctrl = Controller::GetInstance();
		if (ctrl)
			ctrl->SetBridgingMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(learnerComponent->getReferredId()), midiAssi);
	}
}

/**
 * Callback method to be registered with SceneIndexToMidiAssignerComponent to handle scene index to midi assignment selection.
 * @param sender			The SceneIndexToMidiAssignerComponent that sent the assignment.
 * @param scenesToMidiAssi	The sent assignment that was chosen by user
 */
void SettingsSectionsComponent::handleScenesToMidiAssiSet(Component* sender, const std::map<String, JUCEAppBasics::MidiCommandRangeAssignment>& scenesToMidiAssi)
{
	if (m_GenericMIDIRecallSceneAssigner && sender == m_GenericMIDIRecallSceneAssigner.get())
	{
		auto ctrl = Controller::GetInstance();
		if (ctrl)
			ctrl->SetBridgingScenesToMidiAssignmentMapping(PBT_GenericMIDI, static_cast<RemoteObjectIdentifier>(m_GenericMIDIRecallSceneAssigner->getReferredId()), scenesToMidiAssi);
	}
}

/**
 * Callback method to be registered with RemoteObjectToOscAssignerComponent to handle custom osc remap assignment selection.
 * @param roiToCustomOscAssis	The sent assignment that was chosen by user
 */
void SettingsSectionsComponent::handleRemapOscAssisSet(const std::map<RemoteObjectIdentifier, std::pair<juce::String, juce::Range<float>>>& roiToCustomOscAssis)
{
	auto ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->SetBridgingOscRemapAssignments(PBT_RemapOSC, roiToCustomOscAssis);
}

/**
 * Callback method to be registered with IndexToChannelAssignerComponent to handle index to channel remap assignment selection.
 * @param idxToChAssis		The sent assignment that was chosen by user
 */
void SettingsSectionsComponent::handleRTTrPMBeaconIdxAssisSet(const std::map<int, ChannelId>& idxToChAssis)
{
	auto ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->SetBridgingChannelRemapAssignments(PBT_BlacktraxRTTrPM, idxToChAssis);
}

/**
 * Callback method to be registered with IndexToChannelAssignerComponent to handle index to channel remap assignment selection.
 * @param projectDummyData		The sent assignment that was chosen by user
 */
void SettingsSectionsComponent::handleDS100dbprData(const juce::String& projectDummyData)
{
	auto ctrl = Controller::GetInstance();
	if (ctrl)
		ctrl->SetDS100DummyProjectData(DCP_Settings, projectDummyData);
}


} // namespace SpaConBridge
