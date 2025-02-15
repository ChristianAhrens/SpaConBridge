# SpaConBridge Changelog
All notable changes to SpaConBridge will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added

### Changed

### Fixed

## [0.14.1] 2024-10-03
### Added

### Changed
- Restrict iPhone target to landscapeRight and portrait

### Fixed
- Fixed iOS screen indent/island handling
- Updated included modules for general fixes

## [0.14.0] 2024-07-07
### Added

### Changed
- Updated JUCE framework to v8 for improved windows performance (Direct2D)

### Fixed
- Fixed minor stability and performance issues

## [0.13.5] 2024-04-09
### Added
- Added support for handling and simulation of more objects in DS100 emulation mode 'None'

### Changed
- Modified Ocp1 protocol communication to use synchronous thread-decoupled approach for enhanced performance

### Fixed

## [0.13.4] 2024-03-21
### Added
- Added support for more remote objects to OCP1 DS100 communication protocol

### Changed

### Fixed
- Fixed DS100 'Extend' mode to behave correctly when used with bridging to multiple 3rd party protocols
- Fixed ADM OSC incoming polar coordinates processing

## [0.13.3] 2023-12-11
### Added

### Changed

### Fixed
- Fixed bridging processing when 'None' DS100 protocol dummy data is used

## [0.13.2] 2023-12-10
### Added

### Changed
- Refactored Names, Scenes, En-Space related 'Standalone' remote object handling to leverage subscription mechanism where possible (OCP1 + None)

### Fixed
- Fixed OSC processing performance by updating submodule JUCE to develop incl. DatagramSocket buffer size fix
- Fixed NoProtocol dummy animation for En-Space related objects

## [0.13.1] 2023-11-14
### Added
- Added DS100 connection protocol option 'None' dummy data animation option (Off/Circular/Random)

### Changed

### Fixed
- Fixed iOS config xml loading (and dbpr project + csv mapping loading as well) - FINALLY
- Fixed multi sound object 'all' visualization updating on protocol type change and correctly handle lookandfeel changes

## [0.13.0] 2023-10-29
### Added
- Added DS100 connection protocol option 'None', enabling operation without physical device, incl. optional loading of speaker setup and coordinate mapping settings data from dbpr projects

### Changed
- Updated JUCE submodule version to v7.0.8

### Fixed
- Fixed 'all' ma.pping area multi Soundobject visualization mouse interaction for rotated or unusually parametrized mapping areas
- Fixed occasional incorrect online indication state
- Fixed runtime lookAndFeel change text editor text updating
- Fixed protocol bridging to not forward DS100 data acknowledge messages to 3rd party that triggered the change

## [0.12.1] 2023-09-24
### Added

### Changed
- Modified default config to enable showing Soundobject names

### Fixed
- Fixed excessive CPU resource usage for xml processing by introducing 'bridging mute' state caching
- Fixed ADM OSC bridging xyz sending when combined value sending is configured
- Fixed 'all' mapping area multi Soundobject visualization aspect ratio
- Fixed OCP1/AES70 sending of relative y position values as combination of cached x, z and modified y value
- Fixed MIDI learner popup to show menu anchored to the learner component instead of mouse cursor position

## [0.12.0] 2023-09-01
### Added
- Added support for visualizing all coordinate mappings and loudspeaker positions in MultiSoundobject UI (and read them from connected DS100 devices ofc)

### Changed
- Modified BlackTrax RTTrPM protocol bridging settings section to no longer show 'Beta' background
- Modified default config to use settings page as starting point

### Fixed

## [0.11.2] 2023-08-31
### Added
- Added support for BlackTrax RTTrPM protocol bridging x/y axis swapping for 'Relative' coordinate handling
- Added support for BlackTrax RTTrPM protocol bridging x/y axis inversion for both 'Relative' and 'Absolute' corrdinate handling

### Changed
- Modified 'Own IP Address' display to show 'Multi' when the host has more than one valid IPs (clickable for list)

### Fixed
- Fixed multiple data handling issues in bridging module RPBC
- Fixed ZeroconfDiscoverComponent popup to appear at random positions
- Fixed settings page sections layouting issue
- Fixed performance bottlenecks

## [0.11.1] 2023-08-03
### Added
- Added support for BlackTrax RTTrPM protocol bridging x/y axis swapping and origin offset
- Added support for Scenes and En-Space object handling when using AES70/OCP1 (OCA) protocol

### Changed

### Fixed
- Fixed data bridging forwarding for 3rd party value resending after internal changes
- Fixed handling of MatrixNode, MatrixInput and MatrixOutput DelayEnable OSC objects
- Fixed app freeze when entering invalid ip address values for DS100 connections

