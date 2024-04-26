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


#pragma once

#include <JuceHeader.h>
#include "SpaConBridgeCommon.h"


namespace SpaConBridge
{


/**
 * @class SoundobjectSlider
 * @brief SoundobjectSlider class provides a 2D-Slider or "X/Y controller".
 */
class SoundobjectSlider  : public Component
{
public:
	SoundobjectSlider();
	~SoundobjectSlider() override;
    class Listener
    {
    public:
        virtual ~Listener() = default;

        virtual void sliderValueChanged(SoundobjectSlider* slider) = 0;
        virtual void sliderDragStarted(SoundobjectSlider*) {}
        virtual void sliderDragEnded(SoundobjectSlider*) {}
    };
    void AddListener(Listener* listener);
    void RemoveListener(Listener* listener);
	const juce::Point<float>& GetSoundobjectPos();
	void SetSoundobjectPos(const juce::Point<float>& pos, juce::NotificationType notify = juce::dontSendNotification);
	void paint (Graphics& g) override;
	void mouseDown (const MouseEvent& e) override;
	void mouseDrag (const MouseEvent& e) override;
	void mouseUp (const MouseEvent& e) override;

private:
    const juce::Point<float> CalcSoundobjectPosFromMousePos(const juce::Point<float>& mousePos);

	juce::Point<float> m_soundobjectPos;
    ListenerList<SoundobjectSlider::Listener> m_listeners;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundobjectSlider)
};


} // namespace SpaConBridge
