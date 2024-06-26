/* Copyright (c) 2020-2023, Christian Ahrens
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


#include "SettingsPageComponent.h"

#include "SettingsSectionsComponent.h"

#include "../../../Controller.h"
#include "../../../LookAndFeel.h"

#include <Image_utils.h>
#include <FixedFontTextEditor.h>
#include <TextWithImageButton.h>


namespace SpaConBridge
{


/**
 * Class constructor.
 */
SettingsPageComponent::SettingsPageComponent()
	: PageComponentBase(UIPageId::UPI_Settings)
{
	// Apply button for when raw config is visible
	m_settingsRawApplyButton = std::make_unique<TextButton>("Apply");
	m_settingsRawApplyButton->onClick = [this] { onApplyClicked(); };
	addAndMakeVisible(m_settingsRawApplyButton.get());
	// Reset to default button for when raw config is visible
	m_settingsResetToDefaultButton = std::make_unique<TextButton>("Reset config to default");
	m_settingsResetToDefaultButton->onClick = [this] { onResetToDefaultClicked(); };
	addAndMakeVisible(m_settingsResetToDefaultButton.get());
	// Text editor for when raw config is visible
	m_settingsRawEditor = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
	m_settingsRawEditor->setMultiLine(true, false);
	addAndMakeVisible(m_settingsRawEditor.get());

	// load/save config buttons
	m_loadConfigButton = std::make_unique<JUCEAppBasics::TextWithImageButton>("Load config");
	m_loadConfigButton->setImagePosition(Justification::centredLeft);
	m_loadConfigButton->onClick = [this] { onLoadConfigClicked(); };
	addAndMakeVisible(m_loadConfigButton.get());
	m_saveConfigButton = std::make_unique<JUCEAppBasics::TextWithImageButton>("Save config");
	m_saveConfigButton->setImagePosition(Justification::centredLeft);
	m_saveConfigButton->onClick = [this] { onSaveConfigClicked(); };
	addAndMakeVisible(m_saveConfigButton.get());

	// Toggle button for showing/hiding raw config
	m_useRawConfigButton = std::make_unique<TextButton>("Show raw config", "RAW CFG");
	m_useRawConfigButton->setClickingTogglesState(true);
	m_useRawConfigButton->onClick = [this] { onToggleRawConfigVisible(); };
	addAndMakeVisible(m_useRawConfigButton.get());
	onToggleRawConfigVisible();

	// The component containing configuration sections, etc. to be shown in a viewport for scrolling capabilities
	m_settingsComponent = std::make_unique<SettingsSectionsComponent>();
	m_settingsComponent->onContentSizesChangedCallback = [this] {
		resized();
	};
	m_settingsComponent->onContentMinRequiredSizeChangedCallback = [this](const juce::Rectangle<int>&) {
		if (m_tempCachedViewPosition.isOrigin())
			m_tempCachedViewPosition = m_settingsViewport->getViewPosition();
	};
	m_settingsViewport = std::make_unique<Viewport>();
	m_settingsViewport->setViewedComponent(m_settingsComponent.get(), false);
	addAndMakeVisible(m_settingsViewport.get());

	// register this object as config watcher
	auto config = SpaConBridge::AppConfiguration::getInstance();
	if (config)
		config->addWatcher(this, true);
}

/**
 * Class destructor.
 */
