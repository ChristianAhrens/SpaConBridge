/* Copyright(c) 2020 - 2022, Christian Ahrens
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


#include "PageComponentManager.h"

#include "PageContainerComponent.h"

#include "../CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"
#include "../Controller.h"
#include "../MultiSoundobjectComponent.h"
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
	m_activePage = UPI_Soundobjects;
	for (int id = UPI_InvalidMin + 1; id < UPI_InvalidMax; id++) m_enabledPages.push_back(static_cast<UIPageId>(id));
	m_lookAndFeelType = DbLookAndFeelBase::LAFT_Dark;

	m_multiSoundobjectComponent = std::make_unique<MultiSoundobjectComponent>();
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
 * Proxy Getter for the resizer bar ratio in sound objects table.
 * Forwards call to PageContainerComponent.
 * @return	The resizer bar ratio.
 */
float PageComponentManager::GetSoundobjectTableResizeBarRatio()
{
	if (m_pageContainer)
		return m_pageContainer->GetSoundobjectTableResizeBarRatio();
	else
	{
		jassertfalse;
		return 0.5f;
	}
}

/**
 * Proxy Setter for the resizer bar ratio in sound objects table.
 * Forwards call to PageContainerComponent.
 * @param pos	The resizer bar ratio.
 */
void PageComponentManager::SetSoundobjectTableResizeBarRatio(float ratio)
{
	if (m_pageContainer)
		m_pageContainer->SetSoundobjectTableResizeBarRatio(ratio);
}

/**
 * Proxy Getter for the single selection only flag in sound objects table.
 * Forwards call to PageContainerComponent.
 * @return	The single selection only flag.
 */
bool PageComponentManager::GetSoundobjectTableSingleSelectionOnly()
{
	if (m_pageContainer)
		return m_pageContainer->GetSoundobjectTableSingleSelectionOnly();
	else
	{
		jassertfalse;
		return false;
	}
}

/**
 * Proxy Setter for the single selection only flag in sound objects table.
 * Forwards call to PageContainerComponent.
 * @param singleSelectionOnly	The single selection only flag.
 */
