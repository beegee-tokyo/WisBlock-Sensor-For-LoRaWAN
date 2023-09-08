| <center><img src="./assets/rakstar.jpg" alt="RAKstar" width=25%></center>  | ![RAKWireless](./assets/RAK-Whirls.png) | [![Build Status](https://github.com/RAKWireless/RAK13600-PN532/workflows/RAK%20Library%20Build%20CI/badge.svg)](https://github.com/RAKWireless/RAK13600-PN532/actions) |
| -- | -- | -- |

# RAK12034

The BMX160 is a highly integrated, low power 9-axis sensor that provides precise acceleration and angular rate (gyroscopic) and geomagnetic measurement in each spatial direction.<br>
The BMX160 contains 16 bit digtial,triaxial accelerometer 16 bit digital, triaxial gyroscope and geomagnetic sensor.<br>
This library provides basic support for configuring functions and reading data.

[*RAKwireless RAK13600 RFID reader*](https://store.rakwireless.com/products/rfid-module-rak13600)

# Documentation

* **[Product Repository](https://github.com/RAKWireless/RAK13600-PN532)** - Product repository for the RAKWireless RAK12034 module.
* **[Documentation](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK13600/Overview/)** - Documentation and Quick Start Guide for the RAK12034 module.

# Installation

In Arduino IDE, open Sketch->Include Library->Manage Libraries then search for RAK12034 .

In PlatformIO open PlatformIO Home, switch to libraries and search for RAK12034 .
Or install the library project dependencies by adding:

```log
lib_deps = rakwireless/RAK12034-BMX160 library
```

into **`platformio.ini`**

For manual installation, download the archive, unzip it and place the RAK12034-BMX160 folder into the library directory.
In Arduino IDE this is usually <arduinosketchfolder>/libraries/
In PlatformIO this is usually <user/.platformio/lib>

# Usage

The library provides an BMX160 class that allows you to communicate with BMX160 and get the chip data. Check out the examples how to get the chip data.

## This class provides the following methods:
**`bool begin();`**    
Set the i2c addr and init the i2c.
Parameters:

| Direction | Name | Function |
| --------- | ---- | :------- |
| return    |          | true Initialization succeeded,false Initialization failed |

**` void readReg(uint8_t reg, uint8_t *pBuf, uint16_t len);`**    
Get the sensor IIC data.
Parameters:

| Direction | Name | Function                             |
| --------- | ---- | ------------------------------------ |
| in        | reg  | reg register                         |
| out       | pBuf | get the store and buffer of the data |
| in        | len  | len data length to be readed         |
| return    |      | none                                 |

**` void writeReg(uint8_t reg, uint8_t *pBuf, uint16_t len);`**    
write the sensor IIC data.
Parameters:

| Direction | Name | Function                               |
| --------- | ---- | -------------------------------------- |
| in        | reg  | reg register                           |
| in        | pBuf | write the store and buffer of the data |
| in        | len  | len data length to be readed           |
| return    |      | none                                   |

**` void writeBmxReg(uint8_t reg, uint8_t value);`**    
Write data to the BMX register.
Parameters:

| Direction | Name  | Function                         |
| --------- | ----- | -------------------------------- |
| in        | reg   | reg register                     |
| in        | value | data written to the BMX register |
| return    |       | none                             |

**`void setGyroRange(eGyroRange_t bits);`**    
Set gyroscope angular rate range and resolution.
Parameters:

| Direction | Name | Function                                        |
| --------- | ---- | ----------------------------------------------- |
| in        | bits | set gyroscope angular rate range and resolution |
| return    |      | none                                            |

**`void setAccelRange(eAccelRange_t bits);`**    
Allow the selection of the accelerometer g-range.
Parameters:

| Direction | Name | Function |
| --------- | ---- | -------- |
| in | bits | set the accelerometer g-range |
| return | | none |

**` void getAllData( sBmx160SensorData_t *magn,  sBmx160SensorData_t *gyro,  sBmx160SensorData_t *accel);`**    
Get the magn, gyro and accel data 
Parameters:

| Direction | Name | Function |
| --------- | ---- | -------- |
| in        | magn  | to store the magn data  |
| in | gyro | to store the gyro data |
| in | accel | to store the accel data |
| return | | none |

**`bool softReset();`**     
Reset bmx160 hardware.
Parameters:

| Direction | Name | Function |
| --------- | ---- | -------- |
| return |  | true reset succeeded, false reset  failed |

**`void setLowPower();`**    
Disabled the the magn, gyro sensor to reduce power consumption.
Parameters:

| Direction | Name | Function |
| --------- | ---- | -------- |
| return    |         | none |

**`void wakeUp();`**     
Enabled the the magn, gyro sensor.
Parameters:

| Direction | Name | Function |
| --------- | ---- | -------- |
| return    |      | none     |

**` int32_t getTemperature(float *temp);`**    
Get BMX160 Temperature.
Parameters:

| Direction | Name | Function               |
| --------- | ---- | ---------------------- |
| out       | temp | get BMX160 Temperature |
| return    |      | none                   |

**`void InterruptConfig(uint8_t interrupt_enable, uint8_t interrupt_high_th);`**    
Set BMX160 Interrupt.
Parameters:

| Direction | Name              | Function                        |
| --------- | ----------------- | ------------------------------- |
| in        | interrupt_enable  | enable HIGH_G_Interrupt         |
| in        | interrupt_high_th | set the accelerometer threshold |
| return    |                   | none                            |

**`void ODR_Config(uint8_t accelCfg_odr, uint8_t gyroCfg_odr);`**    
Set BMX160 Output data rate.
Parameters:

| Direction | Name         | Function                           |
| --------- | ------------ | ---------------------------------- |
| in        | accelCfg_odr | set accelerometer output data rate |
| in        | gyroCfg_odr  | set  gyroscope output data rate    |
| return    |              | none                               |

**`void get_ORD_Config(float *accelCfg_odr, float *gyroCfg_odr);`**    
Get BMX160 Output data rate.
Parameters:

| Direction | Name         | Function                           |
| --------- | ------------ | ---------------------------------- |
| out       | accelCfg_odr | get accelerometer output data rate |
| out       | gyroCfg_odr  | get gyroscope output data rate     |
| return    |              | none                               |
