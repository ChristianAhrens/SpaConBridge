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


#include "OverviewManager.h"

#include "Overview.h"

#include "../SoundsourceProcessor/SoundsourceProcessor.h"
#include "../SoundsourceProcessor/SoundsourceProcessorEditor.h"
#include "../Controller.h"
#include "../SoundsourceProcessor/SurfaceSlider.h"

#include "../submodules/JUCE-AppBasics/Source/Image_utils.hpp"


namespace SoundscapeBridgeApp
{


/*
===============================================================================
 Class COverviewManager
===============================================================================
*/

/**
 * The one and only instance of COverviewManager.
 */
COverviewManager* COverviewManager::m_singleton = nullptr;

/**
 * Class constructor.
 */
COverviewManager::COverviewManager()
{
	jassert(!m_singleton);	// only one instnce allowed!!
	m_singleton = this;

	//Default overview window properties.
	m_overview = nullptr;
}

/**
 * Destroys the COverviewManager.
 */
COverviewManager::~COverviewManager()
{
	jassert(m_overview == nullptr);

	m_singleton = nullptr;
}

/**
 * Returns the one and only instance of COverviewManager.
 * @return A COverviewManager object or 0.
 * @sa m_singleton, COverviewManager
 */
COverviewManager* COverviewManager::GetInstance()
{
	if (m_singleton == nullptr)
	{
		m_singleton = new COverviewManager();
	}
	return m_singleton;
}

/**
 * Function called when the "Overview" button on the GUI is clicked.
 */
void COverviewManager::OpenOverview()
{
	// Overview window is not currently open -> create it.
	if (m_overview == nullptr)
	{
		m_overview = new COverviewComponent();
	}

	// Overview window already exists -> bring it to the front.
	else
	{
		m_overview->toFront(true);
	}
}

/**
 * Getter for the overview component.
 * This is required to be able to embed the overview in a main component,
 * were the original d&b Soundscape Plugin displayed the overview as a window of its own.
 * @return The overview component.
 */
COverviewComponent* COverviewManager::GetOverview()
{
	if (m_overview == nullptr)
		OpenOverview();

	return m_overview;
}

/**
 * Function called by COverview's destructor to set the local pointer to zero.
 * @param destroy	True to also destroy the COverviewManager itself.
 */
void COverviewManager::CloseOverview(bool destroy)
{
	if (m_overview != nullptr)
	{
		// Close the overview window.
		delete m_overview;
		m_overview = nullptr;
	}

	// Closed overview, so manager no longer needed.
	if (destroy)
		delete this;
}

/**
 * Get the currently active tab within the overview window.
 * @return The currently active tab.
 */
int COverviewManager::GetActiveTab() const
{
	return m_selectedTab;
}

/**
 * Set the currently active tab within the overview window.
 * @param tabIdx	The currently active tab index.
 */
void COverviewManager::SetActiveTab(int tabIdx)
{
	m_selectedTab = tabIdx;
}

/**
 * Get the currently selected coordinate mapping used for the multi-slider.
 * @return The selected mapping area.
 */
int COverviewManager::GetSelectedMapping() const
{
	return m_selectedMapping;
}

/**
 * Set the currently selected coordinate mapping used for the multi-slider.
 * @param mapping	The new selected mapping area.
 */
void COverviewManager::SetSelectedMapping(int mapping)
{
	m_selectedMapping = mapping;
}


} // namespace SoundscapeBridgeApp
