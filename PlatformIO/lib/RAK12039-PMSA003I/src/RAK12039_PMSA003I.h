/**
   @file RAK12039_PMSA003I.h
   @author rakwireless.com
   @brief  PMSA003I IC library.
   @version 0.1
   @date  2022-01-07
   @copyright Copyright (c) 2022
**/

#ifndef __RAK12039_PMSA003I_H__
#define __RAK12039_PMSA003I_H__

#include <Arduino.h>
#include <Wire.h>

#define LIB_DEBUG_PMS     0

#if LIB_DEBUG_PMS > 0
  #define LIB_LOG_PMS(...)                    \
    {                                     \
      Serial.printf("<%s>",__FUNCTION__); \
      Serial.printf("<%d>",__LINE__);     \
      Serial.printf(__VA_ARGS__);         \
      Serial.printf("\n");                \
      delay(10);                          \
    }
#else
  #define LIB_LOG_PMS(...)
#endif

/*!
 *  @brief  Macro definition.
 */
#define PMSA003I_DEV_ADDR   0x12
#define PMSA003I_REG_ID     0x00
#define PMSA003I_CHIP_ID    0x42

#pragma pack(1)
typedef struct 
{
  uint16_t  \
  header,
  date_len,      
  pm10_standard,  
  pm25_standard,
  pm100_standard,
  pm10_env,
  pm25_env,
  pm100_env,
  particles_03um,
  particles_05um, 
  particles_10um,
  particles_25um,
  particles_50um,
  particles_100um;
  uint8_t  version;
  uint8_t  error_code;    // Error code.
  uint16_t check_sum;     // Check code= Start character 1+Start character 2+……..+Data 13 lower eight bits.
} PMSA_Data_t;
#pragma pack()

/*!
 *  @brief  Class that stores state and functions for interacting with
 *          PM2.5 Air Quality Sensor
 */
class RAK_PMSA003I 
{
public:
  RAK_PMSA003I(byte addr = PMSA003I_DEV_ADDR);
  bool begin(TwoWire &wirePort = Wire, uint8_t deviceAddress = PMSA003I_DEV_ADDR);
  bool readDate(PMSA_Data_t *data);

private:
  void readRegister(uint8_t registerAddress, uint8_t *readData, uint8_t size);
  TwoWire *_i2cPort = &Wire; // The generic connection to user's chosen I2C hardware
  uint8_t _deviceAddress;
};

#endif
