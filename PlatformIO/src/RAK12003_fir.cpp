/**
 * @file RAK12003_fir.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief MLX90632 ToF sensor support
 * @version 0.1
 * @date 2022-04-04
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "app.h"
#include <SparkFun_MLX90632_Arduino_Library.h>

/** Instance of sensor class */
MLX90632 fir_sensor;

/**
 * @brief Initialize the MLX90632 sensor
 *
 * @return true if sensor was found
 * @return false if sensor was not found
 */
bool init_rak12003(void)
{
	MLX90632::status init_result;

	Wire.begin();
	fir_sensor.begin(0x3A, Wire, init_result);

	if (init_result != MLX90632::SENSOR_SUCCESS)
	{
		if (init_result == MLX90632::SENSOR_ID_ERROR)
		{
			MYLOG("FIR", "Sensor ID did not match the sensor address. Probably a wiring error.");
		}
		else if (init_result == MLX90632::SENSOR_I2C_ERROR)
		{
			MYLOG("FIR", "Sensor did not respond to I2C properly. Check wiring.");
		}
		else if (init_result == MLX90632::SENSOR_TIMEOUT_ERROR)
		{
			MYLOG("FIR", "Sensor failed to respond.");
		}
		else
		{
			MYLOG("FIR", "Other Error");
		}
		return false;
	}

	return true;
}

/**
 * @brief Read temperature data from MLX90632
 *     Data is added to Cayenne LPP payload as channels
 *     LPP_CHANNEL_TEMP_3 and LPP_CHANNEL_TEMP_4
 *
 */
void read_rak12003(void)
{
	float object_temp = 0.0;
	float sensor_temp = 0.0;

	object_temp = fir_sensor.getObjectTemp();
	sensor_temp = fir_sensor.getSensorTemp();

	MYLOG("FIR", "Sensor %.2f'C Object %.2f'C", sensor_temp, object_temp);

	g_solution_data.addTemperature(LPP_CHANNEL_TEMP_3, sensor_temp);
	g_solution_data.addTemperature(LPP_CHANNEL_TEMP_4, object_temp);
}