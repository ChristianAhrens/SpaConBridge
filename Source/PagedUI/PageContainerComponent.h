/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file was originally part of the Soundscape VST, AU, and AAX Plug-in
and now in a derived version is part of SoundscapeBridgeApp.

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


#pragma once

#include "../SoundscapeBridgeAppCommon.h"
#include "../LookAndFeel.h"


namespace SoundscapeBridgeApp
{


/**
 * Forward declarations
 */
class CustomButtonTabbedComponent;
class SoundobjectTablePageComponent;
class MultiSurfacePageComponent;
class MatrixIOPageComponent;
class SettingsPageComponent;
class StatisticsPageComponent;
class AboutPageComponent;


/**
 * class LedButton, a custom ToggleButton
 */
class LedComponent : public Component
{
public:
	explicit LedComponent()
		: Component()
	{

	}
	~LedComponent() override
	{

	}

	void SetOn(bool on)
	{
		m_on = on;

		repaint();
	}

	void SetHighlightOn(bool on)
	{
		m_highlightOn = on;

		repaint();
	}

	void paint(Graphics& g) override
	{
		auto bounds = getLocalBounds();
		auto ledBounds = bounds.reduced(2).toFloat();

		auto mainColour = Colours::blue;
		auto highlightColour = Colours::blue.brighter();
		auto customlookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
		if (customlookAndFeel)
		{
			mainColour = customlookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::HighlightColor);
			highlightColour = customlookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::HighlightColor).brighter(0.6f);
		}

		// main led circle
		if (m_highlightOn)
		{
			g.setColour(highlightColour);
		}
		else if (m_on)
		{
			g.setColour(mainColour);
		}
		else
		{
			Colour col = getLookAndFeel().findColour(TextButton::buttonColourId);
			if (!isEnabled())
				col = getLookAndFeel().findColour(TextEditor::backgroundColourId);
			g.setColour(col);
		}
		g.fillRoundedRectangle(ledBounds, 10);

		// led border
		g.setColour(getLookAndFeel().findColour(TextEditor::outlineColourId));
		g.drawRoundedRectangle(ledBounds, 10, 1);
	}

private:
	bool	m_on{ false };
	bool	m_highlightOn{ false };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LedComponent)
};


/**
 * Class PageContainerComponent is a simple container used to hold the GUI controls.
 */
class PageContainerComponent :	public Component,
								public Button::Listener,
								private Timer
{
public:
	PageContainerComponent();
	~PageContainerComponent() override;

	void UpdateGui(bool init);

	//==============================================================================
	void lookAndFeelChanged() override;

	//==============================================================================
	void SetActiveTab(int tabIdx);

	//==============================================================================
	void SetLookAndFeelType(DbLookAndFeelBase::LookAndFeelType lookAndFeelType);
	DbLookAndFeelBase::LookAndFeelType GetLookAndFeelType();

	//==============================================================================
	int GetSoundobjectTableRowHeight();
	void SetSoundobjectTableRowHeight(int height);
	int GetMatrixInputTableRowHeight();
	void SetMatrixInputTableRowHeight(int height);
	int GetMatrixOutputTableRowHeight();
	void SetMatrixOutputTableRowHeight(int height);

private:
	//==============================================================================
	void toggleAboutPage();

	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

	//==============================================================================
	void buttonClicked(Button*) override;

	//==============================================================================
	void timerCallback() override;

private:
	//==============================================================================
	std::unique_ptr<Label>							m_versionLabel;			/**> App version label. */
	std::unique_ptr<Label>							m_versionStringLabel;	/**> "Version" string. */
	std::unique_ptr<ImageButton>					m_logoButton;			/**> App logo button triggering about page. */
	std::unique_ptr<DrawableButton>					m_helpButton;			/**< Button to open the github readme url. */
	std::unique_ptr<Label>							m_onlineLabel;			/**> Online indicator label. */
	std::unique_ptr<LedComponent>					m_onlineLed1st;			/**> Button used as Online indicator LED for first DS100. */
	std::unique_ptr<LedComponent>					m_onlineLed2nd;			/**> Button used as Online indicator LED for second DS100. */
	std::unique_ptr<CustomButtonTabbedComponent>	m_tabbedComponent;		/**> A container for tabs. */
	std::unique_ptr<SoundobjectTablePageComponent>	m_soundobjectsPage;		/**> The actual table container inside this component. */
	std::unique_ptr<MultiSurfacePageComponent>		m_multiSliderPage;		/**> Container for multi-slider. */
    std::unique_ptr<MatrixIOPageComponent>          m_matrixIOPage;         /**> Container for matrix inputs/outputs. */
	std::unique_ptr<SettingsPageComponent>			m_settingsPage;			/**> Container for settings component. */
	std::unique_ptr<StatisticsPageComponent>		m_statisticsPage;		/**> Container for statistics component. */
	std::unique_ptr<AboutPageComponent>				m_aboutPage;			/**> Container for about component. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PageContainerComponent)
};


/**
 * Reimplemented TabbedComponent which overrides the createTabButton method in order
 * to provide custom tabBar buttons (See CustomDrawableTabBarButton).
 */
class CustomButtonTabbedComponent : public TabbedComponent
{
public:

	/**
	 * Overview tab indices
	 */
	enum OverviewTabIndex
	{
		OTI_Table = 0,
		OTI_MultiSlider,
        OTI_MatrixIOs,
		OTI_Statistics,
		OTI_Settings,
	};

	CustomButtonTabbedComponent();
	~CustomButtonTabbedComponent() override;

	bool GetIsHandlingChanges();
	void SetIsHandlingChanges(bool isHandlingChanges);

protected:
	TabBarButton* createTabButton(const String& tabName, int tabIndex) override;
	void currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName) override;
	void resized() override;

	bool m_isHandlingChanges{ true };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomButtonTabbedComponent)
};


/**
 * Reimplemented TabBarButton which overrides the paintButton method
 * to show an icon instead of the standard tab name text.
 */
class CustomDrawableTabBarButton : public TabBarButton
{
public:
	CustomDrawableTabBarButton(int tabIdx, TabbedButtonBar& ownerBar);
	~CustomDrawableTabBarButton() override;

	void updateDrawableButtonImageColours();

	void lookAndFeelChanged() override;

protected:
	void paintButton(Graphics&, bool, bool) override;
	void resized() override;

private:
	bool setVisibleDrawable(Drawable* visibleDrawable);

	int m_tabIndex;
	std::unique_ptr<juce::Drawable> m_normalImage, m_overImage, m_downImage, m_disabledImage, m_normalOnImage, m_overOnImage, m_downOnImage, m_disabledOnImage;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomDrawableTabBarButton)
};


} // namespace SoundscapeBridgeApp
