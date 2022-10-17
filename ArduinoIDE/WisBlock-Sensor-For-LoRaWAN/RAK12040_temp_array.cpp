/**
 * @file RAK12040_temp_array.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialize and read data from AMG8833 sensor
 * @version 0.2
 * @date 2022-04-12
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include <SparkFun_GridEYE_Arduino_Library.h>

/** Sensor instance */
GridEYE amg8833;

/** HOT value (in degrees C) to adjust the contrast */
#define HOT 40
/** COLD value (in degrees C) to adjust the contrast */
#define COLD 20

/**
 * @brief This table can be of type int because we map the pixel
 * temperature to 0-3. Temperatures are reported by the
 * library as floats
 *
 */
int pixelTable[64];

/**
 * @brief Initialize the AMG8833 sensor
 *
 * @return true if sensor found
 * @return false if error occured (no check, never happens)
 */
bool init_rak12040(void)
{
	digitalWrite(WB_IO2, HIGH);
	Wire.begin();
	amg8833.begin(0x68, Wire);

	amg8833.setFramerate10FPS();

	return true;
}

/**
 * @brief Read and display measured values
 *
 */
void read_rak12040()
{
	// loop through all 64 pixels on the device and map each float value to a number
	// between 0 and 3 using the HOT and COLD values we set at the top of the sketch
	for (unsigned char i = 0; i < 64; i++)
	{
		pixelTable[i] = map(amg8833.getPixelTemperature(i), COLD, HOT, 0, 3);
	}

	// for (unsigned char x = 0; x < 64; x++)
	// {
	// 	pixelTable[x] = map(amg8833.getPixelTemperature[x], COLD, HOT, 0, 3);
	// 	Serial.print(amg8833.getPixelTemperature[x]);
	// 	Serial.print(" ");
	// }
	// Serial.println();

	// loop through the table of mapped values and print a character corresponding to each
	// pixel's temperature. Add a space between each. Start a new line every 8 in order to
	// create an 8x8 grid
	for (unsigned char i = 0; i < 64; i++)
	{
		if (pixelTable[i] == 0)
		{
			Serial.print(".");
		}
		else if (pixelTable[i] == 1)
		{
			Serial.print("o");
		}
		else if (pixelTable[i] == 2)
		{
			Serial.print("0");
		}
		else if (pixelTable[i] == 3)
		{
			Serial.print("O");
		}
		Serial.print(" ");
		if ((i + 1) % 8 == 0)
		{
			Serial.println();
		}
	}
}
