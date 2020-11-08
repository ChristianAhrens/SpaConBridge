/*
  ==============================================================================

    StatisticsPageComponent.h
    Created: 8 Nov 2020 10:19:43am
    Author:  Christian Ahrens

  ==============================================================================
*/

#pragma once

#include "PageComponentBase.h"

#include "../../SoundscapeBridgeAppCommon.h"
#include "../../ProtocolBridgingWrapper.h"


namespace SoundscapeBridgeApp
{


typedef int PlotConstant;
static constexpr PlotConstant	PC_HOR_RANGE			= 20000;	// 20s on horizontal axis
static constexpr PlotConstant	PC_HOR_DEFAULTSTEPPING	= 200;		// 200ms default resolution
static constexpr PlotConstant	PC_HOR_USERVISUSTEPPING = 1000;		// User is presented with plot legend msg/s to have something more legible than 200ms
static constexpr PlotConstant	PC_VERT_RANGE			= 2;		// 10 msg/s default on vertical axis (2 msg per 200ms interval)


/**
 * StatisticsPlot class provides a rolling plot to show protocol traffic.
 */
class StatisticsPlot :	public Component, 
						private Timer
{
public:
	StatisticsPlot();
	~StatisticsPlot() override;

	void IncreaseCount(ProtocolBridgingType bridgingProtocol);

protected:
	//==============================================================================
	void paint(Graphics& g) override;

private:
	//==============================================================================
	void timerCallback() override;

private:
	juce::Array<ProtocolBridgingType> m_plottedBridgingTypes{ PBT_DiGiCo, PBT_BlacktraxRTTrPM, PBT_GenericOSC };

	int	m_hRange;		/**< Horizontal max plot value (value range) in ms. We use the range from left (0) to right (m_hRange) to plot data. */
	int	m_hStepping;	/**< Horizontal step width in ms. */
	int	m_vRange;		/**< Vertical max plot value (value range). We use the range from bottom (0) to top (m_vRange) where m_vRange
						 *	is dynamically adjusted regarding incoming data to plot. */
	std::map<ProtocolBridgingType, int>					m_currentMsgPerProtocol;	/**< Map to help counting messages per protocol in current interval. This is processed every timer callback to update plot data. */
	std::map<ProtocolBridgingType, std::vector<float>>	m_plotData;					/**< Data for plotting. Primitive vector of floats that represents the msg count per hor. step width. */
	std::map<ProtocolBridgingType, Colour>				m_plotColours;				/** Individual colour for each protocol plot. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatisticsPlot)
};


/**
 * StatisticsLog class provides a rolling log to show protocol data.
 */
class StatisticsLog :	public Component,
						private Timer
{
public:
	StatisticsLog();
	~StatisticsLog() override;

	//==============================================================================
	void AddMessageData(ProtocolBridgingType bridgingType, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData);

protected:
	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

private:
	//==============================================================================
	void timerCallback() override;

private:
	CodeDocument							m_doc;				/**< Document object used by codeeditorcomponent for content. */
	std::unique_ptr<CodeEditorComponent>	m_textBox;			/**< The actual component to show log text within window. */
	std::vector<String>						m_loggingQueue;		/**< List of message strings to be printed on next flush timer callback. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatisticsLog)
};


/**
 * Class StatisticsPageComponent is a component that contains elements for
 * protocol traffic plotting and logging
 */
class StatisticsPageComponent : public PageComponentBase,
								public ProtocolBridgingWrapper::Listener
{
public:
	StatisticsPageComponent();
	~StatisticsPageComponent() override;

	//==============================================================================
	void UpdateGui(bool init) override;

protected:
	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

	//==========================================================================
	void HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier Id, RemoteObjectMessageData& msgData) override;

private:
	std::unique_ptr<StatisticsPlot>	m_plotComponent;	/**> Plotting component. */
	std::unique_ptr<StatisticsLog>	m_logComponent;		/**> Logging component. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatisticsPageComponent)
};


} // namespace SoundscapeBridgeApp