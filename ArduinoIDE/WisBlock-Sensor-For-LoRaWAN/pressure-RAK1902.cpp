/**
 * @file pressure.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialize and read values from the LPS22HB sensor
 * @version 0.2
 * @date 2022-01-30
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include <Adafruit_LPS2X.h>
#include <Adafruit_Sensor.h>

/** Sensor instance */
Adafruit_LPS22 lps22hb;

/**
 * @brief Initialize barometric pressure sensor
 *
 * @return true if sensor was found
 * @return false if initialization failed
 */
bool init_rak1902(void)
{
	if (found_sensors[PRESS_ID].i2c_num == 1)
	{
		if (!lps22hb.begin_I2C(0x5c, &Wire))
		{
			MYLOG("PRESS", "Could not initialize LPS2X on Wire");
			return false;
		}
	}
	else
	{
#if WIRE_INTERFACES_COUNT > 1
		if (!lps22hb.begin_I2C(0x5c, &Wire1))
		{
			MYLOG("PRESS", "Could not initialize LPS2X on Wire1");
			return false;
		}
#else
		return false;
#endif
	}

	lps22hb.setDataRate(LPS22_RATE_ONE_SHOT); // LPS22_RATE_ONE_SHOT
	return true;
}

/**
 * @brief Read the barometric pressure
 *     Data is added to Cayenne LPP payload as channel
 *     LPP_CHANNEL_PRESS
 *
 */
void read_rak1902(void)
{
	lps22hb.setDataRate(LPS22_RATE_75_HZ); // LPS22_RATE_ONE_SHOT
	MYLOG("PRESS", "Reading LPS22HB");
	sensors_event_t temp;
	sensors_event_t pressure;

	lps22hb.getEvent(&pressure, &temp);

	uint16_t press_int = (uint16_t)(pressure.pressure * 10);

	MYLOG("PRESS", "P: %.2f", (float)press_int / 10.0);

	g_solution_data.addBarometricPressure(LPP_CHANNEL_PRESS, press_int);

	lps22hb.setDataRate(LPS22_RATE_ONE_SHOT); // LPS22_RATE_ONE_SHOT
}