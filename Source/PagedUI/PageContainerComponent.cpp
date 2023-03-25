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


#include "PageContainerComponent.h"

#include "PageComponentManager.h"
#include "PageComponents/SoundobjectTablePage/SoundobjectTablePageComponent.h"
#include "PageComponents/MultiSoundobjectPage/MultiSoundobjectPageComponent.h"
#include "PageComponents/MatrixIOPage/MatrixIOPageComponent.h"
#include "PageComponents/SettingsPage/SettingsPageComponent.h"
#include "PageComponents/StatisticsPage/StatisticsPageComponent.h"
#include "PageComponents/AboutPage/AboutPageComponent.h"
#include "PageComponents/ScenesPage/ScenesPageComponent.h"
#include "PageComponents/EnSpacePage/EnSpacePageComponent.h"

#include "../Controller.h"
#include "../SoundobjectSlider.h"

#include <Image_utils.h>


namespace SpaConBridge
{


/*
===============================================================================
 Class PageContainerComponent
===============================================================================
*/

/**
 * Class constructor.
 */
PageContainerComponent::PageContainerComponent()
{
	// Help button
	m_helpButton = std::make_unique<DrawableButton>("Help", DrawableButton::ButtonStyle::ImageFitted);
	m_helpButton->addListener(this);
	addAndMakeVisible(m_helpButton.get());
	lookAndFeelChanged();

	// Online
	m_onlineButton = std::make_unique<TextButton>("Online");
	//m_onlineButton->setJustificationType(Justification::centred);
	m_onlineButton->setClickingTogglesState(true);
	m_onlineButton->addListener(this);
	addAndMakeVisible(m_onlineButton.get());
	m_connectedLed1st = std::make_unique<LedComponent>();
	m_connectedLed1st->setEnabled(false);
	addAndMakeVisible(m_connectedLed1st.get());
	m_connectedLed2nd = std::make_unique<LedComponent>();
	m_connectedLed2nd->setEnabled(false);
	addAndMakeVisible(m_connectedLed2nd.get());

	// app logo button and procssor version label
	m_logoButton = std::make_unique<ImageButton>("LogoButton");
	m_logoButton->setImages(false, true, true,
		ImageCache::getFromMemory(BinaryData::SpaConBridge_png, BinaryData::SpaConBridge_pngSize), 1.0f, Colours::transparentWhite,
		Image(), 1.0f, Colours::transparentWhite,
		Image(), 1.0f, Colours::transparentWhite);
	m_logoButton->addListener(this);
	addAndMakeVisible(m_logoButton.get());
	m_versionLabel = std::make_unique<Label>("Version", String(JUCE_STRINGIFY(JUCE_APP_VERSION)));
	m_versionLabel->setJustificationType(Justification::centred);
	m_versionLabel->setFont(Font(11));
	addAndMakeVisible(m_versionLabel.get());
	m_versionStringLabel = std::make_unique<Label>("VersionString", "Version");
	m_versionStringLabel->setJustificationType(Justification::centred);
	m_versionStringLabel->setFont(Font(11));
	addAndMakeVisible(m_versionStringLabel.get());

	// Create the pages.
	m_soundobjectsPage = std::make_unique<SoundobjectTablePageComponent>();
	m_multiSoundobjectsPage = std::make_unique<MultiSoundobjectPageComponent>();
    m_matrixIOPage = std::make_unique<MatrixIOPageComponent>();
	m_statisticsPage = std::make_unique<StatisticsPageComponent>();
	m_scenesPage = std::make_unique<ScenesPageComponent>();
	m_enSpacePage = std::make_unique<EnSpacePageComponent>();
	m_settingsPage = std::make_unique<SettingsPageComponent>();
	m_aboutPage = std::make_unique<AboutPageComponent>();
	m_aboutPage->onCloseClick = [=] { toggleAboutPage(); };

	// Create the tab component
	m_tabbedComponent = std::make_unique<CustomButtonTabbedComponent>();
	m_tabbedComponent->setTabBarDepth(44);
	m_tabbedComponent->setOutline(0);
	m_tabbedComponent->setIndent(0);
	addAndMakeVisible(m_tabbedComponent.get());

	// Add the page tabs.
	m_tabbedComponent->SetIsHandlingChanges(false);
	m_tabbedComponent->addTab(GetPageNameFromId(UPI_Soundobjects), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_soundobjectsPage.get(), false, UPI_Soundobjects);
	m_tabbedComponent->addTab(GetPageNameFromId(UPI_MultiSoundobjects), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_multiSoundobjectsPage.get(), false, UPI_MultiSoundobjects);
	m_tabbedComponent->addTab(GetPageNameFromId(UPI_MatrixIOs), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_matrixIOPage.get(), false, UPI_MatrixIOs);
	m_tabbedComponent->addTab(GetPageNameFromId(UPI_Scenes), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_scenesPage.get(), false, UPI_Scenes);
	m_tabbedComponent->addTab(GetPageNameFromId(UPI_EnSpace), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_enSpacePage.get(), false, UPI_EnSpace);
	m_tabbedComponent->addTab(GetPageNameFromId(UPI_Statistics), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_statisticsPage.get(), false, UPI_Statistics);
	m_tabbedComponent->addTab(GetPageNameFromId(UPI_Settings), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_settingsPage.get(), false, UPI_Settings);

	m_tabbedComponent->SetIsHandlingChanges(true);

	// Start GUI-refreshing timer.
	startTimer(GUI_UPDATE_RATE_SLOW);

	// push the logo button to front to overcome issue of overlapping tabbed component grabbing mouse interaction
	m_logoButton->toFront(false);
}

/**
 * Class destructor.
 */
PageContainerComponent::~PageContainerComponent()
{
}

/**
 * Reimplemented to paint background and logo.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void PageContainerComponent::paint(Graphics& g)
{
	int w = getLocalBounds().getWidth();
	int h = getLocalBounds().getHeight();	

	// Bars above and below
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	g.fillRect(getLocalBounds());

	// Background
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));
	g.fillRect(Rectangle<int>(0, 43, w, h - 87));

	// Little lines between version and logo
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));
	g.fillRect(Rectangle<int>(w - 39, 6, 1, 30));
	g.fillRect(Rectangle<int>(w - 86, 6, 1, 30));

	// Draw little line below right and left overlap of tabbedcomponent buttonbar to match with the line which is automatically drawn 
	// by the CustomButtonTabbedComponent's CustomDrawableTabBarButton.
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));
	g.drawRect(Rectangle<int>(0, 43, 40, 1), 1);
	g.drawRect(Rectangle<int>(w - 86, 43, 86, 1), 1);
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void PageContainerComponent::resized()
{
	int w = getLocalBounds().getWidth();

	// bottom bar with online label and led
	FlexBox bottomBarFB;
	bottomBarFB.flexDirection = FlexBox::Direction::row;
	bottomBarFB.justifyContent = FlexBox::JustifyContent::center;
	bottomBarFB.alignContent = FlexBox::AlignContent::center;
	bottomBarFB.items.addArray({
		// Help button
		FlexItem(*m_helpButton.get()).withWidth(27).withHeight(27).withMargin(FlexItem::Margin(5, 0, 5, 10)),
		// Spacing
		FlexItem().withFlex(1),
		});

	auto ctrl = Controller::GetInstance();
	if (ctrl)
	{
		// Online
		bottomBarFB.items.add(FlexItem(*m_onlineButton.get()).withWidth(65).withHeight(25).withMargin(FlexItem::Margin(5, 5, 5, 0)));
		// depending on controller extension mode, we use one or two leds
		if (ctrl->GetExtensionMode() == ExtensionMode::EM_Off)
		{
			bottomBarFB.items.add(FlexItem(*m_connectedLed1st.get()).withWidth(24).withHeight(24).withMargin(FlexItem::Margin(5, 10, 5, 0)));
		}
		else
		{
			bottomBarFB.items.add(FlexItem(*m_connectedLed1st.get()).withWidth(24).withHeight(24).withMargin(FlexItem::Margin(5, 0, 5, 0)));
			bottomBarFB.items.add(FlexItem(*m_connectedLed2nd.get()).withWidth(24).withHeight(24).withMargin(FlexItem::Margin(5, 10, 5, 0)));
		}
	}

	bottomBarFB.performLayout(getLocalBounds().removeFromBottom(40));

	// Name and Version label
	m_versionStringLabel->setBounds(w - 89, 3, 55, 25);
	m_versionLabel->setBounds(w - 87, 21, 42, 15);

	// logo button (triggers about page)
	m_logoButton->setBounds(w - 35, 7, 30, 30);

	// Tab container takes up the entire window minus the bottom bar (with the IP etc).
	// See CustomButtonTabbedComponent::resized().
	m_tabbedComponent->setBounds(Rectangle<int>(0, 0, w, getLocalBounds().getHeight() - 45));

	// Resize overview table container.
	auto rect = Rectangle<int>(0, 44, w, getLocalBounds().getHeight() - 89);
	if (!m_soundobjectsPage->isOnDesktop())
		m_soundobjectsPage->setBounds(rect);
	if (!m_multiSoundobjectsPage->isOnDesktop())
		m_multiSoundobjectsPage->setBounds(rect);
    if (!m_matrixIOPage->isOnDesktop())
		m_matrixIOPage->setBounds(rect);
	if (!m_settingsPage->isOnDesktop())
		m_settingsPage->setBounds(rect);
	if (!m_statisticsPage->isOnDesktop())
		m_statisticsPage->setBounds(rect);
	if (!m_scenesPage->isOnDesktop())
		m_scenesPage->setBounds(rect);
	if (!m_enSpacePage->isOnDesktop())
		m_enSpacePage->setBounds(rect);

	// finally resize the overlay component, if set, visible and therefor on top of everything else at all
	if (m_overlayComponent && m_overlayComponent->isVisible())
	{
		m_overlayComponent->setBounds(getLocalBounds());
		m_overlayComponent->toFront(false);
	}
	
}

/**
 * Callback function for button clicks.
 * @param button	The button object that was clicked.
 */
void PageContainerComponent::buttonClicked(Button* button)
{
	if (m_onlineButton && m_onlineButton.get() == button)
	{
		auto ctrl = Controller::GetInstance();
		if (ctrl)
			ctrl->SetOnline(DCP_PageContainer, m_onlineButton->getToggleState());
	}
	else if (m_logoButton && m_logoButton.get() == button && m_aboutPage)
	{
		toggleAboutPage();
	}
	else if (m_helpButton && m_helpButton.get() == button)
	{
		auto helpURLString = GetRepositoryBaseWebUrl() + "README.md";

		if (m_tabbedComponent)
		{
			auto currentPageId = GetPageIdFromName(m_tabbedComponent->getCurrentTabName());
			auto currentPageIdentificationString = GetDocumentationSectionIdentification(currentPageId);
			if (currentPageIdentificationString.isNotEmpty())
				helpURLString += "/" + currentPageIdentificationString;
		}

		URL(helpURLString).launchInDefaultBrowser();
	}
}

/**
 * Helper method to open about overlay page if not currently open or close it, if it currently is displayed.
 */
void PageContainerComponent::toggleAboutPage()
{
	if (m_aboutPage)
	{
		if (m_aboutPage->isVisible())
		{
			auto pageMgr = SpaConBridge::PageComponentManager::GetInstance();
			if (pageMgr)
			{
				auto pageContainer = pageMgr->GetPageContainer();
				if (pageContainer)
					pageContainer->ClearOverlayComponent();
			}
		}
		else
		{
			m_aboutPage->setVisible(true);
			auto pageMgr = SpaConBridge::PageComponentManager::GetInstance();
			if (pageMgr)
			{
				auto pageContainer = pageMgr->GetPageContainer();
				if (pageContainer)
					pageContainer->SetOverlayComponent(m_aboutPage.get());
			}
		}
	}
}

/**
* Helper method to get the component for a given pageId
* @param	pageId	The id of the page to get the component for.
* @return	The PageComponentBase pointer of the component for the given pageId or nullptr if invalid.
*/
PageComponentBase* PageContainerComponent::GetComponentForPageId(const UIPageId pageId)
{
	if (UPI_Soundobjects == pageId)
		return m_soundobjectsPage.get();
	else if (UPI_MultiSoundobjects == pageId)
		return m_multiSoundobjectsPage.get();
	else if (UPI_MatrixIOs == pageId)
		return m_matrixIOPage.get();
	else if (UPI_Settings == pageId)
		return m_settingsPage.get();
	else if (UPI_Statistics == pageId)
		return m_statisticsPage.get();
	else if (UPI_Scenes == pageId)
		return m_scenesPage.get();
	else if (UPI_EnSpace == pageId)
		return m_enSpacePage.get();
	else
		return nullptr;
}

/**
 * Timer callback function, which will be called at regular intervals to update the GUI.
 * Reimplemented from base class Timer.
 */
void PageContainerComponent::timerCallback()
{
	UpdateGui(false);
}

/**
 * Update GUI elements with the current parameter values.
 * @param init	True to ignore any changed flags and update the OSC config parameters 
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void PageContainerComponent::UpdateGui(bool init)
{
	auto ctrl = Controller::GetInstance();

	if (ctrl && m_onlineButton)
	{
		auto online = ctrl->IsOnline();
		if (m_onlineButton->getToggleState() != online)
			m_onlineButton->setToggleState(online, dontSendNotification);
	}

	if (ctrl && m_connectedLed1st && m_connectedLed2nd)
	{
		auto secondDS100Used = (ctrl->GetExtensionMode() != ExtensionMode::EM_Off);
		auto secondDS100Visible = m_connectedLed2nd->isVisible();
		if (secondDS100Used != secondDS100Visible)
		{
			m_connectedLed2nd->setVisible(secondDS100Used);
			resized();
		}
		if (ctrl->PopParameterChanged(DCP_PageContainer, DCT_Connected) || init)
		{
			auto connected1 = ctrl->IsFirstDS100Connected();
			auto master1 = ctrl->IsFirstDS100Master();
			m_connectedLed1st->SetOn(connected1);
			m_connectedLed1st->SetHighlightOn(connected1 && master1);
			if (secondDS100Used)
			{
				auto connected2 = ctrl->IsSecondDS100Connected();
				auto master2 = ctrl->IsSecondDS100Master();
				m_connectedLed2nd->SetOn(connected2);
				m_connectedLed2nd->SetHighlightOn(connected2 && master2);
			}
		}
	}


	// updating is always required when init is set.
	// starting of refresh timer only when page is visible.
	auto updateSoundObjects = init;
	auto startRefreshSoundObjects = false;
	auto updateMultiSoundobjects = init;
	auto startRefreshMultiSoundobjects = false;
	auto updateMatrixIOs = init;
	auto startRefreshMatrixIOs = false;
	auto updateScenes = init;
	auto startRefreshScenes = false;
	auto updateEnSpace = init;
	auto startRefreshEnSpace = false;
	auto updateStatistics = init;
	auto startRefreshStatistics = false;
	auto updateSettings = init;
	auto startRefreshSettings = false;
	
	// queue an update for all windowed pages
	if (m_soundobjectsPage)
		updateSoundObjects = updateSoundObjects || m_soundobjectsPage->isOnDesktop();
	if (m_multiSoundobjectsPage)
		updateMultiSoundobjects = updateMultiSoundobjects || m_multiSoundobjectsPage->isOnDesktop();
	if (m_matrixIOPage)
		updateMatrixIOs = updateMatrixIOs || m_matrixIOPage->isOnDesktop();
	if (m_scenesPage)
		updateScenes = updateScenes || m_scenesPage->isOnDesktop();
	if (m_enSpacePage)
		updateEnSpace = updateEnSpace || m_enSpacePage->isOnDesktop();
	if (m_statisticsPage)
		updateStatistics = updateStatistics ||m_statisticsPage->isOnDesktop();
	if (m_settingsPage)
		updateSettings = updateSettings || m_settingsPage->isOnDesktop();

	// queue an update and also if not already active continuous updating for the current page
	if (m_tabbedComponent)
	{
		auto currentPageId = GetPageIdFromName(m_tabbedComponent->getCurrentTabName());
		switch (currentPageId)
		{
		case UPI_Soundobjects:
			updateSoundObjects = true;
			startRefreshSoundObjects = true;
			break;
		case UPI_MultiSoundobjects:
			updateMultiSoundobjects = true;
			startRefreshMultiSoundobjects = true;
			break;
		case UPI_MatrixIOs:
			updateMatrixIOs = true;
			startRefreshMatrixIOs = true;
			break;
		case UPI_Scenes:
			updateScenes = true;
			startRefreshScenes = true;
			break;
		case UPI_EnSpace:
			updateEnSpace = true;
			startRefreshEnSpace = true;
			break;
		case UPI_Statistics:
			updateStatistics = true;
			startRefreshStatistics = true;
			break;
		case UPI_Settings:
			updateSettings = true;
			startRefreshSettings = true;
			break;
		default:
			break;
		}
	}

	// perform updating and continuous updating 
	if (updateSoundObjects)
	{
		if (m_soundobjectsPage)
			m_soundobjectsPage->UpdateGui(init);
	}
	if (startRefreshSoundObjects)
	{
		if (getTimerInterval() != GUI_UPDATE_RATE_SLOW)
			startTimer(GUI_UPDATE_RATE_SLOW);
	}

	if (updateMultiSoundobjects)
	{
		if (m_multiSoundobjectsPage)
			m_multiSoundobjectsPage->UpdateGui(init);
	}
	if (startRefreshMultiSoundobjects)
	{
		if (getTimerInterval() != GUI_UPDATE_RATE_FAST)
			startTimer(GUI_UPDATE_RATE_FAST);
	}

	if (updateMatrixIOs)
	{
		if (m_matrixIOPage)
			m_matrixIOPage->UpdateGui(init);
	}
	if (startRefreshMatrixIOs)
	{
		if (getTimerInterval() != GUI_UPDATE_RATE_SLOW)
			startTimer(GUI_UPDATE_RATE_SLOW);
	}

	if (updateScenes)
	{
		if (m_scenesPage)
			m_scenesPage->UpdateGui(init);
	}
	if (startRefreshScenes)
	{
		if (getTimerInterval() != GUI_UPDATE_RATE_SUPERSLOW)
			startTimer(GUI_UPDATE_RATE_SUPERSLOW);
	}
		
	if (updateEnSpace)
	{
		if (m_enSpacePage)
			m_enSpacePage->UpdateGui(init);
	}
	if (startRefreshEnSpace)
	{
		if (getTimerInterval() != GUI_UPDATE_RATE_SUPERSLOW)
			startTimer(GUI_UPDATE_RATE_SUPERSLOW);
	}

	if (updateStatistics)
	{
		if (m_statisticsPage)
			m_statisticsPage->UpdateGui(init);
	}
	if (startRefreshStatistics)
	{
		if (getTimerInterval() != GUI_UPDATE_RATE_SUPERSLOW)
			startTimer(GUI_UPDATE_RATE_SUPERSLOW);
	}

	if (updateSettings)
	{
		if (m_settingsPage)
			m_settingsPage->UpdateGui(init);
	}
	if (startRefreshSettings)
	{
		if (getTimerInterval() != GUI_UPDATE_RATE_SUPERSLOW)
			startTimer(GUI_UPDATE_RATE_SUPERSLOW);
	}
}

/**
 * Sets the contained page components initializing state.
 * This is used to prevent the pages from each posting config update triggers
 * while themselves being updated with fresh config.
 * @param	initializing	The init state to set to the pages
 */
void PageContainerComponent::SetPagesBeingInitialized(bool initializing)
{
	// the tab component does send config update triggers as well when set to handling changes
	m_tabbedComponent->SetIsHandlingChanges(!initializing);

	m_soundobjectsPage->SetPageIsInitializing(initializing);
	m_multiSoundobjectsPage->SetPageIsInitializing(initializing);
	m_matrixIOPage->SetPageIsInitializing(initializing);
	m_settingsPage->SetPageIsInitializing(initializing);
	m_statisticsPage->SetPageIsInitializing(initializing);
	m_aboutPage->SetPageIsInitializing(initializing);
	m_scenesPage->SetPageIsInitializing(initializing);
	m_enSpacePage->SetPageIsInitializing(initializing);
}

/**
 * Method to externally set the currently active page.
 * This is used e.g. to restore the current active page from config file on app start.
 * @param pageId	The page id to set active
 */
void PageContainerComponent::SetActivePage(UIPageId pageId)
{
	jassert(pageId > UPI_InvalidMin && pageId < UPI_InvalidMax);

	m_soundobjectsPage->SetPageIsVisible(UPI_Soundobjects == pageId);
	m_multiSoundobjectsPage->SetPageIsVisible(UPI_MultiSoundobjects == pageId);
	m_matrixIOPage->SetPageIsVisible(UPI_MatrixIOs == pageId);
	m_settingsPage->SetPageIsVisible(UPI_Settings == pageId);
	m_statisticsPage->SetPageIsVisible(UPI_Statistics == pageId);
	m_scenesPage->SetPageIsVisible(UPI_Scenes == pageId);
	m_enSpacePage->SetPageIsVisible(UPI_EnSpace == pageId);

	m_tabbedComponent->setCurrentTabIndex(m_tabbedComponent->getTabNames().indexOf(GetPageNameFromId(pageId)));
}

/**
 * Method to open a given page in a separate window.
 * @param	pageId		The page id to open as window.
 * @param	windowPos	The screen positionwhere the window origin shall be placed
 */
void PageContainerComponent::OpenPageAsWindow(UIPageId pageId, const juce::Point<int>& windowPos)
{
	if (pageId <= UPI_InvalidMin || pageId >= UPI_Settings)
		return;

	m_tabbedComponent->SetIsHandlingChanges(false);
	if (m_tabbedComponent->getCurrentTabIndex() == m_tabbedComponent->getTabNames().indexOf(GetPageNameFromId(pageId)))
	{
		auto newActiveTabPageId = int(pageId);
		// loop over pages to find the next one to activate (that is not undocked to separate window)
		PageComponentBase* page = nullptr;
		do
		{
			newActiveTabPageId++;
			// if we have not found a page that is not shown as separate window, abort to avoid infinite looping
			if (newActiveTabPageId == pageId)
				break;
			// back to first tab if we have iterated until last
			if (newActiveTabPageId == UPI_About)
				newActiveTabPageId = UPI_InvalidMin + 1;

			page = GetComponentForPageId(UIPageId(newActiveTabPageId));
		} while (page && page->isOnDesktop());

		// set page (in-)visibility for all pages that currently are shown as tabs
		for (auto pageIdIter = int(UPI_InvalidMin + 1); pageIdIter < UPI_About; pageIdIter++)
		{
			page = GetComponentForPageId(static_cast<UIPageId>(pageIdIter));
			if (page)
			{
				if (!page->isOnDesktop())
					page->SetPageIsVisible(pageIdIter == newActiveTabPageId);
			}
		}

		m_tabbedComponent->setCurrentTabIndex(m_tabbedComponent->getTabNames().indexOf(GetPageNameFromId(static_cast<UIPageId>(newActiveTabPageId))));
	}
	m_tabbedComponent->removeTab(m_tabbedComponent->getTabNames().indexOf(GetPageNameFromId(static_cast<UIPageId>(pageId))));
	m_tabbedComponent->SetIsHandlingChanges(true);

	auto windowStyleFlags = ComponentPeer::StyleFlags::windowAppearsOnTaskbar
		| ComponentPeer::StyleFlags::windowHasCloseButton
		| ComponentPeer::StyleFlags::windowHasMaximiseButton
		| ComponentPeer::StyleFlags::windowHasTitleBar
		| ComponentPeer::StyleFlags::windowIsResizable;

	auto newWindowBounds = juce::Rectangle<int>(0, 0, 100, 100);
	if (auto mainComponent = Desktop::getInstance().getComponent(0))
		newWindowBounds = mainComponent->getBounds();
	newWindowBounds.setPosition(windowPos);

	auto windowedPage = GetComponentForPageId(pageId);
	if (windowedPage)
	{
		windowedPage->setOpaque(true);
		windowedPage->SetPageIsVisible(true);
		windowedPage->addToDesktop(windowStyleFlags);
		windowedPage->setBounds(newWindowBounds);
		windowedPage->setVisible(true);
	}

	// notify all pages of the windowing of the page
	for (auto pageIdIter = int(UPI_InvalidMin + 1); pageIdIter < UPI_About; pageIdIter++)
	{
		auto page = GetComponentForPageId(static_cast<UIPageId>(pageIdIter));
		if (page)
			page->NotifyPageWasWindowed(pageId, true);
	}
}

/**
 * Method to open a given page as a tab of tabbed component member.
 * @param pageId	The page id to open as tab.
 */
void PageContainerComponent::OpenPageAsTab(UIPageId pageId)
{
	// mute change broadcasting while we modify the tabs
	m_tabbedComponent->SetIsHandlingChanges(false);

	auto newTabPage = GetComponentForPageId(pageId);
	if (newTabPage)
	{
		// start clearing currently enabled tabs and recreate the ones to be enabled from now on
		m_tabbedComponent->clearTabs();

		for (auto pageIdIter = int(UPI_InvalidMin + 1); pageIdIter < UPI_About; pageIdIter++)
		{
			auto id = static_cast<UIPageId>(pageIdIter);
			auto tabPage = GetComponentForPageId(id);
			if (tabPage)
			{
				tabPage->NotifyPageWasWindowed(pageId, false);
				if (!tabPage->isOnDesktop() || id == pageId) // if the page currently is a tab or is the one that shall be added as new tab
					m_tabbedComponent->addTab(GetPageNameFromId(id), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), tabPage, false, id);
			}
		}
		m_tabbedComponent->setCurrentTabIndex(m_tabbedComponent->getTabNames().indexOf(GetPageNameFromId(pageId)));
	}

