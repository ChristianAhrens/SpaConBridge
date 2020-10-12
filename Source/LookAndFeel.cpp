/*
  ==============================================================================

    LookAndFeel.cpp
    Created: 12 Oct 2020 9:07:00pm
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "LookAndFeel.h"

namespace SoundscapeBridgeApp
{

DbLookAndFeelBase::DbLookAndFeelBase()
{
}

DbLookAndFeelBase::~DbLookAndFeelBase()
{
}

void DbLookAndFeelBase::InitColours()
{
	setColour(ColourScheme::windowBackground, GetDbColor(DbColor::MidColor));
	setColour(ColourScheme::widgetBackground, GetDbColor(DbColor::DarkColor));
	setColour(ColourScheme::menuBackground, GetDbColor(DbColor::DarkColor));
	setColour(ColourScheme::outline, GetDbColor(DbColor::ButtonColor));
	setColour(ColourScheme::defaultText, GetDbColor(DbColor::TextColor));
	setColour(ColourScheme::defaultFill, GetDbColor(DbColor::MidColor));
	setColour(ColourScheme::highlightedText, GetDbColor(DbColor::TextColor));
	setColour(ColourScheme::highlightedFill, GetDbColor(DbColor::HighlightColor));
	setColour(ColourScheme::menuText, GetDbColor(DbColor::TextColor));

	setColour(ResizableWindow::backgroundColourId, GetDbColor(DbColor::MidColor));

	setColour(TextEditor::backgroundColourId, GetDbColor(DbColor::DarkColor));
	setColour(TextEditor::textColourId, GetDbColor(DbColor::TextColor));
	setColour(TextEditor::highlightColourId, GetDbColor(DbColor::HighlightColor));
	setColour(TextEditor::highlightedTextColourId, GetDbColor(DbColor::TextColor));
	setColour(TextEditor::outlineColourId, GetDbColor(DbColor::WindowColor));
	setColour(TextEditor::focusedOutlineColourId, GetDbColor(DbColor::WindowColor));
	setColour(TextEditor::shadowColourId, GetDbColor(DbColor::MidColor).darker());

	setColour(ComboBox::arrowColourId, GetDbColor(DbColor::TextColor));
	setColour(ComboBox::backgroundColourId, GetDbColor(DbColor::DarkColor));
	setColour(ComboBox::buttonColourId, GetDbColor(DbColor::MidColor));
	setColour(ComboBox::focusedOutlineColourId, GetDbColor(DbColor::WindowColor));
	setColour(ComboBox::outlineColourId, GetDbColor(DbColor::WindowColor));
	setColour(ComboBox::textColourId, GetDbColor(DbColor::TextColor));

	setColour(PopupMenu::backgroundColourId, GetDbColor(DbColor::MidColor));
	setColour(PopupMenu::textColourId, GetDbColor(DbColor::TextColor));
	setColour(PopupMenu::headerTextColourId, GetDbColor(DbColor::TextColor));
	setColour(PopupMenu::highlightedBackgroundColourId, GetDbColor(DbColor::HighlightColor));
	setColour(PopupMenu::highlightedTextColourId, GetDbColor(DbColor::TextColor));

	setColour(TextButton::buttonColourId, GetDbColor(DbColor::ButtonColor)); // this applies for DrawableButton as well
	setColour(TextButton::buttonOnColourId, GetDbColor(DbColor::ButtonColor).brighter()); // this applies for DrawableButton as well
	setColour(TextButton::textColourOffId, GetDbColor(DbColor::TextColor));
	setColour(TextButton::textColourOnId, GetDbColor(DbColor::TextColor));

	setColour(DrawableButton::textColourId, GetDbColor(DbColor::TextColor));
	setColour(DrawableButton::textColourOnId, GetDbColor(DbColor::TextColor));
	setColour(DrawableButton::backgroundColourId, GetDbColor(DbColor::MidColor));
	setColour(DrawableButton::backgroundOnColourId, GetDbColor(DbColor::HighlightColor));

	setColour(ListBox::backgroundColourId, GetDbColor(DbColor::MidColor));
	setColour(ListBox::outlineColourId, GetDbColor(DbColor::DarkLineColor));
	setColour(ListBox::textColourId, GetDbColor(DbColor::TextColor));

	setColour(TableHeaderComponent::textColourId, GetDbColor(DbColor::TextColor));
	setColour(TableHeaderComponent::backgroundColourId, GetDbColor(DbColor::MidColor));
	setColour(TableHeaderComponent::outlineColourId, GetDbColor(DbColor::DarkLineColor));
	setColour(TableHeaderComponent::highlightColourId, GetDbColor(DbColor::HighlightColor));

	setColour(ScrollBar::backgroundColourId, GetDbColor(DbColor::MidColor));
	setColour(ScrollBar::thumbColourId, GetDbColor(DbColor::DarkTextColor));
	setColour(ScrollBar::trackColourId, GetDbColor(DbColor::MidColor));

	setColour(TableListBox::backgroundColourId, GetDbColor(DbColor::MidColor));
	setColour(TableListBox::outlineColourId, GetDbColor(DbColor::DarkLineColor));
	setColour(TableListBox::textColourId, GetDbColor(DbColor::TextColor));

	setColour(CodeEditorComponent::backgroundColourId, GetDbColor(DbColor::MidColor));
	setColour(CodeEditorComponent::defaultTextColourId, GetDbColor(DbColor::TextColor));
	setColour(CodeEditorComponent::highlightColourId, GetDbColor(DbColor::HighlightColor));
	setColour(CodeEditorComponent::lineNumberBackgroundId, GetDbColor(DbColor::LightColor));
	setColour(CodeEditorComponent::lineNumberTextId, GetDbColor(DbColor::DarkTextColor));

	setColour(Slider::ColourIds::backgroundColourId, GetDbColor(DbColor::DarkColor));
	setColour(Slider::ColourIds::rotarySliderFillColourId, GetDbColor(DbColor::DarkColor));
	setColour(Slider::ColourIds::rotarySliderOutlineColourId, GetDbColor(DbColor::DarkLineColor));
	setColour(Slider::ColourIds::textBoxBackgroundColourId, GetDbColor(DbColor::DarkColor));
	setColour(Slider::ColourIds::textBoxHighlightColourId, GetDbColor(DbColor::HighlightColor));
	setColour(Slider::ColourIds::textBoxOutlineColourId, GetDbColor(DbColor::WindowColor));
	setColour(Slider::ColourIds::textBoxTextColourId, GetDbColor(DbColor::TextColor));
	setColour(Slider::ColourIds::thumbColourId, GetDbColor(DbColor::ButtonColor));
	setColour(Slider::ColourIds::trackColourId, GetDbColor(DbColor::MidColor));
}

void DbLookAndFeelBase::drawButtonBackground(Graphics& g,
    Button& button,
    const Colour& backgroundColour,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
	auto cornerSize = 2.0f;
	auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

	auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
		.withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

	if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
		baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

	g.setColour(baseColour);

	auto flatOnLeft = button.isConnectedOnLeft();
	auto flatOnRight = button.isConnectedOnRight();
	auto flatOnTop = button.isConnectedOnTop();
	auto flatOnBottom = button.isConnectedOnBottom();

	if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
	{
		Path path;
		path.addRoundedRectangle(bounds.getX(), bounds.getY(),
			bounds.getWidth(), bounds.getHeight(),
			cornerSize, cornerSize,
			!(flatOnLeft || flatOnTop),
			!(flatOnRight || flatOnTop),
			!(flatOnLeft || flatOnBottom),
			!(flatOnRight || flatOnBottom));

		g.fillPath(path);

		g.setColour(button.findColour(ComboBox::outlineColourId));
		g.strokePath(path, PathStrokeType(1.0f));
	}
	else
	{
		g.fillRoundedRectangle(bounds, cornerSize);

		g.setColour(button.findColour(ComboBox::outlineColourId));
		g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
	}
}


DarkDbLookAndFeel::DarkDbLookAndFeel()
{
	InitColours();
}

DarkDbLookAndFeel::~DarkDbLookAndFeel()
{
}

Colour DarkDbLookAndFeel::GetDbColor(DbColor color)
{
	switch (color)
	{
	case WindowColor:
		return Colour(27, 27, 27);
	case DarkLineColor:
		return Colour(49, 49, 49);
	case DarkColor:
		return Colour(67, 67, 67);
	case MidColor:
		return Colour(83, 83, 83);
	case ButtonColor:
		return Colour(125, 125, 125);
	case LightColor:
		return Colour(201, 201, 201);
	case TextColor:
		return Colour(238, 238, 238);
	case DarkTextColor:
		return Colour(180, 180, 180);
	case HighlightColor:
		return Colour(115, 140, 155);
	case FaderGreenColor:
		return Colour(140, 180, 90);
	case ButtonBlueColor:
		return Colour(27, 120, 163);
	case ButtonRedColor:
		return Colour(226, 41, 41);
	default:
		break;
	}

	jassertfalse;
	return Colours::black;
}

}
