/* Copyright (c) 2020-2022, Christian Ahrens
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


#include "MatrixInputProcessorEditor.h"

#include "MatrixInputProcessor.h"

#include "../../Controller.h"
#include "../Parameters.h"

#include "../../PagedUI/PageContainerComponent.h"

#include <Image_utils.h>


namespace SpaConBridge
{


/*
===============================================================================
 Class MatrixInputProcessorEditor
===============================================================================
*/

/**
 * Object constructor.
 * This is the base class for the component that acts as the GUI for an AudioProcessor.
 * @param parent	The audio processor object to act as parent.
 */
MatrixInputProcessorEditor::MatrixInputProcessorEditor(MatrixInputProcessor& parent)
	: AudioProcessorEditor(&parent)
{
	const Array<AudioProcessorParameter*>& params = parent.getParameters();
	if (params.size() == 3)
	{
		auto fparam = dynamic_cast<AudioParameterFloat*> (params[MII_ParamIdx_LevelMeterPreMute]);
		m_MatrixInputLevelMeterSlider = std::make_unique<LevelMeterSlider>(fparam->name, LevelMeterSlider::LMM_ReadOnly);
		m_MatrixInputLevelMeterSlider->setRange(fparam->range.start, fparam->range.end, fparam->range.interval);
		m_MatrixInputLevelMeterSlider->setValue(fparam->get(), dontSendNotification);
		m_MatrixInputLevelMeterSlider->addListener(this);
		addAndMakeVisible(m_MatrixInputLevelMeterSlider.get());

		fparam = dynamic_cast<AudioParameterFloat*> (params[MII_ParamIdx_Gain]);
		m_MatrixInputGainSlider = std::make_unique<Slider>(fparam->name);
		m_MatrixInputGainSlider->setRange(fparam->range.start, fparam->range.end, fparam->range.interval);
		m_MatrixInputGainSlider->setValue(fparam->get(), dontSendNotification);
		m_MatrixInputGainSlider->setSliderStyle(Slider::LinearHorizontal);
		m_MatrixInputGainSlider->setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
		m_MatrixInputGainSlider->addListener(this);
		addAndMakeVisible(m_MatrixInputGainSlider.get());

		auto iparam = dynamic_cast<AudioParameterInt*> (params[MII_ParamIdx_Mute]);
		m_MatrixInputMuteButton = std::make_unique<DrawableButton>(iparam->name, DrawableButton::ButtonStyle::ImageOnButtonBackground);
		m_MatrixInputMuteButton->setClickingTogglesState(true);
		m_MatrixInputMuteButton->setToggleState(iparam->get() == 1 ? true : false, dontSendNotification);
		m_MatrixInputMuteButton->setButtonText("Mute");
		m_MatrixInputMuteButton->addListener(this);
		addAndMakeVisible(m_MatrixInputMuteButton.get());

		lookAndFeelChanged();
	}
	
	// Start GUI-refreshing timer.
	startTimer(GUI_UPDATE_RATE_FAST);

	setSize(20, 20);
}

/**
 * Object destructor.
 */
MatrixInputProcessorEditor::~MatrixInputProcessorEditor()
{
	stopTimer();

	processor.editorBeingDeleted(this);
}

/**
 * Helper getter method for the parent matrixinput processor's input id
 * @return	The matrixinput id of the parent processor or invalid if no parent processor is present.
 */
MatrixInputId MatrixInputProcessorEditor::GetMatrixInputId()
{
	MatrixInputProcessor* pro = dynamic_cast<MatrixInputProcessor*>(getAudioProcessor());
	if (pro)
		return pro->GetMatrixInputId();

	return INVALID_ADDRESS_VALUE;
}

/**
 * Helper method to update the drawables used for buttons to match the text colour
 */
void MatrixInputProcessorEditor::updateDrawableButtonImageColours()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// Update drawable button images with updated lookAndFeel colours
	UpdateDrawableButtonImages(m_MatrixInputMuteButton, BinaryData::volume_off24px_svg, &getLookAndFeel());

	// determine the right red colour from lookandfeel
	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (dblookAndFeel)
	{
		auto redColour = dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::ButtonRedColor);

		// set the images to button
		m_MatrixInputMuteButton->setColour(TextButton::ColourIds::buttonOnColourId, redColour.brighter(0.05f));
	}
}

