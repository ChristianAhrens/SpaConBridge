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


namespace SpaConBridge
{


/**
 *	Custom reimplementation of a Texteditor that simply shows
 *	the systems current IP and if this is not unique,
 *	a popup with all alternative IPs the host system uses.
 */
class IPAddressDisplay : public TextEditor
{
public:
	IPAddressDisplay();

	void addPopupMenuItems(PopupMenu& menuToAddTo, const MouseEvent* mouseClickEvent) override;

	const std::vector<juce::IPAddress> getRelevantIPs();

protected:
	void mouseDown(const MouseEvent& e) override;

private:
	bool IsMultiCast(const juce::IPAddress& address);
	bool IsUPnPDiscoverAddress(const juce::IPAddress& address);
	bool IsLoopbackAddress(const juce::IPAddress& address);
	bool IsBroadcastAddress(const juce::IPAddress& address);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IPAddressDisplay)
};


} // namespace SpaConBridge
