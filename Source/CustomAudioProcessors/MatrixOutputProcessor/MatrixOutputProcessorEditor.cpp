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


#include "MatrixOutputProcessorEditor.h"

#include "MatrixOutputProcessor.h"

#include "../../Controller.h"
#include "../Parameters.h"

#include "../../PagedUI/PageContainerComponent.h"

#include <Image_utils.h>


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
 Class MatrixOutputProcessorEditor
===============================================================================
*/

/**
 * Object constructor.
 * This is the base class for the component that acts as the GUI for an AudioProcessor.
 * @param parent	The audio processor object to act as parent.
 */
MatrixOutputProcessorEditor::MatrixOutputProcessorEditor(MatrixOutputProcessor& parent)
	: AudioProcessorEditor(&parent)
{
	const Array<AudioProcessorParameter*>& params = parent.getParameters();
	if (params.size() == 3)
	{
		auto fparam = dynamic_cast<AudioParameterFloat*> (params[MOI_ParamIdx_LevelMeterPostMute]);
		m_MatrixOutputLevelMeterSlider = std::make_unique<Slider>(fparam->name);
		m_MatrixOutputLevelMeterSlider->setRange(fparam->range.start, fparam->range.end, fparam->range.interval);
		m_MatrixOutputLevelMeterSlider->setSliderStyle(Slider::SliderStyle::LinearBar);
		m_MatrixOutputLevelMeterSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
		m_MatrixOutputLevelMeterSlider->addListener(this);
		addAndMakeVisible(m_MatrixOutputLevelMeterSlider.get());

		fparam = dynamic_cast<AudioParameterFloat*> (params[MOI_ParamIdx_Gain]);
		m_MatrixOutputGainSlider = std::make_unique<Slider>(fparam->name);
		m_MatrixOutputGainSlider->setRange(fparam->range.start, fparam->range.end, fparam->range.interval);
		m_MatrixOutputGainSlider->setSliderStyle(Slider::SliderStyle::LinearBar);
		m_MatrixOutputGainSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
		m_MatrixOutputGainSlider->addListener(this);
		addAndMakeVisible(m_MatrixOutputGainSlider.get());

		auto iparam = dynamic_cast<AudioParameterInt*> (params[MOI_ParamIdx_Mute]);
		m_MatrixOutputMuteButton = std::make_unique<DrawableButton>(iparam->name, DrawableButton::ButtonStyle::ImageOnButtonBackground);
		m_MatrixOutputMuteButton->setButtonText("Mute");
		m_MatrixOutputMuteButton->addListener(this);
		addAndMakeVisible(m_MatrixOutputMuteButton.get());

		lookAndFeelChanged();
	}

	// Start GUI-refreshing timer.
	startTimer(GUI_UPDATE_RATE_FAST);

	setSize(20, 20);
}

/**
 * Object destructor.
 */
MatrixOutputProcessorEditor::~MatrixOutputProcessorEditor()
{
	stopTimer();

	processor.editorBeingDeleted(this);
}

/**
 * Helper method to update the drawables used for buttons to match the text colour
 */
void MatrixOutputProcessorEditor::updateDrawableButtonImageColours()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (!dblookAndFeel)
		return;

	// create the required button drawable images based on lookandfeel colours
	String imageName = BinaryData::mobiledata_off24px_svg;
	std::unique_ptr<juce::Drawable> NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage;
	JUCEAppBasics::Image_utils::getDrawableButtonImages(imageName, NormalImage, OverImage, DownImage, DisabledImage, NormalOnImage, OverOnImage, DownOnImage, DisabledOnImage,
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkTextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::DarkLineColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor),
		dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));

	// determine the right red colour from lookandfeel
	auto redColour = dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::ButtonRedColor);

	// set the images to button
	m_MatrixOutputMuteButton->setColour(TextButton::ColourIds::buttonOnColourId, redColour.brighter(0.05f));
	m_MatrixOutputMuteButton->setImages(NormalImage.get(), OverImage.get(), DownImage.get(), DisabledImage.get(), NormalOnImage.get(), OverOnImage.get(), DownOnImage.get(), DisabledOnImage.get());
}

/**
 * Reimplemented from component to update button drawables correctly
 */
void MatrixOutputProcessorEditor::lookAndFeelChanged()
{
	Component::lookAndFeelChanged();
	updateDrawableButtonImageColours();
}

/**
 * Helper function to get the pointer to a procssor parameter based on the slider assigned to it.
 * @param slider	The slider object for which the parameter is desired.
 * @return			The desired procssor parameter.
 */
