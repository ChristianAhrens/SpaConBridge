## BlackTrax RTTrPM bridging settings

![Showreel.023.png](../Showreel/Showreel.023.png "BlackTrax RTTrPM Bridging Settings")

Minimal demo video for example setup with BlackTrax simulation software (Simulator software package can be obtained from BlackTrax at https://blacktrax.cast-soft.com/developer/):

_[![Example setup with BlackTrax simulation software](https://img.youtube.com/vi/qJaJkVd-tLA/0.jpg)](https://www.youtube.com/watch?v=qJaJkVd-tLA)_


### Tracker simulation network connection setup

![SpaConBridge-Blacktrax-ConnectionSetup.png](BlackTraxRTTrPM/SpaConBridge-Blacktrax-ConnectionSetup.png "BlackTrax RTTrPM Connection Settings")

### Tracker simulation protocol contents setup

![SpaConBridge-Blacktrax-ComContentsSetup.png](BlackTraxRTTrPM/SpaConBridge-Blacktrax-ComContentsSetup.png "BlackTrax RTTrPM Communication Protocol Contents Settings")

### Beacon Index to Soundobject mapping

![SpaConBridge-Blacktrax-BeaconIdxToSoundobjectTraffic.png](BlackTraxRTTrPM/SpaConBridge-Blacktrax-BeaconIdxToSoundobjectTraffic.png "BlackTrax RTTrPM Beacon Idx to Soundobject number")


### Implemented Soundscape remote objects for protocol bridging

| RTTrPM packet module | Internal remote object | |
| -- | -- | -- |
| _TrackedPointPosition_ | Soundobject Position XY* |
| _TrackedPointAccelerationAndVelocity_ | Soundobject Position XY* |
| _CentroidPosition_ | Soundobject Position XY* |
| _CentroidAccelerationAndVelocity_ | Soundobject Position XY* |
| _OrientationQuaternion_ | _not used_ |
| _OrientationEuler_ | _not used_ |
| _ZoneCollisionDetection_ | _not used_ |

*Depending on MappingAreaId config, mapped Soundobject position XY **or** absolute Soundobject position XY objects are generated