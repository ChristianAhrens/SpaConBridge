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
#include "../AppConfiguration.h"


namespace SoundscapeBridgeApp
{


static constexpr char* DS100_IP_EDIT_NAME = "DS100_IP_EDIT_NAME";
static constexpr char* DS100_IP_LABEL_NAME = "DS100_IP_LABEL_NAME";
static constexpr char* DIGICO_IP_EDIT_NAME = "DIGICO_IP_EDIT_NAME";
static constexpr char* DIGICO_IP_LABEL_NAME = "DIGICO_IP_LABEL_NAME";
static constexpr char* GENERICOSC_IP_EDIT_NAME = "GENERICOSC_IP_EDIT_NAME";
static constexpr char* GENERICOSC_IP_LABEL_NAME = "GENERICOSC_IP_LABEL_NAME";


/*
===============================================================================
	Class HeaderWithElmListComponent
===============================================================================
*/

/**
 * Class constructor.
 */
HeaderWithElmListComponent::HeaderWithElmListComponent()
{
	m_headerLabel = std::make_unique<Label>("HEADER_LABEL");
	addAndMakeVisible(m_headerLabel.get());

	m_activeToggle = std::make_unique<ToggleButton>();
	m_activeToggle->onStateChange = [this] { updateToggleActive(); };
	addAndMakeVisible(m_activeToggle.get());
	m_activeToggleLabel = std::make_unique<Label>("ACTIVE_TOGGLE_LABEL");
	m_activeToggleLabel->attachToComponent(m_activeToggle.get(), true);
	addAndMakeVisible(m_activeToggleLabel.get());

	updateToggleActive();
}

/**
 * Class destructor.
 */
HeaderWithElmListComponent::~HeaderWithElmListComponent()
{

}

/**
 *
 */
void HeaderWithElmListComponent::updateToggleActive()
{
	m_toggleState = m_hasActiveToggle ? m_activeToggle->getToggleState() : true;

	m_headerLabel->setEnabled(m_toggleState);
	for (auto const& component : m_components)
		component.first->setEnabled(m_toggleState);

	if (toggleIsActiveCallback)
		toggleIsActiveCallback(m_toggleState);

	resized();
	repaint();
}

/**
 *
 */
void HeaderWithElmListComponent::setHasActiveToggle(bool hasActiveToggle)
{
	m_hasActiveToggle = hasActiveToggle;

	m_activeToggle->setVisible(hasActiveToggle);
	m_activeToggleLabel->setVisible(hasActiveToggle);

	updateToggleActive();
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
void HeaderWithElmListComponent::addComponent(Component* compo, bool includeInLayout)
{
	if (!compo)
		return;

	addAndMakeVisible(compo);
	m_components.push_back(std::make_pair(std::unique_ptr<Component>(compo), includeInLayout));

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
	auto bounds = getLocalBounds();

	FlexBox headerfb;
	headerfb.flexDirection = FlexBox::Direction::row;
	headerfb.items.addArray({
		FlexItem(*m_headerLabel.get()).withMaxHeight(25).withFlex(1, 1),
		FlexItem(*m_activeToggle.get()).withMaxHeight(25).withFlex(0, 2, 25)
		});

	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.items.add(FlexItem(headerfb).withMaxHeight(25).withFlex(1).withMargin(FlexItem::Margin(2, 2, 2, 2)));
	for (auto const& component : m_components)
	{
		auto includeInLayout = component.second;
		if (includeInLayout)
		{
			fb.items.add(FlexItem(*component.first.get()).withMaxHeight(25).withFlex(1).withMargin(FlexItem::Margin(5, 5, 5, 120 + 5)));
		}
	}
	fb.performLayout(bounds);
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
	m_useRawConfigButton->onStateChange = [this] { onToggleRawConfigVisible(); };
	addAndMakeVisible(m_useRawConfigButton.get());
	m_useRawConfigLabel = std::make_unique<Label>("RAW CFG", "Show raw config");
	m_useRawConfigLabel->attachToComponent(m_useRawConfigButton.get(), true);
	addAndMakeVisible(m_useRawConfigLabel.get());
	onToggleRawConfigVisible();

	// DS100 settings section
	m_DS100Settings = std::make_unique<HeaderWithElmListComponent>();
	m_DS100Settings->setHeaderText("DS100");
	m_DS100Settings->setHasActiveToggle(false);
	addAndMakeVisible(m_DS100Settings.get());

	auto ipAddressEdit = std::make_unique<CTextEditor>(DS100_IP_EDIT_NAME);
	ipAddressEdit->setText("127.0.01");
	auto ipAddressLabel = std::make_unique<CLabel>(DS100_IP_LABEL_NAME);
	ipAddressLabel->setText("IP Address", dontSendNotification);
	ipAddressLabel->attachToComponent(ipAddressEdit.get(), true);
	m_DS100Settings->addComponent(ipAddressLabel.release(), false);
	m_DS100Settings->addComponent(ipAddressEdit.release(), true);

	// DiGiCo settings section
	m_DiGiCoBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_DiGiCoBridgingSettings->setHeaderText("DiGiCo Bridging");
	m_DiGiCoBridgingSettings->setHasActiveToggle(true);
	addAndMakeVisible(m_DiGiCoBridgingSettings.get());

	ipAddressEdit = std::make_unique<CTextEditor>(DIGICO_IP_EDIT_NAME);
	ipAddressEdit->setText("127.0.01");
	ipAddressLabel = std::make_unique<CLabel>(DIGICO_IP_LABEL_NAME);
	ipAddressLabel->setText("IP Address", dontSendNotification);
	ipAddressLabel->attachToComponent(ipAddressEdit.get(), true);
	m_DiGiCoBridgingSettings->addComponent(ipAddressLabel.release(), false);
	m_DiGiCoBridgingSettings->addComponent(ipAddressEdit.release(), true);

	// Generic OSC settings section
	m_GenericOSCBridgingSettings = std::make_unique<HeaderWithElmListComponent>();
	m_GenericOSCBridgingSettings->setHeaderText("Generic OSC Bridging");
	m_GenericOSCBridgingSettings->setHasActiveToggle(true);
	addAndMakeVisible(m_GenericOSCBridgingSettings.get());

	ipAddressEdit = std::make_unique<CTextEditor>(GENERICOSC_IP_EDIT_NAME);
	ipAddressEdit->setText("127.0.01");
	ipAddressLabel = std::make_unique<CLabel>(GENERICOSC_IP_LABEL_NAME);
	ipAddressLabel->setText("IP Address", dontSendNotification);
	ipAddressLabel->attachToComponent(ipAddressEdit.get(), true);
	m_GenericOSCBridgingSettings->addComponent(ipAddressLabel.release(), false);
	m_GenericOSCBridgingSettings->addComponent(ipAddressEdit.release(), true);

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

	// regular configuration elements
	FlexBox fb;
	fb.flexDirection = FlexBox::Direction::column;
	fb.items.addArray({ 
		FlexItem(*m_DS100Settings.get()).withFlex(1).withMargin(FlexItem::Margin(3, 3, 3, 3)), 
		FlexItem(*m_DiGiCoBridgingSettings.get()).withFlex(1).withMargin(FlexItem::Margin(3, 3, 3, 3)),
		FlexItem(*m_GenericOSCBridgingSettings.get()).withFlex(1).withMargin(FlexItem::Margin(3, 3, 3, 3)) });
	fb.performLayout(bounds);

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

void CSettingsContainer::onConfigUpdated()
{
	auto config = AppConfiguration::getInstance();
	if (config)
	{
		auto configXml = config->getConfigState();
		auto configText = configXml->toString();
		m_settingsRawEditor->setText(configText);
	}
}

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

void CSettingsContainer::onToggleRawConfigVisible()
{
	if (m_useRawConfigButton->getToggleState())
	{
		m_applyButton->setVisible(true);
		m_applyButton->toFront(true);
		m_settingsRawEditor->setVisible(true);
		m_settingsRawEditor->toFront(true);
	}
	else
	{
		m_applyButton->setVisible(false);
		m_settingsRawEditor->setVisible(false);
	}
}


} // namespace SoundscapeBridgeApp
