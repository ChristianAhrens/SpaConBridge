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


#pragma once

#include "../SpaConBridgeCommon.h"
#include "../LookAndFeel.h"


namespace SpaConBridge
{


/**
 * Forward declarations
 */
class CustomButtonTabbedComponent;
class SoundobjectTablePageComponent;
class MultiSoundobjectPageComponent;
class MatrixIOPageComponent;
class SettingsPageComponent;
class StatisticsPageComponent;
class AboutPageComponent;
class ScenesPageComponent;
class EnSpacePageComponent;
class PageComponentBase;


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

	void SetPagesBeingInitialized(bool initializing);

	//==============================================================================
	void lookAndFeelChanged() override;

	//==============================================================================
	void SetActivePage(UIPageId pageId);

	//==============================================================================
	void OpenPageAsWindow(UIPageId pageId);
	void OpenPageAsTab(UIPageId pageId);

	//==============================================================================
	void SetEnabledPages(const std::vector<UIPageId>& enabledPages);

	//==============================================================================
	float GetSoundobjectTableResizeBarRatio();
	void SetSoundobjectTableResizeBarRatio(float ratio);

	//==============================================================================
	bool GetSoundobjectTableSingleSelectionOnly();
	void SetSoundobjectTableSingleSelectionOnly(bool singleSelectionOnly);

	//==============================================================================
	int GetSoundobjectTableRowHeight();
	void SetSoundobjectTableRowHeight(int height);
	int GetMatrixInputTableRowHeight();
	void SetMatrixInputTableRowHeight(int height);
	int GetMatrixOutputTableRowHeight();
	void SetMatrixOutputTableRowHeight(int height);

	//==============================================================================
	bool GetMatrixInputTableCollapsed();
	void SetMatrixInputTableCollapsed(bool collapsed);
	bool GetMatrixOutputTableCollapsed();
	void SetMatrixOutputTableCollapsed(bool collapsed);

	//==============================================================================
	std::vector<std::pair<std::pair<int, int>, std::string>> GetScenesPagePinnedScenes();
	void SetScenesPagePinnedScenes(const std::vector<std::pair<std::pair<int, int>, std::string>>& pinnedScenes);

	//==============================================================================
	void SetOverlayComponent(Component* componentToOverlay);
	void ClearOverlayComponent();

private:
	//==============================================================================
	void toggleAboutPage();

	//==============================================================================
	PageComponentBase* GetComponentForPageId(const UIPageId& pageId);

	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

	//==============================================================================
	void buttonClicked(Button*) override;

	//==============================================================================
	void timerCallback() override;

private:
	//==============================================================================
	std::unique_ptr<Label>							m_versionLabel;				/**< App version label. */
	std::unique_ptr<Label>							m_versionStringLabel;		/**< "Version" string. */
	std::unique_ptr<ImageButton>					m_logoButton;				/**< App logo button triggering about page. */
	std::unique_ptr<DrawableButton>					m_helpButton;				/**< Button to open the github readme url. */
	std::unique_ptr<TextButton>						m_onlineButton;				/**< Online/Offline toggle button. */
	std::unique_ptr<LedComponent>					m_connectedLed1st;			/**< Button used as Connected indicator LED for first DS100. */
	std::unique_ptr<LedComponent>					m_connectedLed2nd;			/**< Button used as Connected indicator LED for second DS100. */
	std::unique_ptr<CustomButtonTabbedComponent>	m_tabbedComponent;			/**< A container for tabs. */
	std::unique_ptr<SoundobjectTablePageComponent>	m_soundobjectsPage;			/**< The actual table container inside this component. */
	std::unique_ptr<MultiSoundobjectPageComponent>	m_multiSoundobjectsPage;	/**< Container for multi-slider. */
    std::unique_ptr<MatrixIOPageComponent>          m_matrixIOPage;				/**< Container for matrix inputs/outputs. */
	std::unique_ptr<SettingsPageComponent>			m_settingsPage;				/**< Container for settings component. */
	std::unique_ptr<StatisticsPageComponent>		m_statisticsPage;			/**< Container for statistics component. */
	std::unique_ptr<AboutPageComponent>				m_aboutPage;				/**< Container for about component. */
	std::unique_ptr<ScenesPageComponent>			m_scenesPage;				/**< Container for scenes component. */
	std::unique_ptr<EnSpacePageComponent>			m_enSpacePage;				/**< Container for EnSpace component. */

	Component* m_overlayComponent{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PageContainerComponent)
};


/**
 * Reimplemented TabbedComponent which overrides the createTabButton method in order
 * to provide custom tabBar buttons (See CustomDrawableTabBarButton).
 */
class CustomButtonTabbedComponent : public TabbedComponent
{
public:
	CustomButtonTabbedComponent();
	~CustomButtonTabbedComponent() override;

	bool GetIsHandlingChanges();
	void SetIsHandlingChanges(bool isHandlingChanges);

protected:
	//==============================================================================
	TabBarButton* createTabButton(const String& tabName, int tabIndex) override;
	void currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName) override;

	//==============================================================================
	void resized() override;

private:
	//==============================================================================
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
	CustomDrawableTabBarButton(UIPageId pageId, TabbedButtonBar& ownerBar);
	~CustomDrawableTabBarButton() override;

	void updateDrawableButtonImageColours();

	//==============================================================================
	void lookAndFeelChanged() override;

	//==============================================================================
	std::function<void(UIPageId)> onButtonDraggedForTabDetaching;

protected:
	//==============================================================================
	void paintButton(Graphics&, bool, bool) override;

	//==============================================================================
	void resized() override;

	//==============================================================================
	void mouseUp(const MouseEvent& event) override;

private:
	bool setVisibleDrawable(Drawable* visibleDrawable);

	UIPageId m_pageId;
	std::unique_ptr<juce::Drawable> m_normalImage, m_overImage, m_downImage, m_disabledImage, m_normalOnImage, m_overOnImage, m_downOnImage, m_disabledOnImage;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomDrawableTabBarButton)
};


} // namespace SpaConBridge
