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


#include "IPAddressDisplay.h"

#include "LookAndFeel.h"


namespace SpaConBridge
{


/*
===============================================================================
 Class RangeEditorComponent
===============================================================================
*/

/**
* Class constructor.
*/
IPAddressDisplay::IPAddressDisplay()
    : juce::TextEditor()
{
    lookAndFeelChanged(); // Do this first, since all text that is added to a TextEditor uses the colour currently set

    auto ip = juce::IPAddress::getLocalAddress();
    auto allIPs = getRelevantIPs();

    auto hasMultiIPs = allIPs.size() > 1;

    if (hasMultiIPs)
        setText("<<Multiple IPs>>");
    else if (!IsMultiCast(ip) && !IsUPnPDiscoverAddress(ip) && !IsLoopbackAddress(ip) && !IsBroadcastAddress(ip))
        setText(ip.toString());
    else
        setText("<<None>>");

    if (hasMultiIPs)
        setPopupMenuEnabled(true);

    setEnabled(false);
}

/**
    * Reimplemented to create custom popup menu contents: THe ip addresses used by this host instead of copy/cut/paste default actions.
    * @param	menuToAddTo			The menu to populate
    * @param	mouseClickEvent		Initiating mouse event that is ignored
    */
void IPAddressDisplay::addPopupMenuItems(juce::PopupMenu& menuToAddTo, const juce::MouseEvent* mouseClickEvent)
{
    menuToAddTo.clear();
    ignoreUnused(mouseClickEvent);

    // ip address edit tooltip with all other ips than primary if multiple present in system
    auto relevantIpAddresses = getRelevantIPs();
    for (auto const& ip : relevantIpAddresses)
    {
        if (ip.toString() != getText())
        {
            menuToAddTo.addItem(ip.toString(), false, false, std::function<void()>());
        }
    }
    setEnabled(false);
}

/**
 * Reimplemented with the single purpose to have the texteditor text colour reflect its deactive state.
 */
void IPAddressDisplay::lookAndFeelChanged()
{
    juce::TextEditor::lookAndFeelChanged();

    applyColourToAllText(getLookAndFeel().findColour(juce::TextEditor::ColourIds::textColourId).withAlpha(0.7f));
}

/**
    * Helper method that filters results of juce::IPAddress::getAllAddresses to only return the IPs that are in fact of interest to the user on UI.
    * @return  The list of relevant IPs
    */
const std::vector<juce::IPAddress> IPAddressDisplay::getRelevantIPs()
{
    auto relevantIPs = std::vector<juce::IPAddress>();

    auto localIpAddresses = juce::IPAddress::getAllAddresses();
    for (auto const& ip : localIpAddresses)
    {
        if (!IsMultiCast(ip) && !IsUPnPDiscoverAddress(ip) && !IsLoopbackAddress(ip) && !IsBroadcastAddress(ip))
            relevantIPs.push_back(ip);
    }

    return relevantIPs;
}

/**
    * Helper method to test if a given IP is in multicast range
    * @param	address		The address to test
    * @return	True if the ip is in multicast range, false if not
    */
bool IPAddressDisplay::IsMultiCast(const juce::IPAddress& address)
{
    return address.toString().contains("224.0.0.");
}

/**
    * Helper method to test if a given IP is UPnP SSDP discovery ip
    * @param	address		The address to test
    * @return	True if the ip is the UPnP address, false if not
    */
bool IPAddressDisplay::IsUPnPDiscoverAddress(const juce::IPAddress& address)
{
    return address.toString().contains("239.255.255.250");
}

/**
    * Helper method to test if a given IP is loopback ip
    * @param    address        The address to test
    * @return    True if the ip is the loopback address, false if not
    */
bool IPAddressDisplay::IsLoopbackAddress(const juce::IPAddress& address)
{
    return address.toString().contains("127.0.0.1");
}

/**
    * Helper method to test if a given IP is broadcast ip
    * @param    address        The address to test
    * @return    True if the ip is the broadcast address, false if not
    */
bool IPAddressDisplay::IsBroadcastAddress(const juce::IPAddress& address)
{
    return juce::IPAddress::getInterfaceBroadcastAddress(juce::IPAddress::getLocalAddress()) == address;
}

/**
    * Reimplemented mouse click handling to trigger popup even when clicked with primary or touch.
    * @param   e   The mouse event that occured.
    */
void IPAddressDisplay::mouseDown(const MouseEvent& e)
{
    if (e.originalComponent != this)
        return;

    auto eventCopy = MouseEvent(e.source,
        e.position,
        e.mods.withFlags(juce::ModifierKeys::popupMenuClickModifier), // fake a popup menu click, to trigger the popup even on primary click (or touch)
        e.pressure,
        e.orientation,
        e.rotation,
        e.tiltX,
        e.tiltY,
        e.eventComponent,
        e.originalComponent,
        e.eventTime,
        e.mouseDownPosition,
        e.mouseDownTime,
        e.getNumberOfClicks(),
        e.mouseWasDraggedSinceMouseDown());

    juce::TextEditor::mouseDown(eventCopy);
}


} // namespace SpaConBridge
