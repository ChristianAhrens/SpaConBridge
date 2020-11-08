/*
  ==============================================================================

    StatisticsPageComponent.cpp
    Created: 8 Nov 2020 10:19:43am
    Author:  Christian Ahrens

  ==============================================================================
*/

#include "StatisticsPageComponent.h"

#include "../PageComponentManager.h"

#include "../../SoundsourceProcessor/SoundsourceProcessor.h"
#include "../../Controller.h"
#include "../../SoundsourceProcessor/SurfaceSlider.h"

#include <Image_utils.hpp>


namespace SoundscapeBridgeApp
{


/*
===============================================================================
	Class StatisticsPageComponent
===============================================================================
*/

/**
 * Class constructor.
 */
StatisticsPageComponent::StatisticsPageComponent()
	: PageComponentBase(PCT_Statistics)
{

}

/**
 * Class destructor.
 */
StatisticsPageComponent::~StatisticsPageComponent()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void StatisticsPageComponent::paint(Graphics& g)
{
	// Paint background to cover the controls behind this overlay.
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(Rectangle<int>(0, 0, getLocalBounds().getWidth(), getLocalBounds().getHeight()));
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void StatisticsPageComponent::resized()
{
	auto bounds = getLocalBounds().reduced(5);
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the plugin parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void StatisticsPageComponent::UpdateGui(bool init)
{
	// Will be set to true if any changes relevant to the multi-slider are found.
	bool update = init;
}


} // namespace SoundscapeBridgeApp