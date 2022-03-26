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

#include "SpaConBridgeCommon.h"

#include "LookAndFeel.h"

#include <Image_utils.h>
#include <JuceHeader.h>


namespace SpaConBridge
{



/**
 * Helper method to derive the UI page name from a given page id enum value.
 * @param	pageId	The page id to get the string representation for.
 * @return	The string representation for the given page id.
 */
String GetPageNameFromId(UIPageId pageId)
{
	switch (pageId)
	{
		case UPI_InvalidMin:
			return "InvalidMin";
		case UPI_SoundObjects:
			return "SoundObjects";
		case UPI_MultiSlider:
			return "MultiSlider";
		case UPI_MatrixIOs:
			return "MatrixIOs";
		case UPI_Scenes:
			return "Scenes";
		case UPI_EnSpace:
			return "EnSpace";
		case UPI_Statistics:
			return "Statistics";
		case UPI_Settings:
			return "Settings";
		case UPI_InvalidMax:
		default:
			return "InvalidMax";
	}
}

/**
 * Helper method to derive the UI page id enum value from a given page name.
 * @param	pageName	The page name to get the id enum value for.
 * @return	The id enum value for the given page name.
 */
UIPageId GetPageIdFromName(String pageName)
{
	if (pageName == GetPageNameFromId(UPI_InvalidMin))
		return UPI_InvalidMin;
	if (pageName == GetPageNameFromId(UPI_SoundObjects))
		return UPI_SoundObjects;
	if (pageName == GetPageNameFromId(UPI_MultiSlider))
		return UPI_MultiSlider;
	if (pageName == GetPageNameFromId(UPI_MatrixIOs))
		return UPI_MatrixIOs;
	if (pageName == GetPageNameFromId(UPI_Scenes))
		return UPI_Scenes;
	if (pageName == GetPageNameFromId(UPI_EnSpace))
		return UPI_EnSpace;
	if (pageName == GetPageNameFromId(UPI_Statistics))
		return UPI_Statistics;
	if (pageName == GetPageNameFromId(UPI_Settings))
		return UPI_Settings;
	if (pageName == GetPageNameFromId(UPI_InvalidMax))
		return UPI_InvalidMax;
	else
		return UPI_InvalidMin;
}

/**
 * Helper method to query a user readable short name for a protocol type that can be shown on UI.
 * @param type	The type value to get a short name for.
 * @return	The short name string
 */
String GetProtocolBridgingShortName(ProtocolBridgingType type)
{
	switch (type)
	{
		case PBT_DiGiCo:
			return "DiGiCo";
		case PBT_DAWPlugin:
			return "DAW";
		case PBT_GenericOSC:
			return "OSC";
		case PBT_BlacktraxRTTrPM:
			return "Blacktrax";
		case PBT_GenericMIDI:
			return "MIDI";
		case PBT_YamahaSQ:
			return "YamahaSQ";
		case PBT_HUI:
			return "HUI";
		case PBT_YamahaOSC:
			return "Yamaha";
		case PBT_ADMOSC:
			return "ADM";
		case PBT_DS100:
			return "DS100";
		case PBT_None:
		default:
			return "";
	}
}

/**
 * Helper method to query a user readable name for a protocol type that can be shown on UI.
 * @param type	The type value to get a nice name for.
 * @return	The nice name string
 */
String GetProtocolBridgingNiceName(ProtocolBridgingType type)
{
	switch (type)
	{
	case PBT_DiGiCo:
		return "DiGiCo OSC";
	case PBT_DAWPlugin:
		return "d&b DAW Plugin";
	case PBT_GenericOSC:
		return "d&b Generic OSC";
	case PBT_BlacktraxRTTrPM:
		return "Blacktrax RTTrPM";
	case PBT_GenericMIDI:
		return "Generic MIDI";
	case PBT_YamahaSQ:
		return "Yamaha SQ";
	case PBT_HUI:
		return "Generic HUI";
	case PBT_YamahaOSC:
		return "Yamaha OSC";
	case PBT_ADMOSC:
		return "ADM OSC";
	case PBT_DS100:
		return "DS100";
	case PBT_None:
	default:
		return "";
	}
}

/**
 * Helper method to query a identifying name string for a protocol type that can be used in code or config files.
 * @param type	The type value to get a string id for.
 * @return	The system name string
 */
String GetProtocolBridgingSystemName(ProtocolBridgingType type)
{
	switch (type)
	{
	case PBT_DiGiCo:
		return "DiGiCoOSC";
	case PBT_DAWPlugin:
		return "DAWPlugin";
	case PBT_GenericOSC:
		return "GenericOSC";
	case PBT_BlacktraxRTTrPM:
		return "BlacktraxRTTrPM";
	case PBT_GenericMIDI:
		return "GenericMIDI";
	case PBT_YamahaSQ:
		return "DummyYamahaSQ";
	case PBT_HUI:
		return "DummyHUI";
	case PBT_YamahaOSC:
		return "YamahaOSC";
	case PBT_ADMOSC:
		return "ADMOSC";
	case PBT_DS100:
		return "DS100OSCPolling";
	case PBT_None:
	default:
		return "INVALID";
	}
}

/**
 * Helper method to query a identifying name string for a protocol type that can be used in code or config files.
 * @param type	The type value to get a string id for.
 * @return	The system name string
 */
const Colour GetProtocolBridgingColour(ProtocolBridgingType type)
{
	switch (type)
	{
	case PBT_DiGiCo:
		return Colour(140, 46, 52);
	case PBT_DAWPlugin:
		return Colour(180, 180, 180);
	case PBT_GenericOSC:
		return Colour(255, 217, 115);
	case PBT_BlacktraxRTTrPM:
		return Colour(0, 174, 239);
	case PBT_GenericMIDI:
		return Colour(110, 152, 196);
	case PBT_YamahaOSC:
		return Colour(72, 33, 122);
	case PBT_ADMOSC:
		return Colour(217, 0, 122);
	case PBT_YamahaSQ:
	case PBT_DS100:
	case PBT_HUI:
	case PBT_None:
	default:
		return Colour();
	}
}

/**
 * Helper method to query web repository base url (on gitub).
 */
String GetRepositoryBaseWebUrl()
{
	auto githubURL = String("https://www.github.com");
	auto companyName = String("ChristianAhrens");
	auto appName = JUCEApplication::getInstance()->getApplicationName();
	auto repoBasePath = String("blob/master");
	auto URLString = githubURL + "/" + companyName + "/" + appName + "/" + repoBasePath + "/";

	return URLString;
}

/**
 * Helper method to query web documentation base url (on gitub).
 */
String GetDocumentationBaseWebUrl()
{
	auto docuBasePath = String("Resources/Documentation/");
	return GetRepositoryBaseWebUrl() + docuBasePath;
}

/**
 * Helper method to query web documentation (on gitub) section identification string.
 * @param	pageId	The id of the page (documentation section) to get the identification string for.
 * @return	The identification string for the given documentation section.
 */
String GetDocumentationSectionIdentification(UIPageId pageId)
{
	switch (pageId)
	{
	case UPI_SoundObjects:
		return "#sound-object-table";
	case UPI_MultiSlider:
		return "#twodimensionalpositionslider";
	case UPI_MatrixIOs:
		return "#matrix-inputsoutputs-table";
	case UPI_Scenes:
		return "#scenes";
	case UPI_EnSpace:
		return "#en-space";
	case UPI_Statistics:
		return "#statistics";
	case UPI_Settings:
		return "#settings";
	default:
		return "";
	}
}

/**
 * Helper method to get a user readable error title string for a given error code.
 * @param	errorCode	The errorcode to get a title string for.
 * @return	The title string for the errorcode given.
 */
const String GetErrorTitle(const SpaConBridgeErrorCode errorCode)
{
	switch (errorCode)
	{
	case SEC_LoadConfig_CannotAccess:
	case SEC_LoadConfig_InternalError:
	case SEC_LoadConfig_InvalidFile:
	case SEC_LoadConfig_InvalidConfig:
	case SEC_LoadConfig_ConfigInit:
		return "Loading Failed";
	case SEC_SaveConfig_CannotAccess:
	case SEC_SaveConfig_InternalError:
	case SEC_SaveConfig_InvalidInternalConfig:
	case SEC_SaveConfig_CannotWrite:
		return "Saving Failed";
	case SEC_LoadImage_CannotAccess:
	case SEC_LoadImage_CannotRead:
	case SEC_LoadImage_InvalidImage:
		return "Loading Image Failed";
	case SEC_None:
	default:
		return "Error";
	}
}

/**
 * Helper method to get a user readable error info string for a given error code.
 * @param	errorCode	The errorcode to get an info string for.
 * @return	The info string for the errorcode given.
 */
const String GetErrorInfo(const SpaConBridgeErrorCode errorCode)
{
	switch (errorCode)
	{
	case SEC_LoadConfig_CannotAccess:
		return JUCEApplication::getInstance()->getApplicationName() + " is not allowed to access the chosen configuration file.";
	case SEC_LoadConfig_InternalError:
		return JUCEApplication::getInstance()->getApplicationName() + " encountered an error with its internal configuration.";
	case SEC_LoadConfig_InvalidFile:
		return "The chosen configuration file is invalid for " + JUCEApplication::getInstance()->getApplicationName() + " to initialize from.";
	case SEC_LoadConfig_InvalidConfig:
		return "The chosen configuration file content is invalid for " + JUCEApplication::getInstance()->getApplicationName() + " to initialize from.";
	case SEC_LoadConfig_ConfigInit:
		return JUCEApplication::getInstance()->getApplicationName() + " cannot initialize its configuration with given configuration file.";
	case SEC_SaveConfig_CannotAccess:
		return JUCEApplication::getInstance()->getApplicationName() + " is not allowed to access the chosen configuration file destination.";
	case SEC_SaveConfig_InternalError:
		return JUCEApplication::getInstance()->getApplicationName() + " encountered an error with its internal configuration.";
	case SEC_SaveConfig_InvalidInternalConfig:
		return JUCEApplication::getInstance()->getApplicationName() + " encountered an error with its internal configuration contents.";
	case SEC_SaveConfig_CannotWrite:
		return JUCEApplication::getInstance()->getApplicationName() + " is not allowed to write to the chosen configuration file destination.";
	case SEC_LoadImage_CannotAccess:
		return JUCEApplication::getInstance()->getApplicationName() + " is not allowed to access the chosen image.";
	case SEC_LoadImage_CannotRead:
		return JUCEApplication::getInstance()->getApplicationName() + " is not allowed to read the chosen image.";
	case SEC_LoadImage_InvalidImage:
		return "The chosen image is invalid for usage in " + JUCEApplication::getInstance()->getApplicationName();
	case SEC_None:
	default:
		return "No details available.";
	}
}

/**
 * Helper method to show a notification popup for the user
 * based on the given errorCode.
 * @param	errorCode	The errorcode to show the user notification popup for.
 */
void ShowUserErrorNotification(const SpaConBridgeErrorCode errorCode)
{
	auto errorTitleString = GetErrorTitle(errorCode);
	auto errorInfoString = GetErrorInfo(errorCode);
	AlertWindow::showMessageBoxAsync(AlertWindow::AlertIconType::WarningIcon, errorTitleString, errorInfoString);
}

/**
 * Helper method to update the state images of a given DrawableButton with the given binary
 * SVG image resource data combined with current lookAndFeel settings regarding colouring.
 * This is a proxy implementation forwarding the call to overload using DrawableButton pointer parameter.
 * @param	button				The button object to update.
 * @param	binarySVGStringData	The SVG image data to use to update button images.
 * @param	lookAndFeel			Pointer to the current lookAndFeel. Must be DbLookAndFeelBase derived object pointer.
 * @return	True if the images were successfully set to the given DrawableButton. False if any error occured.
 */
bool UpdateDrawableButtonImages(const std::unique_ptr<JUCEAppBasics::TextWithImageButton>& button, const String& binarySVGStringData, LookAndFeel* lookAndFeel)
{
	return UpdateDrawableButtonImages(button.get(), binarySVGStringData, lookAndFeel);
}

/**
 * Helper method to update the state images of a given DrawableButton with the given binary
 * SVG image resource data combined with current lookAndFeel settings regarding colouring.
 * This is a proxy implementation forwarding the call to overload using DrawableButton pointer parameter.
 * @param	button				The button object to update.
 * @param	binarySVGStringData	The SVG image data to use to update button images.
 * @param	lookAndFeel			Pointer to the current lookAndFeel. Must be DbLookAndFeelBase derived object pointer.
 * @return	True if the images were successfully set to the given DrawableButton. False if any error occured.
 */
bool UpdateDrawableButtonImages(const std::unique_ptr<DrawableButton>& button, const String& binarySVGStringData, LookAndFeel* lookAndFeel)
{
	return UpdateDrawableButtonImages(button.get(), binarySVGStringData, lookAndFeel);
}

/**
 * Helper method to update the state images of a given DrawableButton with the given binary
 * SVG image resource data combined with current lookAndFeel settings regarding colouring.
 * @param	button				The button object to update.
 * @param	binarySVGStringData	The SVG image data to use to update button images.
 * @param	lookAndFeel			Pointer to the current lookAndFeel. Must be DbLookAndFeelBase derived object pointer.
 * @return	True if the images were successfully set to the given DrawableButton. False if any error occured.
 */
bool UpdateDrawableButtonImages(DrawableButton* button, const String& binarySVGStringData, LookAndFeel* lookAndFeel)
{
	if (button == nullptr)
		return false;
	if (lookAndFeel == nullptr)
		return false;

	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(lookAndFeel);
	if (dblookAndFeel == nullptr)
		return false;

	// create the required button drawable images based on lookandfeel colours
	std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
	JUCEAppBasics::Image_utils::getDrawableButtonImages(binarySVGStringData, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

	button->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());

	return true;
}


} // namespace SpaConBridge
