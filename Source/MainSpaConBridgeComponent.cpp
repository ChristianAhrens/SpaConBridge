/* Copyright (c) 2020-2022, Christian Ahrens
 *
 * This file is part of SpaConBridge <https://github.com/ChristianAhrens/SpaConBridge>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "MainSpaConBridgeComponent.h"

#include "Controller.h"
#include "ProcessorSelectionManager.h"

#include "PagedUI/PageContainerComponent.h"
#include "PagedUI/PageComponentManager.h"

#include "CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"
#include "CustomAudioProcessors/MatrixInputProcessor/MatrixInputProcessor.h"
#include "CustomAudioProcessors/MatrixOutputProcessor/MatrixOutputProcessor.h"

#include "WaitingEntertainerComponent.h"

#include <iOS_utils.h>

#include <JuceHeader.h>

namespace SpaConBridge
{

//==============================================================================
MainSpaConBridgeComponent::MainSpaConBridgeComponent()
    : MainSpaConBridgeComponent(nullptr)
{
}

MainSpaConBridgeComponent::MainSpaConBridgeComponent(std::function<void(DbLookAndFeelBase::LookAndFeelType)> lafUpdateCallback)
    : onUpdateLookAndFeel(lafUpdateCallback)
{
    addChildComponent(WaitingEntertainerComponent::GetInstance());

    // a single instance of tooltip window is required and used by JUCE everywhere a tooltip is required.
    m_toolTipWindowInstance = std::make_unique<TooltipWindow>();

    // create the configuration object (is being initialized from disk automatically)
    m_config = std::make_unique<AppConfiguration>(JUCEAppBasics::AppConfigurationBase::getDefaultConfigFilePath());
    m_config->addDumper(this);

    // check if config creation was able to read a valid config from disk...
    if (!m_config->isValid())
    {
        m_config->ResetToDefault();
    }

    // add this main component to watchers
    m_config->addWatcher(this, true); // this initial update cannot yet reach all parts of the app, esp. settings page that relies on fully initialized pagecomponentmanager, therefor a manual watcher update is triggered below

    // enshure the controller singleton is created
    auto ctrl = SpaConBridge::Controller::GetInstance();
    jassert(ctrl);
    ignoreUnused(ctrl);
    // enshure the pagemanager singleton is created
    auto pageMgr = SpaConBridge::PageComponentManager::GetInstance();
    jassert(pageMgr);

    // get the overview component from manager to use as central element for app ui
    auto pageContainer = pageMgr->GetPageContainer();
    addAndMakeVisible(pageContainer);

    // do the initial update for the whole application with config contents
    m_config->triggerWatcherUpdate();

    setSize(960, 640);
}

MainSpaConBridgeComponent::~MainSpaConBridgeComponent()
{
    if (WaitingEntertainerComponent::GetInstance())
        removeChildComponent(WaitingEntertainerComponent::GetInstance());

    if (m_config)
    {
        m_config->clearDumpers();
        m_config->clearWatchers();
    }

    auto pageMgr = SpaConBridge::PageComponentManager::GetInstance();
    if (pageMgr)
    {
        auto pageContainer = pageMgr->GetPageContainer();
        removeChildComponent(pageContainer);
        pageMgr->ClosePageContainer(true);
    }

    auto ctrl = SpaConBridge::Controller::GetInstance();
    if (ctrl)
    {
        // Delete the processor instances held in controller externally,
        // since we otherwise would run into a loop ~Controller -> Controller::RemoveProcessor -> 
        // ~SoundobjectProcessor -> Controller::RemoveProcessor

        for (auto const& sopId : ctrl->GetSoundobjectProcessorIds())
        {
            auto processor = std::unique_ptr<SoundobjectProcessor>(ctrl->GetSoundobjectProcessor(sopId)); // when processor goes out of scope, it is destroyed and the destructor does handle unregistering from ccontroller by itself
            std::unique_ptr<AudioProcessorEditor>(processor->getActiveEditor()).reset();
            processor->releaseResources();
        }

        for (auto const& mipId : ctrl->GetMatrixInputProcessorIds())
        {
            auto processor = std::unique_ptr<MatrixInputProcessor>(ctrl->GetMatrixInputProcessor(mipId)); // when processor goes out of scope, it is destroyed and the destructor does handle unregistering from ccontroller by itself
            std::unique_ptr<AudioProcessorEditor>(processor->getActiveEditor()).reset();
            processor->releaseResources();
        }
        
        for (auto const& mopId : ctrl->GetMatrixOutputProcessorIds())
        {
            auto processor = std::unique_ptr<MatrixOutputProcessor>(ctrl->GetMatrixOutputProcessor(mopId)); // when processor goes out of scope, it is destroyed and the destructor does handle unregistering from ccontroller by itself
            std::unique_ptr<AudioProcessorEditor>(processor->getActiveEditor()).reset();
            processor->releaseResources();
        }

        ctrl->DestroyInstance();
    }

    auto const& selMgr = ProcessorSelectionManager::GetInstance();
    if (selMgr)
    {
        selMgr->DestroyInstance();
    }

    WaitingEntertainerComponent::GetInstance()->DestroyInstance();
}

void MainSpaConBridgeComponent::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void MainSpaConBridgeComponent::resized()
{
    auto safety = JUCEAppBasics::iOS_utils::getDeviceSafetyMargins();
    auto safeBounds = getLocalBounds();
    safeBounds.removeFromTop(safety._top);
    safeBounds.removeFromBottom(safety._bottom);
    safeBounds.removeFromLeft(safety._left);
    safeBounds.removeFromRight(safety._right);

    auto pageMgr = SpaConBridge::PageComponentManager::GetInstance();
    if (pageMgr)
    {
        auto pageContainer = pageMgr->GetPageContainer();
        if (pageContainer)
            pageContainer->setBounds(safeBounds);
    }

    if (WaitingEntertainerComponent::GetInstance() && WaitingEntertainerComponent::GetInstance()->isVisible())
    {
        WaitingEntertainerComponent::GetInstance()->setBounds(getLocalBounds());
    }
}

void MainSpaConBridgeComponent::performConfigurationDump()
{
    auto ctrl = SpaConBridge::Controller::GetInstance();
    if (ctrl)
        m_config->setConfigState(ctrl->createStateXml());

    auto pageMgr = SpaConBridge::PageComponentManager::GetInstance();
    if (pageMgr)
        m_config->setConfigState(pageMgr->createStateXml());

    auto selMgr = SpaConBridge::ProcessorSelectionManager::GetInstance();
    if (selMgr)
        m_config->setConfigState(selMgr->createStateXml());
}

void MainSpaConBridgeComponent::onConfigUpdated()
{
    // get all the modules' configs first, because the initialization process might already trigger dumping, that would override data
    auto ctrlConfigState = m_config->getConfigState(AppConfiguration::getTagName(AppConfiguration::TagID::CONTROLLER));
    auto uiCfgState = m_config->getConfigState(AppConfiguration::getTagName(AppConfiguration::TagID::UICONFIG));
    auto selMgrCfgState = m_config->getConfigState(AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORSELECTIONMANAGER));

    // set the controller modules' config
    auto ctrl = SpaConBridge::Controller::GetInstance();
    if (ctrl)
        ctrl->setStateXml(ctrlConfigState.get());

    // set the overview manager modules' config
    auto pageMgr = SpaConBridge::PageComponentManager::GetInstance();
    if (pageMgr)
        pageMgr->setStateXml(uiCfgState.get());

    // set the processor selection manager modules' config
    auto selMgr = SpaConBridge::ProcessorSelectionManager::GetInstance();
    if (selMgr)
        selMgr->setStateXml(selMgrCfgState.get());

    // set the lookandfeel config (forwards to MainWindow where the magic happens)
    if (uiCfgState)
    {
        auto lookAndFeelXmlElement = uiCfgState->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::LOOKANDFEELTYPE));
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

#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
        auto fullscreenWindowModeXmlElement = uiCfgState->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::FULLSCREENWINDOWMODE));
        if (fullscreenWindowModeXmlElement)
        {
            auto fullscreenWindowModeTextElement = fullscreenWindowModeXmlElement->getFirstChildElement();
            if (fullscreenWindowModeTextElement && fullscreenWindowModeTextElement->isTextElement())
            {
                auto fullscreen = 1 == static_cast<DbLookAndFeelBase::LookAndFeelType>(fullscreenWindowModeTextElement->getText().getIntValue());

                if (onSetWindowMode)
                    onSetWindowMode(fullscreen);
            }
        }
#endif
    }
}

}
