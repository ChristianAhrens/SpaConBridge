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


namespace SoundscapeBridgeApp
{


/**
 * StatisticsPlot class provides a rolling plot to show protocol traffic.
 */
class StatisticsPlot : public Component
{
public:
	StatisticsPlot();
	~StatisticsPlot() override;

protected:
	//==============================================================================
	void paint(Graphics& g) override;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatisticsPlot)
};


/**
 * StatisticsLog class provides a rolling log to show protocol data.
 */
class StatisticsLog : public Component
{
public:
	StatisticsLog();
	~StatisticsLog() override;

protected:
	//==============================================================================
	void paint(Graphics& g) override;

private:

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatisticsLog)
};


/**
 * Class StatisticsPageComponent is a component that contains elements for
 * protocol traffic plotting and logging
 */
class StatisticsPageComponent : public PageComponentBase
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

private:
	std::unique_ptr<StatisticsPlot>	m_plotComponent;	/**> Plotting component. */
	std::unique_ptr<StatisticsLog>	m_logComponent;		/**> Logging component. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatisticsPageComponent)
};


} // namespace SoundscapeBridgeApp