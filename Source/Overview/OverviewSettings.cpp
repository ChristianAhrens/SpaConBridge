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


#include "OverviewSettings.h"

#include "../Controller.h"


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
 *
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
 *
 */
void HeaderWithElmListComponent::setHasActiveToggle(bool hasActiveToggle)
{
	m_hasActiveToggle = hasActiveToggle;

	m_activeToggle->setVisible(hasActiveToggle);
	m_activeToggleLabel->setVisible(hasActiveToggle);

	setElementsActiveState(m_toggleState);
}

/**
 *
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
 *
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
 *
 */
void HeaderWithElmListComponent::paint(Graphics& g)
{
	auto w = getWidth();
	auto h = getHeight();

	if (m_toggleState)
		g.setColour(CDbStyle::GetDbColor(CDbStyle::MidColor));
	else
		g.setColour(CDbStyle::GetDbColor(CDbStyle::MidColor).darker());
	g.fillRect(0, 0, w, h);

	g.setColour(CDbStyle::GetDbColor(CDbStyle::DarkLineColor));
	g.drawRect(0, 0, w, h);
}

/**
 * 
 */
void HeaderWithElmListComponent::resized()
{
	auto headerHeight = 25.0f;
	auto itemHeight = headerHeight;
	auto itemMargin = 5.0f;
	auto headerMargin = 2.0f;
	auto itemCount = 0;

	FlexBox headerfb;
	headerfb.flexDirection = FlexBox::Direction::row;
	headerfb.items.addArray({
		FlexItem(*m_headerLabel.get()).withFlex(1, 1),
		FlexItem(*m_activeToggle.get()).withFlex(0, 2, itemHeight)
		});

	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.justifyContent = FlexBox::JustifyContent::flexStart;
	fb.items.add(FlexItem(headerfb)
		.withHeight(headerHeight)
		.withMargin(FlexItem::Margin(headerMargin, headerMargin, headerMargin, headerMargin)));
	for (auto const& component : m_components)
	{
		auto includeInLayout = component.second.first;
		if (includeInLayout)
		{
			fb.items.add(FlexItem(*component.first.get())
				.withHeight(itemHeight)
				.withMaxWidth(150)
				.withMargin(FlexItem::Margin(itemMargin, itemMargin, itemMargin, 110 + itemMargin)));
			itemCount++;
		}
	}

	auto bounds = getLocalBounds();
	bounds.setHeight(((itemHeight + (2 * itemMargin)) * itemCount) + (headerHeight + (2 * headerMargin)) + 5);
	setSize(bounds.getWidth(), bounds.getHeight());

	fb.performLayout(bounds);

//#ifdef DEBUG
//	DBG(getName() + "::" + __FUNCTION__ + " " + String(bounds.getWidth()) + "x" + String(bounds.getHeight()));
//#endif
}


/*
===============================================================================
 Class CSettingsComponent
===============================================================================
*/

/**
 * Class constructor.
 */
