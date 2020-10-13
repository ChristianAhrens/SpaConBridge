/*
  ==============================================================================

    LookAndFeel.h
    Created: 12 Oct 2020 9:07:00pm
    Author:  Christian Ahrens

  ==============================================================================
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
