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
        PROCESSORCOMSMODE,
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
        case PROCESSORCOMSMODE:
            return "ComsMode";
        case BRIDGING:
            return "Bridging";
		default:
			return "INVALID";
        }
	};

	enum AttributeID
	{
		PROCESSORSOURCEID,
	};
	static String getAttributeName(AttributeID Id)
	{
		switch (Id)
		{
        case PROCESSORSOURCEID:
            return "ProcSrcId";
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
