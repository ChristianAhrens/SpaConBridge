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

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatisticsPageComponent)
};


} // namespace SoundscapeBridgeApp