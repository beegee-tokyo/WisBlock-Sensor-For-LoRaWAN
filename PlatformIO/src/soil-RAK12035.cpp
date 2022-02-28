/**
 * @file soil.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Soil sensor initialization and readings
 * @version 0.1
 * @date 2021-08-17
 *
 * @copyright Copyright (c) 2021
 *
 */
#include "app.h"
#include <RAK12035_SoilMoisture.h>

/** Sensor */
RAK12035 soil_sensor;

/** Structure for calibration values */
struct calib_values_s
{
	uint16_t dry_cal = 75;
	uint16_t wet_cal = 250;
};

/** The calibration values from the sensor */
calib_values_s calib_values;

/** Counter for failed readings */
uint8_t read_fail_counter = 0;

/**
 * @brief Initialize Soil Moisture Sensor
 *
 * @return true Sensor found
 * @return false Sensor not found
 */
bool init_rak12035(void)
{
	MYLOG("SOIL", "Init soil sensor");
	Serial.flush();
	// Check if sensors is available
	bool found_sensor = false;
	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, HIGH);
	pinMode(WB_IO5, INPUT);

	delay(500);

	if (found_sensors[SOIL_ID].i2c_num == 1)
	{
		Wire.begin();
		soil_sensor.setup(Wire);
	}
	else
	{
#if WIRE_INTERFACES_COUNT > 1
		Wire1.begin();
		soil_sensor.setup(Wire1);
#else
		return false;
#endif
	}

	// Initialize the sensor
	soil_sensor.begin();

	uint8_t data = 0;
	uint16_t value = 0;

	// Check the sensor version
	if (!soil_sensor.get_sensor_version(&data))
	{
		MYLOG("SOIL", "No sensor found");
	}
	else
	{
		MYLOG("SOIL", "Sensor FW version %d", data);
		found_sensor = true;
	}

	// Check the soil_sensor calibration values
	if (!soil_sensor.get_dry_cal(&value))
	{
		MYLOG("SOIL", "No Dry calibration");
	}
	else
	{
		MYLOG("SOIL", "Sensor Dry Cal %d", value);
		found_sensor = true;
	}

	// Check the sensor calibration values
	if (!soil_sensor.get_wet_cal(&value))
	{
		MYLOG("SOIL", "No Wet calibration");
	}
	else
	{
		MYLOG("SOIL", "Sensor Wet Cal %d", value);
		found_sensor = true;
	}

	soil_sensor.sensor_sleep();

	return found_sensor;
}

/**
 * @brief Read sensor values from RAK12035
 *     Data is added to Cayenne LPP payload as channel
 *     LPP_CHANNEL_SOIL_TEMP, LPP_CHANNEL_SOIL_HUMID
 *     LPP_CHANNEL_SOIL_HUMID_RAW, LPP_CHANNEL_SOIL_VALID
 *
 */
void read_rak12035(void)
{
	uint16_t sensTemp = 0;
	uint8_t sensHumid = 0;
	uint32_t avgTemp = 0;
	uint32_t avgHumid = 0;
	uint16_t sensCap = 0;
	uint32_t avgCap = 0;

	if (found_sensors[SOIL_ID].i2c_num == 1)
	{
		Wire.begin();
	}
	else
	{
#if WIRE_INTERFACES_COUNT > 1
		Wire1.begin();
#else
		return;
#endif
	}

	// Wake up the sensor
	if (!soil_sensor.sensor_on())
	{
		MYLOG("SOIL", "Can't wake up sensor");
		g_solution_data.addPresence(LPP_CHANNEL_SOIL_VALID, 0);

		read_fail_counter++;

		if (read_fail_counter == 5)
		{
			read_fail_counter = 0;
			delay(1000);
			api_reset();
		}
		return;
	}

	// Get the sensor values
	bool got_value = false;
	for (int retry = 0; retry < 3; retry++)
	{
		if (soil_sensor.get_sensor_moisture(&sensHumid) && soil_sensor.get_sensor_temperature(&sensTemp))
		{
			got_value = true;
			retry = 4;
			avgTemp = sensTemp;
			avgHumid = sensHumid;
			soil_sensor.get_sensor_capacitance(&sensCap);

			delay(250);
			for (int avg = 0; avg < 50; avg++)
			{
				delay(250);
				if (soil_sensor.get_sensor_temperature(&sensTemp))
				{
					avgTemp += sensTemp;
					avgTemp /= 2;
				}

				if (soil_sensor.get_sensor_moisture(&sensHumid))
				{
					avgHumid += sensHumid;
					avgHumid /= 2;
				}

				if (soil_sensor.get_sensor_capacitance(&sensCap))
				{
					avgCap += sensCap;
					avgCap /= 2;
				}
			}
		}
	}

	MYLOG("SOIL", "Sensor reading was %s", got_value ? "success" : "unsuccessful");
	MYLOG("SOIL", "T %.2f H %ld C %ld", (double)(avgTemp / 10.0), avgHumid, avgCap);

	g_solution_data.addTemperature(LPP_CHANNEL_SOIL_TEMP, avgTemp);
	g_solution_data.addRelativeHumidity(LPP_CHANNEL_SOIL_HUMID, avgHumid);
	g_solution_data.addAnalogInput(LPP_CHANNEL_SOIL_HUMID_RAW, avgCap);
	g_solution_data.addPresence(LPP_CHANNEL_SOIL_VALID, (got_value ? 1 : 0));

	soil_sensor.sensor_sleep();
}

