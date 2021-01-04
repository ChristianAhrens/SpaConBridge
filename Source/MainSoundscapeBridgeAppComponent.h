/* Copyright (c) 2020-2021, Christian Ahrens
 *
 * This file is part of SoundscapeBridgeApp <https://github.com/ChristianAhrens/SoundscapeBridgeApp>
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

#pragma once

#include "AppConfiguration.h"
#include "LookAndFeel.h"

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
    MainSoundscapeBridgeAppComponent(std::function<void(DbLookAndFeelBase::LookAndFeelType)> lafUpdateCallback);
    ~MainSoundscapeBridgeAppComponent() override;

    //==========================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    //==========================================================================
    void performConfigurationDump() override;

    //==========================================================================
    void onConfigUpdated() override;

    //==========================================================================
    std::function<void(DbLookAndFeelBase::LookAndFeelType)>	onUpdateLookAndFeel;

private:
    std::unique_ptr<AppConfiguration>           m_config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainSoundscapeBridgeAppComponent)
};

};