void PageComponentManager::SetSoundobjectTableSingleSelectionOnly(bool singleSelectionOnly)
{
	if (m_pageContainer)
		m_pageContainer->SetSoundobjectTableSingleSelectionOnly(singleSelectionOnly);
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
 * Getter for the internal multi soundobject component to be able to use it in different places of the ui
 * @return	The internal member component.
 */
std::unique_ptr<MultiSoundobjectComponent>& PageComponentManager::GetMultiSoundobjectComponent()
{
	return m_multiSoundobjectComponent;
}

/**
 * Proxy Getter for the selected mapping area in MultiSoundobject Page.
 * Forwards the call to PageContainerComponent.
 * @return	The selected mapping area in MultiSoundobject Page.
 */
MappingAreaId PageComponentManager::GetMultiSoundobjectMappingArea()
{
	if (m_multiSoundobjectComponent)
		return m_multiSoundobjectComponent->GetSelectedMapping();
	else
	{
		jassertfalse;
		return MappingAreaId::MAI_Invalid;
	}
}

/**
 * Proxy Setter for the selected mapping area in MultiSoundobject Page.
 * Forwards the call to PageContainerComponent.
 * @param mappingArea	The selected mapping area in MultiSoundobject Page.
 * @param dontSendNotification	Indication if the configuration update shall be triggerd as well
 */
void PageComponentManager::SetMultiSoundobjectMappingArea(MappingAreaId mappingArea, bool dontSendNotification)
{
	if (m_multiSoundobjectComponent)
		m_multiSoundobjectComponent->SetSelectedMapping(mappingArea);

	if (!dontSendNotification)
	{
		triggerConfigurationUpdate(false);
	}
}

/**
 * Proxy Getter for the reverb enabled state in MultiSoundobject Page.
 * Forwards the call to PageContainerComponent.
 * @return	The reverb enabled state in MultiSoundobject Page.
 */
bool PageComponentManager::IsMultiSoundobjectReverbEnabled()
{
	if (m_multiSoundobjectComponent)
		return m_multiSoundobjectComponent->IsReverbEnabled();
	else
	{
		jassertfalse;
		return false;
	}
}

/**
 * Proxy Setter for the reverb enabled state in MultiSoundobject Page.
 * Forwards the call to PageContainerComponent.
 * @param enabled	The reverb enabled state in MultiSoundobject Page.
 * @param dontSendNotification	Indication if the configuration update shall be triggerd as well
 */
void PageComponentManager::SetMultiSoundobjectReverbEnabled(bool enabled, bool dontSendNotification)
{
	if (m_multiSoundobjectComponent)
		m_multiSoundobjectComponent->SetReverbEnabled(enabled);

	if (!dontSendNotification)
	{
		triggerConfigurationUpdate(false);
	}
}

/**
 * Proxy Getter for the spread enabled state in MultiSoundobject Page.
 * Forwards the call to PageContainerComponent.
 * @return	The spread enabled state in MultiSoundobject Page.
 */
bool PageComponentManager::IsMultiSoundobjectSpreadEnabled()
{
	if (m_multiSoundobjectComponent)
		return m_multiSoundobjectComponent->IsSpreadEnabled();
	else
	{
		jassertfalse;
		return false;
	}
}

/**
 * Proxy Setter for the spread enabled state in MultiSoundobject Page.
 * Forwards the call to PageContainerComponent.
 * @param enabled	The spread enabled state in MultiSoundobject Page.
 * @param dontSendNotification	Indication if the configuration update shall be triggerd as well
 */
void PageComponentManager::SetMultiSoundobjectSpreadEnabled(bool enabled, bool dontSendNotification)
{
	if (m_multiSoundobjectComponent)
		m_multiSoundobjectComponent->SetSpreadEnabled(enabled);

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
    
	if (m_multiSoundobjectBackgrounds.count(mappingArea) != 0)
		m_multiSoundobjectBackgrounds.erase(mappingArea);
	m_multiSoundobjectBackgrounds.insert(std::make_pair(mappingArea, image));

	if (m_multiSoundobjectComponent)
		m_multiSoundobjectComponent->SetBackgroundImage(mappingArea, m_multiSoundobjectBackgrounds.at(mappingArea));

	triggerConfigurationUpdate(false);
}

/**
 * Removes a background image from internal hash and multi soundobject page
 * @param	mappingArea	The mapping area the image shall be erased of
 */
void PageComponentManager::RemoveImageForMapping(MappingAreaId mappingArea)
{
	m_multiSoundobjectBackgrounds.erase(mappingArea);

	if (m_multiSoundobjectComponent)
		m_multiSoundobjectComponent->RemoveBackgroundImage(mappingArea);

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
 */
void PageComponentManager::SetLookAndFeelType(DbLookAndFeelBase::LookAndFeelType lookAndFeelType, bool dontUpdateConfig)
{
	m_lookAndFeelType = lookAndFeelType;

	if (!dontUpdateConfig)
	{
		triggerConfigurationUpdate(true); // we do want to include watcher update, since only that way the whole application is set to new LAFT (onConfigUpdated in main component)
	}
}

#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
/**
 * Getter for the fullscreen windowmode member value.
 * @return The bool indication if current windowmode is fullscreen.
 */
bool PageComponentManager::IsFullscreenWindowMode() const
{
	return m_fullscreenWindowMode;
}

/**
 * Setter for the fullscreen windowmode member.
 * @param fullscreen	The bool indicator if setting to fullscreen or non-fullscreen shall be performed
 * @param dontUpdateConfig	Indication if the configuration update shall be triggerd as well
 */
void PageComponentManager::SetFullscreenWindowMode(bool fullscreen, bool dontUpdateConfig)
{
	m_fullscreenWindowMode = fullscreen;

	if (!dontUpdateConfig)
	{
		triggerConfigurationUpdate(true); // we do want to include watcher update, since only that way the whole application is set to new LAFT (onConfigUpdated in main component)
	}
}
#endif

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

#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
	auto fullscreenWindowModeXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::FULLSCREENWINDOWMODE));
	if (fullscreenWindowModeXmlElement)
	{
		auto fullscreenWindowModeTextElement = fullscreenWindowModeXmlElement->getFirstChildElement();
		if (fullscreenWindowModeTextElement && fullscreenWindowModeTextElement->isTextElement())
		{
			auto fullscreen = 1 == fullscreenWindowModeTextElement->getText().getIntValue();

			SetFullscreenWindowMode(fullscreen, true);
		}
	}
