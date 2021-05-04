# SpaConBridge Changelog
All notable changes to SpaConBridge will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]

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
