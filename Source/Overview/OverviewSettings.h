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


#pragma once

#include "../About.h"
#include "../Gui.h"
#include "../SoundscapeBridgeAppCommon.h"
#include "../AppConfiguration.h"


namespace SoundscapeBridgeApp
{

/**
 * HeaderWithElmListComponent is a component to hold a header component with multiple other components in a specific layout.
 */
class HeaderWithElmListComponent : public Component
{
public:
	HeaderWithElmListComponent();
	~HeaderWithElmListComponent() override;

	void setHasActiveToggle(bool hasActiveToggle);
	void setHeaderText(String headerText);
	void addComponent(Component* compo);

	void activeToggleClicked();

	std::function<void(bool)>	toggleIsActive;

protected:
	void paint(Graphics&) override;
	void resized() override;

private:
	bool									m_hasActiveToggle{ false };
	std::unique_ptr<ToggleButton>			m_activeToggle;
	std::unique_ptr<Label>					m_activeToggleLabel;
	std::unique_ptr<Label>					m_headerLabel;
	std::vector<std::unique_ptr<Component>>	m_components;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderWithElmListComponent)
};

/**
 * CSettingsContainer is a component to hold multiple components that are dedicated to app configuration.
 */
class CSettingsContainer : public AOverlay, public AppConfiguration::Watcher
{
public:
	CSettingsContainer();
	~CSettingsContainer() override;

	void UpdateGui(bool init) override;

	void onConfigUpdated() override;

protected:
	void paint(Graphics&) override;
	void resized() override;

	void onApplyClicked();

	void onToggleRawConfigVisible();

private:
	std::unique_ptr<HeaderWithElmListComponent>	m_DS100Settings;
	std::unique_ptr<HeaderWithElmListComponent>	m_DiGiCoBridgingSettings;
	std::unique_ptr<HeaderWithElmListComponent>	m_GenericOSCBridgingSettings;

	std::unique_ptr<TextButton>		m_applyButton;
	std::unique_ptr<TextEditor>		m_settingsRawEditor;
	std::unique_ptr<ToggleButton>	m_useRawConfigButton;
	std::unique_ptr<Label>			m_useRawConfigLabel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CSettingsContainer)
};


} // namespace SoundscapeBridgeApp
