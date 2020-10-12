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
	Colour GetDbColor(DbColor color) override;

private:
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DarkDbLookAndFeel)
};

}