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


#include "SurfaceSlider.h"
#include "Parameters.h"
#include "SoundsourceProcessor.h"
#include "../Controller.h"


namespace SoundscapeBridgeApp
{



/*
===============================================================================
 Class SurfaceSlider
===============================================================================
*/

/**
 * Object constructor.
 * @param parent	The audio processor object to act as parent.
 */
SurfaceSlider::SurfaceSlider(AudioProcessor* parent)
{
	m_parent = parent;
}

/**
 * Object destructor.
 */
SurfaceSlider::~SurfaceSlider()
{
}

/**
 * Reimplemented paint event function.
 * Components can override this method to draw their content. The paint() method gets called when 
 * a region of a component needs redrawing, either because the component's repaint() method has 
 * been called, or because something has happened on the screen that means a section of a window needs to be redrawn.
 * @param g		The graphics context that must be used to do the drawing operations. 
 */
void SurfaceSlider::paint(Graphics& g)
{
	auto w = getLocalBounds().getWidth();
	auto h = getLocalBounds().getHeight();

	// Surface area
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	g.fillRect(0, 0, w, h);

	// Surface frame
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));
	g.drawRect(Rectangle<int>(0, 0, w, h), 2);

	// X knob posiiton
	Path knobOutline;

	float x = 0;
	const Array<AudioProcessorParameter*>& params = m_parent->getParameters();
	AudioParameterFloat* param = dynamic_cast<AudioParameterFloat*> (params[ParamIdx_X]);
	if (param)
		x = static_cast<float>(*param * w);

	// Y knob position
	float y = 0;
	param = dynamic_cast<AudioParameterFloat*> (params[ParamIdx_Y]);
	if (param)
		y = h - (static_cast<float>(*param * h));

	// Paint knob
	float knobSize = 10;
	knobOutline.addEllipse(x - (knobSize / 2), y - (knobSize / 2), knobSize, knobSize);

	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	g.fillPath(knobOutline);
	g.setColour(getLookAndFeel().findColour(Slider::thumbColourId));
	g.strokePath(knobOutline, PathStrokeType(3)); // Stroke width

}

/**
 * Called when a mouse button is pressed. 
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred 
 */
void SurfaceSlider::mouseDown(const MouseEvent& e)
{
	float w = static_cast<float>(getLocalBounds().getWidth());
	float h = static_cast<float>(getLocalBounds().getHeight());

	// Get mouse position and scale it between 0 and 1.
	Point<int> pos = e.getMouseDownPosition();
	float x = jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getX()) / w)));
	float y = 1.0f - jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getY()) / h)));

	SoundsourceProcessor* processor = dynamic_cast<SoundsourceProcessor*>(m_parent);
	if (processor)
	{
		// Set new X and Y values
		GestureManagedAudioParameterFloat* param;
		param = dynamic_cast<GestureManagedAudioParameterFloat*>(m_parent->getParameters()[ParamIdx_X]);
		param->BeginGuiGesture();
		processor->SetParameterValue(DCS_Gui, ParamIdx_X, x);
		
		param = dynamic_cast<GestureManagedAudioParameterFloat*>(m_parent->getParameters()[ParamIdx_Y]);
		param->BeginGuiGesture();
		processor->SetParameterValue(DCS_Gui, ParamIdx_Y, y);
	}
}

/**
 * Called when the mouse is moved while a button is held down. 
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred
 */
void SurfaceSlider::mouseDrag(const MouseEvent& e)
{
	float w = static_cast<float>(getLocalBounds().getWidth());
	float h = static_cast<float>(getLocalBounds().getHeight());

	// Get mouse position and scale it between 0 and 1.
	Point<int> pos = e.getPosition();
	float x = jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getX()) / w)));
	float y = 1.0f - jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getY()) / h)));

	SoundsourceProcessor* plugin = dynamic_cast<SoundsourceProcessor*>(m_parent);
	if (plugin)
	{
		// Set new X and Y values
		plugin->SetParameterValue(DCS_Gui, ParamIdx_X, x);
		plugin->SetParameterValue(DCS_Gui, ParamIdx_Y, y);
	}
}

/**
 * Called when the mouse button is released.
 * Reimplemented just to call EndGuiGesture() to inform the host.
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred
 */