	m_tabbedComponent->SetIsHandlingChanges(true);

	resized();
}

/**
 * Method to externally set the enabled tabs (pages).
 * @param enabledPages	The pages to set active (visible)
 */
void PageContainerComponent::SetEnabledPages(const std::vector<UIPageId>& enabledPages)
{
	// mute change broadcasting while we modify the tabs
	m_tabbedComponent->SetIsHandlingChanges(false);

	// cache the currently active tab to reactivate it after tab recreation (don't default to first tab)
	auto activeTabId = GetPageIdFromName(m_tabbedComponent->getCurrentTabName());

	// start clearing currently enabled tabs and recreate the ones to be enabled from now on
	m_tabbedComponent->clearTabs();

	auto SoundObjectsPageEnabled = std::find(enabledPages.begin(), enabledPages.end(), UPI_Soundobjects) != enabledPages.end();
	auto MultiSliderPageEnabled = std::find(enabledPages.begin(), enabledPages.end(), UPI_MultiSoundobjects) != enabledPages.end();
	auto MatrixIOsPageEnabled = std::find(enabledPages.begin(), enabledPages.end(), UPI_MatrixIOs) != enabledPages.end();
	auto ScenesPageEnabled = std::find(enabledPages.begin(), enabledPages.end(), UPI_Scenes) != enabledPages.end();
	auto EnSpacePageEnabled = std::find(enabledPages.begin(), enabledPages.end(), UPI_EnSpace) != enabledPages.end();
	auto StatisticsPageEnabled = std::find(enabledPages.begin(), enabledPages.end(), UPI_Statistics) != enabledPages.end();

	if (SoundObjectsPageEnabled && m_soundobjectsPage && !m_soundobjectsPage->isOnDesktop())
		m_tabbedComponent->addTab(GetPageNameFromId(UPI_Soundobjects), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_soundobjectsPage.get(), false, UPI_Soundobjects);
	if (MultiSliderPageEnabled && m_multiSoundobjectsPage && !m_multiSoundobjectsPage->isOnDesktop())
		m_tabbedComponent->addTab(GetPageNameFromId(UPI_MultiSoundobjects), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_multiSoundobjectsPage.get(), false, UPI_MultiSoundobjects);
	if (MatrixIOsPageEnabled && m_matrixIOPage && !m_matrixIOPage->isOnDesktop())
		m_tabbedComponent->addTab(GetPageNameFromId(UPI_MatrixIOs), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_matrixIOPage.get(), false, UPI_MatrixIOs);
	if (ScenesPageEnabled && m_scenesPage && !m_scenesPage->isOnDesktop())
		m_tabbedComponent->addTab(GetPageNameFromId(UPI_Scenes), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_scenesPage.get(), false, UPI_Scenes);
	if (EnSpacePageEnabled && m_enSpacePage && !m_enSpacePage->isOnDesktop())
		m_tabbedComponent->addTab(GetPageNameFromId(UPI_EnSpace), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_enSpacePage.get(), false, UPI_EnSpace);
	if (StatisticsPageEnabled && m_statisticsPage && !m_statisticsPage->isOnDesktop())
		m_tabbedComponent->addTab(GetPageNameFromId(UPI_Statistics), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_statisticsPage.get(), false, UPI_Statistics);

	m_tabbedComponent->addTab(GetPageNameFromId(UPI_Settings), getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_settingsPage.get(), false, UPI_Settings);

	// restore the previously active tab
	m_tabbedComponent->setCurrentTabIndex(m_tabbedComponent->getTabNames().indexOf(GetPageNameFromId(activeTabId)));

	// reenable change broadcasting
	m_tabbedComponent->SetIsHandlingChanges(true);
}

