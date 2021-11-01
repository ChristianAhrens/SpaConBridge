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


#include "SoundobjectSlider.h"

#include "Controller.h"

#include "CustomAudioProcessors/Parameters.h"
#include "CustomAudioProcessors/SoundobjectProcessor/SoundobjectProcessor.h"


namespace SpaConBridge
{


/*
===============================================================================
 Class SoundobjectSlider
===============================================================================
*/

/**
 * Object constructor.
 * @param parent	The audio processor object to act as parent.
 */
SoundobjectSlider::SoundobjectSlider(AudioProcessor* parent)
{
	m_parent = parent;
}

/**
 * Object destructor.
 */
SoundobjectSlider::~SoundobjectSlider()
{
}

/**
 * Reimplemented paint event function.
 * Components can override this method to draw their content. The paint() method gets called when 
 * a region of a component needs redrawing, either because the component's repaint() method has 
 * been called, or because something has happened on the screen that means a section of a window needs to be redrawn.
 * @param g		The graphics context that must be used to do the drawing operations. 
 */
void SoundobjectSlider::paint(Graphics& g)
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
	AudioParameterFloat* param = dynamic_cast<AudioParameterFloat*> (params[SPI_ParamIdx_X]);
	if (param)
		x = static_cast<float>(*param * w);

	// Y knob position
	float y = 0;
	param = dynamic_cast<AudioParameterFloat*> (params[SPI_ParamIdx_Y]);
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
void SoundobjectSlider::mouseDown(const MouseEvent& e)
{
	float w = static_cast<float>(getLocalBounds().getWidth());
	float h = static_cast<float>(getLocalBounds().getHeight());

	// Get mouse position and scale it between 0 and 1.
	Point<int> pos = e.getMouseDownPosition();
	float x = jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getX()) / w)));
	float y = 1.0f - jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getY()) / h)));

	SoundobjectProcessor* processor = dynamic_cast<SoundobjectProcessor*>(m_parent);
	if (processor)
	{
		// Set new X and Y values
		GestureManagedAudioParameterFloat* param;
		param = dynamic_cast<GestureManagedAudioParameterFloat*>(m_parent->getParameters()[SPI_ParamIdx_X]);
		param->BeginGuiGesture();
		processor->SetParameterValue(DCP_SoundobjectProcessor, SPI_ParamIdx_X, x);
		
		param = dynamic_cast<GestureManagedAudioParameterFloat*>(m_parent->getParameters()[SPI_ParamIdx_Y]);
		param->BeginGuiGesture();
		processor->SetParameterValue(DCP_SoundobjectProcessor, SPI_ParamIdx_Y, y);
	}
}

/**
 * Called when the mouse is moved while a button is held down. 
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred
 */
void SoundobjectSlider::mouseDrag(const MouseEvent& e)
{
	float w = static_cast<float>(getLocalBounds().getWidth());
	float h = static_cast<float>(getLocalBounds().getHeight());

	// Get mouse position and scale it between 0 and 1.
	Point<int> pos = e.getPosition();
	float x = jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getX()) / w)));
	float y = 1.0f - jmin<float>(1.0, jmax<float>(0.0, (static_cast<float>(pos.getY()) / h)));

	SoundobjectProcessor* procssor = dynamic_cast<SoundobjectProcessor*>(m_parent);
	if (procssor)
	{
		// Set new X and Y values
		procssor->SetParameterValue(DCP_SoundobjectProcessor, SPI_ParamIdx_X, x);
		procssor->SetParameterValue(DCP_SoundobjectProcessor, SPI_ParamIdx_Y, y);
	}
}

/**
 * Called when the mouse button is released.
 * Reimplemented just to call EndGuiGesture() to inform the host.
 * @param e		Details about the position and status of the mouse event, including the source component in which it occurred
 */
void SoundobjectSlider::mouseUp(const MouseEvent& e)
{
	ignoreUnused(e);

	GestureManagedAudioParameterFloat* param;
	param = dynamic_cast<GestureManagedAudioParameterFloat*>(m_parent->getParameters()[SPI_ParamIdx_X]);
	param->EndGuiGesture();

	param = dynamic_cast<GestureManagedAudioParameterFloat*>(m_parent->getParameters()[SPI_ParamIdx_Y]);
	param->EndGuiGesture();
}


} // namespace SpaConBridge
