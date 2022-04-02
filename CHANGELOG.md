# SpaConBridge Changelog
All notable changes to SpaConBridge will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- Added support for SceneRecall via MIDI (extended config UI + MIDI protocol processing)
- Zeroconf discovery of DS100 devices on Windows OS
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
