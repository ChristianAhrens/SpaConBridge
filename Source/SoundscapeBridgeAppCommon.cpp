/* Copyright (c) 2020-2021, Christian Ahrens
 *
 * This file is part of SoundscapeBridgeApp <https://github.com/ChristianAhrens/SoundscapeBridgeApp>
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

#include "SoundscapeBridgeAppCommon.h"

#include <JuceHeader.h>


namespace SoundscapeBridgeApp
{


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
	case PBT_GenericOSC:
		return "Generic OSC";
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
	case PBT_GenericOSC:
		return Colour(255, 217, 115);
	case PBT_BlacktraxRTTrPM:
		return Colour(0, 174, 239);
	case PBT_GenericMIDI:
		return Colour(110, 152, 196);
	case PBT_YamahaOSC:
		return Colour(72, 33, 122);
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


} // namespace SoundscapeBridgeApp
