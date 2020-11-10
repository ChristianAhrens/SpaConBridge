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

#include <JuceHeader.h>


namespace SoundscapeBridgeApp
{


/**
 * Abstract class PageComponentBase.
 * Must be reimplemented to provide a component for an app page.
 */
class PageComponentBase : public Component
{
public:

	/**
	 * Overlay types. There can only be one active at the time.
	 */
	enum PageComponentType
	{
		PCT_Unknown = 0,
		PCT_Overview,
		PCT_MultiSlide,
		PCT_Settings,
		PCT_Statistics,
		PCT_About
	};

	explicit PageComponentBase(PageComponentType type);
	~PageComponentBase() override;

	//==============================================================================
	PageComponentType GetPageComponentType() const;

	//==============================================================================
	virtual void UpdateGui(bool init) = 0;

private:
	PageComponentType	m_pageComponentType;	/**> Type of page as specified by the PageComponentType enum. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PageComponentBase)
};


} // namespace SoundscapeBridgeApp
