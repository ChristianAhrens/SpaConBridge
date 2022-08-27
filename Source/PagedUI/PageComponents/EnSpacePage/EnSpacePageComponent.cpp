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


#include "EnSpacePageComponent.h"

#include "../../../Controller.h"
#include "../../../LookAndFeel.h"


namespace SpaConBridge
{


/*
===============================================================================
	Class EnSpacePageComponent
===============================================================================
*/

/**
 * Class constructor.
 */
EnSpacePageComponent::EnSpacePageComponent()
	: StandalonePollingPageComponentBase(UIPageId::UPI_EnSpace)
{
	AddStandalonePollingObject(ROI_MatrixSettings_ReverbRoomId, RemoteObjectAddressing());
	AddStandalonePollingObject(ROI_MatrixSettings_ReverbPredelayFactor, RemoteObjectAddressing());
	AddStandalonePollingObject(ROI_MatrixSettings_ReverbRearLevel, RemoteObjectAddressing());

	if (GetElementsContainer())
		GetElementsContainer()->setHeaderText("En-Space - Room");

	for (int i = ESRI_Off; i < ESRI_Max; i++)
	{
		m_roomIdButtons[i] = std::make_unique<TextButton>(GetEnSpaceRoomIdName(static_cast<EnSpaceRoomId>(i)));
		m_roomIdButtons.at(i)->addListener(this);
		if (GetElementsContainer())
			GetElementsContainer()->addComponent(m_roomIdButtons.at(i).get(), true, false);
	}

	m_preDelayFactorSlider = std::make_unique<Slider>();
	m_preDelayFactorSlider->setRange(
		juce::Range<double>(ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixSettings_ReverbPredelayFactor).getStart(),
							ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixSettings_ReverbPredelayFactor).getEnd()),
		0.1f);
	m_preDelayFactorSlider->addListener(this);
	m_preDelayFactorLabel = std::make_unique<Label>();
	m_preDelayFactorLabel->setJustificationType(Justification::centred);
	m_preDelayFactorLabel->setText("Predelay Factor", dontSendNotification);
	m_preDelayFactorLabel->attachToComponent(m_preDelayFactorSlider.get(), true);
	if (GetElementsContainer())
	{
		GetElementsContainer()->addComponent(m_preDelayFactorLabel.get(), false, false);
		GetElementsContainer()->addComponent(m_preDelayFactorSlider.get(), true, false);
	}

	m_rearLevelSlider = std::make_unique<Slider>();
	m_rearLevelSlider->setRange(
		juce::Range<double>(ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixSettings_ReverbRearLevel).getStart(),
							ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixSettings_ReverbRearLevel).getEnd()),
		0.1f);
	m_rearLevelSlider->addListener(this);
	m_rearLevelSlider->setTextValueSuffix("dB");
	m_rearLevelLabel = std::make_unique<Label>();
	m_rearLevelLabel->setJustificationType(Justification::centred);
	m_rearLevelLabel->setText("Rear Level", dontSendNotification);
	m_rearLevelLabel->attachToComponent(m_rearLevelSlider.get(), true);
	if (GetElementsContainer())
	{
		GetElementsContainer()->addComponent(m_rearLevelLabel.get(), false, false);
		GetElementsContainer()->addComponent(m_rearLevelSlider.get(), true, false);
	}

	lookAndFeelChanged();

	resized();
}

/**
 * Class destructor.
 */
EnSpacePageComponent::~EnSpacePageComponent()
{
}

/**
 * Helper to resolve ESRI enum values to a readable string
 */
String EnSpacePageComponent::GetEnSpaceRoomIdName(EnSpaceRoomId id)
{
	switch (id)
	{
		case ESRI_Off:
			return "Off";
		case ESRI_ModernSmall:
			return "Modern - small";
		case ESRI_ClassicSmall:
			return "Classic - small";
		case ESRI_ModernMedium:
			return "Modern - medium";
		case ESRI_ClassicMedium:
			return "Classic - medium";
		case ESRI_ModernLarge:
			return "Modern - large";
		case ESRI_ClassicLarge:
			return "Classic - large";
		case ESRI_ModernMedium2:
			return "Modern - medium 2";
		case ESRI_TheatreSmall:
			return "Theatre - small";
		case ESRI_Cathedral:
			return "Cathedral";
		case ESRI_Max:
		default:
			return "None";
	}
}

/**
 * Reimplemented to handle button member clicks.
 * @param	button	The button that has been clicked.
 */
void EnSpacePageComponent::buttonClicked(Button* button)
{
	for (int i = ESRI_Off; i < ESRI_Max; i++)
	{
		if (m_roomIdButtons.count(i) == 0)
			jassertfalse;
		else
		{
			if (m_roomIdButtons.at(i).get() == button)
			{
				button->setToggleState(true, dontSendNotification);
				auto ctrl = Controller::GetInstance();
				if (ctrl)
				{
					RemoteObjectMessageData romd(RemoteObjectAddressing(), ROVT_INT, 1, &i, sizeof(int));
					romd._payloadOwned = false;
					ctrl->SendMessageDataDirect(ROI_MatrixSettings_ReverbRoomId, romd);
				}
			}
			else
			{
				button->setToggleState(false, dontSendNotification);
			}
		}
	}
}

/**
 * Reimplemented to handle slider member value changes.
 * @param	slider	The slider that value has changed.
 */
void EnSpacePageComponent::sliderValueChanged(Slider* slider)
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	if (m_preDelayFactorSlider && m_preDelayFactorSlider.get() == slider)
	{
		m_preDelayFactorChangePending = true;
	}
	else if (m_rearLevelSlider && m_rearLevelSlider.get() == slider)
	{
		m_rearLevelChangePending = true;
	}
}

