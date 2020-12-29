## Generic MIDI bridging settings

![Showreel.012.png](../Showreel/Showreel.012.png "Generic MIDI bridging settings")

### Supported MIDI values/commands

_Currently some hardcoded mapping of MIDI commands to internal remote objects is implemented. This is for testing purposes and resulted from a testsetup with a AKAI MKP mini mk2 (which has a fancy joystick!). A futur implementation will allow configuring custom MIDI-to-RemoteObject mappings, maybe using a kind of MIDI learn mode._

| MIDI command | Internal remote object | |
| -- | -- | -- |
| _MIDI pitch wheel_ | Mapped Sound Object Position XY      | _incoming value range 0...16383 is mapped to X value 0...1_ |
| _MIDI controller (ctrl 2)_ | Mapped Sound Object Position XY      | _incoming value range 0...127 is mapped to X value 0...1_ |
| _MIDI controller (ctrl 1)_ | Mapped Sound Object Position XY      | _incoming value range 0...127 is mapped to Y value 0...1_ |
| _MIDI controller (ctrl 5)_ | Matrix Input ReverbSendGain          | _incoming value range 0...127 is mapped to value 0...1_ |
| _MIDI controller (ctrl 6)_ | Sound Object Spread                  | _incoming value range 0...127 is mapped to value 0...1_ |
| _MIDI NoteOn 48+ (C3)_ | Matrix Input Select                    | _used to externally (de-)select a sound object_ |
| _MIDI NoteOn 48+ (C3)_ | SoundscapeBridgeApp Sound Object Select | _used to externally (de-)select a sound object_ |