# M1-OrientationManager

## Hardware : Defining a new data transmission type
- New communitation protocol is defined via `Hardware[comm].h`, these describe ways to transmit data from a headtracking device to this server. Simply include the [`HardwareAbstract.h`](HardwareAbstract.h) and override the needed functions

### Existing Data Transmissions:
- [Serial](Source/HardwareSerial.h) _using rs232_
- [BLE](Source/HardwareBLE.h) _using simpleble_
- [OSC](Source/HardwareOSC.h)
- [Camera](Source/HardwareCamera.h) _[WIP]_
- [Emulator](Source/HardwareEmulator) _[primarily for debug]_

## Devices : Defining and new headtracking device using an existing "Hardware" data transmission type
- New devices can be declared within the [`Devices/`](Devices) directory, please reference existing examples to best interface with the OSCServer

### Existing Devices Supported:
- [Supperware IMU](https://supperware.co.uk/)
- [MetaWear/mBientLab IMUs](https://mbientlab.com/)
- [M1 IMU](https://www.mach1.tech/products)
- [WitMotion IMUs](https://www.wit-motion.com/) _[WIP]_
