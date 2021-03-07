/* Copyright (c) 2020-2021, Christian Ahrens
 *
 * This file is part of SoundscapeBridgeApp <https://github.com/ChristianAhrens/SoundscapeBridgeApp>
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


namespace SoundscapeBridgeApp
{


/**
 * RowHeightSlider class provides a component that a slider that automatically 
 * switches from linear horizontal to vertical. It also features a Drawable to
 * show a hint to the user on meaning of the slider.
 */
class RowHeightSlider  : public Component, public Slider::Listener
{
public:
	class RowHeightListener
	{
	public:
		//==============================================================================
		/** Destructor. */
		virtual ~RowHeightListener() = default;

		//==============================================================================
		/** Called when the slider member's value is changed.
		*/
		virtual void rowHeightChanged(int rowHeight) = 0;
	};

public:
	RowHeightSlider(const String& componentName);
	~RowHeightSlider() override;

	void SetListener(RowHeightListener* listener);
	void SetSliderRange(double min, double max, double interval);

	//==============================================================================
	void sliderValueChanged(Slider* slider) override;

	//==============================================================================
	void lookAndFeelChanged() override;

	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

private:
	std::unique_ptr<Drawable>	m_arrowComponent;	// the drawable arrow icon
	std::unique_ptr<Slider>		m_slider;			// the slider component

	RowHeightListener* m_listener{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RowHeightSlider)
};


} // namespace SoundscapeBridgeApp
