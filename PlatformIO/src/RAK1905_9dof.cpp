/**
 * @file RAK1905_9dof.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief 9-axis accelerometer functions
 * @version 0.3
 * @date 2022-03-07
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"

#include <Wire.h>
#include <MPU9250_WE.h>

// Forward declarations
void int_callback_rak1905(void);

/** Sensor instance using Wire */
MPU9250_WE mpu_sensor = MPU9250_WE();

/** Interrupt pin, depends on slot */
uint8_t mpu_int_pin = ACC_INT_PIN;

/**
 * @brief Initialize MPU9250 9-axis
 * acceleration sensor
 *
 * @return true If sensor was found and is initialized
 * @return false If sensor initialization failed
 */
bool init_rak1905(void)
{
	// Setup interrupt pin
	pinMode(mpu_int_pin, INPUT);

	Wire.begin();

	if (!mpu_sensor.init())
	{
		MYLOG("9DOF", "Chip ID %02x %02x", mpu_sensor.whoAmI(), mpu_sensor.whoAmIMag());
		MYLOG("9DOF", "9DOF sensor initialization failed");
		return false;
	}

	MYLOG("9DOF", "Chip ID %02x %02x", mpu_sensor.whoAmI(), mpu_sensor.whoAmIMag());

	// Auto offsets
	mpu_sensor.autoOffsets();

	/*  Sample rate divider divides the output rate of the gyroscope and accelerometer.
	 *  Sample rate = Internal sample rate / (1 + divider)
	 *  It can only be applied if the corresponding DLPF is enabled and 0<DLPF<7!
	 *  Divider is a number 0...255
	 */
	mpu_sensor.setSampleRateDivider(5);

	/*  MPU9250_ACC_RANGE_2G      2 g   (default)
	 *  MPU9250_ACC_RANGE_4G      4 g
	 *  MPU9250_ACC_RANGE_8G      8 g
	 *  MPU9250_ACC_RANGE_16G    16 g
	 */
	mpu_sensor.setAccRange(MPU9250_ACC_RANGE_2G);

	/*  Enable/disable the digital low pass filter for the accelerometer
	 *  If disabled the bandwidth is 1.13 kHz, delay is 0.75 ms, output rate is 4 kHz
	 */
	mpu_sensor.enableAccDLPF(true);

	/*  Digital low pass filter (DLPF) for the accelerometer, if enabled
	 *  MPU9250_DPLF_0, MPU9250_DPLF_2, ...... MPU9250_DPLF_7
	 *   DLPF     Bandwidth [Hz]      Delay [ms]    Output rate [kHz]
	 *     0           460               1.94           1
	 *     1           184               5.80           1
	 *     2            92               7.80           1
	 *     3            41              11.80           1
	 *     4            20              19.80           1
	 *     5            10              35.70           1
	 *     6             5              66.96           1
	 *     7           460               1.94           1
	 */
	mpu_sensor.setAccDLPF(MPU9250_DLPF_6);

	/*  Set accelerometer output data rate in low power mode (cycle enabled)
	 *   MPU9250_LP_ACC_ODR_0_24          0.24 Hz
	 *   MPU9250_LP_ACC_ODR_0_49          0.49 Hz
	 *   MPU9250_LP_ACC_ODR_0_98          0.98 Hz
	 *   MPU9250_LP_ACC_ODR_1_95          1.95 Hz
	 *   MPU9250_LP_ACC_ODR_3_91          3.91 Hz
	 *   MPU9250_LP_ACC_ODR_7_81          7.81 Hz
	 *   MPU9250_LP_ACC_ODR_15_63        15.63 Hz
	 *   MPU9250_LP_ACC_ODR_31_25        31.25 Hz
	 *   MPU9250_LP_ACC_ODR_62_5         62.5 Hz
	 *   MPU9250_LP_ACC_ODR_125         125 Hz
	 *   MPU9250_LP_ACC_ODR_250         250 Hz
	 *   MPU9250_LP_ACC_ODR_500         500 Hz
	 */
	// mpu_sensor.setLowPowerAccDataRate(MPU9250_LP_ACC_ODR_125);

	/*  Set the interrupt pin:
	 *  MPU9250_ACT_LOW  = active-low
	 *  MPU9250_ACT_HIGH = active-high (default)
	 */
	mpu_sensor.setIntPinPolarity(MPU9250_ACT_HIGH);

	/*  If latch is enabled the Interrupt Pin Level is held until the Interrupt Status
	 *  is cleared. If latch is disabled the Interrupt Puls is ~50µs (default).
	 */
	mpu_sensor.enableIntLatch(true);

	/*  The Interrupt can be cleared by any read. Otherwise the Interrupt will only be
	 *  cleared if the Interrupt Status register is read (default).
	 */
	mpu_sensor.enableClearIntByAnyRead(false);

	/*  Enable/disable interrupts:
	 *  MPU9250_DATA_READY
	 *  MPU9250_FIFO_OVF
	 *  MPU9250_WOM_INT
	 *
	 *  You can enable all interrupts.
	 */
	// mpu_sensor.enableInterrupt(MPU9250_DATA_READY);
	mpu_sensor.enableInterrupt(MPU9250_WOM_INT);
	// myMPU9250.disableInterrupt(MPU9250_FIFO_OVF);

	/*  Set the Wake On Motion Threshold
	 *  Choose 1 (= 4 mg) ..... 255 (= 1020 mg);
	 */
	mpu_sensor.setWakeOnMotionThreshold(128); // 128 = ~0.5 g

	/*  Enable/disable wake on motion (WOM) and  WOM mode:
	 *  MPU9250_WOM_DISABLE
	 *  MPU9250_WOM_ENABLE
	 *  ***
	 *  MPU9250_WOM_COMP_DISABLE   // reference is the starting value
	 *  MPU9250_WOM_COMP_ENABLE    // reference is the last value
	 */
	mpu_sensor.enableWakeOnMotion(MPU9250_WOM_ENABLE, MPU9250_WOM_COMP_DISABLE);

	/* If cycle is set, and standby or sleep are not set, the module will cycle between
	 *  sleep and taking a sample at a rate determined by setLowPowerAccDataRate().
	 */
	// mpu_sensor.enableCycle(true);

	/* You can enable or disable the axes for gyroscope and/or accelerometer measurements.
	 * By default all axes are enabled. Parameters are:
	 * MPU9250_ENABLE_XYZ  //all axes are enabled (default)
	 * MPU9250_ENABLE_XY0  // X, Y enabled, Z disabled
	 * MPU9250_ENABLE_X0Z
	 * MPU9250_ENABLE_X00
	 * MPU9250_ENABLE_0YZ
	 * MPU9250_ENABLE_0Y0
	 * MPU9250_ENABLE_00Z
	 * MPU9250_ENABLE_000  // all axes disabled
	 */
	// myMPU9250.enableAccAxes(MPU9250_ENABLE_XYZ);
	//  Set the interrupt callback function
	attachInterrupt(mpu_int_pin, int_callback_rak1905, RISING);

	return true;
}

/**
 * @brief Assign/reassing interrupt pin
 *
 * @param new_irq_pin new GPIO to assign to interrupt
 */
void int_assign_rak1905(uint8_t new_irq_pin)
{
	detachInterrupt(mpu_int_pin);
	mpu_int_pin = new_irq_pin;
	attachInterrupt(mpu_int_pin, int_callback_rak1905, RISING);
}

/**
 * @brief ACC interrupt handler
 * @note gives semaphore to wake up main loop
 *
 */
void int_callback_rak1905(void)
{
	api_wake_loop(MOTION_TRIGGER);
}

/**
 * @brief Clear ACC interrupt register to enable next wakeup
 *
 */
void clear_int_rak1905(void)
{
	byte source = mpu_sensor.readAndClearInterrupts();
	if (mpu_sensor.checkInterrupt(source, MPU9250_WOM_INT))
	{
		MYLOG("9DOF", "Interrupt Type: Motion");
	}
}
