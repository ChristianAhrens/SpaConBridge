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


#include "Overview.h"

#include "OverviewManager.h"
#include "OverviewMultisurface.h"
#include "OverviewSettings.h"
#include "OverviewTable.h"

#include "../Controller.h"
#include "../SoundsourceProcessor/SurfaceSlider.h"

#include "../submodules/JUCE-AppBasics/Source/Image_utils.hpp"


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
 Class COverviewComponent
===============================================================================
*/

/**
 * Class constructor.
 */
COverviewComponent::COverviewComponent()
{
	// IP Settings
	m_ipAddressTextEdit = std::make_unique<CTextEditor>("IP Address");
	m_ipAddressTextEdit->addListener(this);
	addAndMakeVisible(m_ipAddressTextEdit.get());
	m_ipAddressLabel = std::make_unique<CLabel>("IP Address Label", "IP Address:");
	addAndMakeVisible(m_ipAddressLabel.get());

	// Online
	m_onlineLed = std::make_unique<CButton>("");
	m_onlineLed->setEnabled(false);
	m_onlineLed->SetCornerRadius(10);
	addAndMakeVisible(m_onlineLed.get());

	// Interval
	m_rateTextEdit = std::make_unique<CTextEditor>("OSC Send Rate");
	m_rateTextEdit->SetSuffix("ms");
	m_rateTextEdit->addListener(this);
	addAndMakeVisible(m_rateTextEdit.get());
	m_rateLabel = std::make_unique<CLabel>("OSC Send Rate", "Interval:");
	addAndMakeVisible(m_rateLabel.get());

	// d&b logo and Plugin version label
	m_appLogo = ImageCache::getFromMemory(BinaryData::SoundscapeBridgeApp_png, BinaryData::SoundscapeBridgeApp_pngSize);
	m_versionLabel = std::make_unique<CLabel>("PluginVersion", String(JUCE_STRINGIFY(JUCE_APP_VERSION)));
	m_versionLabel->setFont(Font(11));
	addAndMakeVisible(m_versionLabel.get());
	m_nameLabel = std::make_unique<CLabel>("PluginName", "Version");
	m_nameLabel->setFont(Font(11));
	m_nameLabel->setColour(Label::textColourId, CDbStyle::GetDbColor(CDbStyle::DarkTextColor));
	addAndMakeVisible(m_nameLabel.get());

	m_titleLabel = std::make_unique<CLabel>("Title", "");
	addAndMakeVisible(m_titleLabel.get());

	// Create the table container.
	m_tableContainer = std::make_unique<COverviewTableContainer>();
	m_multiSliderContainer = std::make_unique<COverviewMultiSurface>();

	// Create a tab container, where the COverviewTableContainer will be one of the tabs.
	m_tabbedComponent = std::make_unique<CTabbedComponent>();
	m_tabbedComponent->setTabBarDepth(44);
	m_tabbedComponent->setOutline(0);
	m_tabbedComponent->setIndent(0);
	addAndMakeVisible(m_tabbedComponent.get());

	// Add the overview tabs.
	m_tabbedComponent->addTab("Table", CDbStyle::GetDbColor(CDbStyle::DarkColor), m_tableContainer.get(), false);
	m_tabbedComponent->addTab("Slider", CDbStyle::GetDbColor(CDbStyle::DarkColor), m_multiSliderContainer.get(), false);
	m_tabbedComponent->addTab("Settings", CDbStyle::GetDbColor(CDbStyle::DarkColor), m_settingsContainer.get(), false);

	// Remember which tab was active before the last time the overview was closed.
	COverviewManager* ovrMgr = COverviewManager::GetInstance();
	if (ovrMgr)
		m_tabbedComponent->setCurrentTabIndex(ovrMgr->GetActiveTab());

	// Start GUI-refreshing timer.
	startTimer(GUI_UPDATE_RATE_SLOW);
}

/**
 * Class destructor.
 */
COverviewComponent::~COverviewComponent()
{
	// Remember which tab was active before the last time the overview was closed.
	COverviewManager* ovrMgr = COverviewManager::GetInstance();
	if (ovrMgr && m_tabbedComponent)
		ovrMgr->SetActiveTab(m_tabbedComponent->getCurrentTabIndex());
}