/**
 * Reimplemented method to handle changed look and feel data.
 * This makes shure the help buttons' svg images are colored correctly.
 */
void PageContainerComponent::lookAndFeelChanged()
{
	// first forward the call to base implementation
	Component::lookAndFeelChanged();

	// Update drawable button images with updated lookAndFeel colours
	UpdateDrawableButtonImages(m_helpButton, BinaryData::help24px_svg, &getLookAndFeel());
}

/**
 * Getter for the resizer bar ratio in sound objects table
 * @return	The resizer bar ratio.
 */
float PageContainerComponent::GetSoundobjectTableResizeBarRatio()
{
	if (m_soundobjectsPage)
		return m_soundobjectsPage->GetResizeBarRatio();
	else
	{
		jassertfalse;
		return 0.5f;
	}
}

/**
 * Setter for the resizer bar ratio in sound objects table
 * @param ratio	The resizer bar ratio.
 */
void PageContainerComponent::SetSoundobjectTableResizeBarRatio(float ratio)
{
	if (m_soundobjectsPage)
		return m_soundobjectsPage->SetResizeBarRatio(ratio);
}

/**
 * Getter for the single selection only flag in sound objects table.
 * @return	The single selection only flag.
 */
bool PageContainerComponent::GetSoundobjectTableSingleSelectionOnly()
{
	if (m_soundobjectsPage)
		return m_soundobjectsPage->IsSingleSelectionOnly();
	else
	{
		jassertfalse;
		return false;
	}
}

