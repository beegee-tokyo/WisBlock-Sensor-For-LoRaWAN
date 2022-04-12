/**
 * @file RAK15001_flash.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialization and handling of RAK15001 Flash module
 * @version 0.1
 * @date 2022-04-12
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include "SparkFun_SPI_SerialFlash.h" //Click here to get the library: http://librarymanager/All#SparkFun_SPI_SerialFlash

/** Flash hal instance */
SFE_SPI_FLASH g_flash; // Sparkfun SPI Flash

/** Flag if RAK15001 was found */
bool g_external_flash = false;

/**
 * @brief Initialize access to the Flash memory
 * 
 * @return true if success
 * @return false if failed
 */
bool init_rak15001(void)
{
	if (!g_flash.begin(WB_SPI_CS)) // Start access to the flash
	{
		MYLOG("FLASH", "E: Flash access failed, check the settings");
		return false;
	}

	g_external_flash = true;
	
	sfe_flash_manufacturer_e mfgID = g_flash.getManufacturerID();
	if (mfgID != SFE_FLASH_MFG_UNKNOWN)
	{
		MYLOG("FLASH","Manufacturer ID: 0x%02X %s", g_flash.getManufacturerID(),g_flash.manufacturerIDString(mfgID));
	}
	else
	{
		MYLOG("FLASH","Unknown manufacturer ID: 0x%02X", g_flash.getManufacturerID());
	}

	MYLOG("FLASH","Device ID: 0x%02X",g_flash.getDeviceID());
	return true;
}

/**
 * @brief Read a block of bytes from the flash
 * 
 * @param address Address in Flash
 * @param buffer Buffer to write data into
 * @param size Number of bytes
 * @return true if success
 * @return false if failed
 */
bool read_rak15001(uint16_t address, uint8_t *buffer, uint16_t size)
{
	// Read the bytes
	uint16_t bytes_check = g_flash.readBlock(address, buffer, size);
	// Check if number of bytes is same as requester
	if (bytes_check != size)
	{
		MYLOG("FLASH", "E: Bytes read %d, requested %d", bytes_check, size);
		return false;
	}
	return true;
}

/**
 * @brief Write a block of bytes into the flash
 * 
 * @param address Address in Flash
 * @param buffer Buffer with the data to write
 * @param size Number of bytes
 * @return true if success
 * @return false if failed
 */
bool write_rak15001(uint16_t address, uint8_t *buffer, uint16_t size)
{
	uint8_t check_buff[size];
	// Write the bytes
	uint16_t bytes_check = g_flash.writeBlock(address, buffer, size);
	g_flash.blockingBusyWait();
	// Check if number of bytes is same as requester
	if (bytes_check != size)
	{
		MYLOG("FLASH", "E: Bytes written %d, requested %d", bytes_check, size);
		return false;
	}
	// Read back the data
	bytes_check = g_flash.readBlock(address, check_buff, size);
	// Check if number of bytes is same as requested
	if (bytes_check != size)
	{
		MYLOG("FLASH", "E: Bytes read back %d, requested %d", bytes_check, size);
		return false;
	}
	// Check if read data is same as requested data
	if (memcmp(check_buff, buffer, size) != 0)
	{
		MYLOG("FLASH", "E: Bytes read back are not the same as written");
		return false;
	}
	return true;
}
