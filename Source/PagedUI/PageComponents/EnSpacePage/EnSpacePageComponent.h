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

#include "../StandalonePollingPageComponentBase.h"


namespace SpaConBridge
{


/**
 * class EnSpacePageComponent provides control for DS100 scene transport.
 */
class EnSpacePageComponent :	public StandalonePollingPageComponentBase,
								public TextButton::Listener,
								public Slider::Listener
{
public:
	enum EnSpaceRoomId
	{
		ESRI_Off = 0,
		ESRI_ModernSmall,
		ESRI_ClassicSmall,
		ESRI_ModernMedium,
		ESRI_ClassicMedium,
		ESRI_ModernLarge,
		ESRI_ClassicLarge,
		ESRI_ModernMedium2,
		ESRI_TheatreSmall,
		ESRI_Cathedral,
		ESRI_Max,
	};

public:
	explicit EnSpacePageComponent();
	~EnSpacePageComponent() override;

	String GetEnSpaceRoomIdName(EnSpaceRoomId id);

	//==============================================================================
	void UpdateGui(bool init) override;

	//==========================================================================
	void buttonClicked(Button* button) override;

	//==========================================================================
	void sliderValueChanged(Slider* slider) override;

	//==========================================================================
	void lookAndFeelChanged() override;

protected:
	void HandleObjectDataInternal(RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData) override;

private:
	std::map<int, std::unique_ptr<TextButton>>	m_roomIdButtons;

	std::unique_ptr<Label>	m_preDelayFactorLabel;
	std::unique_ptr<Slider>	m_preDelayFactorSlider;
	std::unique_ptr<Label>	m_rearLevelLabel;
	std::unique_ptr<Slider>	m_rearLevelSlider;

	bool m_preDelayFactorChangePending{ false };
	float m_preDelayFactorChange{ 0.0f };
	bool m_rearLevelChangePending{ false };
	float m_rearLevelChange{ 0.0f };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnSpacePageComponent)
};


} // namespace SpaConBridge
