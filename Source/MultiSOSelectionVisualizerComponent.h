/* Copyright (c) 2020-2022, Christian Ahrens
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


#pragma once

#include <JuceHeader.h>


namespace SpaConBridge
{


/**
 * MultiSOSelectionVisualizerComponent is a helper to encapsulate painting of
 * custom multiselection visualization and touch/click interaction
 */
class MultiSOSelectionVisualizerComponent : public juce::Component
{

public:
	MultiSOSelectionVisualizerComponent();
	virtual ~MultiSOSelectionVisualizerComponent() override;

	//==========================================================================
	void SetSelectionVisuActive(bool active = true);
	const std::vector<juce::Point<float>>& GetSelectionPoints();
	void SetSelectionPoints(const std::vector<juce::Point<float>>& points);
	void UpdateSelectionPoints(const std::vector<juce::Point<float>>& points);

	bool IsSelectionVisuActive();

	//==========================================================================
	bool IsPrimaryInteractionActive();
	bool IsSecondaryInteractionActive();

	//==========================================================================
	std::function<void()> onMouseInteractionStarted = nullptr;
	std::function<void(const juce::Point<int>&)> onMouseXYPosChanged = nullptr;
	std::function<void(const juce::Point<int>&)> onMouseXYPosFinished = nullptr;
	std::function<void(const juce::Point<float>&, const float, const float)> onMouseRotAndScaleChanged = nullptr;
	std::function<void(const juce::Point<float>&, const float, const float)> onMouseRotAndScaleFinished = nullptr;

protected:
	//==========================================================================
	void paint(Graphics& g) override;

	void mouseDown(const MouseEvent& e) override;
	void mouseDrag(const MouseEvent& e) override;
	void mouseUp(const MouseEvent& e) override;

private:
	//==========================================================================
	void lookAndFeelChanged() override;

	//==========================================================================
	const juce::Point<float> DeriveSecondaryHandleFromCOG(const juce::Point<float>& cog);

	//==========================================================================
	bool							m_selectionVisuActive{ false };
	std::vector<juce::Point<float>>	m_selectionPoints;
	bool							m_currentlyPrimaryInteractedWith{ false };
	bool							m_currentlySecondaryInteractedWith{ false };

	juce::Point<float>				m_startCOG;
	juce::Point<float>				m_startSecondaryHandle;

	juce::Point<float>				m_currentVirtCOG;
	juce::Point<float>				m_currentVirtSecondaryHandle;

	juce::Colour					m_multitselectionIndicationColour;

	std::unique_ptr<XmlElement>		m_cog_svg_xml;
	std::unique_ptr<XmlElement>		m_secHndl_svg_xml;
	std::unique_ptr<juce::Drawable>	m_cog_drawable;
	std::unique_ptr<juce::Drawable>	m_secHndl_drawable;

	float m_handleSize{ 0.0f };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiSOSelectionVisualizerComponent)
};


} // namespace SpaConBridge
