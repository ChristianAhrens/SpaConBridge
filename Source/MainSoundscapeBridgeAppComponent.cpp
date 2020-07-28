/*
  ==============================================================================

    MainComponent.cpp
    Created: 18 Jul 2020 6:33:56pm
    Author:  musah

  ==============================================================================
*/

#include "MainSoundscapeBridgeAppComponent.h"

#include "Controller.h"

#include "Overview/OverviewManager.h"

#include "../submodules/JUCE-AppBasics/Source/iOS_utils.hpp"

#include <JuceHeader.h>

//==============================================================================
MainSoundscapeBridgeAppComponent::MainSoundscapeBridgeAppComponent()
{
    auto ctrl = SoundscapeBridgeApp::CController::GetInstance();
    ignoreUnused(ctrl);
    auto ovrMgr = SoundscapeBridgeApp::COverviewManager::GetInstance();
    if (ovrMgr)
    {
        m_overview = ovrMgr->GetOverview();
        addAndMakeVisible(m_overview);
    }

    setSize(896, 414);
}

MainSoundscapeBridgeAppComponent::~MainSoundscapeBridgeAppComponent()
{
    removeChildComponent(m_overview);
    m_overview = nullptr;

    auto ctrl = SoundscapeBridgeApp::CController::GetInstance();
    if (ctrl)
        ctrl->DestroyInstance();
}

void MainSoundscapeBridgeAppComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(SoundscapeBridgeApp::CDbStyle::GetDbColor(SoundscapeBridgeApp::CDbStyle::DarkColor));
}

void MainSoundscapeBridgeAppComponent::resized()
{
    auto safety = JUCEAppBasics::iOS_utils::getDeviceSafetyMargins();
    auto safeBounds = getLocalBounds();
    safeBounds.removeFromTop(safety._top);
    safeBounds.removeFromBottom(safety._bottom);
    safeBounds.removeFromLeft(safety._left);
    safeBounds.removeFromRight(safety._right);

    m_overview->setBounds(safeBounds);
}
