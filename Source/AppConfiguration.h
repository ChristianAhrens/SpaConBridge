#pragma once

#include <JuceHeader.h>

#include "../submodules/JUCE-AppBasics/Source/AppConfigurationBase.h"

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
		default:
			return "INVALID";
        }
	};

public:
	AppConfiguration(const File &file);
	~AppConfiguration() override;

	bool isValid() override;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppConfiguration)
};

}
