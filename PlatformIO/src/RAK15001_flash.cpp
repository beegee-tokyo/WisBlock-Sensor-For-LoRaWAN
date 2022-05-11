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
#include "RAK_FLASH_SPI.h" //Click here to get the library: http://librarymanager/All#RAK_Storage

// SPI Flash Interface instance
RAK_FlashInterface_SPI g_flashTransport(SS, SPI);

/** SPI Flash instance */
RAK_FLASH_SPI g_flash(&g_flashTransport);

/** Flash definition structure for GD25Q16C Flash */
SPIFlash_Device_t g_RAK15001{
	.total_size = (1UL << 21),
	.start_up_time_us = 5000,
	.manufacturer_id = 0xc8,
	.memory_type = 0x40,
	.capacity = 0x15,
	.max_clock_speed_mhz = 15,
	.quad_enable_bit_mask = 0x00,
	.has_sector_protection = false,
	.supports_fast_read = true,
	.supports_qspi = false,
	.supports_qspi_writes = false,
	.write_status_register_split = false,
	.single_status_byte = true,
};

/** Flag if RAK15001 was found */
bool g_has_rak15001 = false;

/**
 * @brief Initialize the RAK15001 module
 *
 * @return true if module found and init ok
 * @return false if module not found or init failed
 */
bool init_rak15001(void)
{
	MYLOG("FLASH", "SS = %d", SS);

	if (!g_flash.begin(&g_RAK15001)) // Start access to the flash
	{
		MYLOG("FLASH", "Flash access failed, check the settings");
		return false;
	}
	if (!g_flash.waitUntilReady(5000))
	{
		MYLOG("FLASH", "Busy timeout");
		return false;
	}

	MYLOG("FLASH", "Device ID: 0x%02X", g_flash.getJEDECID());
	MYLOG("FLASH", "Size: %ld", g_flash.size());
	MYLOG("FLASH", "Pages: %d", g_flash.numPages());
	MYLOG("FLASH", "Page Size: %d", g_flash.pageSize());

	g_has_rak15001 = true;
	return true;
}

/**
 * @brief Read data from a sector of the RAK15001
 *
 * @param sector Flash sector, valid 0 to 511
 * @param buffer Buffer to read the data to
 * @param size Number of bytes to read
 * @return true If no error
 * @return false If read failed
 */
bool read_rak15001(uint16_t sector, uint8_t *buffer, uint16_t size)
{
	if (sector > 511)
	{
		MYLOG("FLASH", "Invalid sector");
		return false;
	}

	// Read the bytes
	if (!g_flash.readBuffer(sector * 4096, buffer, size))
	{
		MYLOG("FLASH", "Read failed");
		return false;
	}
	return true;
}

/**
 * @brief Write data to a sector of the RAK15001
 *
 * @param sector Flash sector, valid 0 to 511
 * @param buffer Buffer with the data to be written
 * @param size Number of bytes to write
 * @return true If write succeeded
 * @return false If write failed or readback data is not the same
 */
bool write_rak15001(uint16_t sector, uint8_t *buffer, uint16_t size)
{
	if (sector > 511)
	{
		MYLOG("FLASH", "Invalid sector");
		return false;
	}

	// Format the sector
	if (!g_flash.eraseSector(sector))
	{
		MYLOG("FLASH", "Erase failed");
	}

	uint8_t check_buff[size];
	// Write the bytes
	if (!g_flash.writeBuffer(sector * 4096, buffer, size))
	{
		MYLOG("FLASH", "Write failed");
		return false;
	}

	if (!g_flash.waitUntilReady(5000))
	{
		MYLOG("FLASH", "Busy timeout");
		return false;
	}

	// Read back the data
	if (!g_flash.readBuffer(sector * 4096, check_buff, size))
	{
		MYLOG("FLASH", "Read back failed");
		return false;
	}

	// Check if read data is same as requested data
	if (memcmp(check_buff, buffer, size) != 0)
	{
		MYLOG("FLASH", "Bytes read back are not the same as written");
		Serial.println("Adr  Write buffer     Read back");
		for (int idx = 0; idx < size; idx++)
		{
			Serial.printf("%03d  %02X   %02X\r\n", idx, buffer[idx], check_buff[idx]);
		}
		Serial.println(" ");
		return false;
	}
	return true;
}
