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

#include "PageComponentBase.h"
#include "HeaderWithElmListComponent.h"

#include "../../ProtocolBridgingWrapper.h"


namespace SpaConBridge
{


/**
 * class StandalonePollingPageComponentBase is supposed to be used
 * as base component for pages that use remote objects for internal use only without
 * submitting them as active for bridging.
 */
class StandalonePollingPageComponentBase :	public PageComponentBase,
											public ProtocolBridgingWrapper::Listener
{
public:
	explicit StandalonePollingPageComponentBase(PageComponentType type);
	~StandalonePollingPageComponentBase() override;

	HeaderWithElmListComponent* GetElementsContainer();

	//==============================================================================
	void UpdateGui(bool init) override;

	//==========================================================================
	void paint(Graphics&) override;
	void resized() override;

protected:
	const std::map<RemoteObjectIdentifier, std::vector<RemoteObjectAddressing>>& GetStandalonePollingObjects() const;
	void SetStandalonePollingObjects(const std::map<RemoteObjectIdentifier, std::vector<RemoteObjectAddressing>>& objects);
	void AddStandalonePollingObject(const RemoteObjectIdentifier roi, const RemoteObjectAddressing& addressing);

	//==========================================================================
	void HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData) override;

	//==============================================================================
	virtual void HandleObjectDataInternal(RemoteObjectIdentifier objectId, const RemoteObjectMessageData& msgData) = 0;

private:
	void triggerPollOnce();

	std::map<RemoteObjectIdentifier, std::vector<RemoteObjectAddressing>>	m_objectsForStandalonePolling;	/**< Objects that are registered for 'monitoring' */

	//==============================================================================
	std::unique_ptr<HeaderWithElmListComponent>	m_elementsContainer;
	std::unique_ptr<Viewport>					m_elementsContainerViewport;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StandalonePollingPageComponentBase)
};


} // namespace SpaConBridge