SettingsPageComponent::~SettingsPageComponent()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void SettingsPageComponent::paint(Graphics& g)
{
	// Paint background to cover the controls behind this overlay.
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(getLocalBounds());
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void SettingsPageComponent::resized()
{
	auto bounds = getLocalBounds().reduced(5);

	// toggle button for visibility of raw config textfield
	auto bottomBarControlBounds = bounds.removeFromBottom(25);
	auto bottomBarWidth = bottomBarControlBounds.getWidth();
	if (m_loadConfigButton && m_saveConfigButton)
	{
		if (bottomBarWidth >= 330)
		{
			m_useRawConfigButton->setVisible(true);
			m_useRawConfigButton->setBounds(bottomBarControlBounds.removeFromRight(110));
			bottomBarControlBounds.removeFromRight(5);
		}
		else
			m_useRawConfigButton->setVisible(false);

		if (bottomBarWidth >= 205)
		{
			m_loadConfigButton->setVisible(true);
			m_loadConfigButton->setBounds(bottomBarControlBounds.removeFromRight(105));
			bottomBarControlBounds.removeFromRight(5);
			m_saveConfigButton->setVisible(true);
			m_saveConfigButton->setBounds(bottomBarControlBounds.removeFromRight(105));
		}
		else
		{
			m_loadConfigButton->setVisible(false);
			m_saveConfigButton->setVisible(false);
		}
	}

	bounds.removeFromBottom(5);

	if (m_settingsComponent && m_settingsViewport)
	{
		// cache the currently viewed position before resizing stuff, leading to a reset to viewing 0,0
		// (only when no viewed position was already cached to be preserved beforehand)
		if (m_tempCachedViewPosition.isOrigin())
			m_tempCachedViewPosition = m_settingsViewport->getViewPosition();

		m_settingsComponent->setBounds(bounds);
		m_settingsViewport->setBounds(bounds);

		if (m_settingsViewport->canScrollVertically() || m_settingsViewport->canScrollHorizontally())
		{
			auto boundsWithoutScrollbars = bounds;

			if (m_settingsViewport->canScrollVertically())
				boundsWithoutScrollbars.setWidth(bounds.getWidth() - m_settingsViewport->getVerticalScrollBar().getWidth());

			if (m_settingsViewport->canScrollHorizontally())
				boundsWithoutScrollbars.setHeight(bounds.getHeight() - m_settingsViewport->getHorizontalScrollBar().getHeight());

			m_settingsComponent->setBounds(boundsWithoutScrollbars);
		}

		// reactivate the viewed position after resizing took place
		m_settingsViewport->setViewPosition(m_tempCachedViewPosition);
		m_tempCachedViewPosition.setXY(0, 0); // reset viewed position to not be dangling
	}

	// raw config textfield, etc. - not always visible!
	auto buttonHeadBounds = bounds.removeFromTop(25);
	m_settingsRawApplyButton->setBounds(buttonHeadBounds.removeFromLeft(static_cast<int>(0.5f * buttonHeadBounds.getWidth())));
	m_settingsResetToDefaultButton->setBounds(buttonHeadBounds);
	m_settingsRawEditor->setBounds(bounds);
}

/**
 * Reimplemented from component to change drawablebutton icon data.
 */
void SettingsPageComponent::lookAndFeelChanged()
{
	Component::lookAndFeelChanged();

	// Update drawable button images with updated lookAndFeel colours
	UpdateDrawableButtonImages(m_loadConfigButton, BinaryData::folder_open24px_svg, &getLookAndFeel());
	UpdateDrawableButtonImages(m_saveConfigButton, BinaryData::save24px_svg, &getLookAndFeel());
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the procssor parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void SettingsPageComponent::UpdateGui(bool init)
{
	if (init)
	{
		onConfigUpdated();
	}
}

/**
 * Overridden from AppConfiguration Watcher to be able
 * to live react on config changes and update the table contents.
 */
void SettingsPageComponent::onConfigUpdated()
{
	// trigger updating the settings visu in general
	m_settingsComponent->processUpdatedConfig();

	// if the raw config is currently visible, go into updating it as well
	if (m_useRawConfigButton->getToggleState())
	{
		auto config = AppConfiguration::getInstance();
		if (config)
		{
			// get the config for filling raw texteditor (meant for debugging, ...)
			auto configXml = config->getConfigState();
			auto configText = configXml->toString();
			m_settingsRawEditor->setText(configText);
		}
	}
}

/**
 * Method to be called when user clicks on 'apply config'
 * to in consequence dump text from editor on ui to an xml
 * and feed it to current configuration.
 */
void SettingsPageComponent::onApplyClicked()
{
	auto config = SpaConBridge::AppConfiguration::getInstance();
	if (config != nullptr)
	{
		XmlDocument configXmlDocument(m_settingsRawEditor->getText());
		auto configXmlElement = configXmlDocument.getDocumentElement();
		if (configXmlElement)
		{
			auto controllerXmlElement = configXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::CONTROLLER));
			if (controllerXmlElement)
				config->setConfigState(std::make_unique<XmlElement>(*controllerXmlElement));

			auto uiCfgXmlElement = configXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::UICONFIG));
			if (uiCfgXmlElement)
				config->setConfigState(std::make_unique<XmlElement>(*uiCfgXmlElement));
			
			auto selMgrCfgXmlElement = configXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORSELECTIONMANAGER));
			if (selMgrCfgXmlElement)
				config->setConfigState(std::make_unique<XmlElement>(*selMgrCfgXmlElement));

			config->triggerWatcherUpdate();
		}
	}
}

/**
 * Method to be called when user clicks on reset to default config button
 */
