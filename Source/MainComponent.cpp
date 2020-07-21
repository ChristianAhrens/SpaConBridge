/*
  ==============================================================================

    MainComponent.cpp
    Created: 18 Jul 2020 6:33:56pm
    Author:  musah

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainComponent.h"

#include "../submodules/JUCE-AppBasics/Source/iOS_utils.hpp"

//==============================================================================
MainComponent::MainComponent()
{
    SoundscapeApp::COverviewManager* ovrMgr = SoundscapeApp::COverviewManager::GetInstance();
    if (ovrMgr)
    {
        m_overview = ovrMgr->GetOverview();
        addAndMakeVisible(m_overview);
    }

    setSize(896, 414);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(SoundscapeApp::CDbStyle::GetDbColor(SoundscapeApp::CDbStyle::DarkColor));
}

void MainComponent::resized()
{
    auto safety = JUCEAppBasics::iOS_utils::getDeviceSafetyMargins();
    auto safeBounds = getLocalBounds();
    safeBounds.removeFromTop(safety._top);
    safeBounds.removeFromBottom(safety._bottom);
    safeBounds.removeFromLeft(safety._left);
    safeBounds.removeFromRight(safety._right);

    m_overview->setBounds(safeBounds);
}
