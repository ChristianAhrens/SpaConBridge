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


#pragma once

#include "../About.h"
#include "../Gui.h"
#include "../SoundscapeBridgeAppCommon.h"


namespace SoundscapeBridgeApp
{


/**
 * Forward declarations
 */
class CTabbedComponent;
class OverviewTableContainer;
class COverviewMultiSurface;
class CSettingsContainer;


/**
 * class LedButton, a custom ToggleButton
 */
class LedButton : public ToggleButton
{
public:
	explicit LedButton()
		: ToggleButton()
	{

	}
	~LedButton() override
	{

	}

protected:
	void paintButton(Graphics& g, bool isMouseOverButton, bool isButtonDown) override
	{
		Rectangle<int> bounds = getLocalBounds();
		Rectangle<float> buttonRectF = Rectangle<float>(2.5f, 2.5f, bounds.getWidth() - 4.0f, bounds.getHeight() - 4.0f);
		bool on = getToggleState();
		bool enabled = isEnabled();

		// Button's main colour
		if (on)
		{
			Colour col = DbStyle::GetDbColor(DbStyle::ButtonBlueColor);
			if (isButtonDown)
				col = col.brighter(0.1f);
			else if (isMouseOverButton)
				col = col.brighter(0.05f);
			g.setColour(col);
		}
		else
		{
			Colour col = DbStyle::GetDbColor(DbStyle::ButtonColor);
			if (!enabled)
				col = getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker();
			else if (isButtonDown)
				col = DbStyle::GetDbColor(DbStyle::ButtonBlueColor).brighter(0.05f);
			else if (isMouseOverButton)
				col = getLookAndFeel().findColour(ResizableWindow::backgroundColourId).brighter(0.05f);
			g.setColour(col);
		}

		g.fillRoundedRectangle(buttonRectF, 10);
		//g.setColour(CDbStyle::GetDbColor(CDbStyle::WindowColor));
		g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
		g.drawRoundedRectangle(buttonRectF, 10, 1);
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LedButton)
};


/**
 * Class COverviewComponent is a simple container used to hold the GUI controls.
 */
class COverviewComponent : public Component,
	public TextEditor::Listener,
	private Timer
{
public:
	COverviewComponent();
	~COverviewComponent() override;
	void UpdateGui(bool init);

	void SetActiveTab(int tabIdx);

private:
	void paint(Graphics&) override;
	void resized() override;

	void textEditorFocusLost(TextEditor &) override;
	void textEditorReturnKeyPressed(TextEditor &) override;

	void timerCallback() override;

private:
	std::unique_ptr<Label>					m_versionLabel;			/**> App version label. */
	std::unique_ptr<Label>					m_nameLabel;			/**> App name. */
	std::unique_ptr<Label>					m_titleLabel;			/**> Overview title label. */
	Image									m_appLogo;				/**> Logo image. */
	std::unique_ptr<Label>					m_rateLabel;			/**> Send/receive rate label. */
	std::unique_ptr<TextEditor>			m_rateTextEdit;			/**> Text editor for the OSC send/receive rate in ms. */
	std::unique_ptr<Label>					m_onlineLabel;			/**> Online indicator label. */
	std::unique_ptr<LedButton>				m_onlineLed;			/**> Button used as Online indicator LED. */
	std::unique_ptr<CTabbedComponent>		m_tabbedComponent;		/**> A container for tabs. */
	std::unique_ptr<OverviewTableContainer> m_tableContainer;		/**> The actual table container inside this component. */
	std::unique_ptr<COverviewMultiSurface>	m_multiSliderContainer;	/**> Container for multi-slider. */
	std::unique_ptr<CSettingsContainer>		m_settingsContainer;	/**> Container for settings component. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(COverviewComponent)
};


/**
 * Reimplemented TabbedComponent which overrides the createTabButton method in order
 * to provide custom tabBar buttons (See CTabBarButton).
 */
class CTabbedComponent : public TabbedComponent
{
public:

	/**
	 * Overview tab indeces
	 */
	enum OverviewTabIndex
	{
		OTI_Table = 0,
		OTI_MultiSlider,
		OTI_Settings,
	};

	CTabbedComponent();
	~CTabbedComponent() override;

	bool GetIsHandlingChanges();
	void SetIsHandlingChanges(bool isHandlingChanges);

protected:
	TabBarButton* createTabButton(const String& tabName, int tabIndex) override;
	void currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName) override;
	void resized() override;

	bool m_isHandlingChanges{ true };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CTabbedComponent)
};


/**
 * Reimplemented TabBarButton which overrides the paintButton method
 * to show an icon instead of the standard tab name text.
 */
class CTabBarButton : public TabBarButton
{
public:
	CTabBarButton(int tabIdx, TabbedButtonBar& ownerBar);
	~CTabBarButton() override;

protected:
	void paintButton(Graphics&, bool, bool) override;
	void resized() override;

private:
	bool setVisibleDrawable(Drawable* visibleDrawable);

	int m_tabIndex;
	std::unique_ptr<juce::Drawable> m_normalImage, m_overImage, m_downImage, m_disabledImage, m_normalOnImage, m_overOnImage, m_downOnImage, m_disabledOnImage;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CTabBarButton)
};


} // namespace SoundscapeBridgeApp
