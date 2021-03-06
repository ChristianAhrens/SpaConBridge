/* Copyright (c) 2020-2021, Christian Ahrens
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
	: JUCEAppBasics::AppConfigurationBase(file)
{

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

}
