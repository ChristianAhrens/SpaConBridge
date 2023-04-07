/* Copyright (c) 2023, Christian Ahrens
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


#include "RangeEditorComponent.h"

#include "LookAndFeel.h"


namespace SpaConBridge
{


/*
===============================================================================
 Class RangeEditorComponent
===============================================================================
*/

/**
 * Object constructor.
 */
RangeEditorComponent::RangeEditorComponent()
	: RangeEditorComponent("")
{
}

/**
 * Object constructor.
 */
RangeEditorComponent::RangeEditorComponent(float minVal, float maxVal, const juce::String& minValLabel, const juce::String& maxValLabel)
	: RangeEditorComponent()
{
	SetRange(minVal, maxVal);
	SetRangeLabels(minValLabel, maxValLabel);
}

/**
 * Object constructor.
 */
RangeEditorComponent::RangeEditorComponent(const String& componentName)
	: Component(componentName)
{
	m_minValEditor = std::make_unique<TextEditor>();
	m_minValEditor->setInputFilter(new FloatValueInputFilter, true);
	m_minValEditor->addListener(this);
	addAndMakeVisible(m_minValEditor.get());

	m_minValLabel = std::make_unique<Label>("min");
	m_minValLabel->attachToComponent(m_minValEditor.get(), true);
	addAndMakeVisible(m_minValLabel.get());

	m_maxValEditor = std::make_unique<TextEditor>();
	m_maxValEditor->setInputFilter(new FloatValueInputFilter, true);
	m_maxValEditor->addListener(this);
	addAndMakeVisible(m_maxValEditor.get());

	m_maxValLabel = std::make_unique<Label>("max");
	m_maxValLabel->attachToComponent(m_maxValEditor.get(), true);
	addAndMakeVisible(m_maxValLabel.get());

	lookAndFeelChanged();
}

/**
 * Object destructor.
 */
RangeEditorComponent::~RangeEditorComponent()
{
}

/**
 * Setter method for the internal listener object member.
 * This listener will be notified of changes made on the
 * value of the internal slider member.
 * @param	listener	The new listner object to set as internal listener memeber object pointer.
 */
void RangeEditorComponent::SetListener(RangeEditorComponent::Listener* listener)
{
	m_listener = listener;
}

/**
 * Setter for the current range values of the two internal text editor objects.
 * @param	minVal	The value for the min val text editor object
 * @param	maxVal	The value for the max val text editor object
 */
void RangeEditorComponent::SetRange(float minVal, float maxVal)
{
	if (m_minValEditor)
		m_minValEditor->setText(juce::String(minVal));
	if (m_maxValEditor)
		m_maxValEditor->setText(juce::String(maxVal));

	UpdateTextEditorValues();
}

/**
 * Getter for the current range value (combined min+max values in the internal text editors).
 * @return	The float range currently set in the two internal text editor objects.
 */
const juce::Range<float> RangeEditorComponent::GetRange()
{
	juce::Range<float> currentRange{ 0.0f, 0.0f };

	if (m_minValEditor)
		currentRange.setStart(m_minValEditor->getText().getFloatValue());
	if (m_maxValEditor)
		currentRange.setEnd(m_maxValEditor->getText().getFloatValue());

	return currentRange;
}

/**
 * Setter for the label strings to be shown in the two label objects
 * that are attached to the two internal text editor objects as user hint.
 * @param	minValLabel		The string to show alongside the min val texteditor
 * @param	maxValLabel		The string to show alongside the max val texteditor
 */
void RangeEditorComponent::SetRangeLabels(const juce::String& minValLabel, const juce::String& maxValLabel)
{
	if (m_minValLabel)
		m_minValLabel->setText(minValLabel, juce::dontSendNotification);
	if (m_maxValLabel)
		m_maxValLabel->setText(maxValLabel, juce::dontSendNotification);
}

/**
 * Setter for the suffix to be shown in the two label objects
 * that are attached to the two internal text editor objects as user hint.
 * @param	suffix		The suffix string to show after the values in texteditors
 */
void RangeEditorComponent::SetRangeValueSuffix(const juce::String& suffix)
{
	m_valueSuffix = suffix;

	UpdateTextEditorValues();
}

/**
 * Reimplemented to handle changed range values
 * @param	editor	The texteditor (min or max) that has encountered a return key press
 */
void RangeEditorComponent::textEditorReturnKeyPressed(TextEditor& editor)
{
	if ((m_minValEditor.get() == &editor || m_maxValEditor.get() == &editor) && m_listener)
	{
		m_listener->rangeChanged(this);
	}
}

/**
 * Reimplemented to handle changed range values
 * @param	editor	The texteditor (min or max) that has lost focus
 */
void RangeEditorComponent::textEditorFocusLost(TextEditor& editor)
{
	// identical processing as when return key was pressed
	textEditorReturnKeyPressed(editor);
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void RangeEditorComponent::resized()
{
	auto bounds = getLocalBounds();
	auto quarterWidth = bounds.getWidth() / 4;

	if (m_minValLabel)
		m_minValLabel->setBounds(bounds.removeFromLeft(quarterWidth));
	if (m_minValEditor)
		m_minValEditor->setBounds(bounds.removeFromLeft(quarterWidth));
	if (m_maxValLabel)
		m_maxValLabel->setBounds(bounds.removeFromLeft(quarterWidth));
	if (m_maxValEditor)
		m_maxValEditor->setBounds(bounds);
}

/**
 * Private helper to append the value suffix to the text shown in the editors.
 */
void RangeEditorComponent::UpdateTextEditorValues()
{
	if (m_minValEditor)
		m_minValEditor->setText(m_minValEditor->getText() + " " + m_valueSuffix);
	if (m_maxValEditor)
		m_maxValEditor->setText(m_maxValEditor->getText() + " " + m_valueSuffix);
}


} // namespace SpaConBridge