/**
 * Reimplemented method to handle updated object data for objects that have been added for standalone polling.
 * @param	objectId	The remote object identifier of the object that shall be handled.
 * @param	msgData		The remote object message data that was received and shall be handled.
 */
void EnSpacePageComponent::HandleObjectDataInternal(RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData)
{
	if (msgData._addrVal != RemoteObjectAddressing())
		return;
	if (msgData._valCount != 1)
		return;

	switch (objectId)
	{
	case ROI_MatrixSettings_ReverbRoomId:
		{
			if (msgData._valType != ROVT_INT)
				return;
			if (msgData._payloadSize != sizeof(int))
				return;

			auto newRoomId = *static_cast<int*>(msgData._payload);
			auto rriR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixSettings_ReverbRoomId);
			newRoomId = jlimit(static_cast<int>(rriR.getStart()), static_cast<int>(rriR.getEnd()), newRoomId);

			for (int i = ESRI_Off; i < ESRI_Max; i++)
			{
				if (m_roomIdButtons.count(i) == 0)
					jassertfalse;
				else
				{
					auto shouldBeOn = (newRoomId == i);

					if (m_roomIdButtons.at(i))
						m_roomIdButtons.at(i)->setToggleState(shouldBeOn, dontSendNotification);
				}
			}
		}
		break;
	case ROI_MatrixSettings_ReverbPredelayFactor:
		{
		if (msgData._valType != ROVT_FLOAT)
			return;
		if (msgData._payloadSize != sizeof(float))
			return;

		auto newPreDelayFactor = *static_cast<float*>(msgData._payload);

		if (m_preDelayFactorChangePending)
		{
			if (std::roundf(10 * m_preDelayFactorChange) == std::roundf(10 * newPreDelayFactor))
				m_preDelayFactorChangePending = false;
			return;
		}

		auto rpfR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixSettings_ReverbPredelayFactor);
		newPreDelayFactor = jlimit(static_cast<float>(rpfR.getStart()), static_cast<float>(rpfR.getEnd()), newPreDelayFactor);

		if (m_preDelayFactorSlider)
			m_preDelayFactorSlider->setValue(newPreDelayFactor);
		}
		break;
	case ROI_MatrixSettings_ReverbRearLevel:
		{
		if (msgData._valType != ROVT_FLOAT)
			return;
		if (msgData._payloadSize != sizeof(float))
			return;

		auto newReverbRearLevel = *static_cast<float*>(msgData._payload);

		if (m_rearLevelChangePending)
		{
			if (std::roundf(10 * m_rearLevelChange) == std::roundf(10 * newReverbRearLevel))
				m_rearLevelChangePending = false;
			return;
		}

		auto rrLR = ProcessingEngineConfig::GetRemoteObjectRange(ROI_MatrixSettings_ReverbRearLevel);
		newReverbRearLevel = jlimit(static_cast<float>(rrLR.getStart()), static_cast<float>(rrLR.getEnd()), newReverbRearLevel);

		if (m_rearLevelSlider)
			m_rearLevelSlider->setValue(newReverbRearLevel);
		}
		break;
	default:
		break;
	}
}

/**
 * Reimplemented from component to change slider track colours.
 */
void EnSpacePageComponent::lookAndFeelChanged()
{
	Component::lookAndFeelChanged();

	auto dblookAndFeel = dynamic_cast<DbLookAndFeelBase*>(&getLookAndFeel());
	if (!dblookAndFeel)
		return;

	if (m_preDelayFactorSlider)
		m_preDelayFactorSlider->setColour(Slider::trackColourId, dblookAndFeel->GetDbColor(DbLookAndFeelBase::DarkColor).darker());
	if (m_rearLevelSlider)
		m_rearLevelSlider->setColour(Slider::trackColourId, dblookAndFeel->GetDbColor(DbLookAndFeelBase::DarkColor).darker());
}

/**
 * Reimplemented to both trigger the base implementation and also send pending slider value changes.
 * @param	init	True if this update is an initial one, false if it is a regular
 */
void EnSpacePageComponent::UpdateGui(bool init)
{
	StandalonePollingPageComponentBase::UpdateGui(init);

	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	if (m_preDelayFactorChangePending && m_preDelayFactorSlider)
	{
		auto pdfVal = static_cast<float>(m_preDelayFactorSlider->getValue());

		if (m_preDelayFactorChange != pdfVal)
		{
			m_preDelayFactorChange = pdfVal;

			RemoteObjectMessageData romd;
			romd._valType = ROVT_FLOAT;
			romd._valCount = 1;
			romd._payloadOwned = false;
			romd._payloadSize = sizeof(float);
			romd._payload = &m_preDelayFactorChange;
			ctrl->SendMessageDataDirect(ROI_MatrixSettings_ReverbPredelayFactor, romd);
		}
	}
	
	if (m_rearLevelChangePending && m_rearLevelSlider)
	{
		auto rlVal = static_cast<float>(m_rearLevelSlider->getValue());

		if (m_rearLevelChange != rlVal)
		{
			m_rearLevelChange = rlVal;

			RemoteObjectMessageData romd;
			romd._valType = ROVT_FLOAT;
			romd._valCount = 1;
			romd._payloadOwned = false;
			romd._payloadSize = sizeof(float);
			romd._payload = &m_rearLevelChange;
			ctrl->SendMessageDataDirect(ROI_MatrixSettings_ReverbRearLevel, romd);
		}
	}
}


} // namespace SpaConBridge
