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


#pragma once

#include "SoundsourceProcessor.h"
#include "Gui.h"
#include "SurfaceSlider.h"
#include <utility>	//<USE std::unique_ptr


namespace SoundscapeBridgeApp
{


/**
 * Class MainProcessorEditor, a component that acts as the GUI for the AudioProcessor. 
 */
class SoundsourceProcessorEditor :
	public AudioProcessorEditor,
	public TextEditor::Listener,
	public Slider::Listener,
	public ComboBox::Listener,
	private Timer
{
public:
	SoundsourceProcessorEditor(SoundsourceProcessor&);
	~SoundsourceProcessorEditor() override;

	void paint(Graphics&) override;
	void resized() override;
	void UpdateGui(bool init);

private:
	CAudioParameterFloat* GetParameterForSlider(CSlider* slider);
	void sliderValueChanged(Slider *slider) override;
	void sliderDragStarted(Slider* slider) override;
	void sliderDragEnded(Slider* slider) override;
	void textEditorReturnKeyPressed(TextEditor &) override;
	void comboBoxChanged(ComboBox *comboBox) override;
	void timerCallback() override;
	bool getResizePaintAreaSplit(Rectangle<int>& twoDSurfaceArea, Rectangle<int>& parameterEditArea);

	/**
	 * Horizontal slider for X axis.
	 */
	std::unique_ptr<CSlider>	m_xSlider;

	/**
	 * Vertical slider for Y axis.
	 */
	std::unique_ptr<CSlider>	m_ySlider;

	/**
	 * Slider for ReverbSendGain
	 */
	std::unique_ptr<CSlider>	m_reverbSendGainSlider;

	/**
	 * Slider for SourceSpread
	 */
	std::unique_ptr<CSlider>	m_sourceSpreadSlider;

	/**
	 * ComboBox for DelayMode
	 */
	std::unique_ptr<ComboBox>	m_delayModeComboBox;

	/**
	 * X axis slider label
	 */
	std::unique_ptr<CLabel>	m_xAxisLabel;

	/**
	 * Y axis slider label
	 */
	std::unique_ptr<CLabel>	m_yAxisLabel;

	/**
	 * ReverbSendGain slider label
	 */
	std::unique_ptr<CLabel>	m_reverbSendGainLabel;

	/**
	 * SourceSpread slider label
	 */
	std::unique_ptr<CLabel>	m_sourceSpreadLabel;

	/**
	 * DelayMode ComboBox label
	 */
	std::unique_ptr<CLabel>	m_delayModeLabel;

	/*
	 * 2D Slider component.
	 */
	std::unique_ptr<CSurfaceSlider> m_surfaceSlider;

	/**
	 * Plug-in display name label. On the hosts which support updateTrackProperties or changeProgramName, 
	 * this will show the track's name where this Plug-in is located.
	 */
	std::unique_ptr<CLabel>	m_displayNameLabel;

	/**
	 * Used to allow some tolerance when switching between fast and slow refresh rates for the GUI. 
	 * Once this counter reaches GUI_UPDATE_DELAY_TICKS, and no parameters have changed, the GUI will
	 * switch to GUI_UPDATE_RATE_SLOW. Switches to GUI_UPDATE_RATE_FAST happen immediately after any change.
	 */
	int m_ticksSinceLastChange = 0;

#ifdef JUCE_DEBUG
	/**
	 * Special textfield used for displaying debugging messages.
	 */
	std::unique_ptr<TextEditor>	m_debugTextEdit;
#endif

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundsourceProcessorEditor)
};


} // namespace SoundscapeBridgeApp