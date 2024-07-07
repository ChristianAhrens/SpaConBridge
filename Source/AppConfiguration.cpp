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

#include "AppConfiguration.h"

namespace SpaConBridge
{

AppConfiguration::AppConfiguration(const File& file)
	: JUCEAppBasics::AppConfigurationBase()
{
	InitializeBase(file, JUCEAppBasics::AppConfigurationBase::Version::FromString(SPACONBRIDGE_CONFIG_VERSION));
}

AppConfiguration::~AppConfiguration()
{
}

bool AppConfiguration::isValid()
{
	return isValid(m_xml);
}

bool AppConfiguration::isValid(const std::unique_ptr<XmlElement>& xmlConfig)
{
	if (!JUCEAppBasics::AppConfigurationBase::isValid(xmlConfig))
		return false;

	auto uiCfgSectionElement = xmlConfig->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::UICONFIG));
	if (uiCfgSectionElement)
	{
		auto activeTabXmlElement = uiCfgSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::ACTIVETAB));
		if (activeTabXmlElement)
		{
			auto activeTabTextElement = activeTabXmlElement->getFirstChildElement();
			if (!activeTabTextElement || !activeTabTextElement->isTextElement())
				return false;
		}
		else
			return false;

		auto lookAndFeelTypeXmlElement = uiCfgSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::LOOKANDFEELTYPE));
		if (lookAndFeelTypeXmlElement)
		{
			auto lookAndFeelTypeTextElement = lookAndFeelTypeXmlElement->getFirstChildElement();
			if (!lookAndFeelTypeTextElement || !lookAndFeelTypeTextElement->isTextElement())
				return false;
		}
		else
			return false;

#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
		auto fullscreenWindowModeXmlElement = uiCfgSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::FULLSCREENWINDOWMODE));
		if (fullscreenWindowModeXmlElement)
		{
			auto fullscreenWindowModeTextElement = fullscreenWindowModeXmlElement->getFirstChildElement();
			if (!fullscreenWindowModeTextElement || !fullscreenWindowModeTextElement->isTextElement())
				return false;
		}
		else
			return false;
#endif

	}
	else
		return false;

	auto ctrlSectionElement = xmlConfig->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::CONTROLLER));
	if (ctrlSectionElement)
	{
		auto soundSourceProcessorsSectionElement = ctrlSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::SOUNDOBJECTPROCESSORS));
		if (!soundSourceProcessorsSectionElement)
			return false;
		auto matrixInputProcessorsSectionElement = ctrlSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::MATRIXINPUTPROCESSORS));
		if (!matrixInputProcessorsSectionElement)
			return false;
		auto matrixOutputProcessorsSectionElement = ctrlSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::MATRIXOUTPUTPROCESSORS));
		if (!matrixOutputProcessorsSectionElement)
			return false;
		auto bridgingSectionElement = ctrlSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::BRIDGING));
		if (!bridgingSectionElement)
			return false;
	}
	else
		return false;

	return true;
}

bool AppConfiguration::ResetToDefault()
{
	auto xmlConfig = juce::parseXML(String(BinaryData::Default_config, BinaryData::Default_configSize));
	if (xmlConfig)
	{

		if (SpaConBridge::AppConfiguration::isValid(xmlConfig))
		{

			SetFlushAndUpdateDisabled();
			if (resetConfigState(std::move(xmlConfig)))
			{
				ResetFlushAndUpdateDisabled();
				return true;
			}
			else
			{
				jassertfalse; // stop here when debugging, since invalid configurations often lead to endless debugging sessions until this simple explanation was found...
				ResetFlushAndUpdateDisabled();

				// ...and trigger generation of a valid config if not.
				triggerConfigurationDump();
			}
		}
		else
		{
			jassertfalse; // stop here when debugging, since invalid configurations often lead to endless debugging sessions until this simple explanation was found...

			// ...and trigger generation of a valid config if not.
			triggerConfigurationDump();
		}
	}
	else
	{
		jassertfalse; // stop here when debugging, since invalid configurations often lead to endless debugging sessions until this simple explanation was found...

		// ...and trigger generation of a valid config if not.
		triggerConfigurationDump();
	}

	return false;
}

bool AppConfiguration::HandleConfigVersionConflict(const JUCEAppBasics::AppConfigurationBase::Version& configVersionFound)
{
	if (configVersionFound != JUCEAppBasics::AppConfigurationBase::Version::FromString(SPACONBRIDGE_CONFIG_VERSION))
	{
		auto conflictTitle = "Incompatible configuration version";
		auto conflictInfo = "The configuration file version detected\ncannot be handled by this version of " + juce::JUCEApplication::getInstance()->getApplicationName();
#ifdef DEBUG
		conflictInfo << "\n(Found " + configVersionFound.ToString() + ", expected " + SPACONBRIDGE_CONFIG_VERSION + ")";
#endif
		juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::WarningIcon, conflictTitle, conflictInfo, "Reset to default", "Quit", nullptr, ModalCallbackFunction::create([this](int result) {
			if (1 == result)
			{
				ResetToDefault();
			}
			else
			{
				juce::JUCEApplication::getInstance()->quit();
			}
		}));

		return false;
	}
	else
		return true;
}	


} // namespace SpaConBridge