/**
 * Setter for the single selection only flag in sound objects table.
 * @param singleSelectionOnly	The single selection only flag.
 */
void PageContainerComponent::SetSoundobjectTableSingleSelectionOnly(bool singleSelectionOnly)
{
	if (m_soundobjectsPage)
		m_soundobjectsPage->SetSingleSelectionOnly(singleSelectionOnly);
}

/**
 * Getter for the row height in sound objects table
 * @return	The table row height.
 */
int PageContainerComponent::GetSoundobjectTableRowHeight()
{
	if (m_soundobjectsPage)
		return m_soundobjectsPage->GetRowHeight();
	else
	{
		jassertfalse;
		return 0;
	}
}

/**
 * Setter for the row height in sound objects table
 * @param height	The table row height.
 */
void PageContainerComponent::SetSoundobjectTableRowHeight(int height)
{
	if (m_soundobjectsPage)
		return m_soundobjectsPage->SetRowHeight(height);
}

/**
 * Getter for the row height in matrix inputs table
 * @return	The table row height.
 */
int PageContainerComponent::GetMatrixInputTableRowHeight()
{
	if (m_matrixIOPage)
		return m_matrixIOPage->GetInputsRowHeight();
	else
	{
		jassertfalse;
		return 0;
	}
}

/**
 * Setter for the row height in matrix inputs table
 * @param height	The table row height.
 */
