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

#include <JuceHeader.h>

#include "SpaConBridgeCommon.h"

#include "MainSpaConBridgeComponent.h"

#include "LookAndFeel.h"

namespace SpaConBridge
{

//==============================================================================
class SpaConBridgeApplication : public JUCEApplication
{
public:
    //==============================================================================
    SpaConBridgeApplication() {}

    const String getApplicationName() override { return ProjectInfo::projectName; }
    const String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return true; }

    //==============================================================================
    void initialise(const String& commandLine) override
    {
        ignoreUnused(commandLine);

        m_mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override
    {
        m_mainWindow = nullptr; // (deletes our window)
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const String& commandLine) override
    {
        ignoreUnused(commandLine);
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow : public DocumentWindow
    {
    public:
        MainWindow(String name) : DocumentWindow(name,
            Desktop::getInstance().getDefaultLookAndFeel()
            .findColour(ResizableWindow::backgroundColourId),
            DocumentWindow::allButtons)
        {
            updateLookAndFeel();

#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
            m_mainComponent = std::make_unique<MainSpaConBridgeComponent>([=](DbLookAndFeelBase::LookAndFeelType type) { updateLookAndFeel(type); }, [=](bool fullscreenWindow) { setWindowMode(fullscreenWindow); });
#else
            m_mainComponent = std::make_unique<MainSpaConBridgeComponent>([=](DbLookAndFeelBase::LookAndFeelType type) { updateLookAndFeel(type); });
#endif

            setUsingNativeTitleBar(true);
            setContentOwned(m_mainComponent.get(), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
#else
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
#endif

            setVisible(true);
        }

        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
            class uses a lot of them, so by overriding you might break its functionality.
            It's best to do all your work in your content component instead, but if
            you really have to override any DocumentWindow methods, make sure your
            subclass also calls the superclass's method.
        */

        void updateLookAndFeel(DbLookAndFeelBase::LookAndFeelType type = DbLookAndFeelBase::LookAndFeelType::LAFT_OSdynamic)
        {
            switch (type)
            {
            case DbLookAndFeelBase::LookAndFeelType::LAFT_DefaultJUCE:
                m_customLookAndFeel = nullptr;
                break;
            case DbLookAndFeelBase::LookAndFeelType::LAFT_Light:
                m_customLookAndFeel = std::unique_ptr<LookAndFeel>(new LightDbLookAndFeel);
                break;
            case DbLookAndFeelBase::LookAndFeelType::LAFT_Dark:
            case DbLookAndFeelBase::LookAndFeelType::LAFT_OSdynamic:
            default:
                m_customLookAndFeel = std::unique_ptr<LookAndFeel>(new DarkDbLookAndFeel);
                break;
            }
            m_customLookAndFeel->setUsingNativeAlertWindows(true);

            Desktop::getInstance().setDefaultLookAndFeel(m_customLookAndFeel.get());
        }

#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
        void setWindowMode(bool fullscreenWindow)
        {
            if (fullscreenWindow)
            {
                setResizable(false, false);
                setFullScreen(true);
            }
            else
            {
                setFullScreen(false);
                setResizable(true, true);
                centreWithSize(getWidth(), getHeight());
            }
        }
#endif

    private:
        std::unique_ptr<LookAndFeel>                m_customLookAndFeel; // our own look and feel implementation instance
        std::unique_ptr<MainSpaConBridgeComponent>  m_mainComponent;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> m_mainWindow;
};

}

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (SpaConBridge::SpaConBridgeApplication)