CSettingsComponent::CSettingsComponent()
{
	m_ipAddressEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(15, "1234567890.");;
	m_portEditFilter = std::make_unique<TextEditor::LengthAndCharacterRestriction>(5, "1234567890");

	// DS100 settings section
	m_DS100Settings = std::make_unique<HeaderWithElmListComponent>();
	m_DS100Settings->setHeaderText("DS100");
	m_DS100Settings->setHasActiveToggle(false);
	addAndMakeVisible(m_DS100Settings.get());

	m_DS100IpAddressEdit = std::make_unique<CTextEditor>();
	m_DS100IpAddressEdit->addListener(this);
	m_DS100IpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_DS100IpAddressLabel = std::make_unique<CLabel>();
	m_DS100IpAddressLabel->setText("IP Address", dontSendNotification);
	m_DS100IpAddressLabel->attachToComponent(m_DS100IpAddressEdit.get(), true);
	m_DS100Settings->addComponent(m_DS100IpAddressLabel.get(), false, false);
	m_DS100Settings->addComponent(m_DS100IpAddressEdit.get(), true, false);

	m_DS100Settings->resized();

	// DiGiCo settings section
	m_DiGiCoBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_DiGiCoBridgingSettings->setHeaderText("DiGiCo Bridging");
	m_DiGiCoBridgingSettings->setHasActiveToggle(true);
	m_DiGiCoBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_DiGiCoBridgingSettings.get());

	m_DiGiCoIpAddressEdit = std::make_unique<CTextEditor>();
	m_DiGiCoIpAddressEdit->addListener(this);
	m_DiGiCoIpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_DiGiCoIpAddressLabel = std::make_unique<CLabel>();
	m_DiGiCoIpAddressLabel->setText("IP Address", dontSendNotification);
	m_DiGiCoIpAddressLabel->attachToComponent(m_DiGiCoIpAddressEdit.get(), true);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoIpAddressLabel.get(), false, false);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoIpAddressEdit.get(), true, false);

	m_DiGiCoListeningPortEdit = std::make_unique<CTextEditor>();
	m_DiGiCoListeningPortEdit->addListener(this);
	m_DiGiCoListeningPortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_DiGiCoListeningPortLabel = std::make_unique<CLabel>();
	m_DiGiCoListeningPortLabel->setText("Listening Port", dontSendNotification);
	m_DiGiCoListeningPortLabel->attachToComponent(m_DiGiCoListeningPortEdit.get(), true);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoListeningPortLabel.get(), false, false);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoListeningPortEdit.get(), true, false);

	m_DiGiCoRemotePortEdit = std::make_unique<CTextEditor>();
	m_DiGiCoRemotePortEdit->addListener(this);
	m_DiGiCoRemotePortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_DiGiCoRemotePortLabel = std::make_unique<CLabel>();
	m_DiGiCoRemotePortLabel->setText("Remote Port", dontSendNotification);
	m_DiGiCoRemotePortLabel->attachToComponent(m_DiGiCoRemotePortEdit.get(), true);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoRemotePortLabel.get(), false, false);
	m_DiGiCoBridgingSettings->addComponent(m_DiGiCoRemotePortEdit.get(), true, false);

	m_DiGiCoBridgingSettings->resized();

	// Generic OSC settings section
	m_GenericOSCBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_GenericOSCBridgingSettings->setHeaderText("Generic OSC Bridging");
	m_GenericOSCBridgingSettings->setHasActiveToggle(true);
	m_GenericOSCBridgingSettings->toggleIsActiveCallback = [=](HeaderWithElmListComponent* settingsSection, bool activeState) { setSettingsSectionActiveState(settingsSection, activeState); };
	addAndMakeVisible(m_GenericOSCBridgingSettings.get());

	m_GenericOSCIpAddressEdit = std::make_unique<CTextEditor>();
	m_GenericOSCIpAddressEdit->addListener(this);
	m_GenericOSCIpAddressEdit->setInputFilter(m_ipAddressEditFilter.get(), false);
	m_GenericOSCIpAddressLabel = std::make_unique<CLabel>();
	m_GenericOSCIpAddressLabel->setText("IP Address", dontSendNotification);
	m_GenericOSCIpAddressLabel->attachToComponent(m_GenericOSCIpAddressEdit.get(), true);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCIpAddressLabel.get(), false, false);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCIpAddressEdit.get(), true, false);

	m_GenericOSCListeningPortEdit = std::make_unique<CTextEditor>();
	m_GenericOSCListeningPortEdit->addListener(this);
	m_GenericOSCListeningPortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_GenericOSCListeningPortLabel = std::make_unique<CLabel>();
	m_GenericOSCListeningPortLabel->setText("Listening Port", dontSendNotification);
	m_GenericOSCListeningPortLabel->attachToComponent(m_GenericOSCListeningPortEdit.get(), true);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCListeningPortLabel.get(), false, false);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCListeningPortEdit.get(), true, false);

	m_GenericOSCRemotePortEdit = std::make_unique<CTextEditor>();
	m_GenericOSCRemotePortEdit->addListener(this);
	m_GenericOSCRemotePortEdit->setInputFilter(m_portEditFilter.get(), false);
	m_GenericOSCRemotePortLabel = std::make_unique<CLabel>();
	m_GenericOSCRemotePortLabel->setText("Remote Port", dontSendNotification);
	m_GenericOSCRemotePortLabel->attachToComponent(m_GenericOSCRemotePortEdit.get(), true);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCRemotePortLabel.get(), false, false);
	m_GenericOSCBridgingSettings->addComponent(m_GenericOSCRemotePortEdit.get(), true, false);

	m_GenericOSCBridgingSettings->resized();
}

/**
 * Class destructor.
 */
CSettingsComponent::~CSettingsComponent()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void CSettingsComponent::paint(Graphics& g)
{
	// Paint background to cover the controls behind this overlay.
	g.setColour(CDbStyle::GetDbColor(CDbStyle::DarkColor));
	g.fillRect(Rectangle<int>(0, 0, getLocalBounds().getWidth(), getLocalBounds().getHeight()));
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void CSettingsComponent::resized()
{
	auto margin = 3.0f;

	auto minWidth = 300;
	auto minHeight = m_DS100Settings->getHeight()
		+ m_DiGiCoBridgingSettings->getHeight()
		+ m_GenericOSCBridgingSettings->getHeight()
		+ (3 * 2 * margin);

	auto bounds = getLocalBounds();
	if (bounds.getWidth() < minWidth || bounds.getHeight() < minHeight)
	{
		if (bounds.getWidth() < minWidth)
			bounds.setWidth(minWidth);
		if (bounds.getHeight() < minHeight)
			bounds.setHeight(minHeight);

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
		FlexItem(*m_GenericOSCBridgingSettings.get())
			.withHeight(static_cast<float>(m_GenericOSCBridgingSettings->getHeight()))
			.withMargin(FlexItem::Margin(margin, margin, margin, margin)) });
	fb.performLayout(bounds);
}

/**
 * Reimplemented from TextEditor Listener.
 * This just forwards it to private method that handles relevant changes in editor contents in general.
 * @param editor	The editor component that changes were made in
 */
void CSettingsComponent::textEditorReturnKeyPressed(TextEditor& editor)
{
	textEditorUpdated(editor);
}

