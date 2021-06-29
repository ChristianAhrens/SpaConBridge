/* Copyright (c) 2020-2021, Christian Ahrens
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


#include "WaitingEntertainerComponent.h"


namespace SpaConBridge
{


/*
===============================================================================
 Class WaitingEntertainerComponent
===============================================================================
*/

/**
* The one and only instance of WaitingEntertainer.
*/
WaitingEntertainerComponent* WaitingEntertainerComponent::m_singleton = nullptr;

/**
 * Class constructor.
 */
WaitingEntertainerComponent::WaitingEntertainerComponent()
	: Component()
{
	jassert(!m_singleton);	// only one instnce allowed!!
	m_singleton = this;

	m_progressBarSlider = nullptr;
}

/**
 * Class destructor.
 */
WaitingEntertainerComponent::~WaitingEntertainerComponent()
{
	m_singleton = nullptr;
}

/**
 * Returns the one and only instance of WaitingEntertainerComponent. If it doesn't exist yet, it is created.
 * @return The WaitingEntertainerComponent singleton object.
 * @sa m_singleton, WaitingEntertainerComponent
 */
WaitingEntertainerComponent* WaitingEntertainerComponent::GetInstance()
{
	if (!m_singleton )
	{
		m_singleton = std::make_unique<WaitingEntertainerComponent>().release();
	}
	return m_singleton;
}

/**
 * Triggers destruction of WaitingEntertainerComponent singleton object
 */
void WaitingEntertainerComponent::DestroyInstance()
{
	std::unique_ptr<WaitingEntertainerComponent> death(m_singleton);
}

/**
 * Method to update the progress displayed on UI.
 * The input value is expected to be a normalized ratio value between 0...1
 * that is interpreted as percentage and shown on UI as such.
 * A value smaller than 0 triggers hiding the progress visualization, a value greater than 0 triggers showing visualization.
 * 
 * @param	progress	The normalized progress value to update to.
 */
void WaitingEntertainerComponent::SetNormalizedProgress(double progress)
{
	// update the progress value member that also is used as reference in progressBar itself!
	m_progressValue = jlimit(double(0.0f), double(1.0f), progress);

	// if a progress value to be shown on ui is set, verify that the component is visible
	if (progress >= 0.0f && !isVisible())
	{
		m_progressBarSlider = std::make_unique<Slider>();
		m_progressBarSlider->setRange(0.0f, 100.0f, 1.0f);
		m_progressBarSlider->setSliderStyle(Slider::LinearBar);
		m_progressBarSlider->setTextValueSuffix("%");
		addAndMakeVisible(m_progressBarSlider.get());

		setVisible(true);
		setAlwaysOnTop(true);
	}
	// if an invalid progress value is set, hide the component
	else if (progress < 0.0f && isVisible())
	{
		setVisible(false);
		setAlwaysOnTop(false);

		removeChildComponent(m_progressBarSlider.get());
		m_progressBarSlider.reset();
	}

	// trigger main component to repaint+resize
	if (getParentComponent())
	{
		getParentComponent()->resized();
		getParentComponent()->repaint();
	}
	else
		jassertfalse;

	// update progress bar slider value
	if (m_progressBarSlider)
		m_progressBarSlider->setValue(static_cast<double>(m_progressValue * 100.0f), juce::NotificationType::sendNotificationSync);
}

/**
 * Convenience method to trigger showing the progress visualization.
 * This implementation forwards the call to SetNormalizedProgress method with
 * progress value 0, to trigger showing visu.
 */
void WaitingEntertainerComponent::Show()
{
	if (!isVisible())
		SetNormalizedProgress(0.0f);
}

/**
 * Convenience method to trigger hiding the progress visualization.
 * This implementation forwards the call to SetNormalizedProgress method with
 * progress value -1, to trigger hiding visu.
 */
void WaitingEntertainerComponent::Hide()
{
	SetNormalizedProgress(-1.0f);
}

/**
 * Reimplemented method to handle changed look and feel data.
 * This makes shure the close buttons' svg images are colored correctly.
 */
void WaitingEntertainerComponent::lookAndFeelChanged()
{
	// first forward the call to base implementation
	Component::lookAndFeelChanged();

}

/**
 * Reimplemented to paint the overlay's background.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void WaitingEntertainerComponent::paint(Graphics& g)
{
	// Transparent background overlay
	g.setColour(Colours::black);
	g.setOpacity(0.5f);
	g.fillRect(getLocalBounds());
}

/**
 * Reimplemented to resize and re-postion controls & labels.
 */
void WaitingEntertainerComponent::resized()
{
	auto progressBarHeight = 30;
	auto progressBarWidth = 0.5f * getWidth();

	auto progressBarHMargin = static_cast<int>(0.25f * getWidth());
	auto progressBarVMargin = static_cast<int>(0.5f * (getHeight() - progressBarHeight));

	auto progressBarBounds = getLocalBounds().reduced(progressBarHMargin, progressBarVMargin);

	m_progressBarSlider->setBounds(progressBarBounds);
}


} // namespace SpaConBridge
