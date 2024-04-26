/*
===============================================================================

Copyright (C) 2019 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

This file was originally part of the Soundscape VST, AU, and AAX Plug-in
and now in a derived version is part of SpaConBridge.

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


#include "SoundobjectProcessorEditor.h"

#include "SoundobjectProcessor.h"

#include "../../Controller.h"
#include "../Parameters.h"

#include "../../PagedUI/PageContainerComponent.h"

#include "FixedFontTextEditor.h"


namespace SpaConBridge
{


/*
===============================================================================
 Class SoundobjectProcessorEditor
===============================================================================
*/


bool SoundobjectProcessorEditor::TickTrigger::s_tickHandled{ true };

/**
 * Object constructor.
 * This is the base class for the component that acts as the GUI for an AudioProcessor.
 * @param parent	The audio processor object to act as parent.
 */
SoundobjectProcessorEditor::SoundobjectProcessorEditor(SoundobjectProcessor& parent)
	: AudioProcessorEditor(&parent)
{
	m_soundobjectSlider = std::make_unique<SoundobjectSlider>();
	m_soundobjectSlider->setWantsKeyboardFocus(true);
	m_soundobjectSlider->AddListener(this);
	addAndMakeVisible(m_soundobjectSlider.get());

	const Array<AudioProcessorParameter*>& params = parent.getParameters();
	if (params.size() >= 2)
	{
		//--- X Slider ---//
		AudioParameterFloat* param = dynamic_cast<AudioParameterFloat*> (params[SPI_ParamIdx_X]);
		m_xSlider = std::make_unique<Slider>(param->name);
		m_xSlider->setRange(param->range.start, param->range.end, param->range.interval);
		m_xSlider->setSliderStyle(Slider::LinearHorizontal);
		m_xSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
		m_xSlider->addListener(this);
		addAndMakeVisible(m_xSlider.get());

		// Label for X slider
		m_xAxisLabel = std::make_unique<Label>(param->name, param->name);
		m_xAxisLabel->setJustificationType(Justification::centred);
		addAndMakeVisible(m_xAxisLabel.get());

		//--- Y Slider ---//
		param = dynamic_cast<AudioParameterFloat*> (params[SPI_ParamIdx_Y]);
		m_ySlider = std::make_unique<Slider>(param->name);
		m_ySlider->setRange(param->range.start, param->range.end, param->range.interval);
		m_ySlider->setSliderStyle(Slider::LinearVertical);
		m_ySlider->setTextBoxStyle(Slider::TextBoxLeft, false, 80, 20);
		m_ySlider->addListener(this);
		addAndMakeVisible(m_ySlider.get());

		// Label for Y slider
		m_yAxisLabel = std::make_unique<Label>(param->name, param->name);
		m_yAxisLabel->setJustificationType(Justification::centred);
		addAndMakeVisible(m_yAxisLabel.get());

		if (params.size() == SPI_ParamIdx_MaxIndex)
		{
			//--- ReverbSendGain Slider ---//
			param = dynamic_cast<AudioParameterFloat*> (params[SPI_ParamIdx_ReverbSendGain]);
			m_reverbSendGainSlider = std::make_unique<Slider>(param->name);
			m_reverbSendGainSlider->setRange(param->range.start, param->range.end, param->range.interval);
			m_reverbSendGainSlider->setSliderStyle(Slider::Rotary);
			m_reverbSendGainSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
			m_reverbSendGainSlider->addListener(this);
			addAndMakeVisible(m_reverbSendGainSlider.get());

			// Label for ReverbSendGain
			m_reverbSendGainLabel = std::make_unique<Label>(param->name, param->name);
			m_reverbSendGainLabel->setJustificationType(Justification::centred);
			addAndMakeVisible(m_reverbSendGainLabel.get());

			//--- SourceSpread Slider ---//
			param = dynamic_cast<AudioParameterFloat*> (params[SPI_ParamIdx_ObjectSpread]);
			m_soundobjectSpreadSlider = std::make_unique<Slider>(param->name);
			m_soundobjectSpreadSlider->setRange(param->range.start, param->range.end, param->range.interval);
			m_soundobjectSpreadSlider->setSliderStyle(Slider::Rotary);
			m_soundobjectSpreadSlider->setTextBoxStyle(Slider::TextBoxBelow, false, 80, 20);
			m_soundobjectSpreadSlider->addListener(this);
			addAndMakeVisible(m_soundobjectSpreadSlider.get());

			// Label for SourceSpread
			m_soundobjectSpreadLabel = std::make_unique<Label>(param->name, param->name);
			m_soundobjectSpreadLabel->setJustificationType(Justification::centred);
			addAndMakeVisible(m_soundobjectSpreadLabel.get());

			//--- DelayMode ComboBox ---//
			AudioParameterChoice* choiceParam = dynamic_cast<AudioParameterChoice*> (params[SPI_ParamIdx_DelayMode]);
			m_delayModeComboBox = std::make_unique<ComboBox>(choiceParam->name);
			m_delayModeComboBox->setEditableText(false);
			m_delayModeComboBox->addItem("Off",	1);
			m_delayModeComboBox->addItem("Tight", 2);
			m_delayModeComboBox->addItem("Full", 3);
			m_delayModeComboBox->addListener(this);
			addAndMakeVisible(m_delayModeComboBox.get());

			// Label for DelayMode
			m_delayModeLabel = std::make_unique<Label>(choiceParam->name, choiceParam->name);
			m_delayModeLabel->setJustificationType(Justification::centred);
			addAndMakeVisible(m_delayModeLabel.get());
		}
	}

	setSize(20, 20);
}

