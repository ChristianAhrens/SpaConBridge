/* Copyright (c) 2020-2023, Christian Ahrens
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

#include "MatrixInputProcessor.h"

#include "../../LevelMeterSlider.h"


namespace SpaConBridge
{


/**
 * Class MainProcessorEditor, a component that acts as the GUI for the AudioProcessor. 
 */
class MatrixInputProcessorEditor :
	public juce::AudioProcessorEditor,
	public juce::DrawableButton::Listener,
	public juce::Slider::Listener,
	public juce::MessageListener
{
public:
	explicit MatrixInputProcessorEditor(MatrixInputProcessor&);
	~MatrixInputProcessorEditor() override;

	MatrixInputId	GetMatrixInputId();

	void resized() override;
	void UpdateGui();
	void EnqueueTickTrigger();

private:
	/**
	 * Private message class to act as asynchronous
	 * 'tick'/update trigger via message queue.
	 * To prevent irrelevant processing of multiple queued triggers,
	 * an internal flag is used that signals if a trigger message
	 * is still relevant when dispatched from queue or no longer is relevant
	 * due to an earlier trigger processing already handled things.
	 */
	class TickTrigger : public juce::Message
	{
	public:
		TickTrigger() { s_tickHandled = false; };
		~TickTrigger() {};

		static const bool IsOutdated() { return s_tickHandled; };
		void SetTickHandled() const { s_tickHandled = true; };

	private:
		static bool s_tickHandled;
	};

	GestureManagedAudioParameterFloat* GetParameterForSlider(Slider* slider);
	void sliderValueChanged(Slider *slider) override;
	void sliderDragStarted(Slider* slider) override;
	void sliderDragEnded(Slider* slider) override;
	void buttonClicked(Button* button) override;
	void lookAndFeelChanged() override;
	void handleMessage(const Message& message) override;

	void updateDrawableButtonImageColours();

	std::unique_ptr<LevelMeterSlider>	m_MatrixInputLevelMeterSlider;	/**> LevelMeter for pre mute level */
	std::unique_ptr<Slider>				m_MatrixInputGainSlider;		/**> Slider for input gain */
	std::unique_ptr<DrawableButton>		m_MatrixInputMuteButton;		/**> Button for mute */

	static constexpr int Mute_On = 1;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatrixInputProcessorEditor)
};


} // namespace SpaConBridge