## [0.11.0] 2023-07-16
### Added
- Added support for AES70/OCP1 protocol for DS100 communication (testing, ALPHA)
  - Only objects handled / visible in UI supported for bridging
  - Scenes + En-Space releated objects still in development (WIP)

### Changed

### Fixed

## [0.10.7] 2023-07-16
### Added

### Changed

### Fixed
- Fixed refreshing Soundobject name when data is received

## [0.10.6] - 2023-06-01
### Added

### Changed
- Modified BlackTrax RTTrPM bridging settings to let the user only choose between Centroid and LED position handling
- Modified BlackTrax RTTrPM markdown documentation to refer to actual tracking system config instead of Simulation config

### Fixed
- Fixed multiple BlackTrax RTTrPM processing issues
- Fixed BlackTrax RTTrPM bridging UI 'forwarding mute' button functionality

## [0.10.4] - 2023-05-06
### Added

### Changed
- Streamlined Linux build scripts and fixed make config

### Fixed
- Fixed crash when changing DS100 extension mode config
- Fixed BlackTrax RTTrPM settings beaconIdx to Soundobject assignment dialog to correctly update
- Fixed BlackTrax RTTrPM bridging to refer beaconIdx 1 to Soundobject 1 as default

## [0.10.3] - 2023-04-17
### Added

### Changed

### Fixed
- Fixed iOS mdns DS100 device discovery by adding corresp. entitlement to AppID and .entitlements
- Fixed default Rx+Tx communication mode being active not only in ui but processing also
- Fixed OSCProtocolProcessor shutdown on reception of invalid/unkown OSC input

## [0.10.2] - 2023-04-08
### Added
- Added support for defining coordinate range (origin + extent) for BlackTrax RTTrPM positioning data when using coordinate mapping interpretation
- Added support for BlackTrax RTTrPM beacon index to soundobject number user defined remapping

### Changed
- Changed default SoundObject, MatrixInput and MatrixOutput communication mode to both Rx+Tx being active

### Fixed

## [0.10.0] - 2023-03-25
### Added
- Added drag&drop support to undock tab pages to separate windows

### Changed

### Fixed
- Fixed MatrixIO page change handling that was causing settings and in consequence DS100 conection to be continuously restarted
- Fixed changed protocol data changes to be correctly synced to other active protocols

## [0.9.4] - 2023-03-08
### Added
- Added dropdown to BlackTrax RTTrPM Bridging section for selection of 'Module Type' to use for positioning data

### Changed

### Fixed
- Fixed Soundobject details editor to not send initial set of object values when opened

## [0.9.3] - 2023-03-04
### Added

### Changed
- Improved overall performance when interacting with Soundobject/MatrixInput/MatrixOutput tables

### Fixed
- Fixed Soundobject position value handling to avoid retransmissions leading to jittering position visualization on UI
- Fixed Soundobject 'active' (polling) state sometimes not behaving as configured on UI
- Fixed OSC+RTTrPM protocols UDP socket port reopening when changed on UI

## [0.9.1] - 2023-01-28
### Added

### Changed
- Improved button image handling performance
- Improved 'Customized OSC' protocol bridging to use default channel/record id info if none is provided in custom osc config
- Improved 'Customized OSC' protocol bridging config UI layouting
- Updated JUCE submodule version to v7.0.5

### Fixed
- Fixed MIDI assignments to be cleared from runtime config when deleted on UI (was previously only done on application restart)
- Fixed Soundobject selection handling mixup
- Fixed rotation/scaling in optional extended soundobject multiselection interaction on MultiSoundobject UI
- Fixed 'Customized OSC' protocol bridging to be available for muting per Soundobject/MatrixInput/MatrixOutput

## [0.9.0] - 2023-01-08
### Added
- Added 'Customized OSC' protocol bridging, enabling the user to configure custom OSC strings to use with d&b Soundscape

### Changed
- Modified ADM OSC width parameter '/w' to use range 0...1 instead of '0°...180°' according to latest spec 0.5
- Updated JUCE submodule version to v7.0.4

### Fixed 
- Fixed macOS Zeroconf discovery in DS100 settings
- Fixed MIDI bridging command assignment mixup (Scene Index assignment cmd mixup)

## [0.8.7] - 2022-12-24
### Added
- Added optional extended soundobject multiselection interaction on MultiSoundobject UI
- Added option to ADM OSC bridging settings to select if xy coords shall be sent as combined or individual messages

