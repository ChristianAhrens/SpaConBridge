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


#include "EnSpacePageComponent.h"

#include "../../../Controller.h"


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
	: StandalonePollingPageComponentBase(PCT_EnSpace)
{
	AddStandalonePollingObject(ROI_MatrixSettings_ReverbRoomId, RemoteObjectAddressing());

	if (GetElementsContainer())
		GetElementsContainer()->setHeaderText("En-Space - Room");

	for (int i = ESRI_Off; i < ESRI_Max; i++)
	{
		m_roomIdButtons[i] = std::make_unique<TextButton>(GetEnSpaceRoomIdName(static_cast<EnSpaceRoomId>(i)));
		m_roomIdButtons.at(i)->addListener(this);
		if (GetElementsContainer())
			GetElementsContainer()->addComponent(m_roomIdButtons.at(i).get(), true, false);
	}
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
 * Reimplemented method to handle updated object data for objects that have been added for standalone polling.
 * @param	objectId	The remote object identifier of the object that shall be handled.
 * @param	msgData		The remote object message data that was received and shall be handled.
 */
void EnSpacePageComponent::HandleObjectDataInternal(RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData)
{
	if (objectId != ROI_MatrixSettings_ReverbRoomId)
		return;
	if (msgData._addrVal != RemoteObjectAddressing())
		return;
	if (msgData._valCount != 1)
		return;
	if (msgData._valType != ROVT_INT)
		return;
	if (msgData._payloadSize != sizeof(int))
		return;

	auto newRoomId = static_cast<EnSpaceRoomId>(*static_cast<int*>(msgData._payload));

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


} // namespace SpaConBridge
