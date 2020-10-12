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


#include "Gui.h"


namespace SoundscapeBridgeApp
{


/*
===============================================================================
 Class CDbStyle
===============================================================================
*/

/**
 * Get the desired color.
 * @param color Desired color code.
 */
Colour CDbStyle::GetDbColor(DbColor color)
{
	switch (color)
	{
	case WindowColor:
		return Colour(27, 27, 27);
	case DarkLineColor:
		return Colour(49, 49, 49);
	case DarkColor:
		return Colour(67, 67, 67);
	case MidColor:
		return Colour(83, 83, 83);
	case ButtonColor:
		return Colour(125, 125, 125);
	case LightColor:
		return Colour(201, 201, 201);
	case TextColor:
		return Colour(238, 238, 238);
	case DarkTextColor:
		return Colour(180, 180, 180);
	case HighlightColor:
		return Colour(115, 140, 155);
	case FaderGreenColor:
		return Colour(140, 180, 90);
	case ButtonBlueColor:
		return Colour(27, 120, 163);
	case ButtonRedColor:
		return Colour(226, 41, 41);
	default:
		break;
	}

	jassertfalse;
	return Colours::black;
}


/*
===============================================================================
 Class LedButton
===============================================================================
*/

/**
 * Object constructor.
 * @param componentName		The name to pass to the component for it to use as its name .
 */
LedButton::LedButton()
	: ToggleButton()
{
}

/**
 * Object destructor.
 */
LedButton::~LedButton()
{
}

/**
 * Reimplemented paint function.
 * @param g					A graphics context, used for drawing a component or image.
 * @param isMouseOverButton	True if the mouse is over the button.
 * @param isButtonDown		True if the button is being pressed.
 */
void LedButton::paintButton(Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
	Rectangle<int> bounds = getLocalBounds();
	Rectangle<float> buttonRectF = Rectangle<float>(2.5f, 2.5f, bounds.getWidth() - 4.0f, bounds.getHeight() - 4.0f);
	bool on = getToggleState();
	bool enabled = isEnabled();

	// Button's main colour
	if (on)
	{
		Colour col = CDbStyle::GetDbColor(CDbStyle::ButtonBlueColor);
		if (isButtonDown)
			col = col.brighter(0.1f);
		else if (isMouseOverButton)
			col = col.brighter(0.05f);
		g.setColour(col);
	}
	else
	{
		Colour col = CDbStyle::GetDbColor(CDbStyle::ButtonColor);
		if (!enabled)
			col = col.darker(0.5f);
		else if (isButtonDown)
			col = CDbStyle::GetDbColor(CDbStyle::ButtonBlueColor).brighter(0.05f);
		else if (isMouseOverButton)
			col = col.brighter(0.05f);
		g.setColour(col);
	}

	g.fillRoundedRectangle(buttonRectF, 10);
	g.setColour(CDbStyle::GetDbColor(CDbStyle::WindowColor));
	g.drawRoundedRectangle(buttonRectF, 10, 1);
}


/*
===============================================================================
 Class AOverlay
===============================================================================
*/

/**
 * Class constructor.
 */
AOverlay::AOverlay(OverlayType type)
	: Component()
{
	m_overlayType = type;
}

/**
 * Class destructor.
 */
AOverlay::~AOverlay()
{
}

/**
 * Get this overlay's type.
 */
AOverlay::OverlayType AOverlay::GetOverlayType() const
{
	return m_overlayType;
}


} // namespace SoundscapeBridgeApp