void SettingsPageComponent::onResetToDefaultClicked()
{
	m_useRawConfigButton->setToggleState(false, juce::dontSendNotification);
	onToggleRawConfigVisible();

	auto config = dynamic_cast<AppConfiguration*>(SpaConBridge::AppConfiguration::getInstance());
	if (config != nullptr)
	{
		config->ResetToDefault();
	}
}

/**
 * Method to be used as callback for load button click reaction.
 */
void SettingsPageComponent::onLoadConfigClicked()
{
    // create the file chooser dialog
	auto chooser = std::make_unique<FileChooser>("Select a " + JUCEApplication::getInstance()->getApplicationName() + " config file to load..."); // all filepatterns are allowed for loading (currently seems to not work on iOS and not be regarded on macOS at all)
    // and trigger opening it
	chooser->launchAsync(FileBrowserComponent::openMode|FileBrowserComponent::canSelectFiles, [this](const FileChooser& chooser)
		{
#if JUCE_IOS || JUCE_ANDROID
            auto url = chooser.getURLResult();
        
#if JUCE_IOS
            auto inputStream = std::unique_ptr<juce::InputStream>(juce::URLInputSource(url).createInputStream());
#elif JUCE_ANDROID
            auto androidDocument = juce::AndroidDocument::fromDocument (url);
            auto inputStream = std::unique_ptr<juce::InputStream>(androidDocument.createInputStream());
#endif

			auto tmpFile = SelfDestructingInputStreamBufferFile::CreateFileFromInputStream(inputStream);
			auto fullFilePathName = tmpFile->GetFullPathName();
#else
			auto fullFilePathName = chooser.getResult().getFullPathName();
#endif

			// verify that the result is valid (ok clicked)
			if (!fullFilePathName.isEmpty())
			{
				auto ctrl = Controller::GetInstance();
				if (ctrl)
					ctrl->LoadConfigurationFile(juce::File(fullFilePathName));
			}
            
			delete static_cast<const FileChooser*>(&chooser);
		});
	chooser.release();
}

/**
 * Method to be used as callback for save button click reaction.
 */
void SettingsPageComponent::onSaveConfigClicked()
{
    // prepare a default filename suggestion based on current date and app name
    auto initialFolderPathName = File::getSpecialLocation(File::userDocumentsDirectory).getFullPathName();
    auto initialFileNameSuggestion = Time::getCurrentTime().formatted("%Y-%m-%d_") + JUCEApplication::getInstance()->getApplicationName() + "Config";
    auto initialFilePathSuggestion = initialFolderPathName + File::getSeparatorString() + initialFileNameSuggestion;
    auto initialFileSuggestion = File(initialFilePathSuggestion);
    
    // create the file chooser dialog
	auto chooser = std::make_unique<FileChooser>("Save current " + JUCEApplication::getInstance()->getApplicationName() + " config file as...",
                        initialFileSuggestion, "*.config", true, false, this);
    // and trigger opening it
	chooser->launchAsync(FileBrowserComponent::saveMode, [this](const FileChooser& chooser)
		{
			auto file = chooser.getResult();
			
			// verify that the result is valid (ok clicked)
			if (!file.getFullPathName().isEmpty())
			{
				// enforce the .config extension
				if (file.getFileExtension() != ".config")
					file = file.withFileExtension(".config");

				Controller* ctrl = Controller::GetInstance();
				if (ctrl)
					ctrl->SaveConfigurationFile(file);
			}
			delete static_cast<const FileChooser*>(&chooser);
		});
	chooser.release();
}

/**
 * Method to be called when user clicks on button to toggle raw config visu
 */
void SettingsPageComponent::onToggleRawConfigVisible()
{
	if (m_settingsRawApplyButton && m_settingsRawEditor)
	{
		if (m_useRawConfigButton->getToggleState())
		{
			m_settingsRawApplyButton->setVisible(true);
			m_settingsRawApplyButton->toFront(true);
			m_settingsResetToDefaultButton->setVisible(true);
			m_settingsResetToDefaultButton->toFront(true);
			m_settingsRawEditor->setVisible(true);
			m_settingsRawEditor->toFront(true);

			// manually trigger config refresh, since we did not process config changes while raw settings editor was invisible
			onConfigUpdated();
		}
		else
		{
			m_settingsRawApplyButton->setVisible(false);
			m_settingsResetToDefaultButton->setVisible(false);
			m_settingsRawEditor->setVisible(false);
		}
	}
}


} // namespace SpaConBridge
