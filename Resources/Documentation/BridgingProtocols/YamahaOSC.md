## Yamaha OSC bridging settings

Yamaha OSC bridging support is purely experimental - the commands as described below should be sent by a Plugin available in Rivage series consoles - this has yet to be verified.

![Showreel.013.png](../Showreel/Showreel.013.png "Yamaha OSC bridging settings")

### Supported OSC messages

| OSC input from Rivage series console | Internal remote object | |
| -- | -- | -- |
| _/ymh/src/[CH]/p_ | Mapped Sound Object Position X | _DS100 target mapping area id is filled in from config_ |
| _/ymh/src/[CH]/d_ | Mapped Sound Object Position Y | _DS100 target mapping area id is filled in from config_ |
| _/ymh/src/[CH]/s_ | Matrix Input ReverbSendGain | |
| _/ymh/src/[CH]/w_ | Sound Object Spread | |