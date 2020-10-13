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

	XmlElement* ovrSectionElement = m_xml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::OVERVIEW));
	if (ovrSectionElement)
	{
		if (!ovrSectionElement->hasAttribute(AppConfiguration::getTagName(AppConfiguration::TagID::ACTIVEOVRTAB)))
			return false;
	}
	else
		return false;

	XmlElement* lafSectionElement = m_xml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::LOOKANDFEEL));
	if (lafSectionElement)
	{
		if (!lafSectionElement->hasAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::LOOKANDFEELTYPE)))
			return false;
	}
	else
		return false;

	XmlElement* ctrlSectionElement = m_xml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::CONTROLLER));
	if (ctrlSectionElement)
	{
		XmlElement* soundSourceProcessorsSectionElement = ctrlSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::SOUNDSOURCEPROCESSORS));
		if (!soundSourceProcessorsSectionElement)
			return false;
		XmlElement* bridgingSectionElement = ctrlSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::BRIDGING));
		if (!bridgingSectionElement)
			return false;
	}
	else
		return false;

	return true;
}

}
