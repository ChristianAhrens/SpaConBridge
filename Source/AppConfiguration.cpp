#include "AppConfiguration.h"

namespace SoundscapeBridgeApp
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
	if (!JUCEAppBasics::AppConfigurationBase::isValid())
		return false;

	auto ovrSectionElement = m_xml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::OVERVIEW));
	if (ovrSectionElement)
	{
		auto activeTabXmlElement = ovrSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::ACTIVEOVRTAB));
		if (activeTabXmlElement)
		{
			auto activeTabTextElement = activeTabXmlElement->getFirstChildElement();
			if (!activeTabTextElement || !activeTabTextElement->isTextElement())
				return false;
		}
		else
			return false;

		auto lookAndFeelTypeXmlElement = ovrSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::LOOKANDFEELTYPE));
		if (lookAndFeelTypeXmlElement)
		{
			auto lookAndFeelTypeTextElement = lookAndFeelTypeXmlElement->getFirstChildElement();
			if (!lookAndFeelTypeTextElement || !lookAndFeelTypeTextElement->isTextElement())
				return false;
		}
		else
			return false;

	}
	else
		return false;

	auto ctrlSectionElement = m_xml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::CONTROLLER));
	if (ctrlSectionElement)
	{
		auto soundSourceProcessorsSectionElement = ctrlSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::SOUNDSOURCEPROCESSORS));
		if (!soundSourceProcessorsSectionElement)
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