void PageContainerComponent::SetMatrixInputTableRowHeight(int height)
{
	if (m_matrixIOPage)
		return m_matrixIOPage->SetInputsRowHeight(height);
}

/**
 * Getter for the row height in matrix outputs table
 * @return	The table row height.
 */
int PageContainerComponent::GetMatrixOutputTableRowHeight()
{
	if (m_matrixIOPage)
		return m_matrixIOPage->GetOutputsRowHeight();
	else
	{
		jassertfalse;
		return 0;
	}
}

/**
 * Setter for the row height in matrix outputs table
 * @param height	The table row height.
 */
void PageContainerComponent::SetMatrixOutputTableRowHeight(int height)
{
	if (m_matrixIOPage)
		return m_matrixIOPage->SetOutputsRowHeight(height);
}

/**
 * Getter for the collapsed state of matrix inputs table
 * @return	The table row height.
 */
bool PageContainerComponent::GetMatrixInputTableCollapsed()
{
	if (m_matrixIOPage)
		return m_matrixIOPage->GetInputsCollapsed();
	else
	{
		jassertfalse;
		return false;
	}
}

/**
 * Setter for the collapsed state of matrix inputs table
 * @param height	The table row height.
 */
void PageContainerComponent::SetMatrixInputTableCollapsed(bool collapsed)
{
	if (m_matrixIOPage)
		return m_matrixIOPage->SetInputsCollapsed(collapsed);
}

