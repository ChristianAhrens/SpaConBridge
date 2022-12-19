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


#pragma once

#include "MatrixOutputProcessor.h"

#include "../../LevelMeterSlider.h"


namespace SpaConBridge
{


/**
 * Class MainProcessorEditor, a component that acts as the GUI for the AudioProcessor. 
 */
class MatrixOutputProcessorEditor :
	public AudioProcessorEditor,
	public DrawableButton::Listener,
	public Slider::Listener,
	private Timer
{
public:
	explicit MatrixOutputProcessorEditor(MatrixOutputProcessor&);
	~MatrixOutputProcessorEditor() override;

	MatrixOutputId	GetMatrixOutputId();

	void resized() override;
	void UpdateGui(bool init);

private:
	GestureManagedAudioParameterFloat* GetParameterForSlider(Slider* slider);
	void sliderValueChanged(Slider *slider) override;
	void sliderDragStarted(Slider* slider) override;
	void sliderDragEnded(Slider* slider) override;
	void buttonClicked(Button* button) override;
	void lookAndFeelChanged() override;
	void timerCallback() override;

	void updateDrawableButtonImageColours();

	std::unique_ptr<LevelMeterSlider>	m_MatrixOutputLevelMeterSlider;	/**> LevelMeter for pre mute level */
	std::unique_ptr<Slider>				m_MatrixOutputGainSlider;		/**> Slider for input gain */
	std::unique_ptr<DrawableButton>		m_MatrixOutputMuteButton;		/**> Button for mute */
																	
	int									m_ticksSinceLastChange = 0;		/**> Used to allow some tolerance when switching between fast and slow refresh
                                                                         * rates for the GUI.
                                                                         * Once this counter reaches GUI_UPDATE_DELAY_TICKS, and no parameters have
                                                                         * changed, the GUI will switch to GUI_UPDATE_RATE_SLOW. Switches to
                                                                         * GUI_UPDATE_RATE_FAST happen immediately after any change. */

	static constexpr int Mute_On = 1;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatrixOutputProcessorEditor)
};


} // namespace SpaConBridge