/**
 * Reimplemented to paint background and logo.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void COverviewComponent::paint(Graphics& g)
{
	int w = getLocalBounds().getWidth();
	int h = getLocalBounds().getHeight();	

	// Bars above and below
	g.setColour(CDbStyle::GetDbColor(CDbStyle::MidColor));
	g.fillRect(getLocalBounds());

	// Background
	g.setColour(CDbStyle::GetDbColor(CDbStyle::DarkColor));
	g.fillRect(Rectangle<int>(0, 43, w, h - 87));

	// Little lines between version and logo
	g.setColour(CDbStyle::GetDbColor(CDbStyle::ButtonColor));
	g.fillRect(Rectangle<int>(w - 39, 6, 1, 30));
	g.fillRect(Rectangle<int>(w - 86, 6, 1, 30));

	// Add app logo 
	g.drawImage(m_appLogo, getLocalBounds().getWidth() - 35, 6, 30, 30, 0, 0, 1024, 1024);

	// Draw little line below right and left overlap of tabbedcomponent buttonbar to match with the line which is automatically drawn 
	// by the CTabbedComponent's CTabBarButton.
	g.setColour(Colour(108, 113, 115));
	g.drawRect(Rectangle<int>(0, 43, 40, 1), 1);
	g.drawRect(Rectangle<int>(w - 86, 43, 86, 1), 1);
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void COverviewComponent::resized()
{
	int w = getLocalBounds().getWidth();

	FlexBox bottomBarFB;
	bottomBarFB.flexDirection = FlexBox::Direction::row;
	bottomBarFB.justifyContent = FlexBox::JustifyContent::center;
	bottomBarFB.alignContent = FlexBox::AlignContent::center;
	bottomBarFB.items.addArray({
		// Ip Address
		FlexItem(*m_ipAddressLabel.get()).withWidth(74).withHeight(25).withMargin(FlexItem::Margin(5, 0, 5, 10)),
		FlexItem(*m_ipAddressTextEdit.get()).withHeight(25).withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0)),
		// Rate
		FlexItem(*m_rateLabel.get()).withWidth(65).withHeight(25).withMargin(FlexItem::Margin(5, 0, 5, 0)),
		FlexItem(*m_rateTextEdit.get()).withHeight(25).withFlex(1).withMargin(FlexItem::Margin(5, 0, 5, 0)),
		FlexItem().withFlex(1),
		// Online
		FlexItem(*m_onlineLed.get()).withWidth(24).withHeight(24).withMargin(FlexItem::Margin(5, 10, 5, 0)),
		});
	bottomBarFB.performLayout(getLocalBounds().removeFromBottom(45));

	// Name and Version label
	m_nameLabel->setBounds(w - 89, 3, 55, 25);
	m_versionLabel->setBounds(w - 87, 21, 42, 15);

	// Tab container takes up the entire window minus the bottom bar (with the IP etc).
	// See CTabbedComponent::resized().
	m_tabbedComponent->setBounds(Rectangle<int>(0, 0, w, getLocalBounds().getHeight() - 45));

	// Resize overview table container.
	m_tableContainer->setBounds(Rectangle<int>(0, 44, w, getLocalBounds().getHeight() - 89));
	m_multiSliderContainer->setBounds(Rectangle<int>(0, 44, w, getLocalBounds().getHeight() - 89));
}

/**
 * Callback function for changes to our textEditors.
 * @param textEditor	The TextEditor object whose content has just changed.
 */
void COverviewComponent::textEditorFocusLost(TextEditor& textEditor)
{
	CTextEditor *myEditor = static_cast<CTextEditor*>(&textEditor);
	CController* ctrl = CController::GetInstance();
	if (ctrl && myEditor)
	{
		// IP Address changed
		if (myEditor == m_ipAddressTextEdit.get())
		{
			// IP address validation: If IPAddress::toString() returns the same string
			// which was entered, it is a valid IP address.
			IPAddress ip(myEditor->getText());
			if (ip.toString() == myEditor->getText())
				ctrl->SetIpAddress(DCS_Overview, myEditor->getText());
			else
				myEditor->setText(ctrl->GetIpAddress(), false);
		}

		// OSC message rate has changed
		else if (myEditor == m_rateTextEdit.get())
		{
			ctrl->SetRate(DCS_Overview, myEditor->getText().getIntValue());
		}
	}
}

/**
 * Callback function for Enter key presses on textEditors.
 * @param textEditor	The TextEditor object whose where enter key was pressed.
 */
void COverviewComponent::textEditorReturnKeyPressed(TextEditor& textEditor)
{
	ignoreUnused(textEditor);

	// Remove keyboard focus from this editor.
	// Function textEditorFocusLost will then take care of setting values.
	getParentComponent()->grabKeyboardFocus();
}

/**
 * Timer callback function, which will be called at regular intervals to update the GUI.
 * Reimplemented from base class Timer.
 */
void COverviewComponent::timerCallback()
{
	UpdateGui(false);
}

