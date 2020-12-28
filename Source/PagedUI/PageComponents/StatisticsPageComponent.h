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
#include "../../AppConfiguration.h"


namespace SoundscapeBridgeApp
{


typedef int PlotConstant;
static constexpr PlotConstant	PC_HOR_RANGE			= 20000;	// 20s on horizontal axis
static constexpr PlotConstant	PC_HOR_DEFAULTSTEPPING	= 400;		// 400ms default refresh resolution
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

	//==============================================================================
	void IncreaseCount(ProtocolBridgingType bridgingProtocol);
	void ResetStatisticsPlot();

	//==============================================================================
	void mouseUp(const MouseEvent& e) override;

	//==========================================================================
	std::function<void(bool)>	toggleShowDS100Traffic;

protected:
	//==============================================================================
	void paint(Graphics& g) override;

private:
	//==============================================================================
	void timerCallback() override;

private:
	juce::Array<ProtocolBridgingType> m_plottedBridgingTypes;

	bool m_showDS100Traffic{ false };
	int	m_vertValueRange;	/**< Vertical max plot value (value range). */
	std::map<ProtocolBridgingType, int>					m_currentMsgPerProtocol;	/**< Map to help counting messages per protocol in current interval. This is processed every timer callback to update plot data. */
	std::map<ProtocolBridgingType, std::vector<float>>	m_plotData;					/**< Data for plotting. Primitive vector of floats that represents the msg count per hor. step width. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatisticsPlot)
};


/**
 * StatisticsLog class provides a rolling log to show protocol data.
 */
class StatisticsLog :	public Component,
						private Timer,
						public TableListBoxModel
{
public:
	/**
	 * Enum to define where a log entry originates from.
	 * This is used to e.g. differentiate between different DS100 in log,
	 * but show only a single DS100 category in plot.
	 */
	enum StatisticsLogSource
	{
		SLS_None,
		SLS_DiGiCo,
		SLS_BlacktraxRTTrPM,
		SLS_GenericOSC,
		SLS_GenericMIDI,
		SLS_YamahaSQ,
		SLS_YamahaOSC,
		SLS_HUI,
		SLS_DS100,
		SLS_DS100_ext,
		SLS_DS100_mrr,
	};

	enum StatisticsLogColumn
	{
		SLC_None = 0,		//< Juce column IDs start at 1
		SLC_Number,
		SLC_LogSourceName,
		SLC_ObjectName,
		SLC_SourceId,
		SLC_Value,
		SLC_LogSourceType,
		SLC_MAX_COLUMNS
	};

public:
	StatisticsLog();
	~StatisticsLog() override;

	//==============================================================================
	void AddMessageData(StatisticsLogSource logSourceType, RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData);

	//==========================================================================
	void backgroundClicked(const MouseEvent&) override;
	int getNumRows() override;
	void paintRowBackground(Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
	void paintCell(Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
	int getColumnAutoSizeWidth(int columnId) override;

	//==========================================================================
	void SetShowDS100Traffic(bool show);

protected:
	//==============================================================================
	void resized() override;

private:
	String GetLogSourceName(StatisticsLogSource logSourceType);
	const Colour GetLogSourceColour(StatisticsLogSource logSourceType);

	//==============================================================================
	void timerCallback() override;

private:
	std::unique_ptr<TableListBox>			m_table;				/**< The table component itself. */
	std::map<int, std::map<int, String>>	m_logEntries;			/**< Map of log entry # and map of column and its cell string contents. */
	const int								m_logCount{ 200 };
	int										m_logEntryCounter{ 0 };
	bool									m_dataChanged{ false };
	bool									m_showDS100Traffic{ false };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatisticsLog)
};


/**
 * Class StatisticsPageComponent is a component that contains elements for
 * protocol traffic plotting and logging
 */
class StatisticsPageComponent : public PageComponentBase,
								public ProtocolBridgingWrapper::Listener,
								public AppConfiguration::Watcher
{
public:
	StatisticsPageComponent();
	~StatisticsPageComponent() override;

	//==============================================================================
	void UpdateGui(bool init) override;

	//==========================================================================
	void onConfigUpdated() override;

protected:
	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

	//==========================================================================
	void HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData) override;

private:
	std::unique_ptr<StatisticsPlot>	m_plotComponent;	/**> Plotting component. */
	std::unique_ptr<StatisticsLog>	m_logComponent;		/**> Logging component. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatisticsPageComponent)
};


} // namespace SoundscapeBridgeApp
