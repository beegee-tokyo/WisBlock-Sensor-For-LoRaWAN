/**
 * @file RAK15000_eeprom.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialize and access RAK15000 EEPROM
 * @version 0.1
 * @date 2022-05-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "app.h"
#include <Adafruit_EEPROM_I2C.h>

/** EEPROM class instance */
Adafruit_EEPROM_I2C eeprom;

bool init_rak15000(void)
{
	uint8_t eepr_buff[110];

	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, HIGH); // power on for AT24C02 device
	MYLOG("EEPROM", "Check EEPROM");
	delay(100);

	Wire.begin();
	if (!eeprom.begin(EEPROM_ADDR, &Wire))
	{
		MYLOG("EEPROM", "EEPROM not found");
		return false;
	}
	if (!eeprom.read(0, eepr_buff, 100))
	{
		MYLOG("EEPROM", "EEPROM read error");
		return false;
	}
	MYLOG("EEPROM", "EEPROM read ok");

	return true;
}

/**
 * @brief Read a datablock from the EEPROM
 *
 * @param addr Start address
 * @param buffer Buffer to write data to
 * @param num Number of bytes to read
 * @return true if read was success
 * @return false if read failed
 */
bool read_rak15000(uint16_t addr, uint8_t *buffer, uint16_t num)
{
	if ((addr + num) > MAXADD)
	{
		MYLOG("EEPROM", "Read address or Size error");
		return false;
	}
	return eeprom.read(addr, buffer, num);
}

/**
 * @brief Write a datablock to the EEPROM
 *
 * @param addr Start address
 * @param buffer Buffer with the data to write
 * @param num Number of bytes to write
 * @return true if write was success
 * @return false if write failed
 */
bool write_rak15000(uint16_t addr, uint8_t *buffer, uint16_t num)
{
	if ((addr + num) > MAXADD)
	{
		MYLOG("EEPROM", "Write address or Size error");
		return false;
	}
	return eeprom.write(addr, buffer, num);
}

/***********************************************************************************************/
/***********************************************************************************************/

// #include <RAK_EEPROM_I2C.h>

// /** EEPROM class instance */
// RAK_EEPROM_I2C eeprom;

// /** Default I2C address */
// #define EEPROM_ADDR 0x50
// /** Max address of EEPROM */
// #define MAXADD 262143

// bool init_rak15000(void)
// {
// 	uint8_t eepr_buff[110];

// 	pinMode(WB_IO2, OUTPUT);
// 	digitalWrite(WB_IO2, HIGH); // power on for AT24C02 device
// 	delay(200);
// 	MYLOG("EEPROM", "Check EEPROM");
// 	delay(100);

// 	if (found_sensors[EEPROM_ID].i2c_num == 1)
// 	{
// 		Wire.begin();
// 		if (!eeprom.begin(EEPROM_ADDR, &Wire))
// 		{
// 			MYLOG("EEPROM", "EEPROM not found");
// 			return false;
// 		}
// 		if (!eeprom.read(0, eepr_buff, 8))
// 		{
// 			MYLOG("EEPROM", "EEPROM read error");
// 			return false;
// 		}
// 		MYLOG("EEPROM", "EEPROM read ok");
// 	}
// 	else
// 	{
// #if WIRE_INTERFACES_COUNT > 1
// 		Wire1.begin();
// 		if (!eeprom.begin(EEPROM_ADDR, &Wire1))
// 		{
// 			MYLOG("EEPROM", "EEPROM not found");
// 			return false;
// 		}
// 		if (!eeprom.read(0, eepr_buff, 8))
// 		{
// 			MYLOG("EEPROM", "EEPROM not found");
// 			return false;
// 		}
// 		MYLOG("EEPROM", "EEPROM read ok");
// #else
// 		return false;
// #endif
// 	}
// 	return true;
// }

// /**
//  * @brief Read a datablock from the EEPROM
//  *
//  * @param addr Start address
//  * @param buffer Buffer to write data to
//  * @param num Number of bytes to read
//  * @return true if read was success
//  * @return false if read failed
//  */
// bool read_rak15000(uint16_t addr, uint8_t *buffer, uint16_t num)
// {
// 	if ((addr + num) > MAXADD)
// 	{
// 		MYLOG("EEPROM", "Read address or Size error");
// 		return false;
// 	}
// 	return eeprom.read(addr, buffer, num);
// }

// /**
//  * @brief Write a datablock to the EEPROM
//  *
//  * @param addr Start address
//  * @param buffer Buffer with the data to write
//  * @param num Number of bytes to write
//  * @return true if write was success
//  * @return false if write failed
//  */
// bool write_rak15000(uint16_t addr, uint8_t *buffer, uint16_t num)
// {
// 	if ((addr + num) > MAXADD)
// 	{
// 		MYLOG("EEPROM", "Write address or Size error");
// 		return false;
// 	}
// 	return eeprom.write(addr, buffer, num);
// }