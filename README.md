![Showreel.001.png](Resources/Documentation/Showreel/Showreel.001.png "SpaConBridge Headline Icons")

SpaConSim is a **PRIVATELY** created and driven project. It currently is in **EXPERIMENTAL** development.

Its sourcecode and prebuilt binaries are made publicly available to enable interested users to experiment, extend and create own adaptations.

There is no guarantee for compatibility inbetween versions or for the implemented functionality to be reliable for professional use at all. Use what is provided here at your own risk!

See [LATEST RELEASE](../../releases/latest) for available binary packages.

SpaConBridge was created with the idea in mind to have both a simple ui to monitor or control Sound Object parameters of a d&b audiotechnik Soundscape system, esp. the DS100 Signal Engine, via OSC protocol and at the same time the possibility of controlling interfacing with external control data input (MIDI device, OSC control app, ...).

It is neither suppported nor driven by official d&b activities.

It was originally inspired by "Soundscape DAW Plugin" made publicly available at https://github.com/dbaudio-soundscape/db-Soundscape-DAW-Plugins and "Soundscape Control with DiGiCo SD consoles" made publicly available at https://github.com/dbaudio-soundscape/db-Soundscape-control-with-DiGiCo-SD-Consoles.


**Known Issues:**

* The app freezes / performs poorly when configuration is changed while data is received (freezes the longer, the more Sound Objects are active)  
__Note:__ Major code refactoring is required to solve this
* Zeroconf is not working in builds for Windows OS  
__Note:__ Needs to be implemented...
* LookAndFeel change does not reliably update colour scheme for all UI elements  
__Note:__ Suspicion: Underlying UI framework issue is the cause for this
* MIDI device listing is not updated during runtime  
__Note:__ In consequence, devices being plugged in / coming online while app is running cannot be used unless app is restarted


<a name="toc" />

## Table of contents

