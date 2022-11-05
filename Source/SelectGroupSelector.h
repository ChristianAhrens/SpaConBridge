/* Copyright (c) 2022, Christian Ahrens
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


namespace SpaConBridge
{


/**
 * SelectGroupSelector class provides a dropdown ui component that allows
 * selection of object select groups, managed by SelectionManager.
 */
class SelectGroupSelector  : public ComboBox
{
public:
	class SelectGroupSelectorListener
	{
	public:
		//==============================================================================
		/** Destructor. */
		virtual ~SelectGroupSelectorListener() = default;

		//==============================================================================
		/** Called when the selectors selected item is changed.
		*/
		virtual void selectionChanged(int selected) = 0;
	};

public:
	SelectGroupSelector(const String& componentName);
	~SelectGroupSelector() override;

	void SetListener(SelectGroupSelectorListener* listener);

	//==============================================================================
	//void sliderValueChanged(Slider* slider) override;

	//==============================================================================
	void lookAndFeelChanged() override;

	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

	//==============================================================================
	//static constexpr int _Min = 33;
	//static constexpr int _Max = 66;
	//static constexpr int _Interval = 11;

private:
	//std::unique_ptr<Drawable>	m_arrowComponent;	// the drawable arrow icon
	//std::unique_ptr<Slider>		m_slider;			// the slider component

	SelectGroupSelectorListener* m_listener{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SelectGroupSelector)
};


} // namespace SpaConBridge