#endif

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
			{
				// set a valid default to not leave the user with a blank page in case of an error
				SetActivePage(UPI_Soundobjects, true);
				retVal = false;
			}
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

		auto resizeBarRatioXmlElement = soundobjectTableXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::RESIZERBARRATIO));
		if (resizeBarRatioXmlElement)
		{
			auto resizeBarRatioTextXmlElement = resizeBarRatioXmlElement->getFirstChildElement();
			if (resizeBarRatioTextXmlElement && resizeBarRatioTextXmlElement->isTextElement())
			{
				auto resizerBarRatio = resizeBarRatioTextXmlElement->getText().getFloatValue();

				SetSoundobjectTableResizeBarRatio(resizerBarRatio);
			}
		}

		auto singleSelectionOnlyXmlElement = soundobjectTableXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::SINGLESELECTIONONLY));
		if (singleSelectionOnlyXmlElement)
		{
			auto singleSelectionOnlyTextXmlElement = singleSelectionOnlyXmlElement->getFirstChildElement();
			if (singleSelectionOnlyTextXmlElement && singleSelectionOnlyTextXmlElement->isTextElement())
			{
				auto singleSelectionOnly = singleSelectionOnlyTextXmlElement->getText().getIntValue() == 0 ? false : true;

				SetSoundobjectTableSingleSelectionOnly(singleSelectionOnly);
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

	auto MultiSoundobjectPageXmlElement = stateXml->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::MULTISLIDER));
	if (MultiSoundobjectPageXmlElement)
	{
		auto mappingAreaXmlElement = MultiSoundobjectPageXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::MAPPINGAREA));
		if (mappingAreaXmlElement)
		{
			auto mappingAreaTextXmlElement = mappingAreaXmlElement->getFirstChildElement();
			if (mappingAreaTextXmlElement && mappingAreaTextXmlElement->isTextElement())
			{
				auto mappingArea = static_cast<MappingAreaId>(mappingAreaTextXmlElement->getText().getIntValue());

				SetMultiSoundobjectMappingArea(mappingArea, true);
			}
		}
		auto reverbEnabledXmlElement = MultiSoundobjectPageXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::REVERBENABLED));
		if (reverbEnabledXmlElement)
		{
			auto reverbEnabledTextXmlElement = reverbEnabledXmlElement->getFirstChildElement();
			if (reverbEnabledTextXmlElement && reverbEnabledTextXmlElement->isTextElement())
			{
				auto reverbEnabled = (reverbEnabledTextXmlElement->getText().getIntValue()==1);

				SetMultiSoundobjectReverbEnabled(reverbEnabled, true);
			}
		}
		auto spreadEnabledXmlElement = MultiSoundobjectPageXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::SPREADENABLED));
		if (spreadEnabledXmlElement)
		{
			auto spreadEnabledTextXmlElement = spreadEnabledXmlElement->getFirstChildElement();
			if (spreadEnabledTextXmlElement && spreadEnabledTextXmlElement->isTextElement())
			{
				auto spreadEnabled = (spreadEnabledTextXmlElement->getText().getIntValue() == 1);

				SetMultiSoundobjectSpreadEnabled(spreadEnabled, true);
			}
		}
		auto backgroundImagesXmlElement = MultiSoundobjectPageXmlElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::BACKGROUNDIMAGES));
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
						m_multiSoundobjectBackgrounds.insert(std::make_pair(mapping, ImageFileFormat::loadFrom(inputStream)));

						if (m_multiSoundobjectComponent)
							m_multiSoundobjectComponent->SetBackgroundImage(mapping, m_multiSoundobjectBackgrounds.at(mapping));
					}
				}
				else
				{
					m_multiSoundobjectBackgrounds.erase(mapping);

					if (m_multiSoundobjectComponent)
						m_multiSoundobjectComponent->RemoveBackgroundImage(mapping);
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

#if USE_FULLSCREEN_WINDOWMODE_TOGGLE
	auto fullscreenWindowModeXmlElement = uiCfgXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::FULLSCREENWINDOWMODE));
	if (fullscreenWindowModeXmlElement)
		fullscreenWindowModeXmlElement->addTextElement(String(IsFullscreenWindowMode() ? 1 : 0));
