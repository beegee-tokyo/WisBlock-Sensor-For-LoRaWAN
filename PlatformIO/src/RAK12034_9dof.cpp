/**
 * @file RAK12034_9dof.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief 9-axis accelerometer functions
 * @version 0.1
 * @date 2022-04-13
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include <Rak_BMX160.h>

// 0x01 - accelerometer X axis high threshold interrupt
// 0x02 - accelerometer y axis high threshold interrupt
// 0x04 - accelerometer z axis high threshold interrupt
#define HIGH_G_INT 0x07
// Interrupt threshold
// ( 0x80*7.81 ) mg(2g range)
// ( 0x80*15.63 )mg (4g range)
// ( 0x80*31.25 )mg (8g range)
// ( 0x80* 62.5 )mg (16g range)
#define HIGH_G_THRESHOLD 0x80

// Forward declarations
void int_callback_rak12034(void);

/** Sensor instance using Wire */
RAK_BMX160 bmx160(&Wire);

//******************************************************************//
// RAK12034 INT1_PIN
//******************************************************************//
// Slot A      WB_IO1
// Slot B      WB_IO2 ( not recommended, pin conflict with IO2)
// Slot C      WB_IO3
// Slot D      WB_IO5
// Slot E      WB_IO4
// Slot F      WB_IO6
//******************************************************************//
//******************************************************************//
// RAK12034 INT2_PIN
//******************************************************************//
// Slot A      WB_IO2 ( not recommended, pin conflict with IO2)
// Slot B      WB_IO1
// Slot C      WB_IO4
// Slot D      WB_IO6
// Slot E      WB_IO3
// Slot F      WB_IO5
//******************************************************************//
/** Interrupt pin, depends on slot */
uint8_t bmx_int_pin = WB_IO4;

/**
 * @brief Initialize BMX160 9-axis
 * acceleration sensor
 *
 * @return true If sensor was found and is initialized
 * @return false If sensor initialization failed
 */
bool init_rak12034(void)
{
	// Setup interrupt pin
	pinMode(bmx_int_pin, INPUT_PULLDOWN);

	Wire.begin();
	if (bmx160.begin() != true)
	{
		MYLOG("BMX160", "9DOF sensor initialization failed");
		return false;
	}

	// Check if it is the expected chip
	uint8_t chip_id = 0;
	bmx160.readReg(BMX160_CHIP_ID_ADDR, &chip_id, 1);
	if (chip_id != BMX160_CHIP_ID)
	{
		MYLOG("BMX160", "Wrong chip ID %02X", chip_id);
		return false;
	}
	MYLOG("BMX160", "Chip ID %02X", chip_id);

	// Enable the gyroscope and accelerometer sensor
	bmx160.wakeUp();

	uint8_t PMU_Status = 0;
	bmx160.readReg(0x03, &PMU_Status, 1);
	MYLOG("BMX160", "PMU_Status=%x\r\n", PMU_Status);

	// Enable HIGH_G_Interrupt and set the accelerometer threshold
	bmx160.InterruptConfig(HIGH_G_INT, HIGH_G_THRESHOLD);

	// Set output data rate
	bmx160.ODR_Config(BMX160_ACCEL_ODR_200HZ, BMX160_GYRO_ODR_200HZ);

	/**
	   enum{eGyroRange_2000DPS,
			 eGyroRange_1000DPS,
			 eGyroRange_500DPS,
			 eGyroRange_250DPS,
			 eGyroRange_125DPS
			 }eGyroRange_t;
	 **/
	bmx160.setGyroRange(eGyroRange_500DPS);

	/**
		enum{eAccelRange_2G,
			 eAccelRange_4G,
			 eAccelRange_8G,
			 eAccelRange_16G
			 }eAccelRange_t;
	*/
	bmx160.setAccelRange(eAccelRange_2G);

	//  Set the interrupt callback function
	attachInterrupt(bmx_int_pin, int_callback_rak12034, RISING);

	return true;
}

/**
 * @brief Assign/reassing interrupt pin
 *
 * @param new_irq_pin new GPIO to assign to interrupt
 */
void int_assign_rak12034(uint8_t new_irq_pin)
{
	detachInterrupt(bmx_int_pin);
	bmx_int_pin = new_irq_pin;
	attachInterrupt(bmx_int_pin, int_callback_rak12034, RISING);
}

/**
 * @brief ACC interrupt handler
 * @note gives semaphore to wake up main loop
 *
 */
void int_callback_rak12034(void)
{
	api_wake_loop(MOTION_TRIGGER);
}

/**
 * @brief Clear ACC interrupt register to enable next wakeup
 *
 */
void clear_int_rak12034(void)
{
	sBmx160SensorData_t Omagn, Ogyro, Oaccel;

	float Temp = 0;
	bmx160.getTemperature(&Temp);
	MYLOG("BMX160", "Temperature: %.2f", Temp);

	/* Get a new sensor event */
	bmx160.getAllData(&Omagn, &Ogyro, &Oaccel);
	/* Display the magnetometer results (magn is magnetometer in uTesla) */
	MYLOG("BMX160", "M X: %f Y: %f Z: %f uT", Omagn.x, Omagn.y, Omagn.z);

	/* Display the gyroscope results (gyroscope data is in °/s) */
	MYLOG("BMX160", "G X: %f Y: %f Z: %f °/s", Ogyro.x, Ogyro.y, Ogyro.z);

	/* Display the accelerometer results (accelerometer data is in m/s^2) */
	MYLOG("BMX160", "A X: %f Y: %f Z: %f m/s^2", Oaccel.x, Oaccel.y, Oaccel.z);
}
