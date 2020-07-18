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
    auto processor = std::make_unique<SoundscapeApp::MainProcessor>();
    auto processorEditor = std::make_unique<SoundscapeApp::MainProcessorEditor>(*processor.get());
    addAndMakeVisible(processorEditor.get());

    m_processors.insert(std::make_pair(processor->GetPluginId(), std::make_pair(std::move(processor), std::move(processorEditor))));

    SoundscapeApp::COverviewManager* ovrMgr = SoundscapeApp::COverviewManager::GetInstance();
    if (ovrMgr)
    {
        m_overview = ovrMgr->GetOverview();
        addAndMakeVisible(m_overview);
    }

    setSize(976, 380);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint (juce::Graphics& /*g*/)
{

}

void MainComponent::resized()
{
    auto isPortrait = getLocalBounds().getHeight() > getLocalBounds().getWidth();
    auto safety = JUCEAppBasics::iOS_utils::getDeviceSafetyMargins();
    auto safeBounds = getLocalBounds();
    safeBounds.removeFromTop(safety._top);
    safeBounds.removeFromBottom(safety._bottom);
    safeBounds.removeFromLeft(safety._left);
    safeBounds.removeFromRight(safety._right);

    if (isPortrait)
    {
        FlexBox fb;
        fb.flexDirection = FlexBox::Direction::column;
        fb.justifyContent = FlexBox::JustifyContent::center;

        fb.items.addArray({
            FlexItem(*m_overview).withFlex(1),
            FlexItem(*m_processors.begin()->second.second.get()).withFlex(1)
            });
        fb.performLayout(safeBounds.toFloat());
    }
    else
    {
        FlexBox fb;
        fb.flexDirection = FlexBox::Direction::row;
        fb.justifyContent = FlexBox::JustifyContent::center;

        fb.items.addArray({
            FlexItem(*m_overview).withFlex(1),
            FlexItem(*m_processors.begin()->second.second.get()).withFlex(1)
            });
        fb.performLayout(safeBounds.toFloat());
    }
}
