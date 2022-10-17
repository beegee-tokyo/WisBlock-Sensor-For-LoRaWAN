/**
 * @file RAK12025_gyro.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief 3-axis accelerometer functions
 * @version 0.3
 * @date 2022-01-29
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include <I3G4250D.h>

//******************************************************************//
// RAK12025 interrupt guide
//******************************************************************//
// INT1
// Slot A      WB_IO2 ( == power control )
// Slot B      WB_IO1 ( not recommended, INT pin conflict with IO2)
// Slot C      WB_IO4
// Slot D      WB_IO6
// Slot E      WB_IO3
// Slot F      WB_IO5
//******************************************************************//
// INT2
// Slot A      WB_IO1
// Slot B      WB_IO2 ( not recommended, pin conflict with IO2)
// Slot C      WB_IO3
// Slot D      WB_IO5
// Slot E      WB_IO4
// Slot F      WB_IO6
//******************************************************************//

// Forward declarations
void int_callback_rak12025(void);

/** Temporary as library does not support Wire 1 */
I3G4250D gyro_sensor;

/** Buffer for scaled gyro data */
I3G4250D_DataScaled gyro_data = {0};

/**
 * @brief Initialize I3G4250D gyroscope
 *
 * @return true If sensor was found and is initialized
 * @return false If sensor initialization failed
 */
bool init_rak12025(void)
{
	uint8_t error = -1;
	uint8_t id = 0;

	// Setup interrupt pin
	pinMode(GYRO_INT_PIN, INPUT);

	error = gyro_sensor.I3G4250D_Init(0xFF, 0x00, 0x00, 0x00, 0x00, I3G4250D_SCALE_500);
	if (error != 0)
	{
		MYLOG("GYRO", "init fail");
		return false;
	}

	gyro_sensor.readRegister(0x0F, &id, 1);
	MYLOG("GYRO", "Gyroscope Device ID = %02X", id);

	if (id != 0xD3)
	{
		MYLOG("GYRO", "wrong ID");
		return false;
	}

	/* The value of 1 LSB of the threshold corresponds to ~7.5 mdps
	 * Set Threshold ~100 dps on X, Y and Z axis
	 */

	gyro_sensor.I3G4250D_SetTresholds(0x1415, 0x1415, 0x1415);

	gyro_sensor.I3G4250D_InterruptCtrl(I3G4250D_INT_CTR_XLI_ON | I3G4250D_INT_CTR_YLI_ON | I3G4250D_INT_CTR_ZLI_ON);

	gyro_sensor.I3G4250D_Enable_INT1();

	pinMode(GYRO_INT_PIN, INPUT); // Connect with I3G4250D INT1.
	attachInterrupt(digitalPinToInterrupt(GYRO_INT_PIN), int_callback_rak12025, RISING);

	return true;
}

/**
 * @brief Gyro interrupt handler
 * @note gives semaphore to wake up main loop
 *
 */
void int_callback_rak12025(void)
{
	api_wake_loop(MOTION_TRIGGER);
}

/**
 * @brief Reset the interrupt on the I3G4250D chip
 *
 */
void clear_int_rak12025(void)
{
	gyro_sensor.I3G4250D_GetInterruptSrc();
}

/**
 * @brief Get Gyro measurements
 *     Data is added to Cayenne LPP payload as channels
 *     LPP_CHANNEL_GYRO
 *
 */
void read_rak12025(void)
{
	uint8_t eventSrc = gyro_sensor.I3G4250D_GetInterruptSrc();

#if MY_DEBUG > 0
	if (eventSrc & I3G4250D_INT_CTR_XLI_ON)
	{
		MYLOG("GYRO", "Interrupt generation on X LOW event.");
	}
	if (eventSrc & I3G4250D_INT_CTR_XHI_ON)
	{
		MYLOG("GYRO", "Interrupt generation on X HIGH event.");
	}
	if (eventSrc & I3G4250D_INT_CTR_YLI_ON)
	{
		MYLOG("GYRO", "Interrupt generation on Y LOW event.");
	}
	if (eventSrc & I3G4250D_INT_CTR_YHI_ON)
	{
		MYLOG("GYRO", "Interrupt generation on Y HIGH event.");
	}
	if (eventSrc & I3G4250D_INT_CTR_ZLI_ON)
	{
		MYLOG("GYRO", "Interrupt generation on Z LOW event.");
	}
	if (eventSrc & I3G4250D_INT_CTR_ZHI_ON)
	{
		MYLOG("GYRO", "Interrupt generation on Z HIGH event.");
	}
#endif

	gyro_data = gyro_sensor.I3G4250D_GetScaledData();

	g_solution_data.addGyrometer(LPP_CHANNEL_GYRO, gyro_data.x, gyro_data.y, gyro_data.z);
}
