/**
   @file ADC_SGM58031.h
   @author rakwireless.com
   @brief This code is designed to config SGM58031 ADC device and handle the data
   @version 1.0.1
   @date 2022-01-19

   @copyright Copyright (c) 2022

*/
#ifndef ADC_SGM58031_H
#define ADC_SGM58031_H

#include "Arduino.h"
#include <Wire.h>

// I2C ADDRESS/BITS
#define SGM58031_DEFAULT_ADDRESS (0x48) // 1001 000 (ADDR = GND)
#define SGM58031_VDD_ADDRESS (0x49)     // 1001 001 (ADDR = VDD)
#define SGM58031_SDA_ADDRESS (0x4A)     // 1001 010 (ADDR = SDA)
#define SGM58031_SCL_ADDRESS (0x4B)     // 1001 011 (ADDR = SCL)

// COMMAND BYTE REGISTER
#define SGM58031_CONVERSION_REGISTER (0x00)
#define SGM58031_CONFIG_REGISTER (0x01)
#define SGM58031_LOW_THRESH_REGISTER (0x02)
#define SGM58031_HIGH_THRESH_REGISTER (0x03)
#define SGM58031_CONFIG1_REGISTER (0x04)
#define SGM58031_CHIP_ID_REGISTER (0x05)
#define SGM58031_GN_TRIM1_REGISTER (0x06)

//PGA
#define SGM58031_FS_6_144   6.144 
#define SGM58031_FS_4_096   4.096
#define SGM58031_FS_2_048   2.048
#define SGM58031_FS_1_024   1.024
#define SGM58031_FS_0_512   0.512
#define SGM58031_FS_0_256   0.256

#define DEVICE_ID 0x0080

class RAK_ADC_SGM58031
{
public:
  RAK_ADC_SGM58031();
  RAK_ADC_SGM58031(TwoWire *w);
  RAK_ADC_SGM58031(int addr);
  RAK_ADC_SGM58031(TwoWire *w, int addr);
  
  void begin() ;

  uint8_t writeByteRegister(uint8_t reg, uint8_t data);
  uint8_t writeWordRegister(uint8_t reg, uint16_t data);
  uint8_t readByteRegister(uint8_t reg);
  uint16_t readWordRegister(uint8_t reg);

  void setAlertLowThreshold(uint16_t threshold);  // Sets the lower limit threshold used to determine the alert condition
  uint16_t readAlertLowThreshold();               // read the lower limit threshold from register
  void setAlertHighThreshold(uint16_t threshold); // Sets the hysteresis value used to determine the alert condition
  uint16_t readAlertHighThreshold();              // read the upper limit threshold from register

  void setConfig(uint16_t data);
  uint16_t getConfig();
  void setConfig1(uint16_t data);
  uint16_t getConfig1();

  uint16_t getChipID();
  void set_GN_Trim1(uint16_t data);
  uint16_t get_GN_Trim1();
  uint16_t getAdcValue();


  // Functions to set and get values
  void setVoltageResolution(float value); // the _VOLT_RESOLUTION default is 5.0V if3.3V use 3.3
  float getVoltageResolution();           // readback the ReferenceVoltage
  float getVoltage();

private:
  TwoWire *_wire;
  int i2cAddress;
  float ReferenceVoltage = 3.3; // if referencevoltage 5V use 5.0
};
#endif
