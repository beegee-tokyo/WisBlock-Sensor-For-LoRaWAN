/**
 * @file RAK14008_gesture.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Gesture sensor functions
 * @version 0.3
 * @date 2022-02-15
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"

#include <RevEng_PAJ7620.h>

// Forward declarations
void int_callback_rak14008(void);

/** Sensor instance */
RevEng_PAJ7620 gesture_sensor = RevEng_PAJ7620();

/**
 * @brief Initialize Gesture sensor
 * acceleration sensor
 *
 * @return true If sensor was found and is initialized
 * @return false If sensor initialization failed
 */
bool init_rak14008(void)
{
	// Setup interrupt pin
	pinMode(GESTURE_INT_PIN, INPUT);

	uint8_t error = 0;
	if (found_sensors[GESTURE_ID].i2c_num == 1)
	{
		error = gesture_sensor.begin(&Wire);
	}
	else
	{
#if WIRE_INTERFACES_COUNT > 1
		error = gesture_sensor.begin(&Wire1);
#else
		return false;
#endif
	}

	if (error != 1)
	{
		MYLOG("GEST", "Gesture sensor initialization failed");
		return false;
	}

	// Set the interrupt callback function
	attachInterrupt(GESTURE_INT_PIN, int_callback_rak14008, FALLING);

	return true;
}

/**
 * @brief Gesture interrupt handler
 * @note gives semaphore to wake up main loop
 *
 */
void int_callback_rak14008(void)
{
	api_wake_loop(MOTION_TRIGGER);
}

/**
 * @brief Read detected guesture values
 *     Data is added to Cayenne LPP payload as channels
 *     LPP_CHANNEL_GESTURE
 *
 */
void read_rak14008(void)
{
	uint8_t gesture;						// Gesture is an enum type from RevEng_PAJ7620.h
	gesture = gesture_sensor.readGesture(); // Read back current gesture (if any) of type Gesture
	switch (gesture)
	{
	case GES_FORWARD:
		MYLOG("GEST", "FORWARD");
		break;
	case GES_BACKWARD:
		MYLOG("GEST", "BACKWARD");
		break;
	case GES_LEFT:
		MYLOG("GEST", "LEFT");
		break;
	case GES_RIGHT:
		MYLOG("GEST", "RIGHT");
		break;
	case GES_UP:
		MYLOG("GEST", "UP");
		break;
	case GES_DOWN:
		MYLOG("GEST", "DOWN");
		break;
	case GES_CLOCKWISE:
		MYLOG("GEST", "CLOCKWISE");
		break;
	case GES_ANTICLOCKWISE:
		MYLOG("GEST", "ANTICLOCKWISE");
		break;
	case GES_WAVE:
		MYLOG("GEST", "GES_WAVE");
		break;
	case GES_NONE:
		break;
	}
	g_solution_data.addDigitalInput(LPP_CHANNEL_GESTURE, gesture);
}
