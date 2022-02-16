/**
 * @file acc.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief 3-axis accelerometer functions
 * @version 0.3
 * @date 2022-01-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "app.h"

#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// Forward declarations
void int_callback_rak1904(void);

/** Sensor instance using Wire */
Adafruit_LIS3DH acc_sensor_1(&Wire);
/** Sensor instance using Wire1 */
Adafruit_LIS3DH acc_sensor_2(&Wire1);
/** Pointer to used instance */
Adafruit_LIS3DH *acc_sensor;

/** For internal usage */
TwoWire *usedWire;

/**
 * @brief Read RAK1904 register
 *     Added here because Adafruit made that function private :-(
 *
 * @param chip_reg register address
 * @param dataToWrite data to write
 * @return true write success
 * @return false write failed
 */
bool rak1904_writeRegister(uint8_t chip_reg, uint8_t dataToWrite)
{
	//Write the byte
	usedWire->beginTransmission(LIS3DH_DEFAULT_ADDRESS);
	usedWire->write(chip_reg);
	usedWire->write(dataToWrite);
	if (usedWire->endTransmission() != 0)
	{
		return false;
	}

	return true;
}

/**
 * @brief Write RAK1904 register
 *     Added here because Adafruit made that function private :-(
 *
 * @param outputPointer
 * @param chip_reg
 * @return true read success
 * @return false read failed
 */
bool rak1904_readRegister(uint8_t *outputPointer, uint8_t chip_reg)
{
	//Return value
	uint8_t result;
	uint8_t numBytes = 1;

	usedWire->beginTransmission(LIS3DH_DEFAULT_ADDRESS);
	usedWire->write(chip_reg);
	if (usedWire->endTransmission() != 0)
	{
		return false;
	}
	usedWire->requestFrom(LIS3DH_DEFAULT_ADDRESS, numBytes);
	while (usedWire->available()) // slave may send less than requested
	{
		result = usedWire->read(); // receive a byte as a proper uint8_t
	}

	*outputPointer = result;
	return true;
}

/**
 * @brief Initialize LIS3DH 3-axis 
 * acceleration sensor
 * 
 * @return true If sensor was found and is initialized
 * @return false If sensor initialization failed
 */
bool init_rak1904(void)
{
	// Setup interrupt pin
	pinMode(ACC_INT_PIN, INPUT);

	if (found_sensors[ACC_ID].i2c_num == 1)
	{
		acc_sensor = &acc_sensor_1;
		Wire.begin();
		usedWire = &Wire;
	}
	else
	{
		acc_sensor = &acc_sensor_2;
		Wire1.begin();
		usedWire = &Wire1;
	}

	acc_sensor->setDataRate(LIS3DH_DATARATE_10_HZ);
	acc_sensor->setRange(LIS3DH_RANGE_4_G);

	if (!acc_sensor->begin())
	{
		MYLOG("ACC", "ACC sensor initialization failed");
		return false;
	}

	// Enable interrupts
	acc_sensor->enableDRDY(true, 1);
	acc_sensor->enableDRDY(false, 2);

	uint8_t data_to_write = 0;
	data_to_write |= 0x20;							  //Z high
	data_to_write |= 0x08;							  //Y high
	data_to_write |= 0x02;							  //X high
	rak1904_writeRegister(LIS3DH_REG_INT1CFG, data_to_write); // Enable interrupts on high tresholds for x, y and z

	// Set interrupt trigger range
	data_to_write = 0;
	if (g_is_helium)
	{
		data_to_write |= 0x03; // A lower threshold for mapping purposes
	}
	else
	{
		data_to_write |= 0x10; // 1/8 range
	}
	rak1904_writeRegister(LIS3DH_REG_INT1THS, data_to_write); // 1/8th range

	// Set interrupt signal length
	data_to_write = 0;
	data_to_write |= 0x01; // 1 * 1/50 s = 20ms
	rak1904_writeRegister(LIS3DH_REG_INT1DUR, data_to_write);

	rak1904_readRegister(&data_to_write, LIS3DH_REG_CTRL5);
	data_to_write &= 0xF3;							//Clear bits of interest
	data_to_write |= 0x08;							//Latch interrupt (Cleared by reading int1_src)
	rak1904_writeRegister(LIS3DH_REG_CTRL5, data_to_write); // Set interrupt to latching

	// Select interrupt pin 1
	data_to_write = 0;
	data_to_write |= 0x40; //AOI1 event (Generator 1 interrupt on pin 1)
	data_to_write |= 0x20; //AOI2 event ()
	rak1904_writeRegister(LIS3DH_REG_CTRL3, data_to_write);

	// No interrupt on pin 2
	rak1904_writeRegister(LIS3DH_REG_CTRL6, 0x00);

	// Enable high pass filter
	rak1904_writeRegister(LIS3DH_REG_CTRL2, 0x01);

	// Set low power mode
	data_to_write = 0;
	rak1904_readRegister(&data_to_write, LIS3DH_REG_CTRL1);
	data_to_write |= 0x08;
	rak1904_writeRegister(LIS3DH_REG_CTRL1, data_to_write);
	delay(100);
	data_to_write = 0;
	rak1904_readRegister(&data_to_write, 0x1E);
	data_to_write |= 0x90;
	rak1904_writeRegister(0x1E, data_to_write);
	delay(100);

	clear_int_rak1904();

	// Set the interrupt callback function
	attachInterrupt(ACC_INT_PIN, int_callback_rak1904, RISING);

	return true;
}

/**
 * @brief ACC interrupt handler
 * @note gives semaphore to wake up main loop
 * 
 */
void int_callback_rak1904(void)
{
	api_wake_loop(MOTION_TRIGGER);
}

/**
 * @brief Clear ACC interrupt register to enable next wakeup
 * 
 */
void clear_int_rak1904(void)
{
	acc_sensor->readAndClearInterrupt();
}