/**
 * Getter for the collapsed state of matrix outputs table
 * @return	The table row height.
 */
bool PageContainerComponent::GetMatrixOutputTableCollapsed()
{
	if (m_matrixIOPage)
		return m_matrixIOPage->GetOutputsCollapsed();
	else
	{
		jassertfalse;
		return false;
	}
}

/**
 * Setter for the collapsed state of matrix outputs table
 * @param height	The table row height.
 */
void PageContainerComponent::SetMatrixOutputTableCollapsed(bool collapsed)
{
	if (m_matrixIOPage)
		return m_matrixIOPage->SetOutputsCollapsed(collapsed);
}

/**
 * Getter for the pinned scenes of Scenes Page
 * @return	The pinned scenes.
 */
std::vector<std::pair<std::pair<int, int>, std::string>> PageContainerComponent::GetScenesPagePinnedScenes()
{
	if (m_scenesPage)
		return m_scenesPage->GetPinnedScenes();
	else
	{
		jassertfalse;
		return std::vector<std::pair<std::pair<int, int>, std::string>>();
	}
}

/**
 * Setter for the pinned scenes of Scenes Page
 * @param pinnedScenes	The pinned scenes.
 */
void PageContainerComponent::SetScenesPagePinnedScenes(const std::vector<std::pair<std::pair<int, int>, std::string>>& pinnedScenes)
{
	if (m_scenesPage)
		m_scenesPage->SetPinnedScenes(pinnedScenes);
}

