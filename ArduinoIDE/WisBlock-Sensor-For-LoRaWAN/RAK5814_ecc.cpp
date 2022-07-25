/**
 * @file RAK5814_ecc.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialize the RAK5814 encryption module
 * @version 0.1
 * @date 2022-04-20
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include <ArduinoECCX08.h>

ECCX08Class eccx08(Wire, 0x59);

bool init_rak5814(void)
{
	Wire.setClock(100000);
	if (!eccx08.begin())
	{
		MYLOG("ECC", "RAK5814 initialization failed");
		Wire.setClock(400000);
		return false;
	}

	MYLOG("ECC", "Serial number %s", eccx08.serialNumber().c_str());

	MYLOG("ECC", "Chip is %slocked", eccx08.locked() ? "" : "not");

	Wire.setClock(400000);
	return true;
}

uint16_t random_num_rak5814(uint16_t min, uint16_t max)
{
	Wire.setClock(100000);
	uint16_t result = eccx08.random(min, max);
	Wire.setClock(400000);
	return result;
}

bool sha256_rak5814(byte *data, uint32_t length, byte *result)
{
	// Wire.setClock(100000);
	// uint32_t index = 0;
	// int result = 0;
	// if (eccx08.beginSHA256() != 0)
	// {
	// 	Wire.setClock(400000);
	// 	return false;
	// }
	// while ((length - 32) > 64)
	// {
	// 	if (eccx08.updateSHA256(&data[index]) != 0)
	// 	{
	// 		Wire.setClock(400000);
	// 		return false;
	// 	}
	// 	index += 32;
	// 	length -= 32;
	// }
	// if (length != 0)
	// {
	// 	byte left_over[64] = {0};
	// 	memcpy(left_over, &data[index], length);
	// 	if (eccx08.updateSHA256(left_over) != 0)
	// 	{
	// 		Wire.setClock(400000);
	// 		return false;
	// 	}
	// }
	// if (eccx08.endSHA256(result) != 0)
	// {
	// 	Wire.setClock(400000);
	// 	return false;
	// }
	// Wire.setClock(400000);
	// return true;

	// eccx08.ecdsaVerify()
	return true;
}