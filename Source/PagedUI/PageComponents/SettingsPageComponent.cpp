/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file is part of the Soundscape VST, AU, and AAX Plug-in.

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


#include "SettingsPageComponent.h"

#include "../../Controller.h"
#include "../../LookAndFeel.h"


namespace SoundscapeBridgeApp
{


/*
===============================================================================
	Class HeaderWithElmListComponent
===============================================================================
*/

/**
 * Class constructor.
 */
HeaderWithElmListComponent::HeaderWithElmListComponent(const String& componentName)
	: Component(componentName)
{
	m_headerLabel = std::make_unique<Label>();
	addAndMakeVisible(m_headerLabel.get());

	m_activeToggle = std::make_unique<ToggleButton>();
	m_activeToggle->onClick = [this] { onToggleActive(); };
	addAndMakeVisible(m_activeToggle.get());
	m_activeToggleLabel = std::make_unique<Label>();
	m_activeToggleLabel->attachToComponent(m_activeToggle.get(), true);
	addAndMakeVisible(m_activeToggleLabel.get());

	setElementsActiveState(m_toggleState);
}

/**
 * Class destructor.
 */
HeaderWithElmListComponent::~HeaderWithElmListComponent()
{
	for (auto& component : m_components)
	{
		auto dontDelete = !component.second.second;
        if (dontDelete)
            component.first.release(); // release the pointer to not have the memory cleaned up for those elements that are still externally managed (flagged by second bool in second pair)
	}
}

/**
 *
 */
void HeaderWithElmListComponent::setToggleActiveState(bool toggleState)
{
	if (m_activeToggle)
		m_activeToggle->setToggleState(toggleState, dontSendNotification);

	m_toggleState = toggleState;

	setElementsActiveState(m_toggleState);
}

/**
 * 
 */
void HeaderWithElmListComponent::setElementsActiveState(bool toggleState)
{
	m_toggleState = toggleState;

	m_headerLabel->setEnabled(m_toggleState);
	for (auto const& component : m_components)
	{
		component.first->setEnabled(m_toggleState);
	}

	resized();
	repaint();
}

/**
 * Callback method for when active/deactive toggle was triggered
 */
void HeaderWithElmListComponent::onToggleActive()
{
	if (m_activeToggle)
	{
		auto newActiveState = m_activeToggle->getToggleState();

		if (newActiveState == m_toggleState)
			return;

		setElementsActiveState(m_hasActiveToggle ? newActiveState : true);

		if (toggleIsActiveCallback)
			toggleIsActiveCallback(this, m_toggleState);
	}
}

/**
 * Method to set if this component shall display the enable/disable togglebutton
 * in its upper right corner or not.
 * @param hasActiveToggle	True if it shall show togglebutton, false if not
 */
void HeaderWithElmListComponent::setHasActiveToggle(bool hasActiveToggle)
{
	m_hasActiveToggle = hasActiveToggle;

	m_activeToggle->setVisible(hasActiveToggle);
	m_activeToggleLabel->setVisible(hasActiveToggle);

	setElementsActiveState(m_toggleState);
}

/**
 * Setter for the header text of this component.
 * @param headerText	The text to use as headline
 */
void HeaderWithElmListComponent::setHeaderText(String headerText)
{
	m_activeToggleLabel->setText("Use " + headerText, dontSendNotification);

	auto font = m_headerLabel->getFont();
	font.setBold(true);
	m_headerLabel->setFont(font);
	m_headerLabel->setText(headerText + " Settings", dontSendNotification);
}

/**
 * Methdo to add a component to internal list of components that shall be layouted vertically.
 * @param compo	The component to add.
 * @param includeInLayout	Bool flag that can indicate if the component shall be made visible in this components 
 *							context but not layouted. (e.g. a lable that is already attached to another component)
 * @param takeOwnerShip		Bool flag that indicates if ownership of the given component shall be taken.
 */
void HeaderWithElmListComponent::addComponent(Component* compo, bool includeInLayout, bool takeOwnership)
{
	if (!compo)
		return;

	addAndMakeVisible(compo);
	m_components.push_back(std::make_pair(std::unique_ptr<Component>(compo), std::make_pair(includeInLayout, takeOwnership)));

	compo->setEnabled(m_toggleState);
}

/**
 * Reimplemented paint method from Component that uses colours from TableListBox to give
 * the user a similar impression as when using a table.
 */
void HeaderWithElmListComponent::paint(Graphics& g)
{
	auto w = getWidth();
	auto h = getHeight();

	if (m_toggleState)
		g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId));
	else
		g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId).darker());
	g.fillRect(0, 0, w, h);

	g.setColour(getLookAndFeel().findColour(TableListBox::outlineColourId));
	g.drawRect(0, 0, w, h);
}

