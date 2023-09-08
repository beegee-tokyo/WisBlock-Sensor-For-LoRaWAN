/**
   @file ADC_SGM58031.cpp
   @author rakwireless.com
   @brief This code is designed to config SGM58031 ADC device and handle the data
   @version 1.0
   @date 2022-01-19
   @copyright Copyright (c) 2022
*/

#include "ADC_SGM58031.h"

/**
   @brief Create the interface object using hardware IIC
 **/
RAK_ADC_SGM58031::RAK_ADC_SGM58031(int addr)
{
  _wire = &Wire;
  i2cAddress = addr;
}

/**
   @brief Create the interface object using hardware IIC
 **/
RAK_ADC_SGM58031::RAK_ADC_SGM58031()
{
  _wire = &Wire;
  i2cAddress = SGM58031_DEFAULT_ADDRESS;
}

/**
   @brief Create the interface object using hardware IIC
 **/
RAK_ADC_SGM58031::RAK_ADC_SGM58031(TwoWire *w, int addr)
{
  _wire = w;
  i2cAddress = addr;
}


/**
   @brief i2c initialization
 **/
void RAK_ADC_SGM58031::begin() {
  _wire->begin();
}

/**
   @brief Create the interface object using hardware IIC
 **/
RAK_ADC_SGM58031::RAK_ADC_SGM58031(TwoWire *w)
{
  _wire = w;
  i2cAddress = SGM58031_DEFAULT_ADDRESS;
}

/**
   @brief Writes 8-bits to the specified destination register
   @param reg   Register address
   @param data  Send data
   @return Returns 0 on success, and others on failure
 **/
uint8_t RAK_ADC_SGM58031::writeByteRegister(uint8_t reg, uint8_t data)
{
  _wire->beginTransmission(i2cAddress);
  _wire->write(reg);
  _wire->write(data);
  return _wire->endTransmission();
}

/**
   @brief Writes 16-bits to the specified destination register
   @param reg   Register address
   @param data  Send data
   @return Returns 0 on success, and others on failure
 **/
uint8_t RAK_ADC_SGM58031::writeWordRegister(uint8_t reg, uint16_t data)
{
  _wire->beginTransmission(i2cAddress);
  _wire->write(reg);
  _wire->write((uint8_t)(data >> 8));
  _wire->write((uint8_t)(data & 0xFF));
  return _wire->endTransmission();
}

/**
   @brief Reads 8-bits from the specified destination register
   @param reg   Register address
   @return the specified destination register value
 **/
uint8_t RAK_ADC_SGM58031::readByteRegister(uint8_t reg)
{
  uint8_t regValue = 0;
  _wire->beginTransmission(i2cAddress);
  _wire->write(reg);
  _wire->endTransmission();
  _wire->requestFrom(i2cAddress, 1);
  if (_wire->available())
  {
    regValue = _wire->read();
  }
  return regValue;
}

/**
   @brief Reads 16-bits from the specified destination register
   @param reg   Register address
   @return the specified destination register value
 **/
uint16_t RAK_ADC_SGM58031::readWordRegister(uint8_t reg)
{
  uint8_t regValue[2] = {0};
  _wire->beginTransmission(i2cAddress);
  _wire->write(reg);
  _wire->endTransmission();
  _wire->requestFrom(i2cAddress, 2);
  if (_wire->available())
  {
    for (uint8_t count = 0; count < 2; count++)
    {
      regValue[count] = _wire->read();
    }
  }
  return (uint16_t)(regValue[0] << 8 | regValue[1]);
}

/**
   @brief Read the voltage of sensor output
   @return Voltage value after conversion
 **/
float RAK_ADC_SGM58031::getVoltage()
{
  float voltage;
  voltage = getAdcValue();
  voltage = voltage * ReferenceVoltage / 32767.0; 
  return voltage;
}

/**
   @brief set the resolution voltage
   @param value  the resolution voltage value
 **/
void RAK_ADC_SGM58031::setVoltageResolution(float value)
{
  ReferenceVoltage = value;
}

/**
   @brief get the resolution voltage
   @return the resolution voltage
 **/
float RAK_ADC_SGM58031::getVoltageResolution()
{
  return ReferenceVoltage;
}

/**
   @brief Sets the lower limit threshold used to determine the alert condition
   @param value  the lower limit threshold
 **/
void RAK_ADC_SGM58031::setAlertLowThreshold(uint16_t threshold)
{
  writeWordRegister(SGM58031_LOW_THRESH_REGISTER, threshold);
}

/**
   @brief Read the lower limit threshold value
   @return the lower limit threshold value
 **/
uint16_t RAK_ADC_SGM58031::readAlertLowThreshold()
{
  return readWordRegister(SGM58031_LOW_THRESH_REGISTER);
}

/**
   @brief Sets the upper limit threshold used to determine the alert condition
   @param value  the upper limit threshold
 **/
void RAK_ADC_SGM58031::setAlertHighThreshold(uint16_t threshold)
{
  writeWordRegister(SGM58031_HIGH_THRESH_REGISTER, threshold);
}

/**
   @brief Read the upper limit threshold value
   @return the upper limit threshold value
 **/
uint16_t RAK_ADC_SGM58031::readAlertHighThreshold()
{
  // mask off the invalid bits in case they were set
  return readWordRegister(SGM58031_HIGH_THRESH_REGISTER);
}

/**
   @brief set the 16-bit register can be used to control the SGM58031 operating mode, input selection, data rate, PGA settings,
  and comparator modes
   @param data  the config data
 **/
void RAK_ADC_SGM58031::setConfig(uint16_t data)
{
  writeWordRegister(SGM58031_CONFIG_REGISTER, data);
}

/**
   @brief Gets the value of config Register
   @return the value of config Register
 **/
uint16_t RAK_ADC_SGM58031::getConfig()
{
  return readWordRegister(SGM58031_CONFIG_REGISTER);
}

/**
   @brief set the 16-bit register can be used to control the SGM58031 new controls
   @param data  the config1 data
 **/
void RAK_ADC_SGM58031::setConfig1(uint16_t data)
{
  writeWordRegister(SGM58031_CONFIG1_REGISTER, data);
}

/**
   @brief Gets the value of config1 Register
   @return the value of config1 Register
 **/
uint16_t RAK_ADC_SGM58031::getConfig1()
{
  return readWordRegister(SGM58031_CONFIG1_REGISTER);
}

/**
   @brief Gets the chip ID
   @return the Chip Id
 **/
uint16_t RAK_ADC_SGM58031::getChipID()
{
  return readWordRegister(SGM58031_CHIP_ID_REGISTER);
}

/**
   @brief ADC gain coefficient for user selecting Config1 register EXT_REF bit as reference
   @param data  the GN_Trim1 data
 **/
void RAK_ADC_SGM58031::set_GN_Trim1(uint16_t data)
{
  writeWordRegister(SGM58031_GN_TRIM1_REGISTER, data);
}

/**
   @brief Gets the value of GN_Trim1 Register
   @return the value of GN_Trim1 Register
 **/
uint16_t RAK_ADC_SGM58031::get_GN_Trim1()
{
  return readWordRegister(SGM58031_GN_TRIM1_REGISTER);
}

/**
   @brief Gets the Conversion Register value
   @return the value of Conversion Register
 **/
uint16_t RAK_ADC_SGM58031::getAdcValue()
{
  return readWordRegister(SGM58031_CONVERSION_REGISTER);
}
