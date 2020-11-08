/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file is part of the Soundscape VST, AU, and AAX Plug-in.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY d&b audiotechnik GmbH & Co. KG "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================
*/


#include "PageContainerComponent.h"

#include "PageComponentManager.h"
#include "PageComponents/TablePageComponent.h"
#include "PageComponents/MultiSurfacePageComponent.h"
#include "PageComponents/SettingsPageComponent.h"
#include "PageComponents/StatisticsPageComponent.h"
#include "PageComponents/AboutPageComponent.h"

#include "../Controller.h"
#include "../SoundsourceProcessor/SurfaceSlider.h"

#include <Image_utils.hpp>


namespace SoundscapeBridgeApp
{


/**
 * Rate at which the Overview GUI will refresh, when the multi-slider tab is selected.
 */
static constexpr int GUI_UPDATE_RATE_FAST = 75;

/**
 * Rate at which the Overview GUI will refresh.
 */
static constexpr int GUI_UPDATE_RATE_SLOW = 120;


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
	// Online
	m_onlineLabel = std::make_unique<Label>("Online Label", "Online:");
	m_onlineLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(m_onlineLabel.get());
	m_onlineLed = std::make_unique<LedButton>();
	m_onlineLed->setEnabled(false);
	addAndMakeVisible(m_onlineLed.get());

	// Interval
	m_rateTextEdit = std::make_unique<TextEditor>("OSC Send Rate");
	m_rateTextEdit->addListener(this);
	addAndMakeVisible(m_rateTextEdit.get());
	m_rateLabel = std::make_unique<Label>("OSC Send Rate", "Interval:");
	m_rateLabel->setJustificationType(Justification::centred);
	addAndMakeVisible(m_rateLabel.get());

	// app logo button and Plugin version label
	m_logoButton = std::make_unique<ImageButton>("LogoButton");
	m_logoButton->setImages(false, true, true,
		ImageCache::getFromMemory(BinaryData::SoundscapeBridgeApp_png, BinaryData::SoundscapeBridgeApp_pngSize), 1.0f, Colours::transparentWhite,
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
	m_tablePage = std::make_unique<TablePageComponent>();
	m_multiSliderPage = std::make_unique<MultiSurfacePageComponent>();
	m_settingsPage = std::make_unique<SettingsPageComponent>();
	m_statisticsPage = std::make_unique<StatisticsPageComponent>();
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
	m_tabbedComponent->addTab("Table", getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_tablePage.get(), false, CustomButtonTabbedComponent::OTI_Table);
	m_tabbedComponent->addTab("Slider", getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_multiSliderPage.get(), false, CustomButtonTabbedComponent::OTI_MultiSlider);
	m_tabbedComponent->addTab("Statistics", getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_statisticsPage.get(), false, CustomButtonTabbedComponent::OTI_Statistics);
	m_tabbedComponent->addTab("Settings", getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(), m_settingsPage.get(), false, CustomButtonTabbedComponent::OTI_Settings);
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

	FlexBox bottomBarFB;
	bottomBarFB.flexDirection = FlexBox::Direction::row;
	bottomBarFB.justifyContent = FlexBox::JustifyContent::center;
	bottomBarFB.alignContent = FlexBox::AlignContent::center;
	bottomBarFB.items.addArray({
		// Rate
		FlexItem(*m_rateLabel.get()).withWidth(65).withHeight(25).withMargin(FlexItem::Margin(5, 0, 5, 10)),
		FlexItem(*m_rateTextEdit.get()).withHeight(25).withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0)),
		FlexItem().withFlex(1),
		// Online
		FlexItem(*m_onlineLabel.get()).withWidth(65).withHeight(25).withMargin(FlexItem::Margin(5, 0, 5, 0)),
		FlexItem(*m_onlineLed.get()).withWidth(24).withHeight(24).withMargin(FlexItem::Margin(5, 10, 5, 0)),
		});
	bottomBarFB.performLayout(getLocalBounds().removeFromBottom(45));

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
	m_tablePage->setBounds(rect);
	m_multiSliderPage->setBounds(rect);
	m_settingsPage->setBounds(rect);
	m_statisticsPage->setBounds(rect);

	// finally resize the aboutpage, if visible and therefor on top of everything else at all
	if (m_aboutPage && m_aboutPage->isVisible())
	{
		m_aboutPage->setBounds(getLocalBounds());
		m_aboutPage->toFront(false);
	}
}

/**
 * Callback function for changes to our textEditors.
 * @param textEditor	The TextEditor object whose content has just changed.
 */
void PageContainerComponent::textEditorFocusLost(TextEditor& textEditor)
{
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		// OSC message rate has changed
		if (&textEditor == m_rateTextEdit.get())
		{
			ctrl->SetRate(DCS_Overview, textEditor.getText().getIntValue());
		}
	}
}

/**
 * Callback function for Enter key presses on textEditors.
 * @param textEditor	The TextEditor object whose where enter key was pressed.
 */
