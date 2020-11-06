/*
  ==============================================================================

    MainComponent.cpp
    Created: 18 Jul 2020 6:33:56pm
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "MainSoundscapeBridgeAppComponent.h"

#include "Controller.h"

#include "Overview/Overview.h"
#include "Overview/OverviewManager.h"

#include "SoundsourceProcessor/SoundsourceProcessor.h"

#include <iOS_utils.hpp>

#include <JuceHeader.h>

namespace SoundscapeBridgeApp
{

//==============================================================================
MainSoundscapeBridgeAppComponent::MainSoundscapeBridgeAppComponent()
    : MainSoundscapeBridgeAppComponent(nullptr)
{
}

MainSoundscapeBridgeAppComponent::MainSoundscapeBridgeAppComponent(std::function<void(DbLookAndFeelBase::LookAndFeelType)> lafUpdateCallback)
    : onUpdateLookAndFeel(lafUpdateCallback)
{
    m_config = std::make_unique<AppConfiguration>(JUCEAppBasics::AppConfigurationBase::getDefaultConfigFilePath());
    m_config->addDumper(this);
    m_config->addWatcher(this);

    // check if config creation was able to read a valid config from disk...
    if (!m_config->isValid())
    {
        jassertfalse; // stop here when debugging, since invalid configurations often lead to endless debugging sessions until this simple explanation was found...

        // ...and trigger generation of a valid config if not.
        m_config->triggerConfigurationDump();
    }

    // enshure the config is processed and contents forwarded to already existing application components.
    onConfigUpdated();
    m_config->triggerWatcherUpdate();

    // enshure the controller singleton is created
    auto ctrl = SoundscapeBridgeApp::CController::GetInstance();
    ignoreUnused(ctrl);

    // enshure the overviewmanager singleton is created
    auto pageMgr = SoundscapeBridgeApp::PageComponentManager::GetInstance();
    if (pageMgr)
    {
        // get the overview component from manager to use as central element for app ui
        auto pageContainer = pageMgr->GetPageContainer();
        addAndMakeVisible(pageContainer);
    }

    setSize(896, 414);
}

MainSoundscapeBridgeAppComponent::~MainSoundscapeBridgeAppComponent()
{
    if (m_config)
    {
        m_config->clearDumpers();
        m_config->clearWatchers();
    }

    auto pageMgr = SoundscapeBridgeApp::PageComponentManager::GetInstance();
    if (pageMgr)
    {
        auto pageContainer = pageMgr->GetPageContainer();
        removeChildComponent(pageContainer);
        pageMgr->ClosePageContainer(true);
    }

    auto ctrl = SoundscapeBridgeApp::CController::GetInstance();
    if (ctrl)
    {
        // Delete the processor instances held in controller externally,
        // since we otherwise would run into a loop ~CController -> CController::RemoveProcessor -> 
        // ~SoundsourceProcessor -> CController::RemoveProcessor
        while(ctrl->GetProcessorCount() > 0)
            delete ctrl->GetProcessor(0);

        ctrl->DestroyInstance();
    }
}

void MainSoundscapeBridgeAppComponent::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void MainSoundscapeBridgeAppComponent::resized()
{
    auto safety = JUCEAppBasics::iOS_utils::getDeviceSafetyMargins();
    auto safeBounds = getLocalBounds();
    safeBounds.removeFromTop(safety._top);
    safeBounds.removeFromBottom(safety._bottom);
    safeBounds.removeFromLeft(safety._left);
    safeBounds.removeFromRight(safety._right);

    auto pageMgr = SoundscapeBridgeApp::PageComponentManager::GetInstance();
    if (pageMgr)
    {
        auto pageContainer = pageMgr->GetPageContainer();
        if (pageContainer)
            pageContainer->setBounds(safeBounds);
    }
}

void MainSoundscapeBridgeAppComponent::performConfigurationDump()
{
    auto ctrl = SoundscapeBridgeApp::CController::GetInstance();
    if (ctrl)
        m_config->setConfigState(ctrl->createStateXml());

    auto pageMgr = SoundscapeBridgeApp::PageComponentManager::GetInstance();
    if (pageMgr)
        m_config->setConfigState(pageMgr->createStateXml());
}

void MainSoundscapeBridgeAppComponent::onConfigUpdated()
{
    // get all the modules' configs first, because the initialization process might already trigger dumping, that would override data
    auto ctrlConfigState = m_config->getConfigState(AppConfiguration::getTagName(AppConfiguration::TagID::CONTROLLER));
    auto ovrConfigState = m_config->getConfigState(AppConfiguration::getTagName(AppConfiguration::TagID::OVERVIEW));

    // set the controller modules' config
    auto ctrl = SoundscapeBridgeApp::CController::GetInstance();
    if (ctrl)
        ctrl->setStateXml(ctrlConfigState.get());

    // set the overview manager modules' config
    auto pageMgr = SoundscapeBridgeApp::PageComponentManager::GetInstance();
    if (pageMgr)
        pageMgr->setStateXml(ovrConfigState.get());

    // set the lookandfeel config (forwards to MainWindow where the magic happens)
    if (ovrConfigState)
    {
        auto lookAndFeelXmlElement = ovrConfigState->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::LOOKANDFEELTYPE));
        if (lookAndFeelXmlElement)
        {
            auto lookAndFeelTextElement = lookAndFeelXmlElement->getFirstChildElement();
            if (lookAndFeelTextElement && lookAndFeelTextElement->isTextElement())
            {
                auto lookAndFeelType = static_cast<DbLookAndFeelBase::LookAndFeelType>(lookAndFeelTextElement->getText().getIntValue());
                
                jassert(lookAndFeelType > DbLookAndFeelBase::LookAndFeelType::LAFT_InvalidFirst && lookAndFeelType < DbLookAndFeelBase::LookAndFeelType::LAFT_InvalidLast);

                if (onUpdateLookAndFeel)
                    onUpdateLookAndFeel(lookAndFeelType);
            }
        }
    }
}

}