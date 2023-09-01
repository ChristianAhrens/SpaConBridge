/* Copyright (c) 2023, Christian Ahrens
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

#include "ProtocolBridgingWrapper.h"

namespace SpaConBridge
{

class StandalonePollingBase : public ProtocolBridgingWrapper::Listener, public juce::Timer
{
public:
	//==============================================================================
	explicit StandalonePollingBase();
	~StandalonePollingBase() override;

	//==============================================================================
	void setRefreshRateMs(int rateInMs);
	void restartTimer();

protected:
	//==============================================================================
	void timerCallback() override;

	//==============================================================================
	const std::map<RemoteObjectIdentifier, std::vector<RemoteObjectAddressing>>& GetStandalonePollingObjects() const;
	void SetStandalonePollingObjects(const std::map<RemoteObjectIdentifier, std::vector<RemoteObjectAddressing>>& objects);
	void AddStandalonePollingObject(const RemoteObjectIdentifier& roi, const RemoteObjectAddressing& addressing);

	//==============================================================================
	void triggerPollOnce();

	//==============================================================================
	void HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData) override;

	//==============================================================================
	virtual void HandleObjectDataInternal(const RemoteObjectIdentifier& roi, const RemoteObjectMessageData& msgData) = 0;

private:
	//==============================================================================
	int m_refreshRateMs { 0 };
	std::map<RemoteObjectIdentifier, std::vector<RemoteObjectAddressing>>	m_objectsForStandalonePolling;	/**< Objects that are registered for 'monitoring' */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandalonePollingBase)
};

}
