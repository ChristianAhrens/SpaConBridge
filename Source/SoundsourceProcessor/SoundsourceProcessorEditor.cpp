/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file is part of the Soundscape VST, AU, and AAX Plug-in.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY d&b audiotechnik GmbH & Co. KG "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

===============================================================================
*/


#include "SoundsourceProcessorEditor.h"

#include "SoundsourceProcessor.h"
#include "Parameters.h"

#include "../Overview/Overview.h"


namespace SoundscapeBridgeApp
{


/**
 * Rate at which the GUI will refresh, after parameter changes have been detected.
 * 33 ms translates to about 30 frames per second.
 */
static constexpr int GUI_UPDATE_RATE_FAST = 33;

/**
 * Rate at which the GUI will refresh, when no parameter changes have taken place for a while.
 */
static constexpr int GUI_UPDATE_RATE_SLOW = 120;

/**
 * After this number of timer callbacks without parameter changes, the timer will switch to GUI_UPDATE_RATE_SLOW.
 */
static constexpr int GUI_UPDATE_DELAY_TICKS = 15;


/*
===============================================================================
 Class SoundsourceProcessorEditor
===============================================================================
*/

/**
 * Object constructor.
 * This is the base class for the component that acts as the GUI for an AudioProcessor.
 * @param parent	The audio processor object to act as parent.
 */
SoundsourceProcessorEditor::SoundsourceProcessorEditor(SoundsourceProcessor& parent)
	: AudioProcessorEditor(&parent)
{
	m_surfaceSlider = std::make_unique<CSurfaceSlider>(&parent);
	m_surfaceSlider->setWantsKeyboardFocus(true);
	addAndMakeVisible(m_surfaceSlider.get());

	const Array<AudioProcessorParameter*>& params = parent.getParameters();
	if (params.size() >= 2)
	{
		//--- X Slider ---//
		AudioParameterFloat* param = dynamic_cast<AudioParameterFloat*> (params[ParamIdx_X]);
		m_xSlider = std::make_unique<Slider>(param->name);
		m_xSlider->setRange(param->range.start, param->range.end, param->range.interval);
		m_xSlider->setSliderStyle(Slider::LinearHorizontal);
		m_xSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
		m_xSlider->addListener(this);
		addAndMakeVisible(m_xSlider.get());

		// Label for X slider
		m_xAxisLabel = std::make_unique<Label>(param->name, param->name);
		addAndMakeVisible(m_xAxisLabel.get());

		//--- Y Slider ---//
		param = dynamic_cast<AudioParameterFloat*> (params[ParamIdx_Y]);
		m_ySlider = std::make_unique<Slider>(param->name);
		m_ySlider->setRange(param->range.start, param->range.end, param->range.interval);
		m_ySlider->setSliderStyle(Slider::LinearVertical);
		m_ySlider->setTextBoxStyle(Slider::TextBoxLeft, false, 80, 20);
		m_ySlider->addListener(this);
		addAndMakeVisible(m_ySlider.get());

		// Label for Y slider
		m_yAxisLabel = std::make_unique<Label>(param->name, param->name);
		addAndMakeVisible(m_yAxisLabel.get());

		if (params.size() == ParamIdx_MaxIndex)
		{
			//--- ReverbSendGain Slider ---//
			param = dynamic_cast<AudioParameterFloat*> (params[ParamIdx_ReverbSendGain]);
			m_reverbSendGainSlider = std::make_unique<CKnob>(param->name);
			m_reverbSendGainSlider->setRange(param->range.start, param->range.end, param->range.interval);
			m_reverbSendGainSlider->setSliderStyle(Slider::Rotary);
			m_reverbSendGainSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
			m_reverbSendGainSlider->addListener(this);
			addAndMakeVisible(m_reverbSendGainSlider.get());

			// Label for ReverbSendGain
			m_reverbSendGainLabel = std::make_unique<Label>(param->name, param->name);
			addAndMakeVisible(m_reverbSendGainLabel.get());

			//--- SourceSpread Slider ---//
			param = dynamic_cast<AudioParameterFloat*> (params[ParamIdx_SourceSpread]);
			m_sourceSpreadSlider = std::make_unique<CKnob>(param->name);
			m_sourceSpreadSlider->setRange(param->range.start, param->range.end, param->range.interval);
			m_sourceSpreadSlider->setSliderStyle(Slider::Rotary);
			m_sourceSpreadSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
			m_sourceSpreadSlider->addListener(this);
			addAndMakeVisible(m_sourceSpreadSlider.get());

			// Label for SourceSpread
			m_sourceSpreadLabel = std::make_unique<Label>(param->name, param->name);
			addAndMakeVisible(m_sourceSpreadLabel.get());

			//--- DelayMode ComboBox ---//
			AudioParameterChoice* choiceParam = dynamic_cast<AudioParameterChoice*> (params[ParamIdx_DelayMode]);
			m_delayModeComboBox = std::make_unique<ComboBox>(choiceParam->name);
			m_delayModeComboBox->setEditableText(false);
			m_delayModeComboBox->addItem("Off",	1);
			m_delayModeComboBox->addItem("Tight", 2);
			m_delayModeComboBox->addItem("Full", 3);
			m_delayModeComboBox->setColour(ComboBox::backgroundColourId, CDbStyle::GetDbColor(CDbStyle::DarkColor));
			m_delayModeComboBox->setColour(ComboBox::textColourId, CDbStyle::GetDbColor(CDbStyle::TextColor));
			m_delayModeComboBox->setColour(ComboBox::outlineColourId, CDbStyle::GetDbColor(CDbStyle::WindowColor));
			m_delayModeComboBox->setColour(ComboBox::buttonColourId, CDbStyle::GetDbColor(CDbStyle::MidColor));
			m_delayModeComboBox->setColour(ComboBox::arrowColourId, CDbStyle::GetDbColor(CDbStyle::TextColor));
			m_delayModeComboBox->addListener(this);
			addAndMakeVisible(m_delayModeComboBox.get());

			// Label for DelayMode
			m_delayModeLabel = std::make_unique<Label>(choiceParam->name, choiceParam->name);
			addAndMakeVisible(m_delayModeLabel.get());
		}
	}

	// Label for Plugin' display name.
	m_displayNameLabel = std::make_unique<Label>("DisplayName");
	m_displayNameLabel->setJustificationType(Justification(Justification::centredLeft));
	m_displayNameLabel->setColour(Label::textColourId, CDbStyle::GetDbColor(CDbStyle::DarkTextColor));
	addAndMakeVisible(m_displayNameLabel.get());

	// Start GUI-refreshing timer.
	startTimer(GUI_UPDATE_RATE_FAST);

	setSize(20, 20);
}

/**
 * Object destructor.
 */
SoundsourceProcessorEditor::~SoundsourceProcessorEditor()
{
	stopTimer();

	processor.editorBeingDeleted(this);
}

/**
 * Helper function to get the pointer to a plugin parameter based on the slider assigned to it.
 * @param slider	The slider object for which the parameter is desired.
 * @return			The desired plugin parameter.
 */
CAudioParameterFloat* SoundsourceProcessorEditor::GetParameterForSlider(Slider* slider)
{
	const Array<AudioProcessorParameter*>& params = getAudioProcessor()->getParameters();
	if (slider == m_xSlider.get())
		return dynamic_cast<CAudioParameterFloat*> (params[ParamIdx_X]);
	else if (slider == m_ySlider.get())
		return dynamic_cast<CAudioParameterFloat*> (params[ParamIdx_Y]);
	else if (slider == m_reverbSendGainSlider.get())
		return dynamic_cast<CAudioParameterFloat*> (params[ParamIdx_ReverbSendGain]);
	else if (slider == m_sourceSpreadSlider.get())
		return dynamic_cast<CAudioParameterFloat*> (params[ParamIdx_SourceSpread]);

	// Should not make it this far.
	jassertfalse;
	return nullptr;
}

/**
 * Callback function for changes to our sliders. Called when the slider's value is changed.
 * This may be caused by dragging it, or by typing in its text entry box, or by a call to Slider::setValue().
 * You can find out the new value using Slider::getValue().
 * @param slider	Slider object which was dragged by user.
 */
void SoundsourceProcessorEditor::sliderValueChanged(Slider* slider)
{
	SoundsourceProcessor* plugin = dynamic_cast<SoundsourceProcessor*>(getAudioProcessor());
	if (plugin)
	{
		AutomationParameterIndex paramIdx = ParamIdx_MaxIndex;
		if (slider == m_xSlider.get())
			paramIdx = ParamIdx_X;
		else if (slider == m_ySlider.get())
			paramIdx = ParamIdx_Y;
		else if (slider == m_reverbSendGainSlider.get())
			paramIdx = ParamIdx_ReverbSendGain;
		else if (slider == m_sourceSpreadSlider.get())
			paramIdx = ParamIdx_SourceSpread;

		plugin->SetParameterValue(DCS_Gui, paramIdx, static_cast<float>(slider->getValue()));
	}
}

/**
 * Called when the slider is about to be dragged.
 * This is called when a drag begins, then it's followed by multiple calls to sliderValueChanged(),
 * and then sliderDragEnded() is called after the user lets go.
 * @param slider	Slider object which was dragged by user.
 */
void SoundsourceProcessorEditor::sliderDragStarted(Slider* slider)
{
	if (CAudioParameterFloat* param = GetParameterForSlider(static_cast<Slider*>(slider)))
		param->BeginGuiGesture();
}

/**
 * Called after a drag operation has finished.
 * @param slider	Slider object which was dragged by user.
 */
void SoundsourceProcessorEditor::sliderDragEnded(Slider* slider)
{
	if (CAudioParameterFloat* param = GetParameterForSlider(static_cast<Slider*>(slider)))
		param->EndGuiGesture();
}

/**
 * Callback function for Enter key presses on textEditors.
 * @param textEditor	The TextEditor object whose where enter key was pressed.
 */
void SoundsourceProcessorEditor::textEditorReturnKeyPressed(TextEditor& textEditor)
{
	ignoreUnused(textEditor);

	// Remove keyboard focus from this editor. 
	// Function textEditorFocusLost will then take care of setting values.
	m_surfaceSlider->grabKeyboardFocus();
}

/**
 * Called when a ComboBox has its selected item changed. 
 * @param comboBox	The combo box which has changed.
 */
void SoundsourceProcessorEditor::comboBoxChanged(ComboBox *comboBox)
{
	SoundsourceProcessor* pro = dynamic_cast<SoundsourceProcessor*>(getAudioProcessor());
	if (pro)
	{
		if (comboBox == m_delayModeComboBox.get())
		{
			pro->SetParameterValue(DCS_Gui, ParamIdx_DelayMode, float(comboBox->getSelectedId() - 1));
		}
	}
}

/**
* Method which gets called when a region of a component needs redrawing, either because the
* component's repaint() method has been called, or because something has happened on the
* screen that means a section of a window needs to be redrawn.
* @param g		Graphics context that must be used to do the drawing operations.
*/
void SoundsourceProcessorEditor::paint(Graphics& g)
{
	Rectangle<int> twoDSurfaceArea = getLocalBounds();
	Rectangle<int> parameterEditArea = getLocalBounds();
	getResizePaintAreaSplit(twoDSurfaceArea, parameterEditArea);

	// Background of 2D slider area
	g.setColour(CDbStyle::GetDbColor(CDbStyle::DarkColor));
	g.fillRect(twoDSurfaceArea);

	// Background of parameter edit elements
	g.setColour(CDbStyle::GetDbColor(CDbStyle::MidColor));
	g.fillRect(parameterEditArea);
	
	// Black frame
	g.setColour(CDbStyle::GetDbColor(CDbStyle::DarkLineColor));
	g.drawRect(getLocalBounds().toFloat());
}

/**
 * Minimal helper method to get the areas for parameter edits
 * and 2D surface slider.
 * This is to avoid code clones in paint and resize methods.
 * @param twoDSurfaceArea	The area to be used for 2D surface controls
 * @param parameterEditArea	The area to be used for parameter controls
 * @return	True if the layout is to be done in portrait, false if in landscape orientation
 */
bool SoundsourceProcessorEditor::getResizePaintAreaSplit(Rectangle<int>& twoDSurfaceArea, Rectangle<int>& parameterEditArea)
{
	auto paramEditStripWidth = 90;
	auto paramEditStripHeight = 105;
	auto isPortrait = getLocalBounds().getHeight() > getLocalBounds().getWidth();

	if (isPortrait)
	{
		twoDSurfaceArea.removeFromBottom(paramEditStripHeight);
		parameterEditArea.removeFromTop(twoDSurfaceArea.getHeight());
	}
	else
	{
		twoDSurfaceArea.removeFromRight(paramEditStripWidth);
		parameterEditArea.removeFromLeft(twoDSurfaceArea.getWidth());
	}

	return isPortrait;
}

/**
* Called when this component's size has been changed.
* This is generally where you'll want to lay out the positions of any subcomponents in your editor.
*/
void SoundsourceProcessorEditor::resized()
{
	//==============================================================================
	Rectangle<int> twoDSurfaceArea = getLocalBounds();
	Rectangle<int> parameterEditArea = getLocalBounds();
	auto isPortrait = getResizePaintAreaSplit(twoDSurfaceArea, parameterEditArea);

	//==============================================================================
	bool surfaceSliderLabelVisible = true;
	if (twoDSurfaceArea.getWidth() < 250 || twoDSurfaceArea.getHeight() < 250)
		surfaceSliderLabelVisible = false;
	auto xSliderStripWidth = surfaceSliderLabelVisible ? 80 : 30;
	auto ySliderStripWidth = surfaceSliderLabelVisible ? 100 : 30;
	twoDSurfaceArea.reduce(5, 5);
	twoDSurfaceArea.removeFromTop(surfaceSliderLabelVisible ? 30 : 10);
	twoDSurfaceArea.removeFromRight(surfaceSliderLabelVisible ? 30 : 10);

	// Y Slider
	auto ySliderBounds = twoDSurfaceArea;
	ySliderBounds.removeFromRight(twoDSurfaceArea.getWidth() - ySliderStripWidth);
	ySliderBounds.removeFromBottom(xSliderStripWidth);
	m_ySlider->setBounds(ySliderBounds);
	m_ySlider->setTextBoxStyle(surfaceSliderLabelVisible ? Slider::TextBoxLeft : Slider::NoTextBox, false, 80, 20);
	ySliderBounds.removeFromTop(50);
	ySliderBounds.removeFromRight(30);
	m_yAxisLabel->setBounds(ySliderBounds);
	m_yAxisLabel->setVisible(surfaceSliderLabelVisible);

	// 2D Surface
	auto surfaceSliderBounds = twoDSurfaceArea;
	surfaceSliderBounds.removeFromLeft(ySliderStripWidth);
	surfaceSliderBounds.removeFromBottom(xSliderStripWidth);
	m_surfaceSlider->setBounds(surfaceSliderBounds);
	
	// X Slider
	auto xSliderBounds = twoDSurfaceArea;
	xSliderBounds.removeFromTop(twoDSurfaceArea.getHeight() - xSliderStripWidth);
	xSliderBounds.removeFromLeft(ySliderStripWidth);
	m_xSlider->setBounds(xSliderBounds.removeFromTop(50));
	m_xSlider->setTextBoxStyle(surfaceSliderLabelVisible ? Slider::TextBoxBelow : Slider::NoTextBox, false, 80, 20);
	m_xAxisLabel->setBounds(xSliderBounds);
	m_xAxisLabel->setVisible(surfaceSliderLabelVisible);


	//==============================================================================
	bool paramSliderLabelVisible = true;
	if (parameterEditArea.getHeight() < 265 && !isPortrait)
		paramSliderLabelVisible = false;
	auto labelHeight = 25;
	auto sliderHeight = paramSliderLabelVisible ? 75 : 55;
	auto labelSliderWidth = 72;

	if (isPortrait)
	{
		auto parameterEditsWidth = 260;
		auto hPos = 0.5f * (parameterEditArea.getWidth() - parameterEditsWidth);
		auto vPos = (getLocalBounds().getHeight() - (labelHeight + sliderHeight));

		// ReverbSendGain Slider
		m_reverbSendGainLabel->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, labelHeight));
		m_reverbSendGainSlider->setBounds(Rectangle<int>(hPos, vPos + 18, labelSliderWidth, sliderHeight));
        m_reverbSendGainSlider->setTextBoxStyle(paramSliderLabelVisible ? Slider::TextBoxBelow : Slider::NoTextBox, false, 80, 20);
		hPos += 85;