/**
 * Reimplemented from Component to dynamically arrange items in vertical direction.
 */
void HeaderWithElmListComponent::resized()
{
	auto activeToggleHeight = 20.0f;
	auto activeToggleMargin = 2.0f;
	auto headerHeight = 25.0f;
	auto headerMargin = 2.0f;
	auto itemHeight = headerHeight;
	auto itemMargin = 5.0f;
	auto itemCount = 0;

	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.justifyContent = FlexBox::JustifyContent::flexStart;
	// Add the enable/disable section toggle, if this section is configured to be toggleable
	if (m_hasActiveToggle)
	{
		fb.items.add(
			FlexItem(*m_activeToggle.get())
			.withAlignSelf(FlexItem::AlignSelf::flexEnd)
			.withWidth(activeToggleHeight + activeToggleMargin)
			.withHeight(activeToggleHeight)
			.withMargin(FlexItem::Margin(activeToggleMargin, activeToggleMargin, 0, activeToggleMargin)));
	}
	// Add the headline section label
	fb.items.add(
		FlexItem(*m_headerLabel.get())
			.withHeight(headerHeight)
			.withMargin(FlexItem::Margin(headerMargin, headerMargin, headerMargin, headerMargin)));
	// Add all the componentes that are flagged to be included in layouting
	for (auto const& component : m_components)
	{
		auto includeInLayout = component.second.first;
		if (includeInLayout)
		{
			fb.items.add(FlexItem(*component.first.get())
				.withHeight(itemHeight)
				.withMaxWidth(150)
				.withMargin(FlexItem::Margin(itemMargin, itemMargin, itemMargin, 130 + itemMargin)));
			itemCount++;
		}
	}

	// Set the accumulated required size of the contents as new component size
	auto bounds = getLocalBounds();
	bounds.setHeight(((itemHeight + (2 * itemMargin)) * itemCount) + (headerHeight + (2 * headerMargin)) + (m_hasActiveToggle ? (activeToggleHeight + (2 * activeToggleMargin)) : 0) + itemMargin);
	setSize(bounds.getWidth(), bounds.getHeight());

	// Trigger the actual layouting based on the calculated bounds
	fb.performLayout(bounds);

//#ifdef DEBUG
//	DBG(getName() + "::" + __FUNCTION__ + " " + String(bounds.getWidth()) + "x" + String(bounds.getHeight()));
//#endif
}


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
	m_ipAddressEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(15, "1234567890.");
	m_portEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(5, "1234567890");
	m_mappingEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(1, "1234");

	// DS100 settings section
	m_DS100Settings = std::make_unique<HeaderWithElmListComponent>();
	m_DS100Settings->setHeaderText("DS100");
	m_DS100Settings->setHasActiveToggle(false);
	addAndMakeVisible(m_DS100Settings.get());

	m_DS100IpAddressEdit = std::make_unique<TextEditor>();
	m_DS100IpAddressEdit->addListener(this);
	m_DS100IpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_DS100IpAddressLabel = std::make_unique<Label>();
	m_DS100IpAddressLabel->setJustificationType(Justification::centred);
	m_DS100IpAddressLabel->setText("IP Address", dontSendNotification);
	m_DS100IpAddressLabel->attachToComponent(m_DS100IpAddressEdit.get(), true);
	m_DS100ZeroconfDiscovery = std::make_unique<JUCEAppBasics::ZeroconfDiscoverComponent>(false, false);
	m_DS100ZeroconfDiscovery->onServiceSelected = [=](JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, JUCEAppBasics::ZeroconfDiscoverComponent::ServiceInfo* info) { handleDS100ServiceSelected(type, info); };
	m_DS100ZeroconfDiscovery->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OSC, RX_PORT_DS100_HOST);
	m_DS100ZeroconfDiscovery->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType::ZST_OCA);
	m_DS100Settings->addComponent(m_DS100IpAddressLabel.get(), false, false);
	m_DS100Settings->addComponent(m_DS100IpAddressEdit.get(), true, false);
	m_DS100Settings->addComponent(m_DS100ZeroconfDiscovery.get(), true, false);

	m_DS100Settings->resized();

	// DiGiCo settings section
	m_DiGiCoBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_DiGiCoBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_DiGiCo) + " Bridging");
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
	m_RTTrPMBridgingSettings->setHasActiveToggle(true);
	m_RTTrPMBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_RTTrPMBridgingSettings.get());

	m_RTTrPMIpAddressEdit = std::make_unique<TextEditor>();
	m_RTTrPMIpAddressEdit->addListener(this);
	m_RTTrPMIpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_RTTrPMIpAddressLabel = std::make_unique<Label>();
	m_RTTrPMIpAddressLabel->setJustificationType(Justification::centred);
	m_RTTrPMIpAddressLabel->setText("IP Address", dontSendNotification);
	m_RTTrPMIpAddressLabel->attachToComponent(m_RTTrPMIpAddressEdit.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMIpAddressLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMIpAddressEdit.get(), true, false);

	m_RTTrPMListeningPortEdit = std::make_unique<TextEditor>();
	m_RTTrPMListeningPortEdit->addListener(this);
	m_RTTrPMListeningPortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_RTTrPMListeningPortLabel = std::make_unique<Label>();
	m_RTTrPMListeningPortLabel->setJustificationType(Justification::centred);
	m_RTTrPMListeningPortLabel->setText("Listening Port", dontSendNotification);
	m_RTTrPMListeningPortLabel->attachToComponent(m_RTTrPMListeningPortEdit.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMListeningPortLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMListeningPortEdit.get(), true, false);

	m_RTTrPMRemotePortEdit = std::make_unique<TextEditor>();
	m_RTTrPMRemotePortEdit->addListener(this);
	m_RTTrPMRemotePortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_RTTrPMRemotePortLabel = std::make_unique<Label>();
	m_RTTrPMRemotePortLabel->setJustificationType(Justification::centred);
	m_RTTrPMRemotePortLabel->setText("Remote Port", dontSendNotification);
	m_RTTrPMRemotePortLabel->attachToComponent(m_RTTrPMRemotePortEdit.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMRemotePortLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMRemotePortEdit.get(), true, false);

	m_RTTrPMInterpretXYRelativeToggle = std::make_unique<ToggleButton>();
	m_RTTrPMInterpretXYRelativeToggle->addListener(this);
	m_RTTrPMInterpretXYRelativeLabel = std::make_unique<Label>();
	m_RTTrPMInterpretXYRelativeLabel->setJustificationType(Justification::centred);
	m_RTTrPMInterpretXYRelativeLabel->setText("Interpret XY as relative", dontSendNotification);
	m_RTTrPMInterpretXYRelativeLabel->attachToComponent(m_RTTrPMInterpretXYRelativeToggle.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMInterpretXYRelativeLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMInterpretXYRelativeToggle.get(), true, false);

	m_RTTrPMMappingAreaEdit = std::make_unique<TextEditor>();
	m_RTTrPMMappingAreaEdit->addListener(this);
	m_RTTrPMMappingAreaEdit->setInputFilter(m_mappingEditFilter.get(), false);
	m_RTTrPMMappingAreaLabel = std::make_unique<Label>();
	m_RTTrPMMappingAreaLabel->setJustificationType(Justification::centred);
	m_RTTrPMMappingAreaLabel->setText("Mapping Area", dontSendNotification);
	m_RTTrPMMappingAreaLabel->attachToComponent(m_RTTrPMMappingAreaEdit.get(), true);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMMappingAreaLabel.get(), false, false);
	m_RTTrPMBridgingSettings->addComponent(m_RTTrPMMappingAreaEdit.get(), true, false);

	m_RTTrPMBridgingSettings->resized();

	// Generic OSC settings section
	m_GenericOSCBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_GenericOSCBridgingSettings->setHeaderText(GetProtocolBridgingNiceName(PBT_GenericOSC) + " Bridging");
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

	auto minWidth = 300;
	auto minHeight = m_DS100Settings->getHeight()
		+ m_DiGiCoBridgingSettings->getHeight()
		+ m_RTTrPMBridgingSettings->getHeight()
		+ m_GenericOSCBridgingSettings->getHeight()
		+ (3 * 2 * margin);

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
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)) });
	fb.performLayout(bounds);
}

