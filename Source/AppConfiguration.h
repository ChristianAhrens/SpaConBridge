/* Copyright (c) 2020-2022, Christian Ahrens
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

#include <JuceHeader.h>

#include <AppConfigurationBase.h>

namespace SpaConBridge
{

class AppConfiguration : public JUCEAppBasics::AppConfigurationBase
{

public:
    enum TagID
    {
        UICONFIG,
        ACTIVETAB,
        SOUNDOBJECTTABLE,
        MATRIXINPUTTABLE,
        MATRIXOUTPUTTABLE,
        ROWHEIGHT,
        COLLAPSED,
        CONTROLLER,
        SOUNDOBJECTPROCESSORS,
        MATRIXINPUTPROCESSORS,
        MATRIXOUTPUTPROCESSORS,
        PROCESSORINSTANCE,
		BRIDGING,
        LOOKANDFEELTYPE,
        ONLINESTATE,
        SCENESPAGE,
        PINNEDSCENES,
        SCENE,
        ENABLEDPAGES,
        MULTISLIDER,
        MAPPINGAREA,
        REVERBENABLED,
        SPREADENABLED,
        BACKGROUNDIMAGES,
        BACKGROUND,
        STATICOBJECTSPOLLING,
        FULLSCREENWINDOWMODE,
    };
    static String getTagName(TagID ID)
    {
        switch(ID)
        {
        case UICONFIG:
            return "UIConfig";
        case ACTIVETAB:
            return "ActiveTab";
        case SOUNDOBJECTTABLE:
            return "SoundObjectTable";
        case MATRIXINPUTTABLE:
            return "MatrixInputTable";
        case MATRIXOUTPUTTABLE:
            return "MatrixOutputTable";
        case ROWHEIGHT:
            return "RowHeight";
        case COLLAPSED:
            return "Collapsed";
        case CONTROLLER:
            return "Controller";
        case SOUNDOBJECTPROCESSORS:
            return "SoundobjectProcessors";
        case MATRIXINPUTPROCESSORS:
            return "MatrixInputProcessors";
        case MATRIXOUTPUTPROCESSORS:
            return "MatrixOutputProcessors";
        case PROCESSORINSTANCE:
            return "Proc";
        case BRIDGING:
            return "Bridging";
        case LOOKANDFEELTYPE:
            return "LookAndFeelType";
        case ONLINESTATE:
            return "Online";
        case SCENESPAGE:
            return "ScenesPage";
        case PINNEDSCENES:
            return "PinnedScenes";
        case SCENE:
            return "Scene";
        case ENABLEDPAGES:
            return "EnabledPages";
        case MULTISLIDER:
            return "MultiSlider";
        case MAPPINGAREA:
            return "MappingArea";
        case REVERBENABLED:
            return "ReverbEnabled";
        case SPREADENABLED:
            return "SpreadEnabled";
        case BACKGROUNDIMAGES:
            return "BackgroundImages";
        case BACKGROUND:
            return "Bkg";
        case STATICOBJECTSPOLLING:
            return "StaticObjectsPolling";
        case FULLSCREENWINDOWMODE:
            return "FullscreenWindowmode";
		default:
			return "INVALID";
        }
	};

	enum AttributeID
	{
		PROCESSORCHANNELID,
        PROCESSORRECORDID,
        PROCESSORCOMSMODE,
        PROCESSORCOLOUR,
        PROCESSORSIZE,
        INDEXMAJOR,
        INDEXMINOR,
	};
	static String getAttributeName(AttributeID Id)
	{
		switch (Id)
		{
        case PROCESSORCHANNELID:
            return "ChannelId";
        case PROCESSORRECORDID:
            return "RecordId";
        case PROCESSORCOMSMODE:
            return "ComsMode";
        case PROCESSORCOLOUR:
            return "Colour";
        case PROCESSORSIZE:
            return "Size";
        case INDEXMAJOR:
            return "IndexMajor";
        case INDEXMINOR:
            return "IndexMinor";
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
