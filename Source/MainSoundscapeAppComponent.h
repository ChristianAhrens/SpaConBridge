/*
  ==============================================================================

    MainComponent.h
    Created: 18 Jul 2020 6:33:56pm
    Author:  musah

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PluginEditor.h"
#include "PluginProcessor.h"
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
    //std::map<juce::int32, std::pair<std::unique_ptr<SoundscapeApp::MainProcessor>, std::unique_ptr<SoundscapeApp::MainProcessorEditor>>> m_processors;
    SoundscapeApp::COverviewComponent *m_overview{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainSoundscapeAppComponent)
};