/**
 * Reimplemented from ToggleButton Listener.
 * This just forwards it to private method that handles relevant changes in editor contents in general.
 * @param editor	The editor component that changes were made in
 */
void SettingsSectionsComponent::buttonClicked(Button* button)
{
	CController* ctrl = CController::GetInstance();
	if (!ctrl)
		return;

	if (m_RTTrPMInterpretXYRelativeToggle && m_RTTrPMInterpretXYRelativeToggle.get() == button)
	{
		// If button is toggled off, set the mapping area id to -1 (meaning that the RTTrPM data will be handled as absolute, not relative to a mapping area)
		if (!m_RTTrPMInterpretXYRelativeToggle->getToggleState())
		{
			m_previousRTTrPMMappingAreaId = ctrl->GetBridgingMappingArea(PBT_BlacktraxRTTrPM);
			ctrl->SetBridgingMappingArea(PBT_BlacktraxRTTrPM, -1);
		}
		else
		{
			ctrl->SetBridgingMappingArea(PBT_BlacktraxRTTrPM, m_previousRTTrPMMappingAreaId);
		}

		processUpdatedConfig();
	}
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
	CController* ctrl = CController::GetInstance();
	if (!ctrl)
		return;

	// DS100 settings section
	if (m_DS100IpAddressEdit && m_DS100IpAddressEdit.get() == &editor)
		ctrl->SetIpAddress(DCS_Gui, m_DS100IpAddressEdit->getText());

	// DiGiCo settings section
	if (m_DiGiCoIpAddressEdit && m_DiGiCoIpAddressEdit.get() == &editor)
		ctrl->SetBridgingIpAddress(PBT_DiGiCo, m_DiGiCoIpAddressEdit->getText());
	if (m_DiGiCoListeningPortEdit && m_DiGiCoListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_DiGiCo, m_DiGiCoListeningPortEdit->getText().getIntValue());
	if (m_DiGiCoRemotePortEdit && m_DiGiCoRemotePortEdit.get() == &editor)
		ctrl->SetBridgingRemotePort(PBT_DiGiCo, m_DiGiCoRemotePortEdit->getText().getIntValue());

	// RTTrPM settings section
	if (m_RTTrPMIpAddressEdit && m_RTTrPMIpAddressEdit.get() == &editor)
		ctrl->SetBridgingIpAddress(PBT_BlacktraxRTTrPM, m_RTTrPMIpAddressEdit->getText());
	if (m_RTTrPMListeningPortEdit && m_RTTrPMListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_BlacktraxRTTrPM, m_RTTrPMListeningPortEdit->getText().getIntValue());
	if (m_RTTrPMRemotePortEdit && m_RTTrPMRemotePortEdit.get() == &editor)
		ctrl->SetBridgingRemotePort(PBT_BlacktraxRTTrPM, m_RTTrPMRemotePortEdit->getText().getIntValue());
	if (m_RTTrPMMappingAreaEdit && m_RTTrPMMappingAreaEdit.get() == &editor)
	{
		ctrl->SetBridgingMappingArea(PBT_BlacktraxRTTrPM, m_RTTrPMMappingAreaEdit->getText().getIntValue());
		m_previousRTTrPMMappingAreaId = m_RTTrPMMappingAreaEdit->getText().getIntValue();
	}

	// Generic OSC settings section
	if (m_GenericOSCIpAddressEdit && m_GenericOSCIpAddressEdit.get() == &editor)
		ctrl->SetBridgingIpAddress(PBT_GenericOSC, m_GenericOSCIpAddressEdit->getText());
	if (m_GenericOSCListeningPortEdit && m_GenericOSCListeningPortEdit.get() == &editor)
		ctrl->SetBridgingListeningPort(PBT_GenericOSC, m_GenericOSCListeningPortEdit->getText().getIntValue());
	if (m_GenericOSCRemotePortEdit && m_GenericOSCRemotePortEdit.get() == &editor)
		ctrl->SetBridgingRemotePort(PBT_GenericOSC, m_GenericOSCRemotePortEdit->getText().getIntValue());
}

