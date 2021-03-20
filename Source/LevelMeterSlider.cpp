/* Copyright (c) 2020-2021, Christian Ahrens
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


#include "LevelMeterSlider.h"

#include "LookAndFeel.h"


namespace SpaConBridge
{


/*
===============================================================================
 Class LevelMeterSlider
===============================================================================
*/

/**
 * Object constructor.
 */
LevelMeterSlider::LevelMeterSlider(const String& componentName, LevelMeterMode mode)
	: Slider(componentName)
{
	m_levelMeterMode = mode;

	setSliderStyle(Slider::SliderStyle::LinearBar);
	setTextBoxStyle(Slider::NoTextBox, true, 0, 0);

	// determine the right green fader colour from lookandfeel
	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (dblookAndFeel)
		setColour(Slider::ColourIds::trackColourId, dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::FaderGreenColor));
}

/**
 * Object destructor.
 */
LevelMeterSlider::~LevelMeterSlider()
{
}

/**
 * Called when a mouse button is pressed. 
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred 
 */
void LevelMeterSlider::mouseDown(const MouseEvent& e)
{
	if (m_levelMeterMode == LMM_ReadOnly)
		ignoreUnused(e);
	else
		return Slider::mouseDown(e);
}

/**
 * Called when the mouse is moved while a button is held down. 
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred
 */
void LevelMeterSlider::mouseDrag(const MouseEvent& e)
{
	if (m_levelMeterMode == LMM_ReadOnly)
		ignoreUnused(e);
	else
		return Slider::mouseDrag(e);
}

/**
 * Called when the mouse button is released.
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred
 */
void LevelMeterSlider::mouseUp(const MouseEvent& e)
{
	if (m_levelMeterMode == LMM_ReadOnly)
		ignoreUnused(e);
	else
		return Slider::mouseUp(e);
}


} // namespace SpaConBridge