		// SourceSpread Slider
		m_sourceSpreadLabel->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, labelHeight));
		m_sourceSpreadSlider->setBounds(Rectangle<int>(hPos, vPos + 18, labelSliderWidth, sliderHeight));
        m_sourceSpreadSlider->setTextBoxStyle(paramSliderLabelVisible ? Slider::TextBoxBelow : Slider::NoTextBox, false, 80, 20);
		hPos += 85;

		// DelayMode ComboBox
		m_delayModeLabel->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, labelHeight));
		m_delayModeComboBox->setBounds(Rectangle<int>(hPos, vPos + 45, labelSliderWidth, labelHeight));
		hPos += 85;
	}
	else
	{
		auto parameterEditsHeight = paramSliderLabelVisible ? 250 : 190;
		auto hPos = getLocalBounds().getWidth() - 80;
		auto vPos = 0.5f *(getLocalBounds().getHeight() - parameterEditsHeight);
	
		// ReverbSendGain Slider
		m_reverbSendGainLabel->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, labelHeight));
		vPos += 18;
		m_reverbSendGainSlider->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, sliderHeight));
		m_reverbSendGainSlider->setTextBoxStyle(paramSliderLabelVisible ? Slider::TextBoxBelow : Slider::NoTextBox, false, 80, 20);
		vPos += paramSliderLabelVisible ? 86 : 56;
	
		// SourceSpread Slider
		m_sourceSpreadLabel->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, labelHeight));
		vPos += 18;
		m_sourceSpreadSlider->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, sliderHeight));
		m_sourceSpreadSlider->setTextBoxStyle(paramSliderLabelVisible ? Slider::TextBoxBelow : Slider::NoTextBox, false, 80, 20);
		vPos += paramSliderLabelVisible ? 86 : 56;
	
		// DelayMode ComboBox
		m_delayModeLabel->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, labelHeight));
		vPos += 25;
		m_delayModeComboBox->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, labelHeight));
		vPos += 86;
	}
}

