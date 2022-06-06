/**
 * @file RAK14003_bar.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Initialize RAK14003 LED bar display and set function
 * @version 0.1
 * @date 2022-02-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include <Adafruit_MCP23X17.h>

/** LED bar chip I2C address */
uint8_t mcp_addr = 0X04;

/** LED bar chip instance */
Adafruit_MCP23X17 mcp;

/**
 * @brief Initialize the LED bar chip
 *
 * @return true success
 * @return false failed
 */
bool init_rak14003(void)
{
	// Reset device
	pinMode(WB_IO4, OUTPUT);
	digitalWrite(WB_IO4, 1);
	delay(10);
	digitalWrite(WB_IO4, 0);
	delay(10);
	digitalWrite(WB_IO4, 1);
	delay(10);

	Wire.begin();
	if (!mcp.begin_I2C(mcp_addr, &Wire))
	{
		MYLOG("LED_BAR", "LED_BAR not found");
		digitalWrite(WB_IO4, LOW);
		// api_deinit_gpio(WB_IO4);
		return false;
	}

	for (int i = 0; i < 16; i++)
	{
		mcp.digitalWrite(i, HIGH); // Turn off all LEDs.
		mcp.pinMode(i, OUTPUT);	   // Set pins as output.
	}
	return true;
}

/**
 * @brief Set which LED's are on or off
 *
 * @param leds 10 byte array, 1 = LED on, 0 = LED off
 */
void set_rak14003(uint8_t *leds)
{
	for (uint8_t idx = 0; idx < 10; idx++)
	{
		if (leds[idx] == 1)
		{
			mcp.digitalWrite(idx, LOW);
		}
		else
		{
			mcp.digitalWrite(idx, HIGH);
		}
	}
}