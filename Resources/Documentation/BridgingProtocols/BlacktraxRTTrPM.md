## Blacktrax RTTrPM bridging settings

![Showreel.013.png](../Showreel/Showreel.013.png "Blacktrax RTTrPM Bridging Settings")

Minimal video tutorial for example setup with Blacktrax simulation software (Simulator software package can be obtained from Blacktrax at https://blacktrax.cast-soft.com/developer/):

_[![Example setup with Blacktrax simulation software](https://img.youtube.com/vi/uIS1dVCuTkE/0.jpg)](https://www.youtube.com/watch?v=uIS1dVCuTkE)_


### Implemented Soundscape remote objects for protocol bridging

| RTTrPM packet module | Internal remote object | |
| -- | -- | -- |
| _TrackedPointPosition_ | Mapped Sound Object Position XY **or** Absolute Sound Object Position XY | _Depending on config, either absolute or relative object with DS100 target mapping area id filled in from config is used._ |
| _TrackedPointAccelerationAndVelocity_ | _not used_ | |
| _CentroidPosition_ | _not used_ | |
| _CentroidAccelerationAndVelocity_ | _not used_ | |
| _OrientationQuaternion_ | _not used_ | |
| _OrientationEuler_ | _not used_ | |
| _ZoneCollisionDetection_ | _not used_ | |