/**
 * Timer callback function, which will be called at regular intervals to update the GUI.
 * Reimplemented from base class Timer.
 */
void SoundsourceProcessorEditor::timerCallback()
{
	// Also update the regular GUI.
	UpdateGui(false);
}

/**
 * Update GUI elements with the current parameter values.
 * @param init	True to ignore any changed flags and update parameters
 *				in the GUI anyway. Good for when opening the GUI for the first time.
 */
void SoundsourceProcessorEditor::UpdateGui(bool init)
{
	ignoreUnused(init); // No need to use this here so far.

	bool somethingChanged = false;

	SoundsourceProcessor* pro = dynamic_cast<SoundsourceProcessor*>(getAudioProcessor());
	if (pro)
	{
		const Array<AudioProcessorParameter*>& params = pro->getParameters();
		AudioParameterFloat* fParam;

		// See if any parameters changed since the last timer callback.
		somethingChanged = (pro->GetParameterChanged(DCS_Gui, DCT_AutomationParameters) ||
							pro->GetParameterChanged(DCS_Gui, DCT_PluginInstanceConfig) ||
							pro->GetParameterChanged(DCS_Gui, DCT_OscConfig));

		if (pro->PopParameterChanged(DCS_Gui, DCT_SourcePosition))
		{
			// Update position of X slider.
			fParam = dynamic_cast<AudioParameterFloat*>(params[ParamIdx_X]);
			if (fParam)
				m_xSlider->setValue(fParam->get(), dontSendNotification);

			// Update position of Y slider.
			fParam = dynamic_cast<AudioParameterFloat*>(params[ParamIdx_Y]);
			if (fParam)
				m_ySlider->setValue(fParam->get(), dontSendNotification);

			// Update the nipple position on the 2D-Slider.
			m_surfaceSlider->repaint();
		}

		if (pro->PopParameterChanged(DCS_Gui, DCT_ReverbSendGain))
		{
			// Update ReverbSendGain slider
			fParam = dynamic_cast<AudioParameterFloat*>(params[ParamIdx_ReverbSendGain]);
			if (fParam)
				m_reverbSendGainSlider->setValue(fParam->get(), dontSendNotification);
		}

		if (pro->PopParameterChanged(DCS_Gui, DCT_SourceSpread))
		{
			// Update SourceSpread slider
			fParam = dynamic_cast<AudioParameterFloat*>(params[ParamIdx_SourceSpread]);
			if (fParam)
				m_sourceSpreadSlider->setValue(fParam->get(), dontSendNotification);
		}

		if (pro->PopParameterChanged(DCS_Gui, DCT_DelayMode))
		{
			// Update DelayMode combo box
			AudioParameterChoice* cParam = dynamic_cast<AudioParameterChoice*>(params[ParamIdx_DelayMode]);
			if (cParam)
			{
				// Need to add 1 because the parameter's indeces go from 0 to 2, while the combo box's ID's go from 1 to 3.
				m_delayModeComboBox->setSelectedId(cParam->getIndex() + 1, dontSendNotification);
			}
		}

		if (pro->PopParameterChanged(DCS_Gui, DCT_SourceID))
		{
			// Update the displayName (Host probably called updateTrackProperties or changeProgramName)
			m_displayNameLabel->setText(pro->getProgramName(0), dontSendNotification);
		}
	}

	if (somethingChanged)
	{
		// At least one parameter was changed -> reset counter to prevent switching to "slow" refresh rate too soon.
		m_ticksSinceLastChange = 0;

		// Parameters have changed in the plugin: Switch to frequent GUI refreshing rate
		if (getTimerInterval() == GUI_UPDATE_RATE_SLOW)
		{
			startTimer(GUI_UPDATE_RATE_FAST);
			DBG("SoundsourceProcessorEditor::timerCallback: Switching to GUI_UPDATE_RATE_FAST");
		}
	}

	else
	{
		// No parameter changed since last timer callback -> increase counter.
		if (m_ticksSinceLastChange < GUI_UPDATE_DELAY_TICKS)
			m_ticksSinceLastChange++;

		// Once counter has reached a certain limit: Switch to lazy GUI refreshing rate
		else if (getTimerInterval() == GUI_UPDATE_RATE_FAST)
		{
			DBG("SoundsourceProcessorEditor::timerCallback(): Switching to GUI_UPDATE_RATE_SLOW");
			startTimer(GUI_UPDATE_RATE_SLOW);
		}
	}
}


} // namespace SoundscapeBridgeApp