void SurfaceSlider::mouseUp(const MouseEvent& e)
{
	ignoreUnused(e);

	GestureManagedAudioParameterFloat* param;
	param = dynamic_cast<GestureManagedAudioParameterFloat*>(m_parent->getParameters()[ParamIdx_X]);
	param->EndGuiGesture();

	param = dynamic_cast<GestureManagedAudioParameterFloat*>(m_parent->getParameters()[ParamIdx_Y]);
	param->EndGuiGesture();
}


/*
===============================================================================
 Class SurfaceMultiSlider
===============================================================================
*/

/**
 * Object constructor.
 */
SurfaceMultiSlider::SurfaceMultiSlider()
{
	m_currentlyDraggedId = INVALID_PROCESSOR_ID;
}

/**
 * Object destructor.
 */
SurfaceMultiSlider::~SurfaceMultiSlider()
{
}

/**
 * Reimplemented paint event function.
 * Components can override this method to draw their content. The paint() method gets called when 
 * a region of a component needs redrawing, either because the component's repaint() method has 
 * been called, or because something has happened on the screen that means a section of a window needs to be redrawn.
 * @param g		The graphics context that must be used to do the drawing operations. 
 */
void SurfaceMultiSlider::paint(Graphics& g)
{
	auto w = getLocalBounds().toFloat().getWidth();
	auto h = getLocalBounds().toFloat().getHeight();

	// Surface background area
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	g.fillRect(Rectangle<float>(0.0f, 0.0f, w, h));

	// Draw grid
	const float dashLengths[2] = { 5.0f, 6.0f };
	const float lineThickness = 1.0f;
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId).brighter(0.15f));
	g.drawDashedLine(Line<float>(w * 0.25f, 0.0f, w * 0.25f, h), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(w * 0.50f, 0.0f, w * 0.50f, h), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(w * 0.75f, 0.0f, w * 0.75f, h), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(0.0f, h * 0.25f, w, h * 0.25f), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(0.0f, h * 0.50f, w, h * 0.50f), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(0.0f, h * 0.75f, w, h * 0.75f), dashLengths, 2, lineThickness);

	// Surface frame
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));
	g.drawRect(Rectangle<float>(0.0f, 0.0f, w, h), 1.5f);

	float knobSize = 10.0f;
	float highlightedKnobSize = 2 * knobSize;

	for (auto const& posKV : m_cachedPositions)
	{
		int inputNo(posKV.second._id);

		// Map the x/y coordinates to the pixel-wise dimensions of the surface area.
		Point<float> pt(posKV.second._pos);
		float x = pt.x * w;
		float y = h - (pt.y * h);

		// Generate a color variant based on the input number, so make the nipples easier to tell from each other.
		Colour shade(juce::uint8(inputNo * 111), juce::uint8(inputNo * 222), juce::uint8(inputNo * 333));
		g.setColour(getLookAndFeel().findColour(Slider::thumbColourId).interpolatedWith(shade, 0.3f));

		// Paint knob
		if (posKV.second._selected)
		{
			g.drawEllipse(Rectangle<float>(x - (highlightedKnobSize / 2.0f), y - (highlightedKnobSize / 2.0f), highlightedKnobSize, highlightedKnobSize), 6.0f);

			// Input number label
			g.setFont(Font(11.0, Font::plain));
			g.drawText(String(inputNo), Rectangle<float>(x - highlightedKnobSize, y + 3, highlightedKnobSize * 2.0f, highlightedKnobSize * 2.0f), Justification::centred, true);
		}
		else
		{
			g.drawEllipse(Rectangle<float>(x - (knobSize / 2.0f), y - (knobSize / 2.0f), knobSize, knobSize), 3.0f);

			// Input number label
			g.setFont(Font(11.0, Font::plain));
			g.drawText(String(inputNo), Rectangle<float>(x - knobSize, y + 3, knobSize * 2.0f, knobSize * 2.0f), Justification::centred, true);
		}
	}
}

/**
 * Called when a mouse button is pressed. 
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred 
 */
