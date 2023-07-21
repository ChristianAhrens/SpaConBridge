/* Copyright (c) 2020-2023, Christian Ahrens
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

#include "StandalonePollingBase.h"

#include "Controller.h"


namespace SpaConBridge
{

/**
 * Constructor
 */
StandalonePollingBase::StandalonePollingBase()
{
	auto ctrl = SpaConBridge::Controller::GetInstance();
	if (ctrl)
		ctrl->AddProtocolBridgingWrapperListener(this);
}

/**
 * Destructor
 */
StandalonePollingBase::~StandalonePollingBase()
{
	stopTimer();
}

/**
 * Setter for the timer refresh interval ms time value.
 * This implementation also starts the timer with the given interval value.
 * @param	rateInMs	The timer interval value in ms.
 */
void StandalonePollingBase::setRefreshRateMs(int rateInMs)
{
	m_refreshRateMs = rateInMs;

	startTimer(m_refreshRateMs);
}

/**
 * Reimplemented callback method that is triggered cyclically to do custom stuff.
 * We use it here to trigger a refresh of the objects configured as standalone polling.
 */
void StandalonePollingBase::timerCallback()
{
	triggerPollOnce();
}

/**
 * Method to handle object data responses.
 * This includes filtering objects that have been added to internal 'monitoring' list and forwarding that data to
 * internal handling method of derived class instance.
 * @param	objectId	The remote object identifier of the object that shall be handled.
 * @param	msgData		The remote object message data that was received and shall be handled.
 */
void StandalonePollingBase::HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData)
{
	// disregard all data that does not originate from relevant processing node (may not occur, since this application only uses a single bridging node!)
	if (nodeId != DEFAULT_PROCNODE_ID)
		return;

	// disregard all data that does not originate from a DS100
	if (senderProtocolId != DS100_1_PROCESSINGPROTOCOL_ID && senderProtocolId != DS100_2_PROCESSINGPROTOCOL_ID)
		return;

	// from here on we require the controller singleton to be available
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// enshure that in parallel extension mode, only the polling request answers of the active device are processed
	if (ctrl->GetExtensionMode() == EM_Parallel)
	{
		// Do neither handle any protocol data from second DS100 if first is set as active one...
		if (ctrl->GetActiveParallelModeDS100() == APM_1st && senderProtocolId != DS100_1_PROCESSINGPROTOCOL_ID)
			return;
		// ...nor any protocol data from first DS100 if second is set as active one in parallel extension mode.
		if (ctrl->GetActiveParallelModeDS100() == APM_2nd && senderProtocolId != DS100_2_PROCESSINGPROTOCOL_ID)
			return;
	}

	// only forward the data corresponding to relevant remote objects to derived implementations
	if (m_objectsForStandalonePolling.count(objectId) > 0)
		if (std::find(m_objectsForStandalonePolling.at(objectId).begin(), m_objectsForStandalonePolling.at(objectId).end(), msgData._addrVal) != m_objectsForStandalonePolling.at(objectId).end())
			HandleObjectDataInternal(objectId, msgData);
}

/**
 * Helper method to do the sending of poll request messages through controller interface.
 */
void StandalonePollingBase::triggerPollOnce()
{
	auto ctrl = Controller::GetInstance();
	if (!ctrl)
		return;

	// send an empty polling request message for each object in our container
	for (auto const& object : m_objectsForStandalonePolling)
	{
		for (auto const& addressing : object.second)
		{
			RemoteObjectMessageData romd(addressing, ROVT_NONE, 0, nullptr, 0);
			ctrl->SendMessageDataDirect(object.first, romd);
		}
	}
}

/**
 * Getter for the map of vectors of the objects that are registered for 'monitoring'.
 * @return	The map of vectors of the objects that are registered for 'monitoring'.
 */
const std::map<RemoteObjectIdentifier, std::vector<RemoteObjectAddressing>>& StandalonePollingBase::GetStandalonePollingObjects() const
{
	return m_objectsForStandalonePolling;
}

/**
 * Setter for the map of vectors of the objects that are registered for 'monitoring'.
 * @param	objects	The map of vectors of the objects that shall be used for 'monitoring'.
 */
void StandalonePollingBase::SetStandalonePollingObjects(const std::map<RemoteObjectIdentifier, std::vector<RemoteObjectAddressing>>& objects)
{
	m_objectsForStandalonePolling = objects;
}

/**
 * Helper method to add a single remote object (incl. addressing) to the map of vectors of the objects that are registered for 'monitoring'.
 * @param	roi		The id of the remote object to add.
 * @param	addressing	The addressing of the remote object.
 */
void StandalonePollingBase::AddStandalonePollingObject(const RemoteObjectIdentifier& roi, const RemoteObjectAddressing& addressing)
{
	if (m_objectsForStandalonePolling.count(roi) > 0)
		if (std::find(m_objectsForStandalonePolling.at(roi).begin(), m_objectsForStandalonePolling.at(roi).end(), addressing) != m_objectsForStandalonePolling.at(roi).end())
			return; // abort and dont append the incoming object, since it already is present in our object container

	m_objectsForStandalonePolling[roi].push_back(addressing);
}


}
