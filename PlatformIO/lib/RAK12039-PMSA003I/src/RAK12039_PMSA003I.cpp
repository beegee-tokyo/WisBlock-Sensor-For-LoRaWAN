/**
   @file RAK12039_PMSA003I.cpp
   @author rakwireless.com
   @brief  PMSA003I Operation Implementation.
   @version 0.1
   @date 2022-01-07
   @copyright Copyright (c) 2022
**/

#include "RAK12039_PMSA003I.h"

/*!
 *  @brief  Initialize the class.
 *  @param  addr: The device address of PMSA003I IIC is 0x12. 
 */
RAK_PMSA003I::RAK_PMSA003I(byte addr) 
{
  _deviceAddress = addr;
}

/*!
 *  @brief  Initalizes the PMSA003I sensor.
 *  @param  wirePort      : IIC interface used.
 *  @param  deviceAddress : Device address should be 0x12. 
 *  @return If the device init successful return true else return false.
 */
bool RAK_PMSA003I::begin(TwoWire &wirePort, uint8_t deviceAddress)
{
  uint8_t sensor_id;

  _deviceAddress = deviceAddress;

  _i2cPort = &wirePort;

  LIB_LOG_PMS("RAK12039");
  delay(100);
  readRegister(PMSA003I_REG_ID , &sensor_id , 1);
   
  LIB_LOG_PMS("ID = 0x%X", sensor_id);
  delay(100);

  if(sensor_id == PMSA003I_CHIP_ID) 
  {
    LIB_LOG_PMS("PMSA003I ID read success.");
    return true;
  }
  else   
  {    
    LIB_LOG_PMS("PMSA003I ID read fail.");                             
    return false;
  }
}

/*!
 *  @brief  Read all data of PMSA003I.
 *  @param  pmsa_data : Read data pointer, refer to the data structure @PMSA_Data_t.
 *  @return Returns true if the read data is successful, otherwise returns false.
 */
bool RAK_PMSA003I::readDate(PMSA_Data_t *pmsa_data)
{
  uint8_t buf8[32];
  uint16_t buf16[16];
  uint16_t sum = 0;

  if (pmsa_data == NULL) 
  {
    LIB_LOG_PMS("Pointer is Null.");
    return false;
  }

  readRegister(PMSA003I_REG_ID , buf8 , 32);

  // Check that start byte is correct!
  if (buf8[0] != 0x42  || buf8[1] != 0x4d) 
  {
    LIB_LOG_PMS("Data read ID error.");
    return false;
  }
  
  for (uint8_t i = 0; i < 30; i++) 
  {
    sum += buf8[i];
    LIB_LOG_PMS("buf8[%d] = %x",i,buf8[i]);
  }
  
  for (uint8_t i = 0; i < 16; i++) 
  {
    buf16[i] = buf8[i*2 + 1];
    buf16[i] += (buf8[i*2] << 8);
    LIB_LOG_PMS("buf16[%d] = %x",i,buf16[i]);
  }

  memcpy((void *)pmsa_data, (void *)buf16, sizeof(PMSA_Data_t));
  LIB_LOG_PMS("pmsa_data.header = %x",pmsa_data->header);
  LIB_LOG_PMS("pmsa_data.date_len = %x",pmsa_data->date_len);

  LIB_LOG_PMS("sum = %x",sum);
  LIB_LOG_PMS("pmsa_data->check_sum = %x",pmsa_data->check_sum);
  
  if (sum != pmsa_data->check_sum) 
  {
    LIB_LOG_PMS("PMSA003I read date fail.");
    return false;
  }
  else
  {
    LIB_LOG_PMS("PMSA003I read date success.");
    return true;
  }
  return true;
}

/*!
 *  @brief  I2c bus read.
 *  @param  registerAddress : Register address.
 *  @param  readData        : Read data pointer.
 *  @param  size            : The length of the written data.
 */
void RAK_PMSA003I::readRegister(uint8_t registerAddress, uint8_t *readData, uint8_t size)
{  
  uint8_t error;
  _i2cPort->beginTransmission(_deviceAddress);
  _i2cPort->write(registerAddress);
  error = _i2cPort->endTransmission();

//  if(error == 0)
//  {
//    LIB_LOG_PMS("Success");
//  }
//  else if(error == 1)
//  {
//    LIB_LOG_PMS("Data too long");
//  }
//  else if(error == 2)
//  {
//    LIB_LOG_PMS("NACK on transmit of address");
//  }
//  else if(error == 3)
//  {
//    LIB_LOG_PMS("NACK on transmit of data");
//  }
//  else if(error == 4)
//  {
//    LIB_LOG_PMS("Other error");
//  }
  
  _i2cPort->requestFrom(_deviceAddress, size);
  
  size_t i = 0;
  while ( _i2cPort->available() )   // Slave may send less than requested.
  {
    readData[i] = _i2cPort->read(); // Receive a byte as a proper uint8_t.
    i++;
  }
}
