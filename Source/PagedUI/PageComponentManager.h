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
#include "../AppConfiguration.h"
#include "../LookAndFeel.h"


namespace SpaConBridge
{


/**
 * Forward declarations
 */
class MultiSoundobjectComponent;
class PageContainerComponent;


/**
 * Class PageComponentManager which takes care of opening and closing the overview window.
 */
class PageComponentManager : public AppConfiguration::XmlConfigurableElement
{
public:
	PageComponentManager();
	virtual ~PageComponentManager();
	static PageComponentManager* GetInstance();

	//==============================================================================
	void OpenPageContainer();
	PageContainerComponent* GetPageContainer();
	void ClosePageContainer(bool destroy);

	//==============================================================================
	UIPageId GetActivePage() const;
	void SetActivePage(UIPageId pageId, bool dontUpdateConfig);

	//==============================================================================
	const std::vector<UIPageId>& GetEnabledPages() const;
	void SetEnabledPages(const std::vector<UIPageId>& enabledPages, bool dontUpdateConfig);

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

	bool IsMatrixInputTableCollapsed();
	void SetMatrixInputTableCollapsed(bool collapsed);
	bool IsMatrixOutputTableCollapsed();
	void SetMatrixOutputTableCollapsed(bool collapsed);

	//==============================================================================
	std::vector<std::pair<std::pair<int, int>, std::string>> GetScenesPagePinnedScenes();
	void SetScenesPagePinnedScenes(const std::vector<std::pair<std::pair<int, int>, std::string>>& pinnedScenes);

	//==============================================================================
	std::unique_ptr<MultiSoundobjectComponent>& GetMultiSoundobjectComponent();

	//==============================================================================
	MappingAreaId GetMultiSoundobjectMappingArea();
	void SetMultiSoundobjectMappingArea(MappingAreaId mappingArea, bool dontSendNotification);
	bool IsMultiSoundobjectReverbEnabled();
	void SetMultiSoundobjectReverbEnabled(bool enabled, bool dontSendNotification);
	bool IsMultiSoundobjectSpreadEnabled();
	void SetMultiSoundobjectSpreadEnabled(bool enabled, bool dontSendNotification);
	bool IsMultiSoundobjectMuSelVisuEnabled();
	void SetMultiSoundobjectMuSelVisuEnabled(bool enabled, bool dontSendNotification);

	void LoadImageForMappingFromFile(MappingAreaId mappingArea, const File& file);
	void RemoveImageForMapping(MappingAreaId mappingArea);

	//==============================================================================
	DbLookAndFeelBase::LookAndFeelType GetLookAndFeelType() const;
	void SetLookAndFeelType(DbLookAndFeelBase::LookAndFeelType lookAndFeelType, bool dontUpdateConfig);

#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
	//==============================================================================
	bool IsFullscreenWindowMode() const;
	void SetFullscreenWindowMode(bool fullscreen, bool dontUpdateConfig);
#endif

	//==============================================================================
	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

protected:
	//==============================================================================
	static PageComponentManager	*m_singleton;			/**< The one and only instance of PageComponentManager. */
	PageContainerComponent		*m_pageContainer;		/**< Pointer to the page winodw. */

	UIPageId							m_activePage{ UPI_InvalidMin };								/**< Remember the last active page. */
	std::vector<UIPageId>				m_enabledPages;												/**< Remember the currently enabled pages. */

	DbLookAndFeelBase::LookAndFeelType	m_lookAndFeelType{ DbLookAndFeelBase::LAFT_InvalidFirst };	/**< Remember the currently selected look and feel type. */

	std::unique_ptr<MultiSoundobjectComponent>	m_multiSoundobjectComponent;
	std::map<MappingAreaId, Image>				m_multiSoundobjectBackgrounds;		/**< The background images to use for multisoundobjectcomp. */

#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
	bool								m_fullscreenWindowMode{ false };
#endif

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PageComponentManager)
};


} // namespace SpaConBridge
