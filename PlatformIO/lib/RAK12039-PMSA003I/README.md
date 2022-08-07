| <center><img src="./assets/rakstar.jpg" alt="RAKstar" width=25%></center>  | ![RAKWireless](./assets/RAK-Whirls.png) | [![Build Status](https://github.com/RAKWireless/RAK13005-TLE7259-Library/workflows/RAK%20Library%20Build%20CI/badge.svg)](https://github.com/RAKWireless/RAK13005-TLE7259-Library/actions) |
| -- | -- | -- |

# RAK12039

The RAK12039 uses the PMSA003I sensor, which is a digital universal particle concentration sensor. Can be used to collect PM1.0, PM2.5 and PM10.0 concentration in both standard & environmental units. This library implements reading and writing of PMSA003I data through the IIC interface.

[*RAKWireless <RAK#> <function>*](https://store.RAKWireless.com/products/lin-bus-module-rak13005)

# Documentation

* **[Product Repository](https://github.com/RAKWireless/RAK12039-PMSA003I -Library)** - Product repository for the RAKWireless RAK12039 Dust module.
* **[Documentation](https://docs.RAKWireless.com/Product-Categories/WisBlock/RAK12039/Overview/)** - Documentation and Quick Start Guide for the RAK12039 Dust module.

# Installation

In Arduino IDE open Sketch->Include Library->Manage Libraries then search for RAK12039.

In PlatformIO open PlatformIO Home, switch to libraries and search for RAK12039.
Or install the library project dependencies by adding

```log
lib_deps =
  RAKWireless/RAKWireless PMSA003I dust library
```

into **`platformio.ini`**

For manual installation download the archive, unzip it and place the RAK12039-PMSA003I -Library folder into the library directory.
In Arduino IDE this is usually <arduinosketchfolder>/libraries/
In PlatformIO this is usually <user/.platformio/lib>

# Usage

This library provides a RAK_PMSA003I class that communicates with the PMSA003I digital general purpose particulate matter concentration sensor. These examples show how to use RAK12039.

- [RAK12039_Dust_Read_PMSA003I](./examples/RAK12039_Dust_Read_PMSA003I) Get PMSA003I sensor data and output data on the serial port.

## This class provides the following methods:

**bool begin(TwoWire &wirePort, uint8_t deviceAddress)**

Initalizes the PMSA003I sensor.

#### Parameters:

| Direction | Name          | Function                                                     |
| --------- | ------------- | ------------------------------------------------------------ |
| in        | wirePort      | IIC interface used.                                          |
| in        | deviceAddress | Device address should be 0x12.                               |
| return    |               | If the device init successful return true else return false. |

**bool readDate(PMSA_Data_t *pmsa_data) **

Read all data of PMSA003I.

#### Parameters:

| Direction | Name      | Function                                                     |
| --------- | --------- | ------------------------------------------------------------ |
| in        | pmsa_data | Read data pointer, refer to the data structure @PMSA_Data_t. |
| return    |           | Returns true if the read data is successful, otherwise returns false. |

**void readRegister(uint8_t registerAddress, uint8_t *readData, uint8_t size) **

I2c bus read.

#### Parameters:

| Direction | Name            | Function                        |
| --------- | --------------- | ------------------------------- |
| in        | registerAddress | Register address.               |
| in        | readData        | Read data pointer.              |
| in        | size            | The length of the written data. |
| return    |                 | none                            |
