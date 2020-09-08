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
		m_settingsRawEditor->setVisible(true);
	}
	else
	{
		m_applyButton->setVisible(false);
		m_settingsRawEditor->setVisible(false);
	}
}


} // namespace SoundscapeBridgeApp