/**
 * Object destructor.
 */
SoundobjectProcessorEditor::~SoundobjectProcessorEditor()
{
	processor.editorBeingDeleted(this);
}

/**
 * Helper function to get the pointer to a processor parameter based on the slider assigned to it.
 * @param slider	The slider object for which the parameter is desired.
 * @return			The desired processor parameter.
 */
GestureManagedAudioParameterFloat* SoundobjectProcessorEditor::GetParameterForSlider(Slider* slider)
{
	const Array<AudioProcessorParameter*>& params = getAudioProcessor()->getParameters();
	if (slider == m_xSlider.get())
		return dynamic_cast<GestureManagedAudioParameterFloat*> (params[SPI_ParamIdx_X]);
	else if (slider == m_ySlider.get())
		return dynamic_cast<GestureManagedAudioParameterFloat*> (params[SPI_ParamIdx_Y]);
	else if (slider == m_reverbSendGainSlider.get())
		return dynamic_cast<GestureManagedAudioParameterFloat*> (params[SPI_ParamIdx_ReverbSendGain]);
	else if (slider == m_soundobjectSpreadSlider.get())
		return dynamic_cast<GestureManagedAudioParameterFloat*> (params[SPI_ParamIdx_ObjectSpread]);

	// Should not make it this far.
	jassertfalse;
	return nullptr;
}

/**
 * Callback function for changes to xy slider. Called when the slider's value is changed.
 * @param slider    Slider object which was dragged by user.
 */
void SoundobjectProcessorEditor::sliderValueChanged(SoundobjectSlider* slider)
{
    SoundobjectProcessor* soProcessor = dynamic_cast<SoundobjectProcessor*>(getAudioProcessor());
    if (soProcessor)
    {
        soProcessor->SetParameterValue(DCP_SoundobjectProcessor, SPI_ParamIdx_X, static_cast<float>(slider->GetSoundobjectPos().getX()));
        soProcessor->SetParameterValue(DCP_SoundobjectProcessor, SPI_ParamIdx_Y, static_cast<float>(slider->GetSoundobjectPos().getY()));
    }
    EnqueueTickTrigger();
}