void PageContainerComponent::SetOverlayComponent(Component* componentToOverlay)
{
	if (componentToOverlay != nullptr)
	{
		m_overlayComponent = componentToOverlay;

		addAndMakeVisible(m_overlayComponent);

		resized();
		repaint();
	}
}

void PageContainerComponent::ClearOverlayComponent()
{
	if (m_overlayComponent)
	{
		m_overlayComponent->setVisible(false);
		removeChildComponent(m_overlayComponent);
	}

	m_overlayComponent = nullptr;

	resized();
	repaint();
}


/*
===============================================================================
 Class CustomButtonTabbedComponent
===============================================================================
*/

/**
 * Class constructor.
 */
CustomButtonTabbedComponent::CustomButtonTabbedComponent()
	: TabbedComponent(TabbedButtonBar::TabsAtTop)
{
}

/**
 * Class destructor.
 */
CustomButtonTabbedComponent::~CustomButtonTabbedComponent()
{
}

/**
 * Reimplemented to create and return custom tab bar buttons.
 * @param tabName	Text on the tab button. Not used in this implementation.
 * @param tabIndex	Index of the tab from left to right, starting at 0.
 * @return	Pointer to a CustomDrawableTabBarButton.
 */
TabBarButton* CustomButtonTabbedComponent::createTabButton(const String& tabName, int tabIndex)
{
	ignoreUnused(tabIndex);

	auto button = new CustomDrawableTabBarButton(GetPageIdFromName(tabName), getTabbedButtonBar());
	button->onButtonDraggedForTabDetaching = [this] (UIPageId pageId, const juce::Point<int>& mouseUpPos) {
		PageComponentManager* pageMgr = PageComponentManager::GetInstance();
		if (pageMgr)
			pageMgr->OpenPageAsWindow(pageId, mouseUpPos, false);
	};

	return button;
}

/**
 * Callback method to indicate the selected tab has been changed.
 * Reimplemented to trigger a GUI update on the newly active tab.
 * @param newCurrentTabIndex	Index of the tab from left to right, starting at 0.
 * @param newCurrentTabName		Name of the tab.
 * @return	Pointer to a CustomDrawableTabBarButton.
 */
void CustomButtonTabbedComponent::currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName)
{
	ignoreUnused(newCurrentTabIndex);

	if (!GetIsHandlingChanges())
		return;

	auto newCurrentPageId = GetPageIdFromName(newCurrentTabName);

	PageComponentManager* pageMgr = PageComponentManager::GetInstance();
	if (pageMgr)
		pageMgr->SetActivePage(newCurrentPageId, false);

	PageContainerComponent* parent = dynamic_cast<PageContainerComponent*>(getParentComponent());
	if (parent)
		parent->UpdateGui(false);
}

/**
 * Reimplemented to re-postion the tabBar so make the tab buttons start further to the right.
 */
void CustomButtonTabbedComponent::resized()
{
	int w = getLocalBounds().getWidth();
	getTabbedButtonBar().setBounds(Rectangle<int>(40, 0, w - (40 + 86), 44));
}

/**
 * Getter for the bool flag that indicates if tab changes should be broadcasted
 * @return True if changes are currently handled (broadcasted), false if not
 */
bool CustomButtonTabbedComponent::GetIsHandlingChanges()
{
	return m_isHandlingChanges;
}

/**
 * Setter for the bool flag that indicates if tab changes should be broadcasted
 * @param isHandlingChanges		The new value to be set as the new internal bool flag state
 */
void CustomButtonTabbedComponent::SetIsHandlingChanges(bool isHandlingChanges)
{
	m_isHandlingChanges = isHandlingChanges;
}


/*
===============================================================================
 Class CustomDrawableTabBarButton
===============================================================================
*/

/**
 * Class constructor.
 * @param pageId	The page id to use.
 * @param ownerBar	TabbedButtonBar object which contains this button.
 */
CustomDrawableTabBarButton::CustomDrawableTabBarButton(UIPageId pageId, TabbedButtonBar& ownerBar)
	: TabBarButton(String(), ownerBar),
	m_pageId(pageId)
{
	updateDrawableButtonImageColours();
}

/**
 * Class destructor.
 */
CustomDrawableTabBarButton::~CustomDrawableTabBarButton()
{
}

/**
 * Helper method to update the drawables used for buttons to match the text colour
 */
void CustomDrawableTabBarButton::updateDrawableButtonImageColours()
{
	String imageName;
	switch (m_pageId)
	{
	case UPI_Soundobjects:
		imageName = BinaryData::vertical_split24px_svg;
		break;
	case UPI_MultiSoundobjects:
		imageName = BinaryData::grain24px_svg;
		break;
    case UPI_MatrixIOs:
        imageName = BinaryData::tune24px_svg;
        break;
	case UPI_Settings:
		imageName = BinaryData::settings24px_svg;
		break;
	case UPI_Statistics:
		imageName = BinaryData::show_chart24px_svg;
		break;
	case UPI_Scenes:
		imageName = BinaryData::slideshow_black_24dp_svg;
		break;
	case UPI_EnSpace:
		imageName = BinaryData::sensors_black_24dp_svg;
		break;
	default:
		imageName = BinaryData::clear_black_24dp_svg;
		break;
	}

	if (m_normalImage)
		removeChildComponent(m_normalImage.get());
	if (m_overImage)
		removeChildComponent(m_overImage.get());
	if (m_downImage)
		removeChildComponent(m_downImage.get());
	if (m_disabledImage)
		removeChildComponent(m_disabledImage.get());
	if (m_normalOnImage)
		removeChildComponent(m_normalOnImage.get());
	if (m_overOnImage)
		removeChildComponent(m_overOnImage.get());
	if (m_downOnImage)
		removeChildComponent(m_downOnImage.get());
	if (m_disabledOnImage)
		removeChildComponent(m_disabledOnImage.get());

	auto customlookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (customlookAndFeel)
		JUCEAppBasics::Image_utils::getDrawableButtonImages(imageName, m_normalImage, m_overImage, m_downImage, m_disabledImage, m_normalOnImage, m_overOnImage, m_downOnImage, m_disabledOnImage,
			customlookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			customlookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
			customlookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			customlookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
			customlookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			customlookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			customlookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
			customlookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));
	else
		JUCEAppBasics::Image_utils::getDrawableButtonImages(imageName, m_normalImage, m_overImage, m_downImage, m_disabledImage, m_normalOnImage, m_overOnImage, m_downOnImage, m_disabledOnImage);

	addChildComponent(m_normalImage.get());
	addChildComponent(m_overImage.get());
	addChildComponent(m_downImage.get());
	addChildComponent(m_disabledImage.get());
	addChildComponent(m_normalOnImage.get());
	addChildComponent(m_overOnImage.get());
	addChildComponent(m_downOnImage.get());
	addChildComponent(m_disabledOnImage.get());
}

