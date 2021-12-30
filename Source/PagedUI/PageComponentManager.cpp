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


#include "PageComponentManager.h"

#include "PageContainerComponent.h"

#include "../CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"
#include "../Controller.h"
#include "../SoundobjectSlider.h"

#include <Image_utils.h>


namespace SpaConBridge
{


/*
===============================================================================
 Class PageComponentManager
===============================================================================
*/

/**
 * The one and only instance of PageComponentManager.
 */
PageComponentManager* PageComponentManager::m_singleton = nullptr;

/**
 * Class constructor.
 */
PageComponentManager::PageComponentManager()
{
	jassert(!m_singleton);	// only one instnce allowed!!
	m_singleton = this;

	// Default overview window properties.
	m_pageContainer = nullptr;

	// Initialization of members to a usable default makes sense 
	// when no config xml is available on app start and therefor 
	// init values of this instance are going to be used for default config.
	m_activePage = UPI_SoundObjects;
	for (int id = UPI_InvalidMin + 1; id < UPI_InvalidMax; id++) m_enabledPages.push_back(static_cast<UIPageId>(id));
	m_lookAndFeelType = DbLookAndFeelBase::LAFT_Dark;
}

/**
 * Destroys the PageComponentManager.
 */
PageComponentManager::~PageComponentManager()
{
	jassert(m_pageContainer == nullptr);

	m_singleton = nullptr;
}

/**
 * Returns the one and only instance of PageComponentManager.
 * @return A PageComponentManager object or 0.
 * @sa m_singleton, PageComponentManager
 */
PageComponentManager* PageComponentManager::GetInstance()
{
	if (m_singleton == nullptr)
	{
		m_singleton = new PageComponentManager();
	}
	return m_singleton;
}

/**
 * Method to trigger creation of page container component if not existing already.
 */
void PageComponentManager::OpenPageContainer()
{
	// Overview window is not currently open -> create it.
	if (m_pageContainer == nullptr)
	{
		m_pageContainer = new PageContainerComponent();
	}

	// Overview window already exists -> bring it to the front.
	else
	{
		m_pageContainer->toFront(true);
	}
}

/**
 * Getter for the PageContainer component.
 * This is required to be able to embed the overview in a main component,
 * were the original d&b Soundscape Plugin displayed the overview as a window of its own.
 * @return The overview component.
 */
PageContainerComponent* PageComponentManager::GetPageContainer()
{
	if (m_pageContainer == nullptr)
		OpenPageContainer();

	return m_pageContainer;
}

/**
 * Function called by PageContainerComponents's destructor to set the local pointer to zero.
 * @param destroy	True to also destroy the PageComponentManager itself.
 */
void PageComponentManager::ClosePageContainer(bool destroy)
{
	if (m_pageContainer != nullptr)
	{
		// Close the overview window.
		delete m_pageContainer;
		m_pageContainer = nullptr;
	}

	// Closed overview, so manager no longer needed.
	if (destroy)
		delete this;
}

/**
 * Getter for the currently active page of the main window.
 * @return The currently active page.
 */
UIPageId PageComponentManager::GetActivePage() const
{
	return m_activePage;
}

/**
 * Setter for the currently active tab of the main window.
 * @param pageIdx	The currently active page id.
 * @param dontUpdateConfig	Indication if the configuration update shall be triggerd as well
 */
void PageComponentManager::SetActivePage(UIPageId pageId, bool dontUpdateConfig)
{
	m_activePage = pageId;

	if (m_pageContainer != nullptr)
	{
		m_pageContainer->SetActivePage(pageId);
	}

	if (!dontUpdateConfig)
	{
		triggerConfigurationUpdate(false);
	}
}

/**
 * Getter for the currently enabled pages of the main window.
 * @return The currently enabled pages.
 */
const std::vector<UIPageId>& PageComponentManager::GetEnabledPages() const
{
	return m_enabledPages;
}

/**
 * Setter for the currently enabled pages of the main window.
 * @param enabledPages	The pages to set as currently currently enabled.
 * @param dontUpdateConfig	Indication if the configuration update shall be triggerd as well
 */
void PageComponentManager::SetEnabledPages(const std::vector<UIPageId>& enabledPages, bool dontUpdateConfig)
{
	m_enabledPages = enabledPages;

	if (m_pageContainer != nullptr)
	{
		m_pageContainer->SetEnabledPages(enabledPages);
	}

	if (!dontUpdateConfig)
	{
		triggerConfigurationUpdate(false);
	}
}

/**
 * Proxy Getter for the row height in sound objects table.
 * Forwards call to PageContainerComponent.
 * @return	The table row height.
 */
int PageComponentManager::GetSoundobjectTableRowHeight()
{
	if (m_pageContainer)
		return m_pageContainer->GetSoundobjectTableRowHeight();
	else
	{
		jassertfalse;
		return 0;
	}
}

/**
 * Proxy Setter for the row height in sound objects table.
 * Forwards call to PageContainerComponent.
 * @param height	The table row height.
 */
void PageComponentManager::SetSoundobjectTableRowHeight(int height)
{
	if (m_pageContainer)
		m_pageContainer->SetSoundobjectTableRowHeight(height);
}

/**
 * Getter for the row height in matrix inputs table.
 * Forwards call to PageContainerComponent.
 * @return	The table row height.
 */
int PageComponentManager::GetMatrixInputTableRowHeight()
{
	if (m_pageContainer)
		return m_pageContainer->GetMatrixInputTableRowHeight();
	else
	{
		jassertfalse;
		return 0;
	}
}

/**
 * Proxy Setter for the row height in matrix inputs table.
 * Forwards call to PageContainerComponent.
 * @param height	The table row height.
 */
void PageComponentManager::SetMatrixInputTableRowHeight(int height)
{
	if (m_pageContainer)
		m_pageContainer->SetMatrixInputTableRowHeight(height);
}

/**
 * Proxy Getter for the row height in matrix outputs table.
 * Forwards call to PageContainerComponent.
 * @return	The table row height.
 */
int PageComponentManager::GetMatrixOutputTableRowHeight()
{
	if (m_pageContainer)
		return m_pageContainer->GetMatrixOutputTableRowHeight();
	else
	{
		jassertfalse;
		return 0;
	}
}

/**
 * Proxy Setter for the row height in matrix outputs table.
 * Forwards call to PageContainerComponent.
 * @param height	The table row height.
 */
void PageComponentManager::SetMatrixOutputTableRowHeight(int height)
{
	if (m_pageContainer)
		m_pageContainer->SetMatrixOutputTableRowHeight(height);
}

/**
 * Proxy Getter for the row height in matrix outputs table.
 * Forwards call to PageContainerComponent.
 * @return	The table row height.
 */
bool PageComponentManager::IsMatrixInputTableCollapsed()
{
	if (m_pageContainer)
		return m_pageContainer->GetMatrixInputTableCollapsed();
	else
	{
		jassertfalse;
		return false;
	}
}

/**
 * Proxy Setter for the collapsed state in matrix inputs table.
 * Forwards call to PageContainerComponent.
 * @param collapsed	The table collapsed state.
 */
void PageComponentManager::SetMatrixInputTableCollapsed(bool collapsed)
{
	if (m_pageContainer)
		m_pageContainer->SetMatrixInputTableCollapsed(collapsed);
}

/**
 * Proxy Getter for the row height in matrix outputs table.
 * Forwards call to PageContainerComponent.
 * @return	The table row height.
 */
bool PageComponentManager::IsMatrixOutputTableCollapsed()
{
	if (m_pageContainer)
		return m_pageContainer->GetMatrixOutputTableCollapsed();
	else
	{
		jassertfalse;
		return false;
	}
}

/**
 * Proxy Setter for the collapsed state in matrix outputs table.
 * Forwards call to PageContainerComponent.
 * @param collapsed	The table collapsed state.
 */
void PageComponentManager::SetMatrixOutputTableCollapsed(bool collapsed)
{
	if (m_pageContainer)
		m_pageContainer->SetMatrixOutputTableCollapsed(collapsed);
}

/**
 * Proxy Getter for the pinned scenes in Scenes Page.
 * Forwards the call to PageContainerComponent.
 * @return	The pinned scenes of Scenes Page.
 */
std::vector<std::pair<std::pair<int, int>, std::string>> PageComponentManager::GetScenesPagePinnedScenes()
{
	if (m_pageContainer)
		return m_pageContainer->GetScenesPagePinnedScenes();
	else
	{
		jassertfalse;
		return std::vector<std::pair<std::pair<int, int>, std::string>>();
	}
}

/**
 * Proxy Setter for the pinned scenes in Scenes Page.
 * Forwards the call to PageContainerComponent.
 * @param pinnedScenes	The pinned scenes of Scenes Page.
 */
void PageComponentManager::SetScenesPagePinnedScenes(const std::vector<std::pair<std::pair<int, int>, std::string>>& pinnedScenes)
{
	if (m_pageContainer)
		m_pageContainer->SetScenesPagePinnedScenes(pinnedScenes);
}

/**
 * Proxy Getter for the selected mapping area in MultiSlider Page.
 * Forwards the call to PageContainerComponent.
 * @return	The selected mapping area in MultiSlider Page.
 */
MappingAreaId PageComponentManager::GetMultiSliderMappingArea()
{
	if (m_pageContainer)
		return m_pageContainer->GetMultiSliderPageMappingArea();
	else
	{
		jassertfalse;
		return MappingAreaId::MAI_Invalid;
	}
}

/**
 * Proxy Setter for the selected mapping area in MultiSlider Page.
 * Forwards the call to PageContainerComponent.
 * @param mappingArea	The selected mapping area in MultiSlider Page.
 * @param dontSendNotification	Indication if the configuration update shall be triggerd as well
 */
void PageComponentManager::SetMultiSliderMappingArea(MappingAreaId mappingArea, bool dontSendNotification)
{
	if (m_pageContainer)
		m_pageContainer->SetMultiSliderPageMappingArea(mappingArea);

	if (!dontSendNotification)
	{
		triggerConfigurationUpdate(false);
	}
}

/**
 * Proxy Getter for the reverb enabled state in MultiSlider Page.
 * Forwards the call to PageContainerComponent.
 * @return	The reverb enabled state in MultiSlider Page.
 */
bool PageComponentManager::IsMultiSliderReverbEnabled()
{
	if (m_pageContainer)
		return m_pageContainer->IsMultiSliderPageReverbEnabled();
	else
	{
		jassertfalse;
		return false;
	}
}

/**
 * Proxy Setter for the reverb enabled state in MultiSlider Page.
 * Forwards the call to PageContainerComponent.
 * @param enabled	The reverb enabled state in MultiSlider Page.
 * @param dontSendNotification	Indication if the configuration update shall be triggerd as well
 */
void PageComponentManager::SetMultiSliderReverbEnabled(bool enabled, bool dontSendNotification)
{
	if (m_pageContainer)
		m_pageContainer->SetMultiSliderPageReverbEnabled(enabled);

	if (!dontSendNotification)
	{
		triggerConfigurationUpdate(false);
	}
}

/**
 * Proxy Getter for the spread enabled state in MultiSlider Page.
 * Forwards the call to PageContainerComponent.
 * @return	The spread enabled state in MultiSlider Page.
 */
bool PageComponentManager::IsMultiSliderSpreadEnabled()
{
	if (m_pageContainer)
		return m_pageContainer->IsMultiSliderPageSpreadEnabled();
	else
	{
		jassertfalse;
		return false;
	}
}

/**
 * Proxy Setter for the spread enabled state in MultiSlider Page.
 * Forwards the call to PageContainerComponent.
 * @param enabled	The spread enabled state in MultiSlider Page.
 * @param dontSendNotification	Indication if the configuration update shall be triggerd as well
 */
void PageComponentManager::SetMultiSliderSpreadEnabled(bool enabled, bool dontSendNotification)
{
	if (m_pageContainer)
		m_pageContainer->SetMultiSliderPageSpreadEnabled(enabled);

	if (!dontSendNotification)
	{
		triggerConfigurationUpdate(false);
	}
}

/**
 * Loads an image from a given file to image cache, inserts it to internal hash of backgrounds and sets the image to multi soundobject page
 * @param	mappingArea	The mapping area the image shall be loaded for
 * @param	file		The file resource to load the image from
 */
void PageComponentManager::LoadImageForMappingFromFile(MappingAreaId mappingArea, const File& file)
{
    if (!file.existsAsFile())
    {
		ShowUserErrorNotification(SEC_LoadImage_CannotAccess);
        return;
    }

	auto inputStream = file.createInputStream(); // test inputstream to verify read access
	if (!inputStream)
	{
		ShowUserErrorNotification(SEC_LoadImage_CannotRead);
		return;
	}
	inputStream.reset(); // clean up the test inputstream
    
    auto image = juce::ImageCache::getFromFile(file);
    if (!image.isValid())
    {
		ShowUserErrorNotification(SEC_LoadImage_InvalidImage);
        return;
    }
    
	if (m_multiSliderBackgrounds.count(mappingArea) != 0)
		m_multiSliderBackgrounds.erase(mappingArea);
	m_multiSliderBackgrounds.insert(std::make_pair(mappingArea, image));

	if (m_pageContainer)
		m_pageContainer->SetMultiSliderPageBackgroundImage(mappingArea, m_multiSliderBackgrounds.at(mappingArea));

	triggerConfigurationUpdate(false);
}

/**
 * Removes a background image from internal hash and multi soundobject page
 * @param	mappingArea	The mapping area the image shall be erased of
 */
void PageComponentManager::RemoveImageForMapping(MappingAreaId mappingArea)
{
	m_multiSliderBackgrounds.erase(mappingArea);

	if (m_pageContainer)
		m_pageContainer->RemoveMultiSliderPageBackgroundImage(mappingArea);

	triggerConfigurationUpdate(false);
}

/**
 * Getter for the look and feel enum type member value.
 * @return The look and feel enum type member value.
 */
DbLookAndFeelBase::LookAndFeelType PageComponentManager::GetLookAndFeelType() const
{
	return m_lookAndFeelType;
}

/**
 * Setter for the look and feel enum type member.
 * @param lookAndFeelType	The look and feel type to set
 * @param dontUpdateConfig	Indication if the configuration update shall be triggerd as well
 * @return The selected mapping area.
 */
void PageComponentManager::SetLookAndFeelType(DbLookAndFeelBase::LookAndFeelType lookAndFeelType, bool dontUpdateConfig)
{
	m_lookAndFeelType = lookAndFeelType;

	if (!dontUpdateConfig)
	{
		triggerConfigurationUpdate(true); // we do want to include watcher update, since only that way the whole application is set to new LAFT (onConfigUpdated in main component)
	}
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to set this objects' settings
 * from a XML element structure that passed as argument.
 * @param stateXml	The XML element containing this objects' configuration data
 * @return	True if the data was read and handled successfuly, false if not.
 */
bool PageComponentManager::setStateXml(XmlElement* stateXml)
{
	// sanity check, if the incoming xml does make sense for this method
	if (!stateXml || (stateXml->getTagName() != AppConfiguration::getTagName(AppConfiguration::TagID::UICONFIG)))
		return false;

	// To prevent that we end up in a recursive ::setStateXml situation, verify that this setStateXml method is not called by itself
	const ScopedXmlChangeLock lock(IsXmlChangeLocked());
	if (!lock.isLocked())
		return false;

	if (!m_pageContainer)
		return false;

	auto retVal = true;

	m_pageContainer->SetPagesBeingInitialized(true);

	// Handle the look and feel type from xml first, since this is set as active dropdown index in overview. If we do not do this first, 
	// the default selected index will we written to config due to update trigger from SetActiveTab
	auto lookAndFeelXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::LOOKANDFEELTYPE));
	if (lookAndFeelXmlElement)
	{
		auto lookAndFeelTextElement = lookAndFeelXmlElement->getFirstChildElement();
		if (lookAndFeelTextElement && lookAndFeelTextElement->isTextElement())
		{
			auto lookAndFeelType = static_cast<DbLookAndFeelBase::LookAndFeelType>(lookAndFeelTextElement->getText().getIntValue());

			if (lookAndFeelType > DbLookAndFeelBase::LAFT_InvalidFirst && lookAndFeelType < DbLookAndFeelBase::LAFT_InvalidLast)
			{
				SetLookAndFeelType(lookAndFeelType, true);
			}
			else
				retVal = false;
		}
	}