/**
 * Called when the slider is about to be dragged.
 * This is called when a drag begins, then it's followed by multiple calls to sliderValueChanged(),
 * and then sliderDragEnded() is called after the user lets go.
 * @param slider    Slider object which was dragged by user.
 */
void SoundobjectProcessorEditor::sliderDragStarted(SoundobjectSlider* slider)
{
    if (slider == m_soundobjectSlider.get())
    {
        if (GestureManagedAudioParameterFloat* paramx = GetParameterForSlider(static_cast<Slider*>(m_xSlider.get())))
            paramx->BeginGuiGesture();
        if (GestureManagedAudioParameterFloat* paramy = GetParameterForSlider(static_cast<Slider*>(m_ySlider.get())))
            paramy->BeginGuiGesture();
    }
}

/**
 * Called after a drag operation has finished.
 * @param slider    Slider object which was dragged by user.
 */
void SoundobjectProcessorEditor::sliderDragEnded(SoundobjectSlider* slider)
{
    if (slider == m_soundobjectSlider.get())
    {
        if (GestureManagedAudioParameterFloat* paramx = GetParameterForSlider(static_cast<Slider*>(m_xSlider.get())))
            paramx->EndGuiGesture();
        if (GestureManagedAudioParameterFloat* paramy = GetParameterForSlider(static_cast<Slider*>(m_ySlider.get())))
            paramy->EndGuiGesture();
    }
}

/**
 * Callback function for changes to our sliders. Called when the slider's value is changed.
 * This may be caused by dragging it, or by typing in its text entry box, or by a call to Slider::setValue().
 * You can find out the new value using Slider::getValue().
 * @param slider	Slider object which was dragged by user.
 */
void SoundobjectProcessorEditor::sliderValueChanged(Slider* slider)
{
	SoundobjectProcessor* soProcessor = dynamic_cast<SoundobjectProcessor*>(getAudioProcessor());
	if (soProcessor)
	{
		SoundobjectParameterIndex paramIdx = SPI_ParamIdx_MaxIndex;
		if (slider == m_xSlider.get())
			paramIdx = SPI_ParamIdx_X;
		else if (slider == m_ySlider.get())
			paramIdx = SPI_ParamIdx_Y;
		else if (slider == m_reverbSendGainSlider.get())
			paramIdx = SPI_ParamIdx_ReverbSendGain;
		else if (slider == m_soundobjectSpreadSlider.get())
			paramIdx = SPI_ParamIdx_ObjectSpread;

		soProcessor->SetParameterValue(DCP_SoundobjectProcessor, paramIdx, static_cast<float>(slider->getValue()));
	}
	EnqueueTickTrigger();
}

/**
 * Called when the slider is about to be dragged.
 * This is called when a drag begins, then it's followed by multiple calls to sliderValueChanged(),
 * and then sliderDragEnded() is called after the user lets go.
 * @param slider	Slider object which was dragged by user.
 */
void SoundobjectProcessorEditor::sliderDragStarted(Slider* slider)
{
	if (GestureManagedAudioParameterFloat* param = GetParameterForSlider(static_cast<Slider*>(slider)))
		param->BeginGuiGesture();
}

/**
 * Called after a drag operation has finished.
 * @param slider	Slider object which was dragged by user.
 */
void SoundobjectProcessorEditor::sliderDragEnded(Slider* slider)
{
	if (GestureManagedAudioParameterFloat* param = GetParameterForSlider(static_cast<Slider*>(slider)))
		param->EndGuiGesture();
}

/**
 * Called when a ComboBox has its selected item changed. 
 * @param comboBox	The combo box which has changed.
 */
void SoundobjectProcessorEditor::comboBoxChanged(ComboBox *comboBox)
{
	SoundobjectProcessor* pro = dynamic_cast<SoundobjectProcessor*>(getAudioProcessor());
	if (pro)
	{
		if (comboBox == m_delayModeComboBox.get())
		{
			pro->SetParameterValue(DCP_SoundobjectProcessor, SPI_ParamIdx_DelayMode, float(comboBox->getSelectedId() - 1));
		}
	}
	EnqueueTickTrigger();
}

