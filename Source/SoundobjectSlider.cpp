/* Copyright (c) 2020-2023, Christian Ahrens
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


SoundobjectSlider::SoundobjectSlider()
{
}

SoundobjectSlider::~SoundobjectSlider()
{
}

void SoundobjectSlider::AddListener(Listener* listener)
{
	m_listeners.add(listener);
}

void SoundobjectSlider::RemoveListener(Listener* listener)
{
	m_listeners.remove(listener);
}

const juce::Point<float>& SoundobjectSlider::GetSoundobjectPos()
{
	return m_soundobjectPos;
}

void SoundobjectSlider::SetSoundobjectPos(const juce::Point<float>& pos, juce::NotificationType notify)
{
	m_soundobjectPos = pos;

	if (juce::sendNotification == notify)
		m_listeners.call([=](Listener& l) { l.sliderValueChanged(this); });

	repaint();
}

void SoundobjectSlider::paint(Graphics& g)
{
	auto w = getLocalBounds().getWidth();
	auto h = getLocalBounds().getHeight();
    
    auto areaColour = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    auto sliderColour = getLookAndFeel().findColour(Slider::thumbColourId);
        
    // Surface area
    g.setColour(areaColour);
    g.fillRect(0, 0, w, h);

    // X knob position
    auto xPoint = static_cast<float>(m_soundobjectPos.getX() * w);

    // Y knob position
    auto yPoint = h - (static_cast<float>(m_soundobjectPos.getY() * h));

    // Paint knob
    float knobSize = 10;
    Path knobOutline;
    knobOutline.addEllipse(xPoint - (knobSize / 2), yPoint - (knobSize / 2), knobSize, knobSize);

    g.setColour(areaColour);
    g.fillPath(knobOutline);
    g.setColour(sliderColour);
    g.strokePath(knobOutline, PathStrokeType(3)); // Stroke width

}

void SoundobjectSlider::mouseDown(const MouseEvent& e)
{
	m_listeners.call([=](Listener& l) { l.sliderDragStarted(this); });

	SetSoundobjectPos(CalcSoundobjectPosFromMousePos(e.getPosition().toFloat()), juce::sendNotification);
}

void SoundobjectSlider::mouseDrag(const MouseEvent& e)
{
	SetSoundobjectPos(CalcSoundobjectPosFromMousePos(e.getPosition().toFloat()), juce::sendNotification);
}

void SoundobjectSlider::mouseUp(const MouseEvent& e)
{
	SetSoundobjectPos(CalcSoundobjectPosFromMousePos(e.getPosition().toFloat()), juce::sendNotification);

	m_listeners.call([=](Listener& l) { l.sliderDragEnded(this); });
}

const juce::Point<float> SoundobjectSlider::CalcSoundobjectPosFromMousePos(const juce::Point<float>& mousePos)
{
	auto fBounds = getLocalBounds().toFloat();
	if (fBounds.isEmpty()) // avoid zerodivison below by sanity checking bounds
		return { 0.0f,0.0f };

	// Get mouse position and scale it between 0 and 1.
	auto x = juce::jlimit<float>(0.0f, 1.0f, mousePos.getX() / fBounds.getWidth());
	auto y = 1.0f - juce::jlimit<float>(0.0f, 1.0f, mousePos.getY() / fBounds.getHeight());

	return { x, y };
}


} // namespace SpaConBridge