	auto activeTabXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::ACTIVETAB));
	if (activeTabXmlElement)
	{
		auto activeTabTextElement = activeTabXmlElement->getFirstChildElement();
		if (activeTabTextElement && activeTabTextElement->isTextElement())
		{
			auto pageId = GetPageIdFromName(activeTabTextElement->getText());

			if (pageId > UPI_InvalidMin && pageId < UPI_InvalidMax)
			{
				SetActivePage(pageId, true);
			}
			else
				retVal = false;
		}
	}

	auto enabledPagesXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::ENABLEDPAGES));
	if (enabledPagesXmlElement)
	{
		auto enabledPages = std::vector<UIPageId>();

		auto enabledPagesTextElement = enabledPagesXmlElement->getFirstChildElement();
		if (enabledPagesTextElement && enabledPagesTextElement->isTextElement())
		{
			auto enabledPagesIdStrings = StringArray();
			enabledPagesIdStrings.addTokens(enabledPagesTextElement->getText(), ", ");
			for (auto const& pageIdString : enabledPagesIdStrings)
				enabledPages.push_back(static_cast<UIPageId>(pageIdString.getIntValue()));
		}

		SetEnabledPages(enabledPages, true);
	}

	auto soundobjectTableXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::SOUNDOBJECTTABLE));
	if (soundobjectTableXmlElement)
	{
		auto rowHeightXmlElement = soundobjectTableXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::ROWHEIGHT));
		if (rowHeightXmlElement)
		{
			auto rowHeightTextXmlElement = rowHeightXmlElement->getFirstChildElement();
			if (rowHeightTextXmlElement && rowHeightTextXmlElement->isTextElement())
			{
				auto rowHeight = rowHeightTextXmlElement->getText().getIntValue();

				if (rowHeight > 0)
				{
					SetSoundobjectTableRowHeight(rowHeight);
				}
				else
					retVal = false;
			}
		}
	}

	auto matrixInputTableXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::MATRIXINPUTTABLE));
	if (matrixInputTableXmlElement)
	{
		auto rowHeightXmlElement = matrixInputTableXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::ROWHEIGHT));
		if (rowHeightXmlElement)
		{
			auto rowHeightTextXmlElement = rowHeightXmlElement->getFirstChildElement();
			if (rowHeightTextXmlElement && rowHeightTextXmlElement->isTextElement())
			{
				auto rowHeight = rowHeightTextXmlElement->getText().getIntValue();

				if (rowHeight > 0)
				{
					SetMatrixInputTableRowHeight(rowHeight);
				}
				else
					retVal = false;
			}
		}

		auto tableCollapsedXmlElement = matrixInputTableXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::COLLAPSED));
		if (tableCollapsedXmlElement)
		{
			auto tableCollapsedTextXmlElement = tableCollapsedXmlElement->getFirstChildElement();
			if (tableCollapsedTextXmlElement && tableCollapsedTextXmlElement->isTextElement())
			{
				auto collapsed = (tableCollapsedTextXmlElement->getText().getIntValue() == 1);;

				SetMatrixInputTableCollapsed(collapsed);
			}
		}
	}

	auto matrixOutputTableXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::MATRIXOUTPUTTABLE));
	if (matrixOutputTableXmlElement)
	{
		auto rowHeightXmlElement = matrixOutputTableXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::ROWHEIGHT));
		if (rowHeightXmlElement)
		{
			auto rowHeightTextXmlElement = rowHeightXmlElement->getFirstChildElement();
			if (rowHeightTextXmlElement && rowHeightTextXmlElement->isTextElement())
			{
				auto rowHeight = rowHeightTextXmlElement->getText().getIntValue();

				if (rowHeight > 0)
				{
					SetMatrixOutputTableRowHeight(rowHeight);
				}
				else
					retVal = false;
			}
		}

		auto tableCollapsedXmlElement = matrixOutputTableXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::COLLAPSED));
		if (tableCollapsedXmlElement)
		{
			auto tableCollapsedTextXmlElement = tableCollapsedXmlElement->getFirstChildElement();
			if (tableCollapsedTextXmlElement && tableCollapsedTextXmlElement->isTextElement())
			{
				auto collapsed = (tableCollapsedTextXmlElement->getText().getIntValue() == 1);

				SetMatrixOutputTableCollapsed(collapsed);
			}
		}
	}

	auto scenesPageXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::SCENESPAGE));
	if (scenesPageXmlElement)
	{
		auto pinnedScenesXmlElement = scenesPageXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::PINNEDSCENES));
		if (pinnedScenesXmlElement)
		{
			auto pinnedScenes = std::vector<std::pair<std::pair<int, int>, std::string>>();
			for (auto sceneXmlElement : pinnedScenesXmlElement->getChildIterator())
			{
				if (sceneXmlElement && sceneXmlElement->getTagName() == AppConfiguration::getTagName(AppConfiguration::TagID::SCENE))
				{
					auto sceneIndexMajor = sceneXmlElement->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::INDEXMAJOR));
					auto sceneIndexMinor = sceneXmlElement->getIntAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::INDEXMINOR));
					auto sceneName = String();
					auto sceneNameTextXmlElement = sceneXmlElement->getFirstChildElement();
					if (sceneNameTextXmlElement && sceneNameTextXmlElement->isTextElement())
						sceneName = sceneNameTextXmlElement->getText();
					pinnedScenes.push_back(std::make_pair(std::make_pair(sceneIndexMajor, sceneIndexMinor), sceneName.toStdString()));
				}
			}

			SetScenesPagePinnedScenes(pinnedScenes);
		}
	}

	auto multisliderPageXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::MULTISLIDER));
	if (multisliderPageXmlElement)
	{
		auto mappingAreaXmlElement = multisliderPageXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::MAPPINGAREA));
		if (mappingAreaXmlElement)
		{
			auto mappingAreaTextXmlElement = mappingAreaXmlElement->getFirstChildElement();
			if (mappingAreaTextXmlElement && mappingAreaTextXmlElement->isTextElement())
			{
				auto mappingArea = static_cast<MappingAreaId>(mappingAreaTextXmlElement->getText().getIntValue());

				SetMultiSliderMappingArea(mappingArea, true);
			}
		}
		auto reverbEnabledXmlElement = multisliderPageXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::REVERBENABLED));
		if (reverbEnabledXmlElement)
		{
			auto reverbEnabledTextXmlElement = reverbEnabledXmlElement->getFirstChildElement();
			if (reverbEnabledTextXmlElement && reverbEnabledTextXmlElement->isTextElement())
			{
				auto reverbEnabled = (reverbEnabledTextXmlElement->getText().getIntValue()==1);

				SetMultiSliderReverbEnabled(reverbEnabled, true);
			}
		}
		auto spreadEnabledXmlElement = multisliderPageXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::SPREADENABLED));
		if (spreadEnabledXmlElement)
		{
			auto spreadEnabledTextXmlElement = spreadEnabledXmlElement->getFirstChildElement();
			if (spreadEnabledTextXmlElement && spreadEnabledTextXmlElement->isTextElement())
			{
				auto spreadEnabled = (spreadEnabledTextXmlElement->getText().getIntValue() == 1);

				SetMultiSliderSpreadEnabled(spreadEnabled, true);
			}
		}
		auto backgroundImagesXmlElement = multisliderPageXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::BACKGROUNDIMAGES));
		if (backgroundImagesXmlElement)
		{
			for (int i = MAI_First; i <= MAI_Fourth; i++)
			{
				auto mapping = static_cast<MappingAreaId>(i);
				auto bkgXmlElement = backgroundImagesXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::BACKGROUND) + String(mapping));
				if (bkgXmlElement)
				{
					MemoryBlock pngData;
					if (pngData.fromBase64Encoding(bkgXmlElement->getStringAttribute("pngData", String())))
					{
						MemoryInputStream inputStream(pngData.getData(), pngData.getSize(), true);
						m_multiSliderBackgrounds.insert(std::make_pair(mapping, ImageFileFormat::loadFrom(inputStream)));

						if (m_pageContainer)
							m_pageContainer->SetMultiSliderPageBackgroundImage(mapping, m_multiSliderBackgrounds.at(mapping));
					}
				}
				else
				{
					m_multiSliderBackgrounds.erase(mapping);

					if (m_pageContainer)
						m_pageContainer->RemoveMultiSliderPageBackgroundImage(mapping);
				}
			}
		}
	}
    
    m_pageContainer->SetPagesBeingInitialized(false);

	// Trigger updating UI with init parameter set.
	// This primarily aims at refreshing the settings page.
	m_pageContainer->UpdateGui(true);

	return retVal;
}