### Changed
- Changed unused settings page sections to be in a minimized collapsed state when unused
- Promoted ADM OSC bridging to Beta instead of Alpha feature
- Degraded DAW plugin protocol bridging to Alpha feature, since several issues have been revealed and require further investigation

### Fixed
- Fixed sending ADM coordinate system announcement message only once when connection is established

## [0.8.6] - 2022-11-20
### Added
- Support for selection of Soundobject selection groups via MIDI or OSC protocol command
- Displaying of all relevant SpaConBridge host IPs when not only a single one can be determined as relevant in TextEditor contextmenu

### Changed

### Fixed
- Fixed MIDI command assignment for MatrixInput_Mute, Scene_Next, Scene_Previous single triggering
- Fixed various issues with MIDI command handling and assignment mapping

## [0.8.4] - 2022-11-08
### Added
- Soundobject xy Position modification for all selected objects when dragging in empty area of MultiSoundobject UI

## [0.8.3] - 2022-11-07
### Added
- Soundobject selection group storing and recalling for enhanced usability of MultiSoundobject UI

### Changed
- Changed default color for new soundobjects on MultiSoundobject UI to known yellowish color for better contrasting default appearance
- Tweaked soundobject label font minimum size on MultiSoundobject UI to still be readable

### Fixed
- Fixed multisoundobject slider to not change xy position when performing multitouch spread/enspacegain change

## [0.8.1] - 2022-10-02
### Added
- Two-finger multitouch support on MultiSoundobject UI to modify Spread Factor and EnSpace Send Gain for multiple Soundobjects at a time (when performing gesture without selecting a Soundobject first)

### Changed

### Fixed
- Fixed Soundobject mouseUp positing messup when using ALT fake multitouch
- Fixed restoration of SoundobjectTablePage resizebar position restoring from config on app start
- Fixed soundobject selecting in multisoundobjectslider when in 'handle selected only' mode

## [0.8.0] - 2022-09-18
### Added
- User selectable single-/multiselection mode in SoundobjectTable
- Two-finger multitouch support on MultiSoundobject UI to modify Spread Factor and EnSpace Send Gain
- Option to reset config to default in raw config view on settings page

### Changed
- Dynamically hide elements of TableControlBar depending on available screen space

### Fixed
- Fixed SoundobjectTable page remaining blank on initial app start until changing tabs manually

## [0.7.2] - 2022-08-29
### Added
- Soundobject 'currently dragged' crosshair inidication on MultiSoundobject UI
- SoundobjectTable resize bar triple-dot drag option indication

### Changed
- Better SoundobjectTable resize bar usability through increased width on touch enabled platforms
- Better SoundobjectTable multiselection usability for iOS/iPadOS (row click toggles selection)

### Fixed
- Linux buildability

## [0.7.1] - 2022-08-28
### Added
- Extended support for multiselection in SoundObject table (Multi SoundObject UI shown instead of single SoundObject Editor)

### Changed
- Refactored SoundObjects page layouting to provide a UI resizing slider inbetween right/left, top/down UI contents
- Updated JUCE framework reference to 7.0.2
- Updated submodule JUCE-AppBasics for improved xml config handling performance (multithreaded disk IO)

### Fixed

## [0.6.1] - 2022-06-28
### Added

### Changed
- Updated JUCE framework reference to 7.0.0

### Fixed
- Fixed SoundObject colour picker refreshing issue
- Fixed SoundObject Mute/RxTx Button colour shading depending on irrelevant Keyboard input focus state

## [0.6.0] - 2022-04-03
### Added
- Added support for SceneRecall via MIDI (extended config UI + MIDI protocol processing)
- Added Zeroconf discovery of DS100 devices on Windows OS
- Added fullscreen kiosk mode on Windows OS
- Added support for d&b DAW plugin bridging
- Added support for disabling Generic OSC 'return channel'

### Changed
- Replaced Servus zeroconf discovery with mdns based selfmade implementation

### Fixed

## [0.5.2] - 2021-12-28
### Added
- Added support for external querying of all currently known bridging values via OSC
- Added Projucer VisualStudio2022 exporter

### Changed
- Removed Zeroconf DS100 discovery UI elements for platforms that are not supported (Windows)
- Modified bottom left help button to open the section of help contents that match the currently active page

### Fixed
- Fixed compilation with JUCE 6.1.3
- Fixed macOS filesystem access by disabling AppSandbox usage

## [0.5.0] - 2021-11-30
### Added
- Added support for displaying channel names in SoundObjects and MultiSlider Page
- Added support for ADM-OSC bridging protocol ('Alpha' feature)
- Added support for Windows installer creation using Inno Setup

### Changed

