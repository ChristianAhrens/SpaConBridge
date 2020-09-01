/*
  ==============================================================================

    MainComponent.h
    Created: 18 Jul 2020 6:33:56pm
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "AppConfiguration.h"

#include <JuceHeader.h>


namespace SoundscapeBridgeApp
{


//==============================================================================
/*
 */
class MainSoundscapeBridgeAppComponent :    public juce::Component,
                                            public AppConfiguration::Dumper,
                                            public AppConfiguration::Watcher
{
public:
    MainSoundscapeBridgeAppComponent();
    ~MainSoundscapeBridgeAppComponent() override;

    //==========================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    //==========================================================================
    void performConfigurationDump() override;

    //==========================================================================
    void onConfigUpdated() override;

private:
    std::unique_ptr<AppConfiguration>           m_config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainSoundscapeBridgeAppComponent)
};

};