/**
 * @file light.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialize and read data from OPT3001 sensor
 * @version 0.2
 * @date 2022-01-30
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
// #include <opt3001.h>

// opt3001 light;
// const uint8_t OPT3001_ADDRESS = 0x44;

// /**
//  * @brief Initialize the Light sensor
//  *
//  * @return true if sensor found and configuration success
//  * @return false if error occured
//  */
// bool init_rak1903(void)
// {
// 	int result = 0;
// 	if (found_sensors[LIGHT_ID].i2c_num == 1)
// 	{
// 		Wire.begin();
// 		result = light.setup(Wire, OPT3001_ADDRESS);
// 	}
// 	else
// 	{
// 		Wire1.begin();
// 		result = light.setup(Wire1, OPT3001_ADDRESS);
// 	}

// 	if (result != 0)
// 	{
// 		MYLOG("LIGHT", "Wrong OPT3001 I2C address");
// 		return false;
// 	}

// 	result = light.detect();
// 	if (result != 0)
// 	{
// 		MYLOG("LIGHT", "OPT3001 not found");
// 		return false;
// 	}

// 	light.config_set(OPT3001_CONVERSION_TIME_800MS);
// 	light.conversion_continuous_enable();

// 	MYLOG("LIGHT", "OPT3001 initialized");
// 	return true;
// }

// /**
//  * @brief Read value from light sensor
//  *     Data is added to Cayenne LPP payload as channel
//  *     LPP_CHANNEL_LIGHT
//  *
//  */
// void read_rak1903()
// {
// 	MYLOG("LIGHT", "Reading OPT3001");
// 	float result_lux = 0.0;
// 	int result = light.lux_read(&result_lux);

// 	if (result != 0)
// 	{
// 		MYLOG("LIGHT", "No data ready, retry");
// 		delay(1000);
// 		result = light.lux_read(&result_lux);
// 		if (result != 0)
// 		{
// 			MYLOG("LIGHT", "Still no data ready, give up");
// 			return;
// 		}
// 	}

// 	uint16_t light_int = (uint16_t)(result_lux);

// 	MYLOG("LIGHT", "L: %.2f", (float)light_int / 1.0);

// 	g_solution_data.addLuminosity(LPP_CHANNEL_LIGHT, light_int);
// }

#include <ClosedCube_OPT3001.h>

/** Sensor instance */
ClosedCube_OPT3001 opt3001;
/** Sensor I2C address */
#define OPT3001_ADDRESS 0x44

/**
 * @brief Initialize the Light sensor
 *
 * @return true if sensor found and configuration success
 * @return false if error occured
 */
bool init_rak1903(void)
{
	Wire.begin();
	if (opt3001.begin(OPT3001_ADDRESS) != NO_ERROR)
	{
		MYLOG("LIGHT", "Could not initialize SHTC3");
		return false;
	}
	// if (found_sensors[LIGHT_ID].i2c_num == 1)
	// {
	// 	Wire.begin();
	// 	if (opt3001.begin(OPT3001_ADDRESS, Wire) != NO_ERROR)
	// 	{
	// 		MYLOG("LIGHT", "Could not initialize SHTC3");
	// 		return false;
	// 	}
	// }
	// else
	// {
	// ClosedCube OPT3001 library supports only use of Wire
	return false;
	// Wire1.begin();
	// if (opt3001.begin(OPT3001_ADDRESS, Wire1) != NO_ERROR)
	// {
	// 	MYLOG("LIGHT", "Could not initialize SHTC3");
	// 	return false;
	// }
	// }

	OPT3001_Config newConfig;

	newConfig.RangeNumber = B1100;
	newConfig.ConvertionTime = B0;
	newConfig.Latch = B1;
	newConfig.ModeOfConversionOperation = B11;

	OPT3001_ErrorCode errorConfig = opt3001.writeConfig(newConfig);
	if (errorConfig != NO_ERROR)
	{
		MYLOG("LIGHT", "Could not configure OPT3001");
		return false;
	}
	return true;
}

/**
 * @brief Read value from light sensor
 *     Data is added to Cayenne LPP payload as channel
 *     LPP_CHANNEL_LIGHT
 *
 */
void read_rak1903()
{
	MYLOG("LIGHT", "Reading OPT3001");
	OPT3001 result = opt3001.readResult();
	if (result.error == NO_ERROR)
	{
		uint16_t light_int = (uint16_t)(result.lux);

		MYLOG("LIGHT", "L: %.2f", (float)light_int / 1.0);

		g_solution_data.addLuminosity(LPP_CHANNEL_LIGHT, light_int);
	}
	else
	{
		MYLOG("LIGHT", "Error reading OPT3001");
		g_solution_data.addLuminosity(LPP_CHANNEL_LIGHT, 0);
	}
}
