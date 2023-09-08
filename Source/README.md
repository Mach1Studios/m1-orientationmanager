# M1-OrientationManager

## Hardware : Defining a new data transmission type
- New communitation protocol is defined via `Hardware[comm].h`, these describe ways to transmit data from a headtracking device to this server. Simply include the [`HardwareAbstract.h`](HardwareAbstract.h) and override the needed functions

### [Existing Data Transmissions](../#currently-supports)

## Devices : Defining and new headtracking device using an existing "Hardware" data transmission type
- New devices can be declared within the [`Devices/`](Devices) directory, please reference existing examples to best interface with the OSCServer

### [Existing Devices Supported](../#currently-supports)