/**
* Method which gets called when a region of a component needs redrawing, either because the
* component's repaint() method has been called, or because something has happened on the
* screen that means a section of a window needs to be redrawn.
* @param g		Graphics context that must be used to do the drawing operations.
*/
void SoundobjectProcessorEditor::paint(Graphics& g)
{
	Rectangle<int> twoDSurfaceArea = getLocalBounds();
	Rectangle<int> parameterEditArea = getLocalBounds();
	getResizePaintAreaSplit(twoDSurfaceArea, parameterEditArea);

	// Background of 2D slider area
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(twoDSurfaceArea);

	// Background of parameter edit elements
	g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId));
	g.fillRect(parameterEditArea);
	
	// Frame
	g.setColour(getLookAndFeel().findColour(TableListBox::outlineColourId));
	g.drawRect(getLocalBounds().toFloat());
    
    // processor id (object #) in upper left corner
    SoundobjectProcessor* pro = dynamic_cast<SoundobjectProcessor*>(getAudioProcessor());
    if (pro)
    {
        auto soundobjectSliderLabelVisible = true;
        if (twoDSurfaceArea.getWidth() < 250 || twoDSurfaceArea.getHeight() < 250)
			soundobjectSliderLabelVisible = false;
        
        auto objNumTitleText = String("#") + String(pro->GetSoundobjectId());
		if (soundobjectSliderLabelVisible)
		{
			if (m_processorName.isEmpty())
				objNumTitleText = String("Object #") + String(pro->GetSoundobjectId());
			else
				objNumTitleText = m_processorName;
		}
        auto titleTextWidth = soundobjectSliderLabelVisible ? 130 : 35;
        auto objNumTitleRect = twoDSurfaceArea.removeFromBottom(25).removeFromLeft(titleTextWidth + 7).removeFromRight(titleTextWidth);
        
        g.setColour(getLookAndFeel().findColour(TableListBox::textColourId));
        g.drawText(objNumTitleText, objNumTitleRect, Justification::centredLeft);
    }
}

/**
 * Minimal helper method to get the areas for parameter edits
 * and 2D surface slider.
 * This is to avoid code clones in paint and resize methods.
 * @param twoDSurfaceArea	The area to be used for 2D surface controls
 * @param parameterEditArea	The area to be used for parameter controls
 * @return	True if the layout is to be done in portrait, false if in landscape orientation
 */
