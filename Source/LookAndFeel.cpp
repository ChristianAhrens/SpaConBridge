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

#include "LookAndFeel.h"

namespace SpaConBridge
{

DbLookAndFeelBase::DbLookAndFeelBase()
{
}

DbLookAndFeelBase::~DbLookAndFeelBase()
{
}

void DbLookAndFeelBase::InitColours()
{
	setColour(ResizableWindow::backgroundColourId, GetDbColor(DbColor::MidColor));

	setColour(TextEditor::backgroundColourId, GetDbColor(DbColor::DarkColor));
	setColour(TextEditor::textColourId, GetDbColor(DbColor::TextColor));
	setColour(TextEditor::highlightColourId, GetDbColor(DbColor::HighlightColor));
	setColour(TextEditor::highlightedTextColourId, GetDbColor(DbColor::TextColor));
	setColour(TextEditor::outlineColourId, GetDbColor(DbColor::WindowColor));
	setColour(TextEditor::focusedOutlineColourId, GetDbColor(DbColor::LightColor));
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
	setColour(TextButton::buttonOnColourId, GetDbColor(DbColor::HighlightColor)); // this applies for DrawableButton as well
	setColour(TextButton::textColourOffId, GetDbColor(DbColor::TextColor));
	setColour(TextButton::textColourOnId, GetDbColor(DbColor::TextColor));

	setColour(DrawableButton::textColourId, GetDbColor(DbColor::TextColor));
	setColour(DrawableButton::textColourOnId, GetDbColor(DbColor::TextColor));
	setColour(DrawableButton::backgroundColourId, GetDbColor(DbColor::MidColor));
	setColour(DrawableButton::backgroundOnColourId, GetDbColor(DbColor::HighlightColor));

	setColour(ToggleButton::textColourId, GetDbColor(DbColor::TextColor));
	setColour(ToggleButton::tickColourId, GetDbColor(DbColor::TextColor));
	setColour(ToggleButton::tickDisabledColourId, GetDbColor(DbColor::DarkTextColor));

	setColour(ListBox::backgroundColourId, GetDbColor(DbColor::MidColor));
	setColour(ListBox::outlineColourId, GetDbColor(DbColor::DarkLineColor));
	setColour(ListBox::textColourId, GetDbColor(DbColor::TextColor));

	setColour(TableHeaderComponent::textColourId, GetDbColor(DbColor::TextColor));
	setColour(TableHeaderComponent::backgroundColourId, GetDbColor(DbColor::MidColor));
	setColour(TableHeaderComponent::outlineColourId, GetDbColor(DbColor::WindowColor));
	setColour(TableHeaderComponent::highlightColourId, GetDbColor(DbColor::HighlightColor));

	setColour(ScrollBar::backgroundColourId, GetDbColor(DbColor::MidColor));
	setColour(ScrollBar::thumbColourId, GetDbColor(DbColor::DarkTextColor));
	setColour(ScrollBar::trackColourId, GetDbColor(DbColor::MidColor));

	setColour(TableListBox::backgroundColourId, GetDbColor(DbColor::MidColor));
	setColour(TableListBox::outlineColourId, GetDbColor(DbColor::WindowColor));
	setColour(TableListBox::textColourId, GetDbColor(DbColor::TextColor));

	setColour(CodeEditorComponent::backgroundColourId, GetDbColor(DbColor::MidColor));
	setColour(CodeEditorComponent::defaultTextColourId, GetDbColor(DbColor::TextColor));
	setColour(CodeEditorComponent::highlightColourId, GetDbColor(DbColor::HighlightColor));
	setColour(CodeEditorComponent::lineNumberBackgroundId, GetDbColor(DbColor::LightColor));
	setColour(CodeEditorComponent::lineNumberTextId, GetDbColor(DbColor::DarkTextColor));

	setColour(Slider::backgroundColourId, GetDbColor(DbColor::DarkColor));
	setColour(Slider::rotarySliderFillColourId, GetDbColor(DbColor::DarkColor));
	setColour(Slider::rotarySliderOutlineColourId, GetDbColor(DbColor::DarkLineColor));
	setColour(Slider::textBoxBackgroundColourId, GetDbColor(DbColor::DarkColor));
	setColour(Slider::textBoxHighlightColourId, GetDbColor(DbColor::HighlightColor));
	setColour(Slider::textBoxOutlineColourId, GetDbColor(DbColor::WindowColor));
	setColour(Slider::textBoxTextColourId, GetDbColor(DbColor::TextColor));
	setColour(Slider::thumbColourId, GetDbColor(DbColor::ThumbColor));
	setColour(Slider::trackColourId, GetDbColor(DbColor::MidColor));

	setColour(Label::textColourId, GetDbColor(DbColor::TextColor));
	setColour(Label::textWhenEditingColourId, GetDbColor(DbColor::TextColor));
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
	case ThumbColor:
		return Colour(135, 135, 135);
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


LightDbLookAndFeel::LightDbLookAndFeel()
{
	InitColours();
}

LightDbLookAndFeel::~LightDbLookAndFeel()
{
}

Colour LightDbLookAndFeel::GetDbColor(DbColor color)
{
	switch (color)
	{
	case WindowColor:
		return Colour(102, 102, 102);
	case DarkLineColor:
		return Colour(250, 250, 250);
	case DarkColor:
		return Colour(242, 242, 242);
	case MidColor:
		return Colour(230, 230, 230);
	case ButtonColor:
		return Colour(197, 197, 197);
	case ThumbColor:
		return Colour(187, 187, 187);
	case LightColor:
		return Colour(49, 49, 49);
	case TextColor:
		return Colour(0, 0, 0);
	case DarkTextColor:
		return Colour(70, 70, 70);
	case HighlightColor:
		return Colour(255, 217, 115);
	case FaderGreenColor:
		return Colour(140, 180, 90);
	case ButtonBlueColor:
		return Colour(50, 155, 205);
	case ButtonRedColor:
		return Colour(230, 0, 0);
	default:
		break;
	}

	jassertfalse;
	return Colours::black;
}

}
