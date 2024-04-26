#pragma once
// Copyright (C) 2023 d&b audiotechnik GmbH & Co. KG. All Rights Reserved.

#include "SoundobjectProcessor.h"

#include "../../SoundobjectSlider.h"


namespace JUCEAppBasics
{
	class FixedFontTextEditor;
	class NiceRotarySlider;
}


namespace SpaConBridge
{


/**
 * Class MainProcessorEditor, a component that acts as the GUI for the AudioProcessor. 
 */
class SoundobjectProcessorEditor :
	public juce::AudioProcessorEditor,
	public juce::Slider::Listener,
	public SoundobjectSlider::Listener,
	public juce::ComboBox::Listener,
	public juce::MessageListener
{
public:
	explicit SoundobjectProcessorEditor(SoundobjectProcessor&);
	~SoundobjectProcessorEditor() override;

	void paint(Graphics&) override;
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
	void sliderValueChanged(SoundobjectSlider* slider) override;
	void sliderDragStarted(SoundobjectSlider* slider) override;
	void sliderDragEnded(SoundobjectSlider* slider) override;
	void sliderValueChanged(Slider *slider) override;
	void sliderDragStarted(Slider* slider) override;
	void sliderDragEnded(Slider* slider) override;
	void comboBoxChanged(ComboBox *comboBox) override;
	bool getResizePaintAreaSplit(Rectangle<int>& twoDSurfaceArea, Rectangle<int>& parameterEditArea);
    void handleMessage(const Message& message) override;

	std::unique_ptr<Slider>				m_xSlider;					/**< Horizontal slider for X axis. */
	std::unique_ptr<Slider>				m_ySlider;					/**< Vertical slider for Y axis. */
	std::unique_ptr<Slider>				m_reverbSendGainSlider;		/**< Slider for ReverbSendGain */
	std::unique_ptr<Slider>				m_soundobjectSpreadSlider;	/**< Slider for SourceSpread */
	std::unique_ptr<ComboBox>			m_delayModeComboBox;		/**< ComboBox for DelayMode */
	std::unique_ptr<Label>				m_xAxisLabel;				/**< X axis slider label */
	std::unique_ptr<Label>				m_yAxisLabel;				/**< Y axis slider label */
	std::unique_ptr<Label>				m_reverbSendGainLabel;		/**< ReverbSendGain slider label */
	std::unique_ptr<Label>				m_soundobjectSpreadLabel;	/**< SourceSpread slider label */
	std::unique_ptr<Label>				m_delayModeLabel;			/**< DelayMode ComboBox label */
	std::unique_ptr<SoundobjectSlider>	m_soundobjectSlider;		/**< 2D Slider component. */
	String								m_processorName;			/**< The processor's user readable name. */
	int									m_ticksSinceLastChange = 0; /**< Used to allow some tolerance when switching between fast and slow refresh
																	 * rates for the GUI.
																	 * Once this counter reaches GUI_UPDATE_DELAY_TICKS, and no parameters have
																	 * changed, the GUI will switch to GUI_UPDATE_RATE_SLOW. Switches to
																	 * GUI_UPDATE_RATE_FAST happen immediately after any change. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundobjectProcessorEditor)
};


} // namespace SpaConBridge