/**
 * Reimplemented from component to update button drawables correctly
 */
void MatrixInputProcessorEditor::lookAndFeelChanged()
{
	Component::lookAndFeelChanged();
	updateDrawableButtonImageColours();

	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (dblookAndFeel)
	{
		// special handling for gain fader, since we have to deal with the same cell background colour as the slider track background
		m_MatrixInputGainSlider->setColour(Slider::backgroundColourId, dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkColor).darker());
		m_MatrixInputGainSlider->setColour(Slider::trackColourId, dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkColor).darker());
		m_MatrixInputGainSlider->setColour(Slider::thumbColourId, dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::ThumbColor).brighter());
	}
}

/**
 * Helper function to get the pointer to a procssor parameter based on the slider assigned to it.
 * @param slider	The slider object for which the parameter is desired.
 * @return			The desired procssor parameter.
 */
GestureManagedAudioParameterFloat* MatrixInputProcessorEditor::GetParameterForSlider(Slider* slider)
{
	auto const& params = getAudioProcessor()->getParameters();
	if (slider == m_MatrixInputLevelMeterSlider.get())
		return dynamic_cast<GestureManagedAudioParameterFloat*> (params[MII_ParamIdx_LevelMeterPreMute]);
	else if (slider == m_MatrixInputGainSlider.get())
		return dynamic_cast<GestureManagedAudioParameterFloat*> (params[MII_ParamIdx_Gain]);
	
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
void MatrixInputProcessorEditor::sliderValueChanged(Slider* slider)
{
	auto miProcessor = dynamic_cast<MatrixInputProcessor*>(getAudioProcessor());
	if (miProcessor)
	{
		auto paramIdx = MII_ParamIdx_MaxIndex;
		if (slider == m_MatrixInputLevelMeterSlider.get())
			paramIdx = MII_ParamIdx_LevelMeterPreMute;
		else if (slider == m_MatrixInputGainSlider.get())
			paramIdx = MII_ParamIdx_Gain;
	
		miProcessor->SetParameterValue(DCP_MatrixInputProcessor, paramIdx, static_cast<float>(slider->getValue()));
	}
}

/**
 * Called when the slider is about to be dragged.
 * This is called when a drag begins, then it's followed by multiple calls to sliderValueChanged(),
 * and then sliderDragEnded() is called after the user lets go.
 * @param slider	Slider object which was dragged by user.
 */
void MatrixInputProcessorEditor::sliderDragStarted(Slider* slider)
{
	if (GestureManagedAudioParameterFloat* param = GetParameterForSlider(static_cast<Slider*>(slider)))
		param->BeginGuiGesture();
}

/**
 * Called after a drag operation has finished.
 * @param slider	Slider object which was dragged by user.
 */
void MatrixInputProcessorEditor::sliderDragEnded(Slider* slider)
{
	if (GestureManagedAudioParameterFloat* param = GetParameterForSlider(static_cast<Slider*>(slider)))
		param->EndGuiGesture();
}

/**
 * Callback function for button clicks on buttons.
 * @param button	The Button object that was pressed.
 */
void MatrixInputProcessorEditor::buttonClicked(Button* button)
{
	auto miProcessor = dynamic_cast<MatrixInputProcessor*>(getAudioProcessor());
	if (miProcessor)
	{
		auto paramIdx = MII_ParamIdx_MaxIndex;
		if (button == m_MatrixInputMuteButton.get())
			paramIdx = MII_ParamIdx_Mute;

		miProcessor->SetParameterValue(DCP_MatrixInputProcessor, paramIdx, static_cast<float>(button->getToggleState() ? 1 : 0));
	}
}

/**
* Called when this component's size has been changed.
* This is generally where you'll want to lay out the positions of any subcomponents in your editor.
*/
void MatrixInputProcessorEditor::resized()
{
	auto margin = 2;
	auto bounds = getLocalBounds();
	bounds.removeFromBottom(1);
	bounds.reduce(margin, margin);

	auto muteBounds = bounds.removeFromLeft(bounds.getHeight()).reduced(margin);
	m_MatrixInputMuteButton->setBounds(muteBounds);

	auto meterBounds = bounds.removeFromTop(static_cast<int>(0.35f * bounds.getHeight())).reduced(margin);
	m_MatrixInputLevelMeterSlider->setBounds(meterBounds);

	auto gainBounds = bounds.reduced(margin);
	m_MatrixInputGainSlider->setBounds(gainBounds);
}

/**
 * Timer callback function, which will be called at regular intervals to update the GUI.
 * Reimplemented from base class Timer.
 */
void MatrixInputProcessorEditor::timerCallback()
{
	// Also update the regular GUI.
	UpdateGui(false);
}

/**
 * Update GUI elements with the current parameter values.
 * @param init	True to ignore any changed flags and update parameters
 *				in the GUI anyway. Good for when opening the GUI for the first time.
 */
void MatrixInputProcessorEditor::UpdateGui(bool init)
{
	ignoreUnused(init); // No need to use this here so far.

	bool somethingChanged = false;

	MatrixInputProcessor* pro = dynamic_cast<MatrixInputProcessor*>(getAudioProcessor());
	if (pro)
	{
		const Array<AudioProcessorParameter*>& params = pro->getParameters();
		AudioParameterFloat* fParam;
		AudioParameterInt* iParam;

		// See if any parameters changed since the last timer callback.
		somethingChanged = (pro->GetParameterChanged(DCP_MatrixInputProcessor, DCT_MatrixInputParameters) ||
							pro->GetParameterChanged(DCP_MatrixInputProcessor, DCT_MatrixInputProcessorConfig) ||
							pro->GetParameterChanged(DCP_MatrixInputProcessor, DCT_CommunicationConfig));

		if (m_MatrixInputLevelMeterSlider && pro->PopParameterChanged(DCP_MatrixInputProcessor, DCT_MatrixInputLevelMeter))
		{
			// Update level meter.
			fParam = dynamic_cast<AudioParameterFloat*>(params[MII_ParamIdx_LevelMeterPreMute]);
			if (fParam)
				m_MatrixInputLevelMeterSlider->setValue(fParam->get(), dontSendNotification);
		}

		if (m_MatrixInputGainSlider && pro->PopParameterChanged(DCP_MatrixInputProcessor, DCT_MatrixInputGain))
		{
			// Update gain slider
			fParam = dynamic_cast<AudioParameterFloat*>(params[MII_ParamIdx_Gain]);
			if (fParam)
				m_MatrixInputGainSlider->setValue(fParam->get(), dontSendNotification);
		}

		if (m_MatrixInputMuteButton && pro->PopParameterChanged(DCP_MatrixInputProcessor, DCT_MatrixInputMute))
		{
			// Update mute button
			iParam = dynamic_cast<AudioParameterInt*>(params[MII_ParamIdx_Mute]);
			if (iParam)
				m_MatrixInputMuteButton->setToggleState(iParam->get() == Mute_On, dontSendNotification);
		}
	}

	if (somethingChanged)
	{
		// At least one parameter was changed -> reset counter to prevent switching to "slow" refresh rate too soon.
		m_ticksSinceLastChange = 0;

		// Parameters have changed in the procssor: Switch to frequent GUI refreshing rate
		if (getTimerInterval() == GUI_UPDATE_RATE_SLOW)
		{
			startTimer(GUI_UPDATE_RATE_FAST);
			DBG("MatrixInputProcessorEditor::timerCallback: Switching to GUI_UPDATE_RATE_FAST");
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
			DBG("MatrixInputProcessorEditor::timerCallback(): Switching to GUI_UPDATE_RATE_SLOW");
			startTimer(GUI_UPDATE_RATE_SLOW);
		}
	}
}


} // namespace SpaConBridge