/**
 * @brief Start sensor calibration
 *
 * @param is_dry  true => dry calibration, false => wet calibration
 * @return uint16_t calibration value
 */
uint16_t start_calib_rak12035(bool is_dry)
{
	MYLOG("SOIL", "Starting calibration for %s", is_dry ? "dry" : "wet");

	uint16_t new_reading = 0;
	uint16_t new_value = 0;
	digitalWrite(LED_GREEN, LOW);
	digitalWrite(LED_BLUE, HIGH);

	// Stop app timer while we do calibration
	api_timer_stop();

	if (found_sensors[SOIL_ID].i2c_num == 1)
	{
		Wire.begin();
	}
	else
	{
#if WIRE_INTERFACES_COUNT > 1
		Wire1.begin();
#else
		return false;
#endif
	}

	if (!soil_sensor.sensor_on())
	{
		MYLOG("SOIL", "Can't wake up sensor");

		if (g_lorawan_settings.send_repeat_time != 0)
		{
			// Calibration finished, restart the timer that will wakeup the loop frequently
			api_timer_restart(g_lorawan_settings.send_repeat_time);
		}

		digitalWrite(LED_BLUE, LOW);
		digitalWrite(LED_GREEN, LOW);

		if (is_dry)
		{
			return 0xFFFF;
		}
		else
		{
			return 0xFFFF;
		}
	}

	soil_sensor.get_sensor_capacitance(&new_value);

	for (int readings = 0; readings < 100; readings++)
	{
		soil_sensor.get_sensor_capacitance(&new_reading);
		if (new_reading != 0xFFFF)
		{
			MYLOG("SOIL", "Capacitance during %s calibration is %d\n", is_dry ? " DRY" : "WET", new_reading);
			new_value += new_reading;
			new_value = new_value / 2;
		}
		else
		{
			MYLOG("SOIL", "Capacitance reading failed\n");
		}
		delay(250);
		digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
		digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
	}

	// Send calibration value
	if (is_dry)
	{
		MYLOG("SOIL", "Dry calibration value %d", new_value);
		soil_sensor.set_dry_cal(new_value);
		calib_values.dry_cal = new_value;
	}
	else
	{
		MYLOG("SOIL", "Wet calibration value %d", new_value);
		soil_sensor.set_wet_cal(new_value);
		calib_values.wet_cal = new_value;
	}

	if (g_lorawan_settings.send_repeat_time != 0)
	{
		// Calibration finished, restart the timer that will wakeup the loop frequently
		api_timer_restart(g_lorawan_settings.send_repeat_time);
	}

	// Return the result

	digitalWrite(LED_BLUE, LOW);
	digitalWrite(LED_GREEN, LOW);
	soil_sensor.sensor_sleep();

	return new_value;
}

/**
 * @brief Get the calibration values from the RAK12035
 *
 * @param is_dry true => dry calibration, false => wet calibration
 * @return uint16_t calibration value
 */
uint16_t get_calib_rak12035(bool is_dry)
{
	uint16_t value = 0;
	soil_sensor.sensor_on();
	if (is_dry)
	{
		if (!soil_sensor.get_dry_cal(&value))
		{
			MYLOG("SOIL", "No Dry calibration");
		}
		else
		{
			MYLOG("SOIL", "Sensor Dry Cal %d", value);
		}
	}
	else
	{
		if (!soil_sensor.get_wet_cal(&value))
		{
			MYLOG("SOIL", "No Wet calibration");
		}
		else
		{
			MYLOG("SOIL", "Sensor Wet Cal %d", value);
		}
	}
	soil_sensor.sensor_sleep();
	return value;
}

/**
 * @brief Set the calibration values manually
 *
 * @param is_dry true => dry calibration, false => wet calibration
 * @param calib_val
 * @return uint16_t calibration value
 */
uint16_t set_calib_rak12035(bool is_dry, uint16_t calib_val)
{
	uint16_t value = 0;
	Wire.begin();
	soil_sensor.sensor_on();
	if (is_dry)
	{
		MYLOG("SOIL", "Dry calibration value %d", calib_val);
		// sensor.set_wet_cal(new_value);
		soil_sensor.set_dry_cal(calib_val);
		calib_values.dry_cal = calib_val;
	}
	else
	{
		MYLOG("SOIL", "Wet calibration value %d", calib_val);
		// sensor.set_dry_cal(new_value);
		soil_sensor.set_wet_cal(calib_val);
		calib_values.wet_cal = calib_val;
	}
	soil_sensor.sensor_sleep();
	Wire.end();
	return value;
}