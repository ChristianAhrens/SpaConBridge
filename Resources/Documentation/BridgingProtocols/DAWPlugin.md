## d&b Soundscape DAW plugin bridging settings

![Showreel.021.png](../Showreel/Showreel.021.png "Soundscape DAW plugin bridging settings")

_This bridging protocol is the same as [Generic OSC](GenericOSC.md) and is only available separately to allow using both a DiGiCo SD series console and other OSC input at the same time, both using d&b [DS100 OSC protocol specification](https://www.dbaudio.com/assets/products/downloads/manuals-documentation/electronics/dbaudio-osc-protocol-ds100-1.3.0-en.pdf)._


### Supported OSC messages for bridging that are also visible in app UI

| OSC input from SD series console | Internal remote object | |
| -- | -- | -- |
| _/dbaudio1/coordinatemapping/source_position_x_ | Mapped Sound Object Position X      |  |
| _/dbaudio1/coordinatemapping/source_position_y_ | Mapped Sound Object Position Y      |  |
| _/dbaudio1/matrixinput/reverbsendgain_ | Matrix Input ReverbSendGain          |  |
| _/dbaudio1/positioning/source_spread_ | Sound Object Spread                  |  |
| _/dbaudio1/positioning/source_delaymode_ | Sound Object Delay Mode              |  |