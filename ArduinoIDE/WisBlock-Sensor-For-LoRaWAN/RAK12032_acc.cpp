/**
 * @file RAK12032_acc.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief 3-axis accelerometer functions
 * @version 0.3
 * @date 2022-04-13
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include <Wire.h>
#ifndef ARDUINO_ARCH_RP2040
#include <SparkFunADXL313.h>

// Forward declarations
void int_callback_rak12032(void);

/** Sensor instance using Wire */
ADXL313 adxl313;

//******************************************************************//
// RAK12032 INT1_PIN
//******************************************************************//
// Slot A      WB_IO1
// Slot B      WB_IO2 ( not recommended, pin conflict with IO2)
// Slot C      WB_IO3
// Slot D      WB_IO5
// Slot E      WB_IO4
// Slot F      WB_IO6
//******************************************************************//
//******************************************************************//
// RAK12032 INT2_PIN
//******************************************************************//
// Slot A      WB_IO2 ( not recommended, pin conflict with IO2)
// Slot B      WB_IO1
// Slot C      WB_IO4
// Slot D      WB_IO6
// Slot E      WB_IO3
// Slot F      WB_IO5
//******************************************************************//

/** Interrupt pin, depends on slot */
uint8_t acc2_int_pin = WB_IO3;

/**
 * @brief Initialize ADXL313 3-axis
 * acceleration sensor
 *
 * @return true If sensor was found and is initialized
 * @return false If sensor initialization failed
 */
bool init_rak12032(void)
{
	// Setup interrupt pin
	pinMode(acc2_int_pin, INPUT);

	Wire.begin();

	if (!adxl313.begin())
	{
		MYLOG("ACC", "ACC sensor initialization failed");
		return false;
	}

	if (!adxl313.checkPartId())
	{
		MYLOG("ACC", "ACC sensor wrong ID");
		return false;
	}

	// To be safe, reset all settings
	adxl313.softReset();

	// Must be in standby before changing settings.
	adxl313.standby();

	adxl313.setRange(ADXL313_RANGE_2_G);

	// setup activity sensing options
	adxl313.setActivityX(true);		  // enable x-axis participation in detecting inactivity
	adxl313.setActivityY(true);		  // disable y-axis participation in detecting inactivity
	adxl313.setActivityZ(false);	  // disable z-axis participation in detecting inactivity
	adxl313.setActivityThreshold(32); // 0-255 (62.5mg/LSB)

	// setup inactivity sensing options
	adxl313.setInactivityX(true);		 // enable x-axis participation in detecting inactivity
	adxl313.setInactivityY(true);		 // disable y-axis participation in detecting inactivity
	adxl313.setInactivityZ(false);		 // disable z-axis participation in detecting inactivity
	adxl313.setInactivityThreshold(255); // 0-255 (62.5mg/LSB)
	adxl313.setTimeInactivity(1);		 // 0-255 (1sec/LSB)

	adxl313.setInterruptMapping(ADXL313_INT_ACTIVITY_BIT, ADXL313_INT1_PIN); // when activity is detected, it will effect the int1 pin on the sensor
	adxl313.setInterruptMapping(ADXL313_INT_INACTIVITY_BIT, ADXL313_INT2_PIN);
	// myAdxl.setInterruptMapping(ADXL313_INT_DATA_READY_BIT, ADXL313_INT1_PIN);

	// enable/disable interrupts
	// note, we set them all here, just in case there were previous settings,
	// that need to be changed for this example to work properly.
	adxl313.ActivityINT(true);	 // enable the activity interrupt
	adxl313.InactivityINT(true); // enable the inactivity interrupt
	adxl313.DataReadyINT(false); // disable dataReady

	adxl313.autosleepOn();

	adxl313.measureModeOn(); // wakes up the sensor from standby and puts it into measurement mode

	// Set the interrupt callback function
	MYLOG("ACC", "Assign interrupts");
	attachInterrupt(acc2_int_pin, int_callback_rak12032, RISING);

	return true;
}

/**
 * @brief Assign/reassing interrupt pin
 *
 * @param new_irq_pin new GPIO to assign to interrupt
 */
void int_assign_rak12032(uint8_t new_irq_pin)
{
	detachInterrupt(acc2_int_pin);
	acc2_int_pin = new_irq_pin;
	attachInterrupt(acc2_int_pin, int_callback_rak12032, RISING);
}

/**
 * @brief ACC interrupt handler
 * @note gives semaphore to wake up main loop
 *
 */
void int_callback_rak12032(void)
{
	// detachInterrupt(acc2_int_pin);
	digitalWrite(LED_BLUE, HIGH);
	api_wake_loop(MOTION_TRIGGER);
}

/**
 * @brief Clear ACC interrupt register to enable next wakeup
 *
 */
void clear_int_rak12032(void)
{
	MYLOG("ACC", "Check interrupt status.");

	adxl313.updateIntSourceStatuses(); // this will update all class intSource.xxxxx variables by reading int source bits.

	if (adxl313.intSource.activity == true)
	{
		MYLOG("ACC", "Activity detected.");
	}
	if (adxl313.intSource.inactivity == true)
	{
		MYLOG("ACC", "Inactivity detected.");
	}

	adxl313.updateIntSourceStatuses(); // this will update all class intSource.xxxxx variables by reading int source bits.
	if (adxl313.intSource.dataReady == true)
	{
		adxl313.readAccel(); // read all 3 axis, they are stored in class variables: myAdxl.x, myAdxl.y and myAdxl.z
		MYLOG("ACC2", "x: %d y: %d z: %d", adxl313.x, adxl313.y, adxl313.z);
	}
}
#else // ARDUINO_ARCH_RP2040
//**********************************************************/
/// \todo Sparkfun library does not work with RP2040 SPI
//**********************************************************/
bool init_rak12032(void)
{
	return false;
}

/**
 * @brief Assign/reassing interrupt pin
 *
 * @param new_irq_pin new GPIO to assign to interrupt
 */
void int_assign_rak12032(uint8_t new_irq_pin)
{
}

/**
 * @brief ACC interrupt handler
 * @note gives semaphore to wake up main loop
 *
 */
void int_callback_rak12032(void)
{
}

/**
 * @brief Clear ACC interrupt register to enable next wakeup
 *
 */
void clear_int_rak12032(void)
{
}

#endif // ARDUINO_ARCH_RP2040