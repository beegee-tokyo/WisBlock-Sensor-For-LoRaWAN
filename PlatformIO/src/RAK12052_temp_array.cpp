/**
 * @file RAK12052_temp_array.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialization and reading of the RAK12052
 * @version 0.1
 * @date 2023-09-15
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "app.h"
#include <RAK12052-MLX90640.h> // Click here to get the library: http://librarymanager/All#RAK12052-MLX90640

/** Sensor instance */
RAK_MLX90640 MLX90640;

/**
 * @brief Initialize the MLX90640 sensor
 *
 * @return true if sensor found
 * @return false if error occured (no check, never happens)
 */
bool init_rak12052(void)
{
	digitalWrite(WB_IO2, HIGH);
	Wire.begin();
	if (!MLX90640.begin())
	{
		MYLOG("IR_ARR", "MLX90640 not found!");
		return false;
	}

	MYLOG("IR_ARR", "MLS90640 Serial Number: %02X%02X%02X", MLX90640.serialNumber[0], MLX90640.serialNumber[1], MLX90640.serialNumber[2]);

	// Set sensor to chess mode
	MLX90640.setMode(MLX90640_CHESS);

	// Set sensor to 18bit resolution
	MLX90640.setResolution(MLX90640_ADC_18BIT);

	// Set sensor to 2Hz refresh rate
	MLX90640.setRefreshRate(MLX90640_2_HZ);

	return true;
}

/**
 * @brief Read and display measured values
 *
 */
void read_rak12052()
{
	if (MLX90640.getFrame(MLX90640.frame) != 0)
	{
		MYLOG("IR_ARR", "MLX90640 reading failed");
		return;
	}

	Serial.println();
	for (uint8_t h = 0; h < 24; h++)
	{
		for (uint8_t w = 0; w < 32; w++)
		{
			float t = MLX90640.frame[h * 32 + w];
			char c = '&';
			if (t < 20)
				c = ' ';
			else if (t < 23)
				c = '.'; //
			else if (t < 25)
				c = '-';
			else if (t < 27)
				c = '*';
			else if (t < 29)
				c = '+';
			else if (t < 31)
				c = 'x';
			else if (t < 33)
				c = '%';
			else if (t < 35)
				c = '#';
			else if (t < 37)
				c = 'X';
			Serial.print(c);
		}
		Serial.println();
	}
}
