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
        LOOKANDFEEL,
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
        case LOOKANDFEEL:
            return "LookAndFeel";
		default:
			return "INVALID";
        }
	};

	enum AttributeID
	{
		PROCESSORSOURCEID,
        PROCESSORMAPPINGID,
        PROCESSORCOMSMODE,
        LOOKANDFEELTYPE,
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
        case LOOKANDFEELTYPE:
            return "LookAndFeelType";
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