/**
 * Proxy method to activate a single bridging protocol in controller.
 * @param sectionType	The protocolType to be active from now on.
 */
void SettingsSectionsComponent::setSettingsSectionActiveState(HeaderWithElmListComponent* settingsSection, bool activeState)
{
	CController* ctrl = CController::GetInstance();
	if (!ctrl)
		return;

	ProtocolBridgingType sectionType = PBT_None;
	if (settingsSection == m_DiGiCoBridgingSettings.get())
		sectionType = PBT_DiGiCo;
	else if (settingsSection == m_RTTrPMBridgingSettings.get())
		sectionType = PBT_BlacktraxRTTrPM;
	else if (settingsSection == m_GenericOSCBridgingSettings.get())
		sectionType = PBT_GenericOSC;

	if (activeState)
		ctrl->SetActiveProtocolBridging(ctrl->GetActiveProtocolBridging() | sectionType);
	else
		ctrl->SetActiveProtocolBridging(ctrl->GetActiveProtocolBridging() & ~sectionType);
}

/**
 * Method to update the elements on UI when app configuration changed.
 * This is called by parent container component when it receives
 * onConfigUpdated call (it's a config listener and subscribed to changes)
 */
void SettingsSectionsComponent::processUpdatedConfig()
{
	CController* ctrl = CController::GetInstance();
	if (!ctrl)
		return;

	// DS100 settings section
	if (m_DS100IpAddressEdit)
		m_DS100IpAddressEdit->setText(ctrl->GetIpAddress());

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
	if (m_RTTrPMIpAddressEdit)
		m_RTTrPMIpAddressEdit->setText(ctrl->GetBridgingIpAddress(PBT_BlacktraxRTTrPM));
	if (m_RTTrPMListeningPortEdit)
		m_RTTrPMListeningPortEdit->setText(String(ctrl->GetBridgingListeningPort(PBT_BlacktraxRTTrPM)), false);
	if (m_RTTrPMRemotePortEdit)
		m_RTTrPMRemotePortEdit->setText(String(ctrl->GetBridgingRemotePort(PBT_BlacktraxRTTrPM)), false);
	if (m_RTTrPMInterpretXYRelativeToggle)
		m_RTTrPMInterpretXYRelativeToggle->setToggleState((ctrl->GetBridgingMappingArea(PBT_BlacktraxRTTrPM) != -1), dontSendNotification);
	if (m_RTTrPMMappingAreaEdit)
	{
		m_RTTrPMMappingAreaEdit->setText(String(ctrl->GetBridgingMappingArea(PBT_BlacktraxRTTrPM)), false);
		m_RTTrPMMappingAreaEdit->setEnabled((ctrl->GetBridgingMappingArea(PBT_BlacktraxRTTrPM) != -1));
	}
	if (m_RTTrPMMappingAreaLabel)
		m_RTTrPMMappingAreaLabel->setEnabled((ctrl->GetBridgingMappingArea(PBT_BlacktraxRTTrPM) != -1));

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
        
        CController* ctrl = CController::GetInstance();
        if (ctrl)
            ctrl->SetIpAddress(DCS_Gui, info->ip);
	}
}


