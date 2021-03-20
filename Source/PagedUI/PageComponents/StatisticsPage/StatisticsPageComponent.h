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

#include "../PageComponentBase.h"

#include "../../../SpaConBridgeCommon.h"
#include "../../../ProtocolBridgingWrapper.h"
#include "../../../AppConfiguration.h"


namespace SpaConBridge
{


/**
 * Forward declarations.
 */
class StatisticsPlot;
class StatisticsLog;


/**
 * Class StatisticsPageComponent is a component that contains elements for
 * protocol traffic plotting and logging
 */
class StatisticsPageComponent : public PageComponentBase,
								public ProtocolBridgingWrapper::Listener,
								public AppConfiguration::Watcher
{
public:
	StatisticsPageComponent();
	~StatisticsPageComponent() override;

	//==============================================================================
	void UpdateGui(bool init) override;

	//==========================================================================
	void onConfigUpdated() override;

protected:
	//==============================================================================
	void paint(Graphics&) override;
	void resized() override;

	//==========================================================================
	void HandleMessageData(NodeId nodeId, ProtocolId senderProtocolId, RemoteObjectIdentifier Id, const RemoteObjectMessageData& msgData) override;

private:
	std::unique_ptr<StatisticsPlot>	m_plotComponent;	/**> Plotting component. */
	std::unique_ptr<StatisticsLog>	m_logComponent;		/**> Logging component. */

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatisticsPageComponent)
};


} // namespace SpaConBridge
