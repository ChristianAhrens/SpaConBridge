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

#pragma once

#include <JuceHeader.h>

#include <AppConfigurationBase.h>

namespace SoundscapeBridgeApp
{

class AppConfiguration : public JUCEAppBasics::AppConfigurationBase
{

public:
    enum TagID
    {
        OVERVIEW,
        ACTIVEOVRTAB,
        CONTROLLER,
        SOUNDSOURCEPROCESSORS,
        PROCESSORINSTANCE,
		BRIDGING,
        LOOKANDFEELTYPE,
    };
    static String getTagName(TagID ID)
    {
        switch(ID)
        {
        case OVERVIEW:
            return "Overview";
        case ACTIVEOVRTAB:
            return "ActiveTab";
        case CONTROLLER:
            return "Controller";
        case SOUNDSOURCEPROCESSORS:
            return "SoundsourceProcessors";
        case PROCESSORINSTANCE:
            return "Proc";
        case BRIDGING:
            return "Bridging";
        case LOOKANDFEELTYPE:
            return "LookAndFeelType";
		default:
			return "INVALID";
        }
	};

	enum AttributeID
	{
		PROCESSORSOURCEID,
        PROCESSORMAPPINGID,
        PROCESSORCOMSMODE,
	};
	static String getAttributeName(AttributeID Id)
	{
		switch (Id)
		{
        case PROCESSORSOURCEID:
            return "ProcSrcId";
        case PROCESSORMAPPINGID:
            return "MappingId";
        case PROCESSORCOMSMODE:
            return "ComsMode";
		default:
			return "INVALID";
		}
	};

public:
	AppConfiguration(const File &file);
	~AppConfiguration() override;

	bool isValid() override;
    static bool isValid(const std::unique_ptr<XmlElement>& xmlConfig);

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppConfiguration)
};

}