#endif

	auto soundobjectTableXmlElement = uiCfgXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::SOUNDOBJECTTABLE));
	if (soundobjectTableXmlElement)
	{
		auto rowHeightXmlElement = soundobjectTableXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::ROWHEIGHT));
		if (rowHeightXmlElement)
			rowHeightXmlElement->addTextElement(String(GetSoundobjectTableRowHeight()));

		auto resizeBarRatioXmlElement = soundobjectTableXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::RESIZERBARRATIO));
		if (resizeBarRatioXmlElement)
			resizeBarRatioXmlElement->addTextElement(String(GetSoundobjectTableResizeBarRatio()));

		auto singleSelectionOnlyXmlElement = soundobjectTableXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::SINGLESELECTIONONLY));
		if (singleSelectionOnlyXmlElement)
			singleSelectionOnlyXmlElement->addTextElement(String(GetSoundobjectTableSingleSelectionOnly() ? 1 : 0));
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

	auto MultiSoundobjectPageXmlElement = uiCfgXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::MULTISLIDER));
	if (MultiSoundobjectPageXmlElement)
	{
		auto mappingAreaXmlElement = MultiSoundobjectPageXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::MAPPINGAREA));
		if (mappingAreaXmlElement)
			mappingAreaXmlElement->addTextElement(String(GetMultiSoundobjectMappingArea()));

		auto reverbEnabledXmlElement = MultiSoundobjectPageXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::REVERBENABLED));
		if (reverbEnabledXmlElement)
			reverbEnabledXmlElement->addTextElement(String(IsMultiSoundobjectReverbEnabled() ? 1 : 0));

		auto spreadEnabledXmlElement = MultiSoundobjectPageXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::SPREADENABLED));
		if (spreadEnabledXmlElement)
			spreadEnabledXmlElement->addTextElement(String(IsMultiSoundobjectSpreadEnabled() ? 1 : 0));

		auto backgroundImagesXmlElement = MultiSoundobjectPageXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::BACKGROUNDIMAGES));
		if (backgroundImagesXmlElement)
		{
			for (int i = MAI_First; i <= MAI_Fourth; i++)
			{
				auto mapping = static_cast<MappingAreaId>(i);
				if (m_multiSoundobjectBackgrounds.count(mapping) > 0 && m_multiSoundobjectBackgrounds.at(mapping).isValid())
				{
					auto bkgXmlElement = backgroundImagesXmlElement->createNewChildElement(AppConfiguration::getTagName(AppConfiguration::TagID::BACKGROUND) + String(mapping));
					if (bkgXmlElement)
					{
						MemoryOutputStream outputStream;
						PNGImageFormat formattedImage;
						formattedImage.writeImageToStream(m_multiSoundobjectBackgrounds.at(mapping), outputStream);
						MemoryBlock pngData(outputStream.getData(), outputStream.getDataSize());

						bkgXmlElement->setAttribute("pngData", pngData.toBase64Encoding());
					}
				}
			}
		}
	}

    return uiCfgXmlElement;
}


} // namespace SpaConBridge
