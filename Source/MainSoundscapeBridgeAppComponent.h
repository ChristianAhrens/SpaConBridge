/*
  ==============================================================================

    MainComponent.h
    Created: 18 Jul 2020 6:33:56pm
    Author:  musah

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "SoundsourceProcessorEditor.h"
#include "SoundsourceProcessor.h"
#include "Overview.h"

//==============================================================================
/*
*/
class MainSoundscapeBridgeAppComponent  : public juce::Component
{
public:
    MainSoundscapeBridgeAppComponent();
    ~MainSoundscapeBridgeAppComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SoundscapeBridgeApp::COverviewComponent *m_overview{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainSoundscapeBridgeAppComponent)
};
