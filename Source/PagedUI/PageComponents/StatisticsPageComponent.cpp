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
	Class StatisticsPlot
===============================================================================
*/

/**
 * Class constructor.
 */
StatisticsPlot::StatisticsPlot()
{

}

/**
 * Class destructor.
 */
StatisticsPlot::~StatisticsPlot()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void StatisticsPlot::paint(Graphics& g)
{
	auto bounds = getLocalBounds().toFloat();
	auto w = bounds.getWidth();
	auto h = bounds.getHeight();

	// Plot background area
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
	g.fillRect(bounds);

	// Plot grid
	const float dashLengths[2] = { 5.0f, 6.0f };
	const float lineThickness = 1.0f;
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId).brighter(0.15f));
	g.drawDashedLine(Line<float>(w * 0.25f, 0.0f, w * 0.25f, h), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(w * 0.50f, 0.0f, w * 0.50f, h), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(w * 0.75f, 0.0f, w * 0.75f, h), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(0.0f, h * 0.25f, w, h * 0.25f), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(0.0f, h * 0.50f, w, h * 0.50f), dashLengths, 2, lineThickness);
	g.drawDashedLine(Line<float>(0.0f, h * 0.75f, w, h * 0.75f), dashLengths, 2, lineThickness);

	// Plot frame
	g.setColour(getLookAndFeel().findColour(TextButton::buttonColourId));
	g.drawRect(bounds, 1.5f);
}


/*
===============================================================================
	Class StatisticsLog
===============================================================================
*/

/**
 * Class constructor.
 */
StatisticsLog::StatisticsLog()
{

}

/**
 * Class destructor.
 */
StatisticsLog::~StatisticsLog()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void StatisticsLog::paint(Graphics& g)
{
	auto bounds = getLocalBounds().toFloat();
	auto w = bounds.getWidth();
	auto h = bounds.getHeight();

	// Background of logging area
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(bounds);

	// Frame
	g.setColour(getLookAndFeel().findColour(TableListBox::outlineColourId));
	g.drawRect(bounds);
}


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
	m_plotComponent = std::make_unique<StatisticsPlot>();
	addAndMakeVisible(m_plotComponent.get());

	m_logComponent = std::make_unique<StatisticsLog>();
	addAndMakeVisible(m_logComponent.get());
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

	// determine the layout direction (we want a ratio of 0.75 to be the switching point)
	auto layoutSwitchAspectRatio = 0.75f;
	float w = bounds.getWidth();
	float h = bounds.getHeight();
	auto aspectRatio = h / (w != 0.0f ? w : 1.0f);
	auto isPortrait = layoutSwitchAspectRatio < aspectRatio;

	// The layouting flexbox with parameters
	FlexBox plotAndLogFlex;
	if (isPortrait)
		plotAndLogFlex.flexDirection = FlexBox::Direction::column;
	else
		plotAndLogFlex.flexDirection = FlexBox::Direction::row;
	plotAndLogFlex.justifyContent = FlexBox::JustifyContent::center;

	plotAndLogFlex.items.add(FlexItem(*m_plotComponent).withFlex(2).withMargin(FlexItem::Margin(5, 5, 5, 5)));
	plotAndLogFlex.items.add(FlexItem(*m_logComponent.get()).withFlex(1).withMargin(FlexItem::Margin(5, 5, 5, 5)));

	plotAndLogFlex.performLayout(bounds.toFloat());
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