/**
 * Reimplemented from TextEditor Listener.
 * This just forwards it to private method that handles relevant changes in editor contents in general.
 * @param editor	The editor component that changes were made in
 */
void CSettingsComponent::textEditorFocusLost(TextEditor& editor)
{
	textEditorUpdated(editor);
}

/**
 * Method to handle relevant changes in text editors by processing them and inserting into config through controller interface
 * @param editor	The editor component that changes were made in
 */
void CSettingsComponent::textEditorUpdated(TextEditor& editor)
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
void CSettingsComponent::setSettingsSectionActiveState(HeaderWithElmListComponent* settingsSection, bool activeState)
{
	CController* ctrl = CController::GetInstance();
	if (!ctrl)
		return;

	ProtocolBridgingType sectionType = PBT_None;
	if (settingsSection == m_DiGiCoBridgingSettings.get())
		sectionType = PBT_DiGiCo;
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
void CSettingsComponent::processUpdatedConfig()
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


/*
===============================================================================
 Class CSettingsContainer
===============================================================================
*/

/**
 * Class constructor.
 */
CSettingsContainer::CSettingsContainer()
	: AOverlay(OT_Settings)
{
	m_applyButton = std::make_unique<TextButton>("Apply");
	m_applyButton->onClick = [this] { onApplyClicked(); };
	addAndMakeVisible(m_applyButton.get());

	m_settingsRawEditor = std::make_unique<TextEditor>();
	m_settingsRawEditor->setMultiLine(true, false);
	addAndMakeVisible(m_settingsRawEditor.get());

	m_useRawConfigButton = std::make_unique<ToggleButton>();
	m_useRawConfigButton->onClick = [this] { onToggleRawConfigVisible(); };
	addAndMakeVisible(m_useRawConfigButton.get());
	m_useRawConfigLabel = std::make_unique<Label>("RAW CFG", "Show raw config");
	m_useRawConfigLabel->attachToComponent(m_useRawConfigButton.get(), true);
	addAndMakeVisible(m_useRawConfigLabel.get());
	onToggleRawConfigVisible();

	m_settingsComponent = std::make_unique<CSettingsComponent>();

	m_settingsViewport = std::make_unique<Viewport>();
	m_settingsViewport->getHorizontalScrollBar().setColour(ScrollBar::backgroundColourId, CDbStyle::GetDbColor(CDbStyle::DarkColor));
	m_settingsViewport->getHorizontalScrollBar().setColour(ScrollBar::thumbColourId, CDbStyle::GetDbColor(CDbStyle::DarkTextColor));
	m_settingsViewport->getHorizontalScrollBar().setColour(ScrollBar::trackColourId, CDbStyle::GetDbColor(CDbStyle::MidColor));
	m_settingsViewport->getVerticalScrollBar().setColour(ScrollBar::backgroundColourId, CDbStyle::GetDbColor(CDbStyle::DarkColor));
	m_settingsViewport->getVerticalScrollBar().setColour(ScrollBar::thumbColourId, CDbStyle::GetDbColor(CDbStyle::DarkTextColor));
	m_settingsViewport->getVerticalScrollBar().setColour(ScrollBar::trackColourId, CDbStyle::GetDbColor(CDbStyle::MidColor));
	m_settingsViewport->setViewedComponent(m_settingsComponent.get(), false);
	addAndMakeVisible(m_settingsViewport.get());

	// register this object as config watcher
	auto config = AppConfiguration::getInstance();
	if (config)
		config->addWatcher(this);
}

/**
 * Class destructor.
 */
CSettingsContainer::~CSettingsContainer()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void CSettingsContainer::paint(Graphics& g)
{
	// Paint background to cover the controls behind this overlay.
	g.setColour(CDbStyle::GetDbColor(CDbStyle::DarkColor));
	g.fillRect(Rectangle<int>(0, 0, getLocalBounds().getWidth(), getLocalBounds().getHeight()));
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void CSettingsContainer::resized()
{
	auto bounds = getLocalBounds().reduced(5);

	// toggle button for visibility of raw config textfield
	auto rcbBounds = bounds.removeFromBottom(20).removeFromRight(150);
	m_useRawConfigButton->setBounds(rcbBounds.removeFromRight(25));

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
	m_applyButton->setBounds(bounds.removeFromTop(20));
	m_settingsRawEditor->setBounds(bounds);
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the plugin parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void CSettingsContainer::UpdateGui(bool init)
{
	ignoreUnused(init);
}

/**
 *
 */
void CSettingsContainer::onConfigUpdated()
{
	auto config = AppConfiguration::getInstance();
	if (config)
	{
		if (m_useRawConfigButton->getToggleState())
		{
			// get the config for filling raw texteditor (meant for debugging, ...)
			auto configXml = config->getConfigState();
			auto configText = configXml->toString();
			m_settingsRawEditor->setText(configText);
		}

		m_settingsComponent->processUpdatedConfig();
	}
}

/**
 *
 */
void CSettingsContainer::onApplyClicked()
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
void CSettingsContainer::onToggleRawConfigVisible()
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


} // namespace SoundscapeBridgeApp
