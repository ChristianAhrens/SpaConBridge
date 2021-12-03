## Yamaha OSC bridging settings

**Yamaha OSC bridging support is purely experimental - if the commands as described below can actually be used in PM series consoles has yet to be verified/tested. If you are interested in this functionality and have the means to try it out, feedback is welcome!**

![Showreel.019.png](../Showreel/Showreel.019.png "Yamaha OSC bridging settings")


### Implemented OSC messages

| OSC input from PM series console | Internal remote object | |
| -- | -- | -- |
| _/ymh/src/[CH]/p_ | Mapped Sound Object Position X | _DS100 target mapping area id is filled in from config_ |
| _/ymh/src/[CH]/d_ | Mapped Sound Object Position Y | _DS100 target mapping area id is filled in from config_ |
| _/ymh/src/[CH]/s_ | Matrix Input ReverbSendGain | |
| _/ymh/src/[CH]/w_ | Sound Object Spread | |