/**
* Reimplemented from Component to recreate the button drawables accordingly.
*/
void CustomDrawableTabBarButton::lookAndFeelChanged()
{
	// update the drawable button images
	updateDrawableButtonImageColours();
	// and forward the call to base implementation
	TabBarButton::lookAndFeelChanged();
}

/**
 * Reimplemented paint function, to display an icon.
 * @param g					A graphics context, used for drawing a component or image.
 * @param isMouseOverButton	True if the mouse is over the button.
 * @param isButtonDown		True if the button is being pressed.
 */
void CustomDrawableTabBarButton::paintButton(Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
	// The original TabBarButton::paintButton draws a gradient on the buttons which
	// are inactive. We don't want that, just paint them with the background color.
	Colour buttonBackground(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	if (getToggleState())
		buttonBackground = getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker();
	else if (isButtonDown)
		buttonBackground = buttonBackground.brighter(0.1f);
	else if (isMouseOverButton)
		buttonBackground = buttonBackground.brighter(0.05f);

	Rectangle<int> activeArea(getActiveArea());
	g.setColour(buttonBackground);
	g.fillRect(activeArea);

	// make the drawable visible that corresponds to the given bool flag values
	Drawable* visibleDrawable{ m_normalImage.get() };
	if (isButtonDown)
		visibleDrawable = m_downImage.get();
	else if (isMouseOverButton)
		visibleDrawable = m_overImage.get();

	if (setVisibleDrawable(visibleDrawable))
		resized();
}

/**
 * Reimplemented to resize and re-postion controls on the tabbarbutton.
 */
void CustomDrawableTabBarButton::resized()
{
	auto iconBounds = Rectangle<int>(0, 0, 25, 25);
	auto activeArea = getActiveArea();
	auto xOffset = (activeArea.getWidth() / 2) - (iconBounds.getWidth() / 2);
	auto yOffset = (activeArea.getHeight() / 2) - (iconBounds.getHeight() / 2);
	iconBounds.setPosition(xOffset, yOffset);

	m_normalImage->setBounds(iconBounds);
	m_overImage->setBounds(iconBounds);
	m_downImage->setBounds(iconBounds);
	m_disabledImage->setBounds(iconBounds);
	m_normalOnImage->setBounds(iconBounds);
	m_overOnImage->setBounds(iconBounds);
	m_downOnImage->setBounds(iconBounds);
	m_disabledOnImage->setBounds(iconBounds);
}

/**
 * Helper method to set one of the drawables visible
 * @param visibleDrawable The drawable to become visible
 * @return True if the drawable was made visible, false if it already was visible or the given pointer is invalid
 */
bool CustomDrawableTabBarButton::setVisibleDrawable(Drawable* visibleDrawable)
{
	// if the drawable is already visible -> return false to indicate that nothing was changed
	if (!visibleDrawable || visibleDrawable->isVisible())
		return false;

	m_normalImage->setVisible(m_normalImage.get() == visibleDrawable);
	m_overImage->setVisible(m_overImage.get() == visibleDrawable);
	m_downImage->setVisible(m_downImage.get() == visibleDrawable);
	m_disabledImage->setVisible(m_disabledImage.get() == visibleDrawable);
	m_normalOnImage->setVisible(m_normalOnImage.get() == visibleDrawable);
	m_overOnImage->setVisible(m_overOnImage.get() == visibleDrawable);
	m_downOnImage->setVisible(m_downOnImage.get() == visibleDrawable);
	m_disabledOnImage->setVisible(m_disabledOnImage.get() == visibleDrawable);

	return true;
}

/**
 * Reimplemented from juce::Component to track if a tab was dragged
 * outside of the bar to have it appear as own window.
 * @param	event	The details of the mouse movement that lead here.
 */
void CustomDrawableTabBarButton::mouseUp(const MouseEvent& event)
{
    
#if JUCE_IOS || JUCE_ANDROID
    TabBarButton::mouseUp(event);
#else
	if (!getTopLevelComponent() || !getParentComponent())
		return TabBarButton::mouseUp(event);
	auto windowPosition = getTopLevelComponent()->getPosition();
	auto tabBarBounds = getParentComponent()->getLocalBounds();
	auto tabBarPosition = getParentComponent()->getPosition();
	auto tabBarButtonPosition = getPosition();
	auto mouseUpPositionInTabBarButton = event.position.toInt();
	auto mouseUpPositionInTabBar = tabBarButtonPosition + mouseUpPositionInTabBarButton;
	if (!tabBarBounds.contains(mouseUpPositionInTabBar) && onButtonDraggedForTabDetaching)
	{
		auto mouseUpPosition = windowPosition + tabBarPosition + mouseUpPositionInTabBar;
		onButtonDraggedForTabDetaching(m_pageId, mouseUpPosition);
	}
	else
        TabBarButton::mouseUp(event);
#endif
}


} // namespace SpaConBridge