GestureManagedAudioParameterFloat* MatrixOutputProcessorEditor::GetParameterForSlider(Slider* slider)
{
	auto const& params = getAudioProcessor()->getParameters();
	if (slider == m_MatrixOutputLevelMeterSlider.get())
		return dynamic_cast<GestureManagedAudioParameterFloat*> (params[MOI_ParamIdx_LevelMeterPostMute]);
	else if (slider == m_MatrixOutputGainSlider.get())
		return dynamic_cast<GestureManagedAudioParameterFloat*> (params[MOI_ParamIdx_Gain]);
	
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
void MatrixOutputProcessorEditor::sliderValueChanged(Slider* slider)
{
	auto moProcessor = dynamic_cast<MatrixOutputProcessor*>(getAudioProcessor());
	if (moProcessor)
	{
		auto paramIdx = MOI_ParamIdx_MaxIndex;
		if (slider == m_MatrixOutputLevelMeterSlider.get())
			paramIdx = MOI_ParamIdx_LevelMeterPostMute;
		else if (slider == m_MatrixOutputGainSlider.get())
			paramIdx = MOI_ParamIdx_Gain;

		moProcessor->SetParameterValue(DCS_MatrixOutputProcessor, paramIdx, static_cast<float>(slider->getValue()));
	}
}

/**
 * Called when the slider is about to be dragged.
 * This is called when a drag begins, then it's followed by multiple calls to sliderValueChanged(),
 * and then sliderDragEnded() is called after the user lets go.
 * @param slider	Slider object which was dragged by user.
 */
void MatrixOutputProcessorEditor::sliderDragStarted(Slider* slider)
{
	if (GestureManagedAudioParameterFloat* param = GetParameterForSlider(static_cast<Slider*>(slider)))
		param->BeginGuiGesture();
}

/**
 * Called after a drag operation has finished.
 * @param slider	Slider object which was dragged by user.
 */
void MatrixOutputProcessorEditor::sliderDragEnded(Slider* slider)
{
	if (GestureManagedAudioParameterFloat* param = GetParameterForSlider(static_cast<Slider*>(slider)))
		param->EndGuiGesture();
}

/**
 * Callback function for button clicks on buttons.
 * @param button	The Button object that was pressed.
 */
void MatrixOutputProcessorEditor::buttonClicked(Button* button)
{
	ignoreUnused(button);

	// Remove keyboard focus from this editor. 
	// Function textEditorFocusLost will then take care of setting values.
	//m_surfaceSlider->grabKeyboardFocus();
}

/**
* Method which gets called when a region of a component needs redrawing, either because the
* component's repaint() method has been called, or because something has happened on the
* screen that means a section of a window needs to be redrawn.
* @param g		Graphics context that must be used to do the drawing operations.
*/
void MatrixOutputProcessorEditor::paint(Graphics& g)
{
	//Rectangle<int> twoDSurfaceArea = getLocalBounds();
	//Rectangle<int> parameterEditArea = getLocalBounds();
	//getResizePaintAreaSplit(twoDSurfaceArea, parameterEditArea);
	//
	//// Background of 2D slider area
	//g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	//g.fillRect(twoDSurfaceArea);
	//
	//// Background of parameter edit elements
	//g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId));
	//g.fillRect(parameterEditArea);
	
	//// Frame
	//g.setColour(getLookAndFeel().findColour(TableListBox::outlineColourId));
	//g.drawRect(getLocalBounds().toFloat());

	g.setColour(Colours::blue);
	g.fillRect(getLocalBounds().toFloat());
    
    //// processor id (object #) in upper left corner
    //MatrixOutputProcessor* pro = dynamic_cast<MatrixOutputProcessor*>(getAudioProcessor());
    //if (pro)
    //{
    //    auto surfaceSliderLabelVisible = true;
    //    if (twoDSurfaceArea.getWidth() < 250 || twoDSurfaceArea.getHeight() < 250)
    //        surfaceSliderLabelVisible = false;
    //    
    //    auto objNumTitleText = (surfaceSliderLabelVisible ? String("Object #") : String("#")) + String(pro->GetMatrixOutputId());
    //    auto titleTextWidth = surfaceSliderLabelVisible ? 73 : 33;
    //    auto objNumTitleRect = twoDSurfaceArea.removeFromBottom(25).removeFromLeft(titleTextWidth + 7).removeFromRight(titleTextWidth);
    //    
    //    g.setColour(getLookAndFeel().findColour(TableListBox::textColourId));
    //    g.drawText(objNumTitleText, objNumTitleRect, Justification::centredLeft);
    //}
}

///**
// * Minimal helper method to get the areas for parameter edits
// * and 2D surface slider.
// * This is to avoid code clones in paint and resize methods.
// * @param twoDSurfaceArea	The area to be used for 2D surface controls
// * @param parameterEditArea	The area to be used for parameter controls
// * @return	True if the layout is to be done in portrait, false if in landscape orientation
// */
//bool MatrixOutputProcessorEditor::getResizePaintAreaSplit(Rectangle<int>& twoDSurfaceArea, Rectangle<int>& parameterEditArea)
//{
//	auto paramEditStripWidth = 90;
//	auto paramEditStripHeight = 105;
//	auto isPortrait = getLocalBounds().getHeight() > getLocalBounds().getWidth();
//
//	if (isPortrait)
//	{
//		twoDSurfaceArea.removeFromBottom(paramEditStripHeight);
//		parameterEditArea.removeFromTop(twoDSurfaceArea.getHeight());
//	}
//	else
//	{
//		twoDSurfaceArea.removeFromRight(paramEditStripWidth);
//		parameterEditArea.removeFromLeft(twoDSurfaceArea.getWidth());
//	}
//
//	return isPortrait;
//}

/**
* Called when this component's size has been changed.
* This is generally where you'll want to lay out the positions of any subcomponents in your editor.
*/
void MatrixOutputProcessorEditor::resized()
{
	auto margin = 2;
	auto bounds = getLocalBounds();

	auto muteBounds = bounds.removeFromLeft(bounds.getHeight()).reduced(margin);
	m_MatrixOutputMuteButton->setBounds(muteBounds);

	auto meterBounds = bounds.removeFromTop(bounds.getHeight() / 2).reduced(margin);
	m_MatrixOutputLevelMeterSlider->setBounds(meterBounds);

	auto gainBounds = bounds.reduced(margin);
	m_MatrixOutputGainSlider->setBounds(gainBounds);
}

/**
 * Timer callback function, which will be called at regular intervals to update the GUI.
 * Reimplemented from base class Timer.
 */
void MatrixOutputProcessorEditor::timerCallback()
{
	// Also update the regular GUI.
	UpdateGui(false);
}

/**
 * Update GUI elements with the current parameter values.
 * @param init	True to ignore any changed flags and update parameters
 *				in the GUI anyway. Good for when opening the GUI for the first time.
 */
void MatrixOutputProcessorEditor::UpdateGui(bool init)
{
	ignoreUnused(init); // No need to use this here so far.

	bool somethingChanged = false;

	MatrixOutputProcessor* pro = dynamic_cast<MatrixOutputProcessor*>(getAudioProcessor());
	if (pro)
	{
		const Array<AudioProcessorParameter*>& params = pro->getParameters();
		AudioParameterFloat* fParam;
		AudioParameterInt* iParam;

		// See if any parameters changed since the last timer callback.
		somethingChanged = (pro->GetParameterChanged(DCS_MatrixOutputProcessor, DCT_MatrixOutputParameters) ||
							pro->GetParameterChanged(DCS_MatrixOutputProcessor, DCT_ProcessorInstanceConfig) ||
							pro->GetParameterChanged(DCS_MatrixOutputProcessor, DCT_CommunicationConfig));

		if (pro->PopParameterChanged(DCS_MatrixOutputProcessor, DCT_MatrixOutputLevelMeter))
		{
			// Update level meter.
			fParam = dynamic_cast<AudioParameterFloat*>(params[MOI_ParamIdx_LevelMeterPostMute]);
			if (fParam)
				m_MatrixOutputLevelMeterSlider->setValue(fParam->get(), dontSendNotification);
		}

		if (pro->PopParameterChanged(DCS_MatrixOutputProcessor, DCT_MatrixOutputGain))
		{
			// Update gain slider
			fParam = dynamic_cast<AudioParameterFloat*>(params[MOI_ParamIdx_Gain]);
			if (fParam)
				m_MatrixOutputGainSlider->setValue(fParam->get(), dontSendNotification);
		}

		if (pro->PopParameterChanged(DCS_MatrixOutputProcessor, DCT_MatrixOutputMute))
		{
			// Update mute button
			iParam = dynamic_cast<AudioParameterInt*>(params[MOI_ParamIdx_Mute]);
			if (iParam)
				m_MatrixOutputMuteButton->setToggleState(iParam->get() == Mute_On, dontSendNotification);
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
			DBG("MatrixOutputProcessorEditor::timerCallback: Switching to GUI_UPDATE_RATE_FAST");
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
			DBG("MatrixOutputProcessorEditor::timerCallback(): Switching to GUI_UPDATE_RATE_SLOW");
			startTimer(GUI_UPDATE_RATE_SLOW);
		}
	}
}


} // namespace SoundscapeBridgeApp
