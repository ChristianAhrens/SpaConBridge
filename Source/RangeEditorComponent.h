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


#pragma once

#include <JuceHeader.h>


namespace SpaConBridge
{


/**
 * RangeEditorComponent class provides a component that ....
 */
class RangeEditorComponent  : public Component, public TextEditor::Listener
{
public:
	class Listener
	{
	public:
		//==============================================================================
		/** Destructor. */
		virtual ~Listener() = default;

		//==============================================================================
		/** Called when the RangeEditorComponent's value is changed.
		*/
		virtual void rangeChanged(RangeEditorComponent* editor) = 0;
	};

	class FloatValueInputFilter : public juce::TextEditor::InputFilter
	{
		juce::String filterNewText(TextEditor& , const String& newInput) override
		{
			if (newInput.containsOnly("-.,0123456789"))
				return newInput.replaceCharacters(",", ".");
			else
				return juce::String();
		}
	};

public:
	explicit RangeEditorComponent();
	explicit RangeEditorComponent(const String& componentName);
	explicit RangeEditorComponent(float minVal, float maxVal, const juce::String& minValLabel, const juce::String& maxValLabel);
	~RangeEditorComponent() override;

	void SetListener(RangeEditorComponent::Listener* listener);
	void SetRange(float minVal, float maxVal);
	const juce::Range<float> GetRange();
	void SetRangeLabels(const juce::String& minValLabel, const juce::String& maxValLabel);

	//==============================================================================
	void textEditorReturnKeyPressed(TextEditor& editor) override;
	void textEditorFocusLost(TextEditor& editor) override;

	//==============================================================================
	void resized() override;

private:
	std::unique_ptr<Label>		m_minValLabel;
	std::unique_ptr<TextEditor>	m_minValEditor;
	std::unique_ptr<Label>		m_maxValLabel;
	std::unique_ptr<TextEditor>	m_maxValEditor;

	RangeEditorComponent::Listener* m_listener{ nullptr };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RangeEditorComponent)
};


} // namespace SpaConBridge
