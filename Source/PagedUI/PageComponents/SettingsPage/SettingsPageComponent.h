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


#pragma once

#include "../PageComponentBase.h"

#include "../../../SpaConBridgeCommon.h"
#include "../../../AppConfiguration.h"
#include "../../../LookAndFeel.h"

/**
 * Fwd. decls.
 */
namespace JUCEAppBasics {
	class FixedFontTextEditor;
	class TextWithImageButton;
}


namespace SpaConBridge
{


/**
 * Forward declarations.
 */
class SettingsSectionsComponent;


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
	void onResetToDefaultClicked();
	void onLoadConfigClicked();
	void onSaveConfigClicked();
	void onToggleRawConfigVisible();

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
	juce::Point<int>							m_tempCachedViewPosition;	// helper member to store a view position that shall be kept when performing a resize

	std::unique_ptr<TextButton>							m_settingsRawApplyButton;
	std::unique_ptr<TextButton>							m_settingsResetToDefaultButton;
	std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>	m_settingsRawEditor;

	std::unique_ptr<JUCEAppBasics::TextWithImageButton>	m_loadConfigButton;
	std::unique_ptr<JUCEAppBasics::TextWithImageButton>	m_saveConfigButton;
	std::unique_ptr<TextButton>							m_useRawConfigButton;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsPageComponent)
};


} // namespace SpaConBridge
