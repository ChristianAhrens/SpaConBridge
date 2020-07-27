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
class MainSoundscapeAppComponent  : public juce::Component
{
public:
    MainSoundscapeAppComponent();
    ~MainSoundscapeAppComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SoundscapeApp::COverviewComponent *m_overview{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainSoundscapeAppComponent)
};
