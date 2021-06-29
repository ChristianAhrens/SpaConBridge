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


#pragma once

#include <JuceHeader.h>

#include "LookAndFeel.h"
#include "SpaConBridgeCommon.h"


namespace SpaConBridge
{


/**
 * Class WaitingEntertainerComponent provides a progressbar on top of everything else with shadowed background.
 */
class WaitingEntertainerComponent : public Component
{
public:
	WaitingEntertainerComponent();
	~WaitingEntertainerComponent() override;

	static WaitingEntertainerComponent* GetInstance();
	void DestroyInstance();

	//==============================================================================
	void SetNormalizedProgress(double progress);

	void Show();
	void Hide();

	//==============================================================================
	void lookAndFeelChanged() override;

protected:
	static WaitingEntertainerComponent* m_singleton;	/**< The one and only instance of WaitingEntertainerComponent. */

	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

private:
	std::unique_ptr<Slider>	m_progressBarSlider;
	double					m_progressValue{ 0.0f };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaitingEntertainerComponent)
};


} // namespace SpaConBridge