void SurfaceMultiSlider::mouseDown(const MouseEvent& e)
{
	float w = static_cast<float>(getLocalBounds().getWidth());
	float h = static_cast<float>(getLocalBounds().getHeight());

	// Mouse click position (in pixel units)
	Point<float> mousePos(static_cast<float>(e.getMouseDownPosition().x), static_cast<float>(e.getMouseDownPosition().y));
	float knobSize = 15.0f;
	float highlightedKnobSize = 2 * knobSize;

	for (auto const& posKV : m_cachedPositions)
	{
		// Map the x/y coordinates to the pixel-wise dimensions of the surface area.
		Point<float> pt(posKV.second._pos);
		float x = pt.x * w;
		float y = h - (pt.y * h);

		Path knobPath;
		if (posKV.second._selected)
			knobPath.addEllipse(Rectangle<float>(x - (highlightedKnobSize / 2.0f), y - (highlightedKnobSize / 2.0f), highlightedKnobSize, highlightedKnobSize));
		else
			knobPath.addEllipse(Rectangle<float>(x - (knobSize / 2.0f), y - (knobSize / 2.0f), knobSize, knobSize));

		// Check if the mouse click landed inside any of the knobs.
		if (knobPath.contains(mousePos))
		{
			// Set this source as "selected" and begin a drag gesture.
			m_currentlyDraggedId = posKV.first;

			auto ctrl = Controller::GetInstance();
			if (ctrl)
			{
				auto processor = ctrl->GetProcessor(m_currentlyDraggedId);
				jassert(processor);
				if (processor)
				{
					GestureManagedAudioParameterFloat* param;
					param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[ParamIdx_X]);
					param->BeginGuiGesture();

					param = dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[ParamIdx_Y]);
					param->BeginGuiGesture();
				}
			}

			// Found a knob to select, skip the rest.
			break;
		}
	}
}

/**
 * Called when the mouse is moved while a button is held down. 
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred
 */
void SurfaceMultiSlider::mouseDrag(const MouseEvent& e)
{
	if (m_currentlyDraggedId != INVALID_PROCESSOR_ID)
	{
		auto ctrl = Controller::GetInstance();
		if (ctrl)
		{
			auto processor = ctrl->GetProcessor(m_currentlyDraggedId);
			if (processor)
			{
				// Get mouse pixel-wise position and scale it between 0 and 1.
				Point<int> pos = e.getPosition();
				float x = jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getX()) / getLocalBounds().getWidth())));
				float y = 1.0f - jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getY()) / getLocalBounds().getHeight())));

				processor->SetParameterValue(DCS_Overview, ParamIdx_X, x);
				processor->SetParameterValue(DCS_Overview, ParamIdx_Y, y);
			}
		}
	}
}

/**
 * Called when the mouse button is released.
 * Reimplemented just to call EndGuiGesture() to inform the host.
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred
 */
void SurfaceMultiSlider::mouseUp(const MouseEvent& e)
{
	ignoreUnused(e);

	if (m_currentlyDraggedId != INVALID_PROCESSOR_ID)
	{
		auto ctrl = Controller::GetInstance();
		if (ctrl)
		{
			auto processor = ctrl->GetProcessor(m_currentlyDraggedId);
			if (processor)
			{
				dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[ParamIdx_X])->EndGuiGesture();
				dynamic_cast<GestureManagedAudioParameterFloat*>(processor->getParameters()[ParamIdx_Y])->EndGuiGesture();

				// Get mouse pixel-wise position and scale it between 0 and 1.
				Point<int> pos = e.getPosition();
				float x = jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getX()) / getLocalBounds().getWidth())));
				float y = 1.0f - jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getY()) / getLocalBounds().getHeight())));

				processor->SetParameterValue(DCS_Overview, ParamIdx_X, x);
				processor->SetParameterValue(DCS_Overview, ParamIdx_Y, y);
			}
		}

		// De-select knob.
		m_currentlyDraggedId = INVALID_PROCESSOR_ID;
	}
}

/**
 * Update the local hash of plugins and their current coordinates.
 * @param positions	Map where the keys are the PluginIds of each source, while values are pairs of the corresponding 
 *					input number and position coordinates (0.0 to 1.0). 
 */
void SurfaceMultiSlider::UpdatePositions(PositionCache positions)
{
	m_cachedPositions = positions;
}


} // namespace SoundscapeBridgeApp