/*
===============================================================================
 Class SettingsPageComponent
===============================================================================
*/

/**
 * Class constructor.
 */
SettingsPageComponent::SettingsPageComponent()
	: PageComponentBase(PCT_Settings)
{
	// Apply button for when raw config is visible
	m_applyButton = std::make_unique<TextButton>("Apply");
	m_applyButton->onClick = [this] { onApplyClicked(); };
	addAndMakeVisible(m_applyButton.get());
	// Text editor for when raw config is visible
	m_settingsRawEditor = std::make_unique<TextEditor>();
	m_settingsRawEditor->setMultiLine(true, false);
	addAndMakeVisible(m_settingsRawEditor.get());

	// Select combobox for look and feel selection
	m_lookAndFeelSelect = std::make_unique<ComboBox>();
	//for (auto t = static_cast<int>(DbLookAndFeelBase::LAFT_InvalidFirst + 1); t < static_cast<int>(DbLookAndFeelBase::LAFT_InvalidLast); ++t)
	//	m_lookAndFeelSelect->addItem(DbLookAndFeelBase::getLookAndFeelName(static_cast<DbLookAndFeelBase::LookAndFeelType>(t)), t);
	m_lookAndFeelSelect->addItem(DbLookAndFeelBase::getLookAndFeelName(DbLookAndFeelBase::LAFT_Dark), DbLookAndFeelBase::LAFT_Dark);
	m_lookAndFeelSelect->addItem(DbLookAndFeelBase::getLookAndFeelName(DbLookAndFeelBase::LAFT_Light), DbLookAndFeelBase::LAFT_Light);
	m_lookAndFeelSelect->onChange = [this] { onSelectedLookAndFeelChanged(); };
	addAndMakeVisible(m_lookAndFeelSelect.get());
	// Label for look and feel selection
	m_lookAndFeelLabel = std::make_unique<Label>("LookAndFeelSelect", "Look and feel:");
	m_lookAndFeelLabel->setJustificationType(Justification::centred);
	m_lookAndFeelLabel->attachToComponent(m_lookAndFeelSelect.get(), true);
	addAndMakeVisible(m_lookAndFeelLabel.get());

	// Toggle button for showing/hiding raw config
	m_useRawConfigButton = std::make_unique<ToggleButton>();
	m_useRawConfigButton->onClick = [this] { onToggleRawConfigVisible(); };
	addAndMakeVisible(m_useRawConfigButton.get());
	// label for showing/hiding raw config
	m_useRawConfigLabel = std::make_unique<Label>("RAW CFG", "Show raw config");
	m_useRawConfigLabel->setJustificationType(Justification::centred);
	m_useRawConfigLabel->attachToComponent(m_useRawConfigButton.get(), true);
	addAndMakeVisible(m_useRawConfigLabel.get());
	onToggleRawConfigVisible();

	// The component containing configuration sections, etc. to be shown in a viewport for scrolling capabilities
	m_settingsComponent = std::make_unique<SettingsSectionsComponent>();
	m_settingsViewport = std::make_unique<Viewport>();
	m_settingsViewport->setViewedComponent(m_settingsComponent.get(), false);
	addAndMakeVisible(m_settingsViewport.get());

	// register this object as config watcher
	auto config = AppConfiguration::getInstance();
	if (config)
	{
		config->addWatcher(this);
	}
}

