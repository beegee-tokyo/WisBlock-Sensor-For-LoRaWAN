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

void start_rak1902(void)
{
	lps22hb.setDataRate(LPS22_RATE_75_HZ); // LPS22_RATE_ONE_SHOT
}

/**
 * @brief Read the barometric pressure
 *     Data is added to Cayenne LPP payload as channel
 *     LPP_CHANNEL_PRESS
 *
 */
void read_rak1902(void)
{
	// lps22hb.setDataRate(LPS22_RATE_75_HZ); // LPS22_RATE_ONE_SHOT
	MYLOG("PRESS", "Reading LPS22HB");
	sensors_event_t temp;
	sensors_event_t pressure;

time_t time_out = millis();
	bool good_reading = lps22hb.getEvent(&pressure, &temp);
	while (!good_reading)
	{
		good_reading = lps22hb.getEvent(&pressure, &temp);
		delay(100);
		if ((millis()-time_out) > 1000)
		{
			MYLOG("PRESS", "Timeout reading");
			return;
		}
	}
	MYLOG("PRESS", "Got valid reading");

	uint16_t press_int = (uint16_t)(pressure.pressure * 10);

	MYLOG("PRESS", "P: %.2f", (float)press_int / 10.0);

	g_solution_data.addBarometricPressure(LPP_CHANNEL_PRESS, press_int);

	lps22hb.setDataRate(LPS22_RATE_ONE_SHOT); // LPS22_RATE_ONE_SHOT
}

/**
 * @brief Calculate and return the altitude
 *        based on the barometric pressure
 *        Requires to have MSL set
 *
 * @return uint16_t
 */
uint16_t get_alt_rak1902(void)
{
	MYLOG("PRESS", "Compute altitude");
	sensors_event_t temp;
	sensors_event_t pressure;
	// pressure in HPa
	lps22hb.getEvent(&pressure, &temp);
	MYLOG("PRESS", "P: %.2f", pressure.pressure);
	
	float read_pressure = pressure.pressure;
	float A = read_pressure / at_MSL; // (1013.25) by default;
	float B = 1 / 5.25588;
	float C = pow(A, B);
	C = 1.0 - C;
	C = C / 0.0000225577;
	uint16_t new_val = C * 100;
	MYLOG("PRESS", "Altitude: %.2f m / %d cm", C, new_val);
	return new_val;
}
