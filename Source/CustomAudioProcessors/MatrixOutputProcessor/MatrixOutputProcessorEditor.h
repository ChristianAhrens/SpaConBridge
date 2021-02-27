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


#pragma once

#include "MatrixOutputProcessor.h"

#include "../../SurfaceSlider.h"


namespace SoundscapeBridgeApp
{


/**
 * Class MainProcessorEditor, a component that acts as the GUI for the AudioProcessor. 
 */
class MatrixOutputProcessorEditor :
	public AudioProcessorEditor,
	public TextEditor::Listener,
	public Slider::Listener,
	private Timer
{
public:
	MatrixOutputProcessorEditor(MatrixOutputProcessor&);
	~MatrixOutputProcessorEditor() override;

	void paint(Graphics&) override;
	void resized() override;
	void UpdateGui(bool init);

private:
	GestureManagedAudioParameterFloat* GetParameterForSlider(Slider* slider);
	void sliderValueChanged(Slider *slider) override;
	void sliderDragStarted(Slider* slider) override;
	void sliderDragEnded(Slider* slider) override;
	void textEditorReturnKeyPressed(TextEditor &) override;
	void timerCallback() override;

	std::unique_ptr<Slider>			m_MatrixOutputLevelMeterSlider;	/**> Slider for ReverbSendGain */
	std::unique_ptr<Slider>			m_MatrixOutputGainSlider;			/**> Slider for SourceSpread */
	std::unique_ptr<DrawableButton>	m_MatrixOutputMuteButton;			/**> ComboBox for DelayMode */
	std::unique_ptr<Label>			m_levelMeterLabel;					/**> X axis slider label */
	std::unique_ptr<Label>			m_gainLabel;						/**> Y axis slider label */
	std::unique_ptr<Label>			m_muteLabel;						/**> ReverbSendGain slider label */
	std::unique_ptr<Label>			m_displayNameLabel;					/**> Plug-in display name label. On the hosts which support updateTrackProperties
																		 * or changeProgramName, this will show the track's name where this Plug-in
																		 * is located. */
	int								m_ticksSinceLastChange = 0;			/**> Used to allow some tolerance when switching between fast and slow refresh
																		 * rates for the GUI.
																		 * Once this counter reaches GUI_UPDATE_DELAY_TICKS, and no parameters have
																		 * changed, the GUI will switch to GUI_UPDATE_RATE_SLOW. Switches to
																		 * GUI_UPDATE_RATE_FAST happen immediately after any change. */

	static constexpr int Mute_On = 1;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatrixOutputProcessorEditor)
};


} // namespace SoundscapeBridgeApp
