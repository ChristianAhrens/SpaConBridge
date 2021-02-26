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

#include "MatrixIOPageComponent.h"

#include "MatrixInputsComponent.h"
#include "MatrixOutputsComponent.h"

#include "../../PageComponentManager.h"

#include "../../../Controller.h"

#include <Image_utils.h>


namespace SoundscapeBridgeApp
{


/*
===============================================================================
	Class MatrixIOPageComponent
===============================================================================
*/

/**
 * Class constructor.
 */
MatrixIOPageComponent::MatrixIOPageComponent()
	: PageComponentBase(PCT_MatrixIOs)
{
	m_inputsComponent = std::make_unique<MatrixInputsComponent>();
	addAndMakeVisible(m_inputsComponent.get());

	m_outputsComponent = std::make_unique<MatrixOutputsComponent>();
	addAndMakeVisible(m_outputsComponent.get());

	auto config = SoundscapeBridgeApp::AppConfiguration::getInstance();
	if (config)
		config->addWatcher(this);
}

/**
 * Class destructor.
 */
MatrixIOPageComponent::~MatrixIOPageComponent()
{
}

/**
 * Reimplemented to paint background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void MatrixIOPageComponent::paint(Graphics& g)
{
	auto bounds = getLocalBounds();
	auto bottomBarBounds = bounds.reduced(8);

	if (IsPortraitAspectRatio())
		bottomBarBounds = bottomBarBounds.removeFromLeft(33);
	else
		bottomBarBounds = bottomBarBounds.removeFromBottom(33);

	// Background
	g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker());
	g.fillRect(bounds);

	// Bottm bar background
	g.setColour(getLookAndFeel().findColour(TableListBox::backgroundColourId));
	g.fillRect(bottomBarBounds);

	// Frame
	g.setColour(getLookAndFeel().findColour(TableListBox::outlineColourId));
	g.drawRect(bottomBarBounds, 1);
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void MatrixIOPageComponent::resized()
{
	auto bounds = getLocalBounds().toFloat().reduced(3);

	// The layouting flexbox with parameters
	FlexBox matrixIOFlex;
	if (IsPortraitAspectRatio())
	{
		matrixIOFlex.flexDirection = FlexBox::Direction::column;
		bounds.removeFromLeft(32);
	}
	else
	{
		matrixIOFlex.flexDirection = FlexBox::Direction::row;
		bounds.removeFromBottom(32);
	}
	matrixIOFlex.justifyContent = FlexBox::JustifyContent::center;

	matrixIOFlex.items.add(FlexItem(*m_inputsComponent).withFlex(1).withMargin(FlexItem::Margin(5, 5, 5, 5)));
	matrixIOFlex.items.add(FlexItem(*m_outputsComponent).withFlex(1).withMargin(FlexItem::Margin(5, 5, 5, 5)));

	matrixIOFlex.performLayout(bounds);
}

/**
 * Minimal helper method to determine if aspect ratio of currently
 * available screen realestate suggests we are in portrait or landscape orientation
 * and be able to use the same determination code in multiple places.
 * 
 * @return	True if we are in portrait, false if in landscape aspect ratio.
 */
bool MatrixIOPageComponent::IsPortraitAspectRatio()
{
	// determine the layout direction (we want a ratio of 0.75 to be the switching point)
	auto layoutSwitchAspectRatio = 0.75f;
	auto w = getLocalBounds().getWidth();
	auto h = getLocalBounds().getHeight();
	auto aspectRatio = h / (w != 0.0f ? w : 1.0f);

	return layoutSwitchAspectRatio < aspectRatio;
}

/**
 * If any relevant parameters have been marked as changed, update the table contents.
 * @param init	True to ignore any changed flags and update the procssor parameters
 *				in the GUI anyway. Good for when opening the Overview for the first time.
 */
void MatrixIOPageComponent::UpdateGui(bool init)
{
	ignoreUnused(init);
}

/**
 * Overridden from AppConfiguration Watcher to be able
 * to live react on config changes and update the table contents.
 */
void MatrixIOPageComponent::onConfigUpdated()
{

}


} // namespace SoundscapeBridgeApp
