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
#include "../SurfaceSlider.h"

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
 * Get the currently active tab within the overview window.
 * @return The currently active tab.
 */
int PageComponentManager::GetActiveTab() const
{
	return m_selectedTab;
}

/**
 * Set the currently active tab within the overview window.
 * @param tabIdx	The currently active tab index.
 */
void PageComponentManager::SetActiveTab(int tabIdx, bool dontSendNotification)
{
	m_selectedTab = tabIdx;

	if (m_pageContainer != nullptr)
	{
		m_pageContainer->SetActiveTab(tabIdx);
	}

	if (!dontSendNotification)
	{
		triggerConfigurationUpdate(false);
	}
}

/**
 * Get the currently selected coordinate mapping used for the multi-slider.
 * @return The selected mapping area.
 */
int PageComponentManager::GetSelectedMapping() const
{
	return m_selectedMapping;
}

/**
 * Set the currently selected coordinate mapping used for the multi-slider.
 * @param mapping	The new selected mapping area.
 */
void PageComponentManager::SetSelectedMapping(int mapping)
{
	m_selectedMapping = mapping;
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
		return 0;
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
 * Get the currently selected coordinate mapping used for the multi-slider.
 * @return The selected mapping area.
 */
DbLookAndFeelBase::LookAndFeelType PageComponentManager::GetLookAndFeelType() const
{
	if (m_pageContainer != nullptr)
	{
		return m_pageContainer->GetLookAndFeelType();
	}
	else
	{
		jassertfalse;
		return DbLookAndFeelBase::LAFT_InvalidFirst;
	}
}

/**
 * Get the currently selected coordinate mapping used for the multi-slider.
 * @return The selected mapping area.
 */
void PageComponentManager::SetLookAndFeelType(DbLookAndFeelBase::LookAndFeelType lookAndFeelType, bool dontSendNotification)
{
	if (m_pageContainer != nullptr)
	{
		m_pageContainer->SetLookAndFeelType(lookAndFeelType);
	}

	if (!dontSendNotification)
	{
		triggerConfigurationUpdate(false);
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
	if (!stateXml || (stateXml->getTagName() != AppConfiguration::getTagName(AppConfiguration::TagID::UICONFIG)))
		return false;

	auto retVal = true;

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
			auto tabIdx = activeTabTextElement->getText().getIntValue();

			if (tabIdx != -1)
			{
				SetActiveTab(tabIdx, true);
			}
			else
				retVal = false;
		}
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
			activeTabXmlElement->addTextElement(String(GetActiveTab()));

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
	}

    return uiCfgXmlElement;
}


} // namespace SpaConBridge
