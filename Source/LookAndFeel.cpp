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

	setColour(AlertWindow::textColourId, GetDbColor(DbColor::TextColor));
	setColour(AlertWindow::outlineColourId, GetDbColor(DbColor::WindowColor));
	setColour(AlertWindow::backgroundColourId, GetDbColor(DbColor::DarkColor));

	setColour(ColourSelector::labelTextColourId, GetDbColor(DbColor::TextColor));
	setColour(ColourSelector::backgroundColourId, GetDbColor(DbColor::MidColor));
}

/**
 * Reimplemented nearly code-cloned method of LookAndFeel_V4, to get a custom looking button.
 */
void DbLookAndFeelBase::drawButtonBackground(	Graphics& g,
												Button& button,
												const Colour& backgroundColour,
												bool shouldDrawButtonAsHighlighted,
												bool shouldDrawButtonAsDown)
{
	auto cornerSize = 2.0f;
	auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

	// Original JUCE code uses multiplied saturation also depending on keyboardfocus. This results in ugly colours with our common red and blue buttons, so we eliminate this handling.
	//	auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
	//		.withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

	auto baseColour = backgroundColour.withMultipliedSaturation(0.9f)
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

/**
 * Reimplemented nearly code-cloned method of LookAndFeel_V4, to get a custom looking AlertBox.
 */
void DbLookAndFeelBase::drawAlertBox(	Graphics& g,
										AlertWindow& alert,
										const Rectangle<int>& textArea,
										TextLayout& textLayout)
{
	g.setColour(alert.findColour(AlertWindow::outlineColourId));
	g.drawRect(alert.getLocalBounds().toFloat(), 1.0f);
	
	auto bounds = alert.getLocalBounds().reduced(1);
	g.reduceClipRegion(bounds);
	
	g.setColour(alert.findColour(AlertWindow::backgroundColourId));
	g.fillRect(bounds.toFloat());

	auto iconSpaceUsed = 0;

	auto iconWidth = 80;
	auto iconSize = jmin(iconWidth + 50, bounds.getHeight() + 20);

	if (alert.containsAnyExtraComponents() || alert.getNumButtons() > 2)
		iconSize = jmin(iconSize, textArea.getHeight() + 50);

	Rectangle<int> iconRect(iconSize / -10, iconSize / -10,
		iconSize, iconSize);

	if (alert.getAlertType() != AlertWindow::NoIcon)
	{
		Path icon;
		char character;
		uint32 colour;

		if (alert.getAlertType() == AlertWindow::WarningIcon)
		{
			character = '!';

			icon.addTriangle((float)iconRect.getX() + (float)iconRect.getWidth() * 0.5f, (float)iconRect.getY(),
				static_cast<float> (iconRect.getRight()), static_cast<float> (iconRect.getBottom()),
				static_cast<float> (iconRect.getX()), static_cast<float> (iconRect.getBottom()));

			icon = icon.createPathWithRoundedCorners(5.0f);
			colour = 0x66ff2a00;
		}
		else
		{
			colour = Colour(0xff00b0b9).withAlpha(0.4f).getARGB();
			character = alert.getAlertType() == AlertWindow::InfoIcon ? 'i' : '?';

			icon.addEllipse(iconRect.toFloat());
		}

		GlyphArrangement ga;
		ga.addFittedText({ (float)iconRect.getHeight() * 0.9f, Font::bold },
			String::charToString((juce_wchar)(uint8)character),
			static_cast<float> (iconRect.getX()), static_cast<float> (iconRect.getY()),
			static_cast<float> (iconRect.getWidth()), static_cast<float> (iconRect.getHeight()),
			Justification::centred, false);
		ga.createPath(icon);

		icon.setUsingNonZeroWinding(false);
		g.setColour(Colour(colour));
		g.fillPath(icon);

		iconSpaceUsed = iconWidth;
	}

	g.setColour(alert.findColour(AlertWindow::textColourId));

	Rectangle<int> alertBounds(bounds.getX() + iconSpaceUsed, 30,
		bounds.getWidth(), bounds.getHeight() - getAlertWindowButtonHeight() - 20);

	textLayout.draw(g, alertBounds.toFloat());
}

/**
 * Reimplemented nearly code-cloned method of LookAndFeel_V4 to get a nicer selected/active colour.
 */
void DbLookAndFeelBase::drawStretchableLayoutResizerBar(Graphics& g, int /*w*/, int /*h*/, bool /*isVerticalBar*/,
	bool isMouseOver, bool isMouseDragging)
{
	if (isMouseOver || isMouseDragging)
		g.fillAll(GetDbColor(DbColor::HighlightColor));
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
