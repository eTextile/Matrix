## eTextile Matrix Sensor - E256 / Pure Data
> A simple patch made to work with the E256 eTextile Matrix Sensor connected via USB to any host.

####  For an explanation of the E256 communication protocol see the [FIRMWARE_README](../Firmware/README.md "FIRMWARE_README").

![preview](https://user-images.githubusercontent.com/9277107/80432942-ea484480-8938-11ea-800a-5bbaf9c863eb.png)

### Requirements
 - [Pure Data](http://puredata.info/downloads)
    - Miller S. Puckette's "vanilla" distribution of Pd is perfect.
 - [Externals](https://puredata.info/docs/faq/how-do-i-install-externals-and-help-files)
    - `comport`, for Virtual COMs
    - `mrpeach`, for SLIP encoding
    - `OSC`, for OSC packing

### Overview

Our entry point is `Get_matrix_OSC.pd` with `e256_input_filters.pd` acting as a sub-patch.

##### Input

The patch establishes a connection over with the `comport` object. However, there are no assumptions made about which port the e256 is available on, so be sure to adjust `open 1` where "1" is your COM port.

##### Configuration

The embedded _e256_SLIP-OSC_input_ sub-patch handles communication with the e256 via SLIP encoded OSC. There are 3 UI objects to adjust for **calibration**, **on/off** and **threshold** adjustment.

##### Output

The output is an OSC stream, available whenever the threshold is met, and sent across the `udpsend` object. This can be received within the patch via the `udpreceive` object, and printed to the console, or sent elsewhere, outside of Pure Data.