bool SoundobjectProcessorEditor::getResizePaintAreaSplit(Rectangle<int>& twoDSurfaceArea, Rectangle<int>& parameterEditArea)
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
void SoundobjectProcessorEditor::resized()
{
	//==============================================================================
	Rectangle<int> twoDSurfaceArea = getLocalBounds();
	Rectangle<int> parameterEditArea = getLocalBounds();
	auto isPortrait = getResizePaintAreaSplit(twoDSurfaceArea, parameterEditArea);

	//==============================================================================
	bool soundobjectSliderLabelVisible = true;
	if (twoDSurfaceArea.getWidth() < 250 || twoDSurfaceArea.getHeight() < 250)
		soundobjectSliderLabelVisible = false;
	auto xSliderStripWidth = soundobjectSliderLabelVisible ? 80 : 30;
	auto ySliderStripWidth = soundobjectSliderLabelVisible ? 100 : 30;
	twoDSurfaceArea.reduce(5, 5);
	twoDSurfaceArea.removeFromTop(soundobjectSliderLabelVisible ? 30 : 10);
	twoDSurfaceArea.removeFromRight(soundobjectSliderLabelVisible ? 30 : 10);

	// Y Slider
	auto ySliderBounds = twoDSurfaceArea;
	ySliderBounds.removeFromRight(twoDSurfaceArea.getWidth() - ySliderStripWidth);
	ySliderBounds.removeFromBottom(xSliderStripWidth);
	m_ySlider->setBounds(ySliderBounds);
	m_ySlider->setTextBoxStyle(soundobjectSliderLabelVisible ? Slider::TextBoxLeft : Slider::NoTextBox, false, 80, 20);
	ySliderBounds.removeFromTop(50);
	ySliderBounds.removeFromRight(30);
	m_yAxisLabel->setBounds(ySliderBounds);
	m_yAxisLabel->setVisible(soundobjectSliderLabelVisible);

	// 2D Surface
	auto surfaceSliderBounds = twoDSurfaceArea;
	surfaceSliderBounds.removeFromLeft(ySliderStripWidth);
	surfaceSliderBounds.removeFromBottom(xSliderStripWidth);
	m_soundobjectSlider->setBounds(surfaceSliderBounds);
	
	// X Slider
	auto xSliderBounds = twoDSurfaceArea;
	xSliderBounds.removeFromTop(twoDSurfaceArea.getHeight() - xSliderStripWidth);
	xSliderBounds.removeFromLeft(ySliderStripWidth);
	m_xSlider->setBounds(xSliderBounds.removeFromTop(50));
	m_xSlider->setTextBoxStyle(soundobjectSliderLabelVisible ? Slider::TextBoxBelow : Slider::NoTextBox, false, 80, 20);
	m_xAxisLabel->setBounds(xSliderBounds);
	m_xAxisLabel->setVisible(soundobjectSliderLabelVisible);


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
		auto hPos = (parameterEditArea.getWidth() - parameterEditsWidth) / 2;
		auto vPos = (getLocalBounds().getHeight() - (labelHeight + sliderHeight));

		// ReverbSendGain Slider
		m_reverbSendGainLabel->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, labelHeight));
		m_reverbSendGainSlider->setBounds(Rectangle<int>(hPos, vPos + 18, labelSliderWidth, sliderHeight));
        m_reverbSendGainSlider->setTextBoxStyle(paramSliderLabelVisible ? Slider::TextBoxBelow : Slider::NoTextBox, false, 80, 20);
		hPos += 85;

		// SourceSpread Slider
		m_soundobjectSpreadLabel->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, labelHeight));
		m_soundobjectSpreadSlider->setBounds(Rectangle<int>(hPos, vPos + 18, labelSliderWidth, sliderHeight));
        m_soundobjectSpreadSlider->setTextBoxStyle(paramSliderLabelVisible ? Slider::TextBoxBelow : Slider::NoTextBox, false, 80, 20);
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
		auto vPos = (getLocalBounds().getHeight() - parameterEditsHeight) / 2;
	
		// ReverbSendGain Slider
		m_reverbSendGainLabel->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, labelHeight));
		vPos += 18;
		m_reverbSendGainSlider->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, sliderHeight));
		m_reverbSendGainSlider->setTextBoxStyle(paramSliderLabelVisible ? Slider::TextBoxBelow : Slider::NoTextBox, false, 80, 20);
		vPos += paramSliderLabelVisible ? 86 : 56;
	
		// SourceSpread Slider
		m_soundobjectSpreadLabel->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, labelHeight));
		vPos += 18;
		m_soundobjectSpreadSlider->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, sliderHeight));
		m_soundobjectSpreadSlider->setTextBoxStyle(paramSliderLabelVisible ? Slider::TextBoxBelow : Slider::NoTextBox, false, 80, 20);
		vPos += paramSliderLabelVisible ? 86 : 56;
	
		// DelayMode ComboBox
		m_delayModeLabel->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, labelHeight));
		vPos += 25;
		m_delayModeComboBox->setBounds(Rectangle<int>(hPos, vPos, labelSliderWidth, labelHeight));
		vPos += 86;
	}
}

/**
 * Reimplemented from juce::MessageListener to process
 * internally posted tick messages.
 * This implementation takes care of not actually executing
 * tick triggers that have been rendered irrelevant while
 * pending in the message queue.
 * @param    message        The message dispatched from queue to be handled.
 *                        Only private TickTrigger message impl. is supported.
 */
