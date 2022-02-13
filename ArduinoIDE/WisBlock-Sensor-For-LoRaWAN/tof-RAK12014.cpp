/**
 * @file tof-RAK12014.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief VL53L01 ToF sensor support
 * @version 0.1
 * @date 2022-02-13
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "app.h"
#include <VL53L0X.h>

/** Instance of sensor class */
VL53L0X tof_sensor;

/** Analog value union */
union analog_s
{
	uint16_t analog16 = 0;
	uint8_t analog8[2];
};

/** Structure to parse uint16_t into a uin8_t array */
analog_s analog_val;

/**
 * @brief Initialize the BME680 sensor
 *
 * @return true if sensor was found
 * @return false if sensor was not found
 */
bool init_rak12014(void)
{
	// On/Off control pin
	pinMode(WB_IO4, OUTPUT);

	// Sensor on
	digitalWrite(WB_IO4, HIGH);

	// Wait for sensor wake-up
	delay(150);
	if (found_sensors[TOF_ID].i2c_num == 1)
	{
		tof_sensor.setBus(&Wire);
		Wire.begin();
	}
	else
	{
		tof_sensor.setBus(&Wire1);
		Wire1.begin();
	}

	tof_sensor.setTimeout(500);
	if (!tof_sensor.init())
	{
		MYLOG("ToF", "Failed to detect and initialize sensor!");
		return false;
	}

	// Set to long range
	// lower the return signal rate limit (default is 0.25 MCPS)
	tof_sensor.setSignalRateLimit(0.1);
	// increase laser pulse periods (defaults are 14 and 10 PCLKs)
	tof_sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
	tof_sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);

	// Sensor off
	digitalWrite(WB_IO4, LOW);

	return true;
}

/**
 * @brief Read ToF data from VL53L01
 *     Data is added to Cayenne LPP payload as channels
 *     LPP_CHANNEL_TOF
 *
 */
void read_rak12014(void)
{
	// Sensor on
	digitalWrite(WB_IO4, HIGH);
	delay(300);

	uint64_t collected = 0;
	bool got_valid_data = false;

	// uint64_t single_reading = (uint64_t)tof_sensor.readRangeContinuousMillimeters();
	uint64_t single_reading = (uint64_t)tof_sensor.readRangeSingleMillimeters();
	if (tof_sensor.timeoutOccurred() || (single_reading == 65535))
	{
		collected = analog_val.analog16;
		MYLOG("ToF", "Timeout");
	}
	else
	{
		// We are measuring against water surface, sometimes waves or reflections can give too high values
		// The tank is only 1200mm deep, so any value above should be discarded
		if (single_reading < 1100)
		{
			collected = single_reading;
			// We got at least one measurement
			got_valid_data = true;
		}
		else
		{
			collected = analog_val.analog16;
			// We got at least one measurement
			got_valid_data = true;
			MYLOG("ToF", "Measured > 1100mm");
		}
	}
	delay(100);

	for (int reading = 0; reading < 10; reading++)
	{
		single_reading = (uint64_t)tof_sensor.readRangeSingleMillimeters();
		if (tof_sensor.timeoutOccurred() || (single_reading == 65535))
		{
			MYLOG("ToF", "Timeout");
		}
		else
		{
			// We are measuring against water surface, sometimes waves or reflections can give too high values
			// The tank is only 1100mm deep, so any value above should be discarded
			if (single_reading < 1100)
			{
				collected += single_reading;
				collected = collected / 2;
				// We got at least one measurement
				got_valid_data = true;
			}
		}
		delay(100);
	}

	// If we failed to get a valid reading, we set it to the last measured value
	if (!got_valid_data)
	{
		collected = analog_val.analog16;
	}

	MYLOG("ToF", "Water level %d mm", 1100 - (uint16_t)collected);
	g_solution_data.addAnalogInput(LPP_CHANNEL_TOF, (float)(collected));
	g_solution_data.addPresence(LPP_CHANNEL_TOF_VALID, got_valid_data);

	// Sensor off
	digitalWrite(WB_IO4, LOW);
	return;
}