/**
 * Overriden from AppConfiguration::XmlConfigurableElement to dump this objects' settings
 * to a XML element structure that is returned and written to config file by the
 * singleton AppConfiguration class implementation.
 * @return	The XML element data that was created.
 */
std::unique_ptr<XmlElement> PageComponentManager::createStateXml()
{
	auto uiCfgXmlElement = std::make_unique<XmlElement>(AppConfiguration::getTagName(AppConfiguration::TagID::UICONFIG));
	if (uiCfgXmlElement)
	{
		auto activeTabXmlElement = uiCfgXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::ACTIVETAB));
		if (activeTabXmlElement)
			activeTabXmlElement->addTextElement(GetPageNameFromId(GetActivePage()));

		auto enabledPagesXmlElement = uiCfgXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::ENABLEDPAGES));
		if (enabledPagesXmlElement)
		{
			auto enabledPagesIdStrings = StringArray();
			for (auto const& pageId : GetEnabledPages())
				enabledPagesIdStrings.add(String(pageId));
			enabledPagesXmlElement->addTextElement(enabledPagesIdStrings.joinIntoString(", "));
		}

		auto lookAndFeelXmlElement = uiCfgXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::LOOKANDFEELTYPE));
		if (lookAndFeelXmlElement)
			lookAndFeelXmlElement->addTextElement(String((GetLookAndFeelType() == DbLookAndFeelBase::LAFT_InvalidFirst) ? DbLookAndFeelBase::LAFT_Dark : GetLookAndFeelType()));

		auto soundobjectTableXmlElement = uiCfgXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::SOUNDOBJECTTABLE));
		if (soundobjectTableXmlElement)
		{
			auto rowHeightXmlElement = soundobjectTableXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::ROWHEIGHT));
			if (rowHeightXmlElement)
				rowHeightXmlElement->addTextElement(String(GetSoundobjectTableRowHeight()));
		}

		auto matrixInputTableXmlElement = uiCfgXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::MATRIXINPUTTABLE));
		if (matrixInputTableXmlElement)
		{
			auto rowHeightXmlElement = matrixInputTableXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::ROWHEIGHT));
			if (rowHeightXmlElement)
				rowHeightXmlElement->addTextElement(String(GetMatrixInputTableRowHeight()));

			auto collapsedXmlElement = matrixInputTableXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::COLLAPSED));
			if (collapsedXmlElement)
				collapsedXmlElement->addTextElement(String(IsMatrixInputTableCollapsed() ? 1 : 0));
		}

		auto matrixOutputTableXmlElement = uiCfgXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::MATRIXOUTPUTTABLE));
		if (matrixOutputTableXmlElement)
		{
			auto rowHeightXmlElement = matrixOutputTableXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::ROWHEIGHT));
			if (rowHeightXmlElement)
				rowHeightXmlElement->addTextElement(String(GetMatrixOutputTableRowHeight()));

			auto collapsedXmlElement = matrixOutputTableXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::COLLAPSED));
			if (collapsedXmlElement)
				collapsedXmlElement->addTextElement(String(IsMatrixOutputTableCollapsed() ? 1 : 0));
		}

		auto scenesPageXmlElement = uiCfgXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::SCENESPAGE));
		if (scenesPageXmlElement)
		{
			auto pinnedScenesXmlElement = scenesPageXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::PINNEDSCENES));
			if (pinnedScenesXmlElement)
			{
				auto pinnedScenes = GetScenesPagePinnedScenes();
				for (auto const& pinnedScene : pinnedScenes)
				{
					auto pinnedSceneXmlElement = pinnedScenesXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::SCENE));
					if (pinnedSceneXmlElement)
					{
						pinnedSceneXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::INDEXMAJOR), pinnedScene.first.first);
						pinnedSceneXmlElement->setAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::INDEXMINOR), pinnedScene.first.second);
						pinnedSceneXmlElement->addTextElement(pinnedScene.second);
					}
				}
			}
		}

		auto multisliderPageXmlElement = uiCfgXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::MULTISLIDER));
		if (multisliderPageXmlElement)
		{
			auto mappingAreaXmlElement = multisliderPageXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::MAPPINGAREA));
			if (mappingAreaXmlElement)
				mappingAreaXmlElement->addTextElement(String(GetMultiSliderMappingArea()));

			auto reverbEnabledXmlElement = multisliderPageXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::REVERBENABLED));
			if (reverbEnabledXmlElement)
				reverbEnabledXmlElement->addTextElement(String(IsMultiSliderReverbEnabled() ? 1 : 0));

			auto spreadEnabledXmlElement = multisliderPageXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::SPREADENABLED));
			if (spreadEnabledXmlElement)
				spreadEnabledXmlElement->addTextElement(String(IsMultiSliderSpreadEnabled() ? 1 : 0));

			auto backgroundImagesXmlElement = multisliderPageXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::BACKGROUNDIMAGES));
			if (backgroundImagesXmlElement)
			{
				for (int i = MAI_First; i <= MAI_Fourth; i++)
				{
					auto mapping = static_cast<MappingAreaId>(i);
					if (m_multiSliderBackgrounds.count(mapping) > 0 && m_multiSliderBackgrounds.at(mapping).isValid())
					{
						auto bkgXmlElement = backgroundImagesXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::BACKGROUND) + String(mapping));
						if (bkgXmlElement)
						{
							MemoryOutputStream outputStream;
							PNGImageFormat formattedImage;
							formattedImage.writeImageToStream(m_multiSliderBackgrounds.at(mapping), outputStream);
							MemoryBlock pngData(outputStream.getData(), outputStream.getDataSize());

							bkgXmlElement->setAttribute("pngData", pngData.toBase64Encoding());
						}
					}
				}
			}
		}
	}

    return uiCfgXmlElement;
}


} // namespace SpaConBridge
