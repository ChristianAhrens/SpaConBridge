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


#include "ScenesPageComponent.h"


namespace SpaConBridge
{


/*
===============================================================================
 Class ScenesPageComponent
===============================================================================
*/

/**
 * Class constructor.
 */
ScenesPageComponent::ScenesPageComponent()
	: StandalonePollingPageComponentBase(PCT_Scenes)
{
	AddStandalonePollingObject(ROI_Scene_SceneIndex, RemoteObjectAddressing());
	AddStandalonePollingObject(ROI_Scene_SceneName, RemoteObjectAddressing());
	AddStandalonePollingObject(ROI_Scene_SceneComment, RemoteObjectAddressing());
}

/**
 * Class destructor.
 */
ScenesPageComponent::~ScenesPageComponent()
{
}

/**
 * Reimplemented method to handle updated object data for objects that have been added for standalone polling.
 * @param	objectId	The remote object identifier of the object that shall be handled.
 * @param	msgData		The remote object message data that was received and shall be handled.
 */
void ScenesPageComponent::HandleObjectDataInternal(RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData)
{
	ignoreUnused(msgData);
	DBG(String(__FUNCTION__) + ProcessingEngineConfig::GetObjectDescription(objectId));
}


} // namespace SpaConBridge
