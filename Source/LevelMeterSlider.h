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


namespace SpaConBridge
{


/**
 * LevelMeterSlider class provides a read-only slider that uses HorizontalBar Sliderstyle.
 */
class LevelMeterSlider  : public Slider
{
public:
	enum LevelMeterMode
	{
		LMM_Invalid = 0,
		LMM_ReadOnly,
		LMM_Normal
	};

public:
	LevelMeterSlider(const String& componentName, LevelMeterMode mode);
	~LevelMeterSlider() override;

	void mouseDown (const MouseEvent& e) override;
	void mouseDrag (const MouseEvent& e) override;
	void mouseUp (const MouseEvent& e) override;

private:
	LevelMeterMode m_levelMeterMode{ LMM_Invalid };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeterSlider)
};


} // namespace SpaConBridge