void SoundobjectProcessorEditor::handleMessage(const Message& message)
{
    if (auto tickTrigger = dynamic_cast<const TickTrigger*>(&message))
    {
        if (!tickTrigger->IsOutdated())
            UpdateGui();

        // Mark the tick event as handled here, even though the object will
        // be deleted by JUCE later on, to allow new tick events to be enqueued
        // from here on.
        tickTrigger->SetTickHandled();
    }
}

/**
 * Public helper to post a new tick trigger
 * message to async message queue.
 */
void SoundobjectProcessorEditor::EnqueueTickTrigger()
{
	if (TickTrigger::IsOutdated())
		postMessage(new TickTrigger());
}

/**
 * Update GUI elements with the current parameter values.
 */
void SoundobjectProcessorEditor::UpdateGui()
{
	bool somethingChanged = false;

	SoundobjectProcessor* pro = dynamic_cast<SoundobjectProcessor*>(getAudioProcessor());
	if (pro)
	{
		const Array<AudioProcessorParameter*>& params = pro->getParameters();

		// See if any parameters changed since the last timer callback.
		somethingChanged = (pro->GetParameterChanged(DCP_SoundobjectProcessor, DCT_SoundobjectParameters) ||
							pro->GetParameterChanged(DCP_SoundobjectProcessor, DCT_SoundobjectProcessorConfig) ||
							pro->GetParameterChanged(DCP_SoundobjectProcessor, DCT_CommunicationConfig));

		if (pro->PopParameterChanged(DCP_SoundobjectProcessor, DCT_SoundobjectPosition))
		{
			// Update position of X slider.
			auto xParam = dynamic_cast<AudioParameterFloat*>(params[SPI_ParamIdx_X]);
			if (xParam)
				m_xSlider->setValue(xParam->get(), dontSendNotification);

			// Update position of Y slider.
			auto yParam = dynamic_cast<AudioParameterFloat*>(params[SPI_ParamIdx_Y]);
			if (yParam)
				m_ySlider->setValue(yParam->get(), dontSendNotification);

			// Update the nipple position on the 2D-Slider.
			if (xParam && yParam)
				m_soundobjectSlider->SetSoundobjectPos({ xParam->get(), yParam->get() });
		}

		if (pro->PopParameterChanged(DCP_SoundobjectProcessor, DCT_ReverbSendGain))
		{
			// Update ReverbSendGain slider
			auto fParam = dynamic_cast<AudioParameterFloat*>(params[SPI_ParamIdx_ReverbSendGain]);
			if (fParam)
				m_reverbSendGainSlider->setValue(fParam->get(), dontSendNotification);
		}

		if (pro->PopParameterChanged(DCP_SoundobjectProcessor, DCT_SoundobjectSpread))
		{
			// Update SourceSpread slider
			auto fParam = dynamic_cast<AudioParameterFloat*>(params[SPI_ParamIdx_ObjectSpread]);
			if (fParam)
				m_soundobjectSpreadSlider->setValue(fParam->get(), dontSendNotification);
		}

		if (pro->PopParameterChanged(DCP_SoundobjectProcessor, DCT_DelayMode))
		{
			// Update DelayMode combo box
			AudioParameterChoice* cParam = dynamic_cast<AudioParameterChoice*>(params[SPI_ParamIdx_DelayMode]);
			if (cParam)
			{
				// Need to add 1 because the parameter's indeces go from 0 to 2, while the combo box's ID's go from 1 to 3.
				m_delayModeComboBox->setSelectedId(cParam->getIndex() + 1, dontSendNotification);
			}
		}

		if (pro->PopParameterChanged(DCP_SoundobjectProcessor, DCT_SoundobjectID))
			m_processorName = pro->getProgramName(0);
	}
}


} // namespace SpaConBridge
