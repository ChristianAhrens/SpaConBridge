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
 * A style for d&b software look and feel.
 */
class CDbStyle
{
public:
	/**
	 * d&b Color codes
	 */
	enum DbColor
	{
		WindowColor,		// 27 27 27	- Window background
		DarkLineColor,		// 49 49 49 - Dark lines between table rows
		DarkColor,			// 67 67 67	- Dark
		MidColor,			// 83 83 83	- Mid
		ButtonColor,		// 125 125 125 - Button off
		LightColor,			// 201 201 201	- Light
		TextColor,			// 238 238 238 - Text
		DarkTextColor,		// 180 180 180 - Dark text
		HighlightColor,		// 115 140 155 - Highlighted text
		FaderGreenColor,	// 140 180 90 - Green sliders
		ButtonBlueColor,	// 28 122 166 - Button Blue
		ButtonRedColor,		// 226 41 41 - Button Red
	};

	CDbStyle() {};
	virtual ~CDbStyle() = default;

	static Colour GetDbColor(DbColor color);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CDbStyle)
};


/**
 * class CTextEditor, a custom TextEditor
 */
class CTextEditor : public TextEditor
{
public:
	explicit CTextEditor(const String& componentName = String(), juce_wchar passwordCharacter = 0);
	~CTextEditor() override;

	void SetSuffix(String suffix);

protected:
	void InitStyle();
	void paint(Graphics &) override;

	String m_suffix;		/**< Suffix to append to displayed text, such as units. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CTextEditor)
};

/**
 * class LedButton, a custom ToggleButton
 */
class LedButton : public ToggleButton
{
public:
	explicit LedButton();
	~LedButton() override;

protected:
	void paintButton(Graphics&, bool isMouseOverButton, bool isButtonDown) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LedButton)
};


/**
 * Abstract class AOverlay.
 * Must be reimplemented to provide a GUI overlay.
 */
class AOverlay : public Component
{
public:

	/**
	 * Overlay types. There can only be one active at the time.
	 */
	enum OverlayType
	{
		OT_Unknown = 0,
		OT_Overview,
		OT_MultiSlide,
		OT_Settings,
		OT_About
	};

	explicit AOverlay(OverlayType type);
	~AOverlay() override;

	OverlayType GetOverlayType() const;
	virtual void UpdateGui(bool init) = 0;

private:
	/**
	 * Type of overlay as specified by the OverlayType enum.
	 */
	OverlayType	m_overlayType;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AOverlay)
};


} // namespace SoundscapeBridgeApp
