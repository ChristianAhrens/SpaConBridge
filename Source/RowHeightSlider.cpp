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


#include "RowHeightSlider.h"

#include "LookAndFeel.h"


namespace SoundscapeBridgeApp
{


/*
===============================================================================
 Class RowHeightSlider
===============================================================================
*/

/**
 * Object constructor.
 */
RowHeightSlider::RowHeightSlider(const String& componentName)
	: Component(componentName)
{
	m_slider = std::make_unique<Slider>(Slider::LinearHorizontal, Slider::NoTextBox);
	m_slider->addListener(this);
	addAndMakeVisible(m_slider.get());

	lookAndFeelChanged();
}

/**
 * Object destructor.
 */
RowHeightSlider::~RowHeightSlider()
{
}

/**
 * Setter method for the internal listener object member.
 * This listener will be notified of changes made on the
 * value of the internal slider member.
 * @param	listener	The new listner object to set as internal listener memeber object pointer.
 */
void RowHeightSlider::SetListener(RowHeightListener* listener)
{
	m_listener = listener;
}

void RowHeightSlider::SetSliderRange(double min, double max, double interval)
{
	if (m_slider)
		m_slider->setRange(min, max, interval);
}

/**
 * Reimplemented to handle changed slider values
 * @param	slider	The slider object that triggered the value change
 */
void RowHeightSlider::sliderValueChanged(Slider* slider)
{
	if (m_slider.get() == slider && m_listener)
	{
		m_listener->rowHeightChanged(static_cast<int>(m_slider->getValue()));
	}
}

/**
 * Reimplemented to correctly handle drawable icon colouring
 */
void RowHeightSlider::lookAndFeelChanged()
{
	Component::lookAndFeelChanged();

	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (dblookAndFeel)
	{

		auto svg_xml = XmlDocument::parse(BinaryData::height24px_svg);
		m_arrowComponent = Drawable::createFromSVG(*(svg_xml.get()));
		m_arrowComponent->replaceColour(Colours::black, dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::TextColor));
		addAndMakeVisible(m_arrowComponent.get());

		if (m_slider)
			m_slider->setColour(Slider::backgroundColourId, dblookAndFeel->GetDbColor(DbLookAndFeelBase::DbColor::MidColor));
	}
}

/**
 * Reimplemented to paint background and logo.
 * @param g		Graphics context that must be used to do the drawing operations.
 */
void RowHeightSlider::paint(Graphics& g)
{
	auto cornerSize = 2.0f;
	auto boxBounds = getLocalBounds();

	g.setColour(getLookAndFeel().findColour(ComboBox::backgroundColourId));
	g.fillRoundedRectangle(boxBounds.toFloat(), cornerSize);

	g.setColour(getLookAndFeel().findColour(ComboBox::outlineColourId));
	g.drawRoundedRectangle(boxBounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1.0f);
}

/**
 * Reimplemented to resize and re-postion controls on the overview window.
 */
void RowHeightSlider::resized()
{
	auto bounds = getLocalBounds();

	auto isPortrait = bounds.getHeight() > bounds.getWidth();
	if (isPortrait)
	{
		auto drawableHeight = static_cast<int>(0.8f * getLocalBounds().getWidth());

		if (m_arrowComponent)
		{
			auto drawableBounds = getLocalBounds().removeFromTop(drawableHeight).reduced(1);
			drawableBounds.reduce(2, 0);
			m_arrowComponent->setTransformToFit(drawableBounds.toFloat(), RectanglePlacement::fillDestination);
		}

		if (m_slider)
		{
			if (m_slider->getSliderStyle() != Slider::LinearVertical)
				m_slider->setSliderStyle(Slider::LinearVertical);

			auto sliderBounds = getLocalBounds().removeFromBottom(getLocalBounds().getHeight() - (drawableHeight - 2));
			m_slider->setBounds(sliderBounds);
		}
	}
	else
	{
		auto drawableWidth = static_cast<int>(0.8f * getLocalBounds().getHeight());

		if (m_arrowComponent)
		{
			auto drawableBounds = getLocalBounds().removeFromLeft(drawableWidth).reduced(1);
			drawableBounds.reduce(0, 2);
			m_arrowComponent->setTransformToFit(drawableBounds.toFloat(), RectanglePlacement::fillDestination);
		}

		if (m_slider)
		{
			if (m_slider->getSliderStyle() != Slider::LinearHorizontal)
				m_slider->setSliderStyle(Slider::LinearHorizontal);

			auto sliderBounds = getLocalBounds().removeFromRight(getLocalBounds().getWidth() - (drawableWidth - 2));
			m_slider->setBounds(sliderBounds);
		}
	}
}


} // namespace SoundscapeBridgeApp
