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

#include "OverlayBase.h"	//<USE OverlayBase
#include <utility>			//<USE std::unique_ptr


namespace SoundscapeBridgeApp
{


/**
 * Class AboutOverlay is a GUI overlay which provides copyright and licensing info.
 * This is the base class for a generic "about" overlay, and must be subclassed for each host format (VST, AAX, etc).
 */
class AboutOverlay : public OverlayBase
{
public:
	AboutOverlay();
	~AboutOverlay() override;

	void UpdateGui(bool init) override;

protected:
	void paint(Graphics&) override;
	void resized() override;

private:
	/**
	 * App version label
	 */
	std::unique_ptr<Label>	m_versionLabel;

	/**
	 * Hyperlink to dbaudio.com
	 */
	std::unique_ptr<HyperlinkButton> m_dbLink;

	/**
	 * JUCE copyright label
	 */
	std::unique_ptr<Label>	m_juceLabel;

	/**
	 * Text field containing the d&b EULA.
	 */
	std::unique_ptr<TextEditor>	m_eulaField;
	

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AboutOverlay)
};


/**
 * Class CAboutOverlayGeneric is a GUI overlay which provides .
 */
class CAboutOverlayGeneric : public AboutOverlay
{
public:
	CAboutOverlayGeneric();
	~CAboutOverlayGeneric() override;

protected:
	void paint(Graphics&) override;
	void resized() override;

private:
	/**
	 * Host format license into
	 */
	std::unique_ptr<Label>	m_formatInfoLabel;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CAboutOverlayGeneric)
};


} // namespace SoundscapeBridgeApp