### Fixed
- Fixed Online state visualization (Restrict to only monitor DS100 communication, not other protocol communication.)

## [0.4.2] - 2021-11-02
### Added

### Changed
- Improved error handling when loading/saving files

### Fixed

## [0.4.1] - 2021-11-01
### Added
- Added support for background images in multi Soundobjects page
- Added support for custom colour and size of Soundobjects in multi Soundobjects page

### Changed

### Fixed
- Fixed file open/save crash by implementing usage of asynchronous in favour of synchronous dialog interaction (JUCE 6.1 compatibility)
- Fixed compilation errors regarding JUCE 6.1 update

## [0.4.0] - 2021-08-15]
### Added
- Added optional visualization of reverb send gain and spread factor to multislider page
- Added support for MatrixInput and -Output Gain/Mute to MIDI Bridging

### Changed
- Introduced massive performance improvement regarding configuration change handling (app start&quit, SoundObject/MatrixInput/MatrixOutput batch add&remove, xml config loading)

### Fixed
- Fixed xml config file loading
- Fixed MIDI handling for SoundObject Delay Mode in case three note on/off triggers are assigned

## [0.3.5] - 2021-07-03
### Added
- Added possiblity to batch-add multiple Soundobject-/MatrixInput-/MatrixOutput-Processors to tables at once

### Changed
- Introduced major refactoring regarding bridging protocol communication 'Mute' state handling to operate on individual remote objects instead of channel numbers

### Fixed
- Fixed sending Soundobject Delay Mode UI value changes by using correct remote object type ROVT_INT
- Fixed EnSpace Page floating point value precision issue that could lead to Predelay Factor and Rear Level faders not updating on incoming value change

## [0.3.4] - 2021-06-13
### Fixed
- Fixed text editors in tables always grabbing keyboard focus on row selection (solves annoying onscreen keyboard popping up on iOS)
- Fixed Scenes page scene indices to not being pinnable if already pinned (avoids access violation on remove and issues with element layouting)
- Fixed Scenes page scene index/name/comment updating to not rely on slow refreshing but react instantaneous on recall/next/previous/pinnedrecall

## [0.3.3] - 2021-05-21
### Added
- Added "Pinnable" Scenes (create custom Scene Index recall buttons by "pinning" current Scene)
- Added "General Settings" section to settings page, allowing enabling/disabling of pages to be shown on UI, and moved existing look and feel selection dropdown from bottom bar to "General Settings" section

### Changed
- Introduced major performance improvement regarding all UI interactions that affect the application xml configuration

### Fixed
- App stability and minor bugs

## [0.3.2] - 2021-05-04
### Added
- Added new tab "Scenes" allowing control of previous/next/recall remote objects and displaying Scene index/name/comment strings
- Added new tab "EnSpace" allowing control of currently active room, predelay factor and rear level

### Changed
- Implemented support for OSC string type messages

### Fixed
- Fixed crash on iOS that occured when cancelling the file load and save dialogs
- Fixed SoundObject Spread, ReverbSendGain and DelayMode messages to be sent to DS100 in correct format
- Fixed RTTrPMProtocolProcessor to not block entire node processing shutdown that lead to active OSC traffic even though the app was set to 'offline' on UI

## [0.3.1] - 2021-04-20
### Added
- Added button to manually toggle between online/offline

### Changed
- Improved dual DS100 "Parallel" communication mode by adding selection of active device

### Fixed
- Various bugfixes

## [0.3.0] - 2021-04-12
### Added
- Added Matrix Inputs/Outputs tables page

### Changed
- Refactored overall tables implementation to be usable for Soundobject-, MatrixInput- and MatrixOutputTable

### Fixed
- Various bugfixes

## [0.2.5] - 2021-02-22
### Added
- Added dual DS100 Signal Engine parallel mode
- Added dual DS100 Signal Engine mirror mode with dynamic master/slave determination

## [0.2.4] - 2021-02-21
### Added
- Added dual DS100 Signal Engine mirror mode with dynamic master/slave determination

### Changed
- Enhanced DS100 'online' connection status visualization for 'extend' and 'mirror' modes

### Fixed
- Various bugfixes

## [0.2.3] - 2021-02-17
### Fixed
- Fixed issue with multiplexing values between two DS100 when in extension mode

## [0.2.2] - 2021-02-07
### Added
- Added support for assigning MIDI input commands to select internal remote object through dedicated MIDI learner component

### Changed
- Introduced major refactoring to internal protocol bridging (now multithreaded)
- Modified protocol bridging to only forward changed remote object values
- Introduced major stability and performance improvements
