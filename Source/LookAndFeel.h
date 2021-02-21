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

class DbLookAndFeelBase : public LookAndFeel_V4
{
public:
	enum LookAndFeelType
	{
		LAFT_InvalidFirst,
		LAFT_DefaultJUCE,
		LAFT_Dark,
		LAFT_Light,
		LAFT_OSdynamic,
		LAFT_InvalidLast
	};
	static String getLookAndFeelName(LookAndFeelType type)
	{
		switch (type)
		{
		case LAFT_DefaultJUCE:
			return "Default JUCE";
		case LAFT_Dark:
			return "Dark";
		case LAFT_Light:
			return "Light";
		case LAFT_OSdynamic:
			return "- dynamic -";
		case LAFT_InvalidFirst:
		case LAFT_InvalidLast:
		default:
			return "INVALID";
		}
	};

	/**
	 * d&b Color codes
	 */
	enum DbColor
	{
		WindowColor,	
		DarkLineColor,	
		DarkColor,
		MidColor,
		ButtonColor,
		ButtonActiveColor,
		ThumbColor,
		LightColor,		
		TextColor,		
		DarkTextColor,	
		HighlightColor,
		FaderGreenColor,
		ButtonBlueColor,
		ButtonRedColor,	
	};

public:
	DbLookAndFeelBase();
	virtual ~DbLookAndFeelBase();

	//==============================================================================
	void InitColours();

	//==============================================================================
	virtual LookAndFeelType GetType() = 0;
	virtual Colour GetDbColor(DbColor color) = 0;

	//==============================================================================
	void drawButtonBackground(Graphics&, Button&, const Colour& backgroundColour,
		bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DbLookAndFeelBase)
};

class DarkDbLookAndFeel : public DbLookAndFeelBase
{

public:
	DarkDbLookAndFeel();
	~DarkDbLookAndFeel() override;

	//==============================================================================
	virtual LookAndFeelType GetType() override { return LAFT_Dark; };
	Colour GetDbColor(DbColor color) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DarkDbLookAndFeel)
};

class LightDbLookAndFeel : public DbLookAndFeelBase
{

public:
	LightDbLookAndFeel();
	~LightDbLookAndFeel() override;

	//==============================================================================
	virtual LookAndFeelType GetType() override { return LAFT_Light; };
	Colour GetDbColor(DbColor color) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LightDbLookAndFeel)
};

}
