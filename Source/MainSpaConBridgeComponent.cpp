/* Copyright (c) 2020-2021, Christian Ahrens
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

#include "PagedUI/PageContainerComponent.h"
#include "PagedUI/PageComponentManager.h"

#include "CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"

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
    auto ctrl = SpaConBridge::Controller::GetInstance();
    ignoreUnused(ctrl);

    // enshure the overviewmanager singleton is created
    auto pageMgr = SpaConBridge::PageComponentManager::GetInstance();
    if (pageMgr)
    {
        // get the overview component from manager to use as central element for app ui
        auto pageContainer = pageMgr->GetPageContainer();
        addAndMakeVisible(pageContainer);
    }

    setSize(896, 414);
}

MainSpaConBridgeComponent::~MainSpaConBridgeComponent()
{
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
        for (auto const& processorId : ctrl->GetSoundobjectProcessorIds())
            delete ctrl->GetSoundobjectProcessor(processorId);

        ctrl->DestroyInstance();
    }
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
}

void MainSpaConBridgeComponent::performConfigurationDump()
{
    auto ctrl = SpaConBridge::Controller::GetInstance();
    if (ctrl)
        m_config->setConfigState(ctrl->createStateXml());

    auto pageMgr = SpaConBridge::PageComponentManager::GetInstance();
    if (pageMgr)
        m_config->setConfigState(pageMgr->createStateXml());
}

void MainSpaConBridgeComponent::onConfigUpdated()
{
    // get all the modules' configs first, because the initialization process might already trigger dumping, that would override data
    auto ctrlConfigState = m_config->getConfigState(AppConfiguration::getTagName(AppConfiguration::TagID::CONTROLLER));
    auto uiCfgState = m_config->getConfigState(AppConfiguration::getTagName(AppConfiguration::TagID::UICONFIG));

    // set the controller modules' config
    auto ctrl = SpaConBridge::Controller::GetInstance();
    if (ctrl)
        ctrl->setStateXml(ctrlConfigState.get());

    // set the overview manager modules' config
    auto pageMgr = SpaConBridge::PageComponentManager::GetInstance();
    if (pageMgr)
        pageMgr->setStateXml(uiCfgState.get());

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
    }
}

}