void PageContainerComponent::textEditorReturnKeyPressed(TextEditor& textEditor)
{
	ignoreUnused(textEditor);

	// Remove keyboard focus from this editor.
	// Function textEditorFocusLost will then take care of setting values.
	getParentComponent()->grabKeyboardFocus();
}

/**
 * Callback function for button clicks.
 * @param button	The button object that was clicked.
 */
void PageContainerComponent::buttonClicked(Button* button)
{
	if (m_logoButton && m_logoButton.get() == button && m_aboutPage)
	{
		toggleAboutPage();
	}
}

void PageContainerComponent::toggleAboutPage()
{
	if (m_aboutPage->isVisible())
	{
		m_aboutPage->setVisible(false);
		removeChildComponent(m_aboutPage.get());
	}
	else
	{
		addAndMakeVisible(m_aboutPage.get());
	}
	resized();
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
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		if (ctrl->PopParameterChanged(DCS_Overview, DCT_MessageRate) || init)
			m_rateTextEdit->setText(String(ctrl->GetRate()) + " ms", false);

		if (ctrl->PopParameterChanged(DCS_Overview, DCT_Online) || init)
			m_onlineLed->setToggleState(ctrl->GetOnline(), NotificationType::dontSendNotification);
	}

	// Save some performance: only update the component inside the currently active tab.
	if (m_tabbedComponent->getCurrentTabIndex() == CustomButtonTabbedComponent::OTI_Table)
	{
		if (m_tablePage)
			m_tablePage->UpdateGui(init);

		// When the overview table is active, no need to refresh GUI very quickly
		if (getTimerInterval() == GUI_UPDATE_RATE_FAST)
		{
			//DBG("PageContainerComponent::timerCallback(): Switching to GUI_UPDATE_RATE_SLOW");
			startTimer(GUI_UPDATE_RATE_SLOW);
		}
	}
	else if (m_tabbedComponent->getCurrentTabIndex() == CustomButtonTabbedComponent::OTI_MultiSlider)
	{
		if (m_multiSliderPage)
			m_multiSliderPage->UpdateGui(init);

		// When multi-slider is active, we refresh the GUI faster
		if (getTimerInterval() == GUI_UPDATE_RATE_SLOW)
		{
			//DBG("PageContainerComponent::timerCallback: Switching to GUI_UPDATE_RATE_FAST");
			startTimer(GUI_UPDATE_RATE_FAST);
		}
	}
}

/**
 * Method to externally set the currently active tab.
 * This is used to restore the current active tab from config file on app start.
 * @param tabIdx	The tab index to set active
 */
void PageContainerComponent::SetActiveTab(int tabIdx)
{
	m_tabbedComponent->setCurrentTabIndex(tabIdx, false);
}

/**
 * Setter for the currently selected look and feel type.
 * @param lookAndFeelType	The look and feel type to set as currently selected in dropdown
 */
void PageContainerComponent::SetLookAndFeelType(DbLookAndFeelBase::LookAndFeelType lookAndFeelType)
{
	m_settingsPage->SetSelectedLookAndFeelType(lookAndFeelType);
}

/**
 * Getter for the currently selected look and feel type.
 * @return	The look and feel type that currently is selected in dropdown
 */
DbLookAndFeelBase::LookAndFeelType PageContainerComponent::GetLookAndFeelType()
{
	return m_settingsPage->GetSelectedLookAndFeelType();
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
	ignoreUnused(tabName);
	return new CustomDrawableTabBarButton(tabIndex, getTabbedButtonBar());
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
	ignoreUnused(newCurrentTabName);

	if (!GetIsHandlingChanges())
		return;

	PageComponentManager* pageMgr = PageComponentManager::GetInstance();
	if (pageMgr)
		pageMgr->SetActiveTab(newCurrentTabIndex, false);

	PageContainerComponent* parent = dynamic_cast<PageContainerComponent*>(getParentComponent());
	if (parent)
		parent->UpdateGui(true);
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
 * @param tabIdx	Tab index starting at 0.
 * @param ownerBar	TabbedButtonBar object which contains this button.
 */
CustomDrawableTabBarButton::CustomDrawableTabBarButton(int tabIdx, TabbedButtonBar& ownerBar)
	: TabBarButton(String(), ownerBar),
	m_tabIndex(tabIdx)
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
	switch (m_tabIndex)
	{
	case CustomButtonTabbedComponent::OTI_Table:
		imageName = BinaryData::vertical_split24px_svg;
		break;
	case CustomButtonTabbedComponent::OTI_MultiSlider:
		imageName = BinaryData::grain24px_svg;
		break;
	case CustomButtonTabbedComponent::OTI_Settings:
		imageName = BinaryData::settings24px_svg;
		break;
	case CustomButtonTabbedComponent::OTI_Statistics:
		imageName = BinaryData::show_chart24px_svg;
		break;
	default:
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


} // namespace SoundscapeBridgeApp
