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
#include <Melopero_AMG8833.h>

/** Sensor instance */
Melopero_AMG8833 amg8833;

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
	amg8833.initI2C(AMG8833_I2C_ADDRESS_A, Wire);

	MYLOG("IR_ARR", "Reset result %s", amg8833.getErrorDescription(amg8833.resetFlagsAndSettings()).c_str());
	MYLOG("IR_ARR", "Setting FPS result %s", amg8833.getErrorDescription(amg8833.setFPSMode(FPS_MODE::FPS_10)).c_str());

	return true;
}

/**
 * @brief Read and display measured values
 *
 */
void read_rak12040()
{
	MYLOG("IR_ARR", "Updating thermistor temperature result %s", amg8833.getErrorDescription(amg8833.updateThermistorTemperature()).c_str());
	MYLOG("IR_ARR", "Updating pixel matrix result %s", amg8833.getErrorDescription(amg8833.updatePixelMatrix()).c_str());

	// loop through all 64 pixels on the device and map each float value to a number
	// between 0 and 3 using the HOT and COLD values we set at the top of the sketch
	for (int x = 0; x < 8; x++)
	{
		for (int y = 0; y < 8; y++)
		{
			pixelTable[x + y] = map(amg8833.pixelMatrix[y][x], COLD, HOT, 0, 3);
			Serial.print(amg8833.pixelMatrix[y][x]);
			Serial.print(" ");
		}
		Serial.println();
	}
	Serial.println();

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
