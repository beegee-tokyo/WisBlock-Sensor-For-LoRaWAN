/**
 * @file touch-RAK14002.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief 3 Touch Button module RAK14002 initialization
 * @version 0.3
 * @date 2022-01-29
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"

#include <Wire.h>
#include <CAP1293.h>

// Forward declarations
void int_callback_rak14002(void);

/** Touch pad module instance using Wire */
CAP1293 touch_pad;

// all touch key status, false is released, true is pressed.
bool pad_status[3] = {false, false, false};

/**
 * @brief Initialize RAK12004
 *        3 button touch pad
 *
 * @return true If touchpad was found and is initialized
 * @return false If touchpad initialization failed
 */
bool init_rak14002(void)
{
	// Setup interrupt pin
	pinMode(TOUCH_INT_PIN, INPUT);

	bool init_result = false;
	if (found_sensors[TOUCH_ID].i2c_num == 1)
	{
		Wire.begin();
		init_result = touch_pad.begin(Wire);
	}
	else
	{
#if WIRE_INTERFACES_COUNT > 1
		Wire1.begin();
		init_result = touch_pad.begin(Wire1);
#else
		return false;
#endif
	}

	if (!init_result)
	{
		MYLOG("TOUCH", "Touch Pad initialization failed");
		return false;
	}

	// Set interupt pin as input
	pinMode(TOUCH_INT_PIN, INPUT);
	// Set the interrupt callback function
	attachInterrupt(TOUCH_INT_PIN, int_callback_rak14002, FALLING);

	// Enable the touch interrupt
	touch_pad.setInterruptEnabled();

	MYLOG("TOUCH", "Touch Pad interrupts enabled");
	return true;
}

/**
 * @brief Get the last status of the touch pads
 *     Data is added to Cayenne LPP payload as channels
 *     LPP_CHANNEL_TOUCH_1, LPP_CHANNEL_TOUCH_2
 *     LPP_CHANNEL_TOUCH_3
 * 
 */
void get_rak14002(void)
{
	MYLOG("TOUCH", "Reading pad status");
	g_solution_data.addPresence(LPP_CHANNEL_TOUCH_1, pad_status[0]);
	g_solution_data.addPresence(LPP_CHANNEL_TOUCH_2, pad_status[1]);
	g_solution_data.addPresence(LPP_CHANNEL_TOUCH_3, pad_status[2]);
	// Enable the touch interrupt
	touch_pad.setInterruptEnabled();
}

/**
 * @brief Read status of the touch pads after an touch interrupt
 * 
 */
void read_rak14002(void)
{
	touch_pad.setInterruptDisabled();

	uint8_t pad_changed = touch_pad.getTouchKeyStatus(pad_status);
	// Left
	if (pad_changed & 0x01)
	{
		if (pad_status[0] == true)
		{
			MYLOG("TOUCH", "Left Pressed");
		}
		else
		{
			MYLOG("TOUCH", "Left Released");
		}
	}

	// Middle
	if (pad_changed & 0x02)
	{
		if (pad_status[1] == true)
		{
			MYLOG("TOUCH", "Middle Pressed");
		}
		else
		{
			MYLOG("TOUCH", "Middle Released");
		}
	}

	// Right
	if (pad_changed & 0x04)
	{
		if (pad_status[2] == true)
		{
			MYLOG("TOUCH", "Right Pressed");
		}
		else
		{
			MYLOG("TOUCH", "Right Released");
		}
	}
}

/**
 * @brief Touch pad interrupt handler
 * @note gives semaphore to wake up main loop
 *
 */
void int_callback_rak14002(void)
{
	api_wake_loop(TOUCH_EVENT | MOTION_TRIGGER);
}
