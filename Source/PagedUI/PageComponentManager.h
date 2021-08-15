/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file was originally part of the Soundscape VST, AU, and AAX Plug-in and now in a derived version is part of SpaConBridge.

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

#include "../SpaConBridgeCommon.h"
#include "../AppConfiguration.h"
#include "../LookAndFeel.h"


namespace SpaConBridge
{


/**
 * Forward declarations
 */
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
	MappingAreaId GetMultiSliderMappingArea();
	void SetMultiSliderMappingArea(MappingAreaId mappingArea, bool dontSendNotification);
	bool IsMultiSliderReverbEnabled();
	void SetMultiSliderReverbEnabled(bool enabled, bool dontSendNotification);
	bool IsMultiSliderSpreadEnabled();
	void SetMultiSliderSpreadEnabled(bool enabled, bool dontSendNotification);

	//==============================================================================
	DbLookAndFeelBase::LookAndFeelType GetLookAndFeelType() const;
	void SetLookAndFeelType(DbLookAndFeelBase::LookAndFeelType lookAndFeelType, bool dontUpdateConfig);

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

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PageComponentManager)
};


} // namespace SpaConBridge