/**
 * Update GUI elements with the current parameter values.
 * @param init	True to ignore any changed flags and update the OSC config parameters 
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void COverviewComponent::UpdateGui(bool init)
{
	CController* ctrl = CController::GetInstance();
	if (ctrl)
	{
		if (ctrl->PopParameterChanged(DCS_Overview, DCT_IPAddress) || init)
			m_ipAddressTextEdit->setText(ctrl->GetIpAddress(), false);

		if (ctrl->PopParameterChanged(DCS_Overview, DCT_MessageRate) || init)
			m_rateTextEdit->setText(String(ctrl->GetRate()), false);

		if (ctrl->PopParameterChanged(DCS_Overview, DCT_Online) || init)
			m_onlineLed->setToggleState(ctrl->GetOnline(), NotificationType::dontSendNotification);
	}

	// Save some performance: only update the component inside the currently active tab.
	if (m_tabbedComponent->getCurrentTabIndex() == CTabbedComponent::OTI_Table)
	{
		if (m_tableContainer)
			m_tableContainer->UpdateGui(init);

		// When the overview table is active, no need to refresh GUI very quickly
		if (getTimerInterval() == GUI_UPDATE_RATE_FAST)
		{
			//DBG("COverviewComponent::timerCallback(): Switching to GUI_UPDATE_RATE_SLOW");
			startTimer(GUI_UPDATE_RATE_SLOW);
		}
	}
	else if (m_tabbedComponent->getCurrentTabIndex() == CTabbedComponent::OTI_MultiSlider)
	{
		if (m_multiSliderContainer)
			m_multiSliderContainer->UpdateGui(init);

		// When multi-slider is active, we refresh the GUI faster
		if (getTimerInterval() == GUI_UPDATE_RATE_SLOW)
		{
			//DBG("COverviewComponent::timerCallback: Switching to GUI_UPDATE_RATE_FAST");
			startTimer(GUI_UPDATE_RATE_FAST);
		}
	}
}


/*
===============================================================================
 Class CTabbedComponent
===============================================================================
*/

/**
 * Class constructor.
 */
CTabbedComponent::CTabbedComponent()
	: TabbedComponent(TabbedButtonBar::TabsAtTop)
{
}

/**
 * Class destructor.
 */
CTabbedComponent::~CTabbedComponent()
{
}

/**
 * Reimplemented to create and return custom tab bar buttons.
 * @param tabName	Text on the tab button. Not used in this implementation.
 * @param tabIndex	Index of the tab from left to right, starting at 0.
 * @return	Pointer to a CTabBarButton.
 */
TabBarButton* CTabbedComponent::createTabButton(const String& tabName, int tabIndex)
{
	ignoreUnused(tabName);
	return new CTabBarButton(tabIndex, getTabbedButtonBar());
}

/**
 * Callback method to indicate the selected tab has been changed.
 * Reimplemented to trigger a GUI update on the newly active tab.
 * @param newCurrentTabIndex	Index of the tab from left to right, starting at 0.
 * @param newCurrentTabName		Name of the tab.
 * @return	Pointer to a CTabBarButton.
 */
void CTabbedComponent::currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName)
{
	ignoreUnused(newCurrentTabIndex);
	ignoreUnused(newCurrentTabName);

	COverviewComponent* parent = dynamic_cast<COverviewComponent*>(getParentComponent());
	if (parent)
		parent->UpdateGui(true);
}

/**
 * Reimplemented to re-postion the tabBar so make the tab buttons start further to the right.
 */
void CTabbedComponent::resized()
{
	int w = getLocalBounds().getWidth();
	getTabbedButtonBar().setBounds(Rectangle<int>(40, 0, w - (40 + 86), 44));
}


/*
===============================================================================
 Class CTabBarButton
===============================================================================
*/

/**
 * Class constructor.
 * @param tabIdx	Tab index starting at 0.
 * @param ownerBar	TabbedButtonBar object which contains this button.
 */
CTabBarButton::CTabBarButton(int tabIdx, TabbedButtonBar& ownerBar)
	: TabBarButton(String(), ownerBar),
	m_tabIndex(tabIdx)
{
	String imageName;
	switch (m_tabIndex)
	{
	case CTabbedComponent::OTI_Table:
		imageName = BinaryData::vertical_split24px_svg;
		break;
	case CTabbedComponent::OTI_MultiSlider:
		imageName = BinaryData::grain24px_svg;
		break;
	case CTabbedComponent::OTI_Settings:
		imageName = BinaryData::settings24px_svg;
		break;
	default:
		break;
	}

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
 * Class destructor.
 */
CTabBarButton::~CTabBarButton()
{
}

/**
 * Reimplemented paint function, to display an icon.
 * @param g					A graphics context, used for drawing a component or image.
 * @param isMouseOverButton	True if the mouse is over the button.
 * @param isButtonDown		True if the button is being pressed.
 */
void CTabBarButton::paintButton(Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
	// The original TabBarButton::paintButton draws a gradient on the buttons which
	// are inactive. We don't want that, just paint them with the background color.
	Colour buttonBackground(CDbStyle::GetDbColor(CDbStyle::MidColor));
	if (getToggleState())
		buttonBackground = CDbStyle::GetDbColor(CDbStyle::DarkColor);
	else if (isButtonDown)
		buttonBackground = buttonBackground.brighter(0.1f);
	else if (isMouseOverButton)
		buttonBackground = buttonBackground.brighter(0.05f);

	Rectangle<int> activeArea(getActiveArea());
	g.setColour(buttonBackground);
	g.fillRect(getActiveArea());

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
void CTabBarButton::resized()
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
bool CTabBarButton::setVisibleDrawable(Drawable* visibleDrawable)
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