/**
 * Class destructor.
 */
SettingsPageComponent::~SettingsPageComponent()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void SettingsPageComponent::paint(Graphics& g)
{
	// Paint background to cover the controls behind this overlay.
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(Rectangle<int>(0, 0, getLocalBounds().getWidth(), getLocalBounds().getHeight()));
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void SettingsPageComponent::resized()
{
	auto bounds = getLocalBounds().reduced(5);

	// toggle button for visibility of raw config textfield
	auto bottomBarControlBounds = bounds.removeFromBottom(25);
	m_useRawConfigButton->setBounds(bottomBarControlBounds.removeFromRight(25));
	m_lookAndFeelSelect->setBounds(bottomBarControlBounds.removeFromLeft(170).removeFromRight(70));

	bounds.removeFromBottom(5);

	m_settingsComponent->setBounds(bounds);
	m_settingsViewport->setBounds(bounds);

	if (m_settingsViewport->isVerticalScrollBarShown() || m_settingsViewport->isHorizontalScrollBarShown())
	{
		auto boundsWithoutScrollbars = bounds;

		if (m_settingsViewport->isVerticalScrollBarShown())
			boundsWithoutScrollbars.setWidth(bounds.getWidth() - m_settingsViewport->getVerticalScrollBar().getWidth());

		if (m_settingsViewport->isHorizontalScrollBarShown())
			boundsWithoutScrollbars.setHeight(bounds.getHeight() - m_settingsViewport->getHorizontalScrollBar().getHeight());

		m_settingsComponent->setBounds(boundsWithoutScrollbars);
	}

	// raw config textfield, etc. - not always visible!
	m_applyButton->setBounds(bounds.removeFromTop(25));
	m_settingsRawEditor->setBounds(bounds);
}

/**
 * Setter for the currently selected look and feel type in dropdown on ui
 * @param lookAndfeelType	The type to set as currently selected
 */
void SettingsPageComponent::SetSelectedLookAndFeelType(DbLookAndFeelBase::LookAndFeelType lookAndFeelType)
{
	if (m_lookAndFeelSelect)
		m_lookAndFeelSelect->setSelectedId(lookAndFeelType, dontSendNotification);
}

/**
 * Getter for the currently selected look and feel type in dropdown on ui
 * @return	The currently selected type
 */
DbLookAndFeelBase::LookAndFeelType SettingsPageComponent::GetSelectedLookAndFeelType()
{
	if (m_lookAndFeelSelect)
	{
		auto lookAndFeelType = static_cast<DbLookAndFeelBase::LookAndFeelType>(m_lookAndFeelSelect->getSelectedId());
		jassert(lookAndFeelType > DbLookAndFeelBase::LookAndFeelType::LAFT_InvalidFirst && lookAndFeelType < DbLookAndFeelBase::LookAndFeelType::LAFT_InvalidLast);
		return lookAndFeelType;
	}
	else
	{
		jassertfalse;
		return DbLookAndFeelBase::LAFT_InvalidFirst;
	}
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the plugin parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void SettingsPageComponent::UpdateGui(bool init)
{
	ignoreUnused(init);
}

/**
 *
 */
void SettingsPageComponent::onConfigUpdated()
{
	auto config = AppConfiguration::getInstance();
	if (config)
	{
		// trigger updating the settings visu in general
		m_settingsComponent->processUpdatedConfig();

		// if the raw config is currently visible, go into updating it as well
		if (m_useRawConfigButton->getToggleState())
		{
			// get the config for filling raw texteditor (meant for debugging, ...)
			auto configXml = config->getConfigState();
			auto configText = configXml->toString();
			m_settingsRawEditor->setText(configText);
		}
	}
}

/**
 *
 */
void SettingsPageComponent::onApplyClicked()
{
	auto config = AppConfiguration::getInstance();
	if (config != nullptr)
	{
		XmlDocument configXmlDocument(m_settingsRawEditor->getText());
		auto configXmlElement = configXmlDocument.getDocumentElement();
		if (configXmlElement)
		{
			auto controllerXmlElement = configXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::CONTROLLER));
			if (controllerXmlElement)
				config->setConfigState(std::make_unique<XmlElement>(*controllerXmlElement));

			auto overviewXmlElement = configXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::OVERVIEW));
			if (overviewXmlElement)
				config->setConfigState(std::make_unique<XmlElement>(*overviewXmlElement));

			config->triggerWatcherUpdate();
		}
	}
}

/**
 *
 */
void SettingsPageComponent::onToggleRawConfigVisible()
{
	if (m_useRawConfigButton->getToggleState())
	{
		m_applyButton->setVisible(true);
		m_applyButton->toFront(true);
		m_settingsRawEditor->setVisible(true);
		m_settingsRawEditor->toFront(true);

		// manually trigger config refresh, since we did not process config changes while raw settings editor was invisible
		onConfigUpdated();
	}
	else
	{
		m_applyButton->setVisible(false);
		m_settingsRawEditor->setVisible(false);
	}
}

/**
 *
 */
void SettingsPageComponent::onSelectedLookAndFeelChanged()
{
	auto config = AppConfiguration::getInstance();
	if (config)
		config->triggerConfigurationDump(true);
}


} // namespace SoundscapeBridgeApp