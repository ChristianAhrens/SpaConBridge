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
#include "../SoundscapeBridgeAppCommon.h"
#include "../AppConfiguration.h"
#include "../LookAndFeel.h"


namespace SoundscapeBridgeApp
{


/**
 * Forward declarations
 */
class COverview;
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

	void OpenPageContainer();
	PageContainerComponent* GetPageContainer();
	void ClosePageContainer(bool destroy);

	int GetActiveTab() const;
	void SetActiveTab(int tabIdx, bool dontSendNotification);

	int GetSelectedMapping() const;
	void SetSelectedMapping(int mapping);

	DbLookAndFeelBase::LookAndFeelType GetLookAndFeelType() const;
	void SetLookAndFeelType(DbLookAndFeelBase::LookAndFeelType lookAndFeelType, bool dontSendNotification);

	std::unique_ptr<XmlElement> createStateXml() override;
	bool setStateXml(XmlElement* stateXml) override;

protected:
	static PageComponentManager	*m_singleton;			/**> The one and only instance of PageComponentManager. */
	PageContainerComponent		*m_pageContainer;		/**> Pointer to the Overview winodw, if any. */
	int							m_selectedTab{ 0 };		/**> Remember the last active tab. */
	int							m_selectedMapping{ 1 };	/**> Remember the last selected coordinate mapping for the multi-slider. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PageComponentManager)
};


} // namespace SoundscapeBridgeApp