* [Quick Start](#quicksetup)
* [UI details](#uidetails)
  * [Sound Object Table](#soundobjecttable)
    * [Selective Sound Object muting](#soundobjectmuting)
    * [Sound Object Parameter editing](#soundobjectparameterediting)
  * [Multi Sound Object XY Pad](#twodimensionalpositionslider)
  * [Matrix IO Table](#matrixiotable)
  * [Statistics](#protocolbridgingtrafficloggingandplotting)
  * [Settings](#appsettings)
* [Supported Sound Object parameters on UI](#soundobjectuiparameters)
* [Supported Matrix Input parameters on UI](#matrixinputuiparameters)
* [Supported Matrix Output parameters on UI](#matrixoutputuiparameters)


<a name="quicksetup" />

## Quick Start

1. If no DS100 is available, the minimal simulation tool [SpaConSim](https://github.com/ChristianAhrens/SpaConSim) or the generic bridging tool [RemoteProtocolBridgeUI](https://github.com/ChristianAhrens/RemoteProtocolBridgeUI) can be used for testing and debugging.
2. Launch SpaConBridge
    * Sound Object table has no entries, no bridging protocol is active
    * App is 'offline' since no Sound Object is active
3. Add some Sound Objects by clicking the 'Add' button
4. Enable receiving object values from DS100 by toggling the button with 'incoming' arrow symbol for at least one Sound Object in the table
5. Go to [Settings](#appsettings) tab
    * If you do not have a DS100 at hand, now is the time to launch [SpaConSim](https://github.com/ChristianAhrens/SpaConSim) simulation tool
6. Set up the DS100 connection
    * iPhoneOS/iPadOS/macOS: Click on discovery button below IP address text edit field to get a list of devices that announce _osc._udp zeroconf service and choose the DS100 or [SpaConSim](https://github.com/ChristianAhrens/SpaConSim) simulation tool you want to connect to
    * Windows: Enter the IP address of the DS100 or [SpaConSim](https://github.com/ChristianAhrens/SpaConSim) simulation tool you want to connect to manually
7. 'Online' inidicator on bottom right of the UI becomes active
    * If using SpaConSim, adjust the refresh rate slider on the tool's UI to the desired interval (= object value change speed). The tool's UI displays current object value polling rate in the performance metering.
8. Go to [Statistics](#protocolbridgingtrafficloggingandplotting) tab and click on DS100 legend item to activate DS100 protocol traffic plotting/logging.
9. Go to [Sound Object Table](#soundobjecttable) tab and select one of the Sound Objects for which you previously activated receiving object values (button with 'incoming' arrow symbol)
10. Details editor is opened for the Sound Object and shows live object value changes
11. Go to [Multi Sound Object XY Pad](#twodimensionalpositionslider) tab to see the live object value changes for all objects with activated value receiving
13. Go to [Sound Object Table](#soundobjecttable) tab, select a Sound Object and enable sending object values to DS100 by toggling the button with 'outgoing' arrow symbol
    * When modifying the x, y, Reverb, Spread and Mode values through UI, they are now sent to DS100. Note: When using [SpaConSim](https://github.com/ChristianAhrens/SpaConSim), set its update interval to 0, otherwise it will immediately update the just sent value with a new simulated value.
14. You now can set up a bridging protocol in [Settings](#appsettings) tab if you like, following instructions provided for each individual [implemented protocol](#appsettingprotocols)
    * Keep in mind that incoming protocol values are only forwarded to DS100 and not directly shown on UI. The updated values are shown on UI as soon as values are reflected by DS100. Without a working connection to DS100 device or simulation, the UI will not show the changes.
    * [Statistics](#protocolbridgingtrafficloggingandplotting) tab shows bridging traffic and therefor can be used to monitor incoming protocl values without a working connection to DS100 device or simulation.


<a name="uidetails" />

## UI details


<a name="soundobjecttable" />

### Sound Object Table

![Showreel.002.png](Resources/Documentation/Showreel/Showreel.002.png "Sound Object Table Overview")

Every row in the table corresponds to an active Sound Object, meaning that it is shown on UI and its values can be received from DS100. This does not affect the pure protocol bridging in underlying module. E.g. in case an external OSC input sends new positioning values for a channel that is not present in the table, the values will still be bridged to DS100. This needs to be kept in mind if muting the input data from a protocol for a Sound Object is desired!
The table allows multiselection and editing of receive, send and mapping area selection for multiple Sound Objects at a time.

<a name="soundobjectmuting" />

![Showreel.003.png](Resources/Documentation/Showreel/Showreel.003.png "Sound Object Table Bridging Mutes")

For every active bridging protocol, a sub column with mute buttons is presented in 'Bridging' column. This allows muting individual data input related to a Sound Object coming in from bridging protocols.
Table multiselection allows editing of mute state for multiple Sound Objects and bridging protocols at a time.

<a name="soundobjectparameterediting" />

![Showreel.004.png](Resources/Documentation/Showreel/Showreel.004.png "Sound Object Table Selection")

Following the selection in Sound Object table, the corresponding Soundscape parameters are shown in an editing area.
The UI layout is designed to adapt to the available screen real estate of the device SpaConBridge is used on. Depending on the screen geometry a vertical or horizontal layout of table and parameter editing area is used. On small screens, legend annotation and control value displays are hidden. (useful for Tablet landscape vs. portrait rotation, small Smartphone screens, embedded device touchscreens, ...)

The selection in Sound Object table and the currently active tab can be externally controlled by bridging protocols that support the commands 'SpaConBridge Sound Object Select' and 'SpaConBridge UI Element Index Select'.


<a name="twodimensionalpositionslider" />

### Multi Sound Object XY Pad

![Showreel.005.png](Resources/Documentation/Showreel/Showreel.005.png "Multislider")

All Sound Objects assigned to the selected DS100 mapping area are shown simultaneously. The dropdown on the bottom can be used to switch the selection to one of the four available mapping areas.
The selection / multiselection in Sound Object table is followed here and reflected in Sound Object circle sizing - selected objects are shown enlarged.


<a name="matrixiotable" />

### Matrix Inputs/Outputs Table

![Showreel.015.png](Resources/Documentation/Showreel/Showreel.015.png "Matrix IO Table Overview")

On the left side (landscape) or top (portrait) of the page, a table for visualization and control of DS100 matrix input channels is shown and on the right side or bottom of the page, a table for matrix outputs.
Every row in both of the tables corresponds to an active Matrix Input or Output, meaning that it is shown on UI and its values can be received from DS100. This does not affect the pure protocol bridging in underlying module. E.g. in case an external OSC input sends new gain values for a channel that is not present in the table, the values will still be bridged to DS100. This needs to be kept in mind if muting the input data from a protocol for a Matrix Input/Output is desired!



<a name="protocolbridgingtrafficloggingandplotting" />

### Statistics

![Showreel.006.png](Resources/Documentation/Showreel/Showreel.006.png "Protocol Bridging Statistics")

Statistics page shows a graphical representation for current bridging protocol load (messages per second) for every active protocol and a tabular log view of the last 200 received messages. The graphical representation shown describes the raw incoming data rate only and contains no information on actual bridging.
Both plot and log are refreshed at a small rate to keep the performance impact on the host system resources low.


<a name="appsettings" />

### Settings

![Showreel.007.png](Resources/Documentation/Showreel/Showreel.007.png "App Settings")

<a name="appsettingsprotocols" />

Settings page is structured in protocol sections.
DS100 OSC communication protocol is always active. The IP address can be manually configured or, if the build of SpaConBridge for the host system supports this, zeroconf discovery button can be used to select on of the discovered devices. OSC UDP communication ports are hardcoded to match the ones used by DS100.

For details on the settings for the implemented protocols, see the individual documentation
  * [d&b DS100 signal bridge communication](Resources/Documentation/BridgingProtocols/DS100.md)
  * [DiGiCo SD series mixing console communication](Resources/Documentation/BridgingProtocols/DiGiCoOSC.md)
  * [Blacktrax tracking system communication *](Resources/Documentation/BridgingProtocols/BlacktraxRTTrPM.md)
  * [Generic d&b OSC protocol communication](Resources/Documentation/BridgingProtocols/GenericOSC.md)
  * [Generic MIDI communication](Resources/Documentation/BridgingProtocols/GenericMIDI.md)
  * [Yamaha OSC communication *](Resources/Documentation/BridgingProtocols/YamahaOSC.md)
  
  &ast; Implemented using simulation only, entirely untested with real ones.

The bottom bar of settings page contains buttons to save and load entire SpaConBridge configurations, as well as a button to show the raw current configuration in a text editor overlay. The latter is mainly useful for debugging purposes.
Depending on the available horizontal UI resolution, buttons are hidden. 'Show raw config' disappears first, since it is least relevant for use. On host devices as smartphones, rotating from portrait to landscape mode can help to make the buttons visible, in case they are hidden in portrait mode.

![Showreel.014.png](Resources/Documentation/Showreel/Showreel.014.png "Light LookAndFeel")


<a name="soundobjectuiparameters" />

## Supported Soundscape parameters on UI

- Absolute Sound Object Position XY
- Matrix Input ReverbSendGain
- Sound Object Spread
- Sound Object Delay Mode


<a name="matrixinputuiparameters" />

## Supported Matrix Input parameters on UI

- Matrix Input Level (pre mute)
- Matrix Input Gain
- Matrix Input Mute


<a name="martixoutputuiparameters" />

## Supported Matrix Output parameters on UI

- Matrix Output Level (post mute)
- Matrix Output Gain
- Matrix Output Mute
