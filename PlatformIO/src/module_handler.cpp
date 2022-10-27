/**
 * @file module-handler.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Find and handle WisBlock sensor modules
 * @version 0.1
 * @date 2022-02-02
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include "module_handler.h"

/**
 * @brief List of all supported WisBlock modules
 *
 */
sensors_t found_sensors[] = {
	// I2C address , I2C bus, found?
	{0x18, false}, //  0 ✔ RAK1904 accelerometer
	{0x44, false}, //  1 ✔ RAK1903 light sensor
	{0x42, false}, //  2 ✔ RAK12500 GNSS sensor
	{0x5c, false}, //  3 ✔ RAK1902 barometric pressure sensor
	{0x70, false}, //  4 ✔ RAK1901 temperature & humidity sensor
	{0x76, false}, //  5 ✔ RAK1906 environment sensor
	{0x20, false}, //  6 ✔ RAK12035 soil moisture sensor !! address conflict with RAK13003
	{0x10, false}, //  7 ✔ RAK12010 light sensor
	{0x51, false}, //  8 ✔ RAK12004 MQ2 CO2 gas sensor !! conflict with RAK15000
	{0x50, false}, //  9 ✔ RAK15000 EEPROM !! conflict with RAK12008
	{0x2C, false}, // 10 ✔ RAK12008 SCT31 CO2 gas sensor
	{0x55, false}, // 11 ✔ RAK12009 MQ3 Alcohol gas sensor
	{0x29, false}, // 12 ✔ RAK12014 Laser ToF sensor
	{0x52, false}, // 13 ✔ RAK12002 RTC module !! conflict with RAK15000
	{0x04, false}, // 14 ✔ RAK14003 LED bargraph module
	{0x59, false}, // 15 ✔ RAK12047 VOC sensor !! conflict with RAK13600, RAK13003, RAK5814
	{0x68, false}, // 16 ✔ RAK12025 Gyroscope address !! conflict with RAK1905
	{0x73, false}, // 17 ✔ RAK14008 Gesture sensor
	{0x3C, false}, // 18 ✔ RAK1921 OLED display
	{0x53, false}, // 19 ✔ RAK12019 LTR390 light sensor !! conflict with RAK15000
	{0x28, false}, // 20 ✔ RAK14002 Touch Button module
	{0x41, false}, // 21 ✔ RAK16000 DC current sensor
	{0x68, false}, // 22 ✔ RAK1905 MPU9250 9DOF sensor !! conflict with RAK12025
	{0x61, false}, // 23 ✔ RAK12037 CO2 sensor !! conflict with RAK16001
	{0x3A, false}, // 24 ✔ RAK12003 IR temperature sensor
	{0x68, false}, // 25 ✔ RAK12040 AMG8833 temperature array sensor
	{0x69, false}, // 26 ✔ RAK12034 BMX160 9DOF sensor
	{0x1D, false}, // 27 ✔ RAK12032 ADXL313 accelerometer
	{0x12, false}, // 28 ✔ RAK12039 PMSA003I particle matter sensor
	{0x55, false}, // 29 ✔ RAK12027 D7S seismic sensor
	{0x57, false}, // 30 RAK12012 MAX30102 heart rate sensor
	{0x54, false}, // 31 RAK12016 Flex sensor
	{0x47, false}, // 32 RAK13004 PWM expander module
	{0x38, false}, // 33 RAK14001 RGB LED module
	{0x5F, false}, // 34 RAK14004 Keypad interface
	{0x61, false}, // 35 RAK16001 ADC sensor !! conflict with RAK12037
	{0x59, false}, // 36 RAK13600 NFC !! conflict with RAK12047, RAK13600, RAK5814
	{0x59, false}, // 37 RAK16002 Coulomb sensor !! conflict with RAK13600, RAK12047, RAK5814
	{0x20, false}, // 38 RAK13003 IO expander module !! conflict with RAK12035
	{0x59, false}, // 39 ✔ RAK5814 ACC608 encryption module (limited I2C speed 100000) !! conflict with RAK12047, RAK13600, RAK13003
};

/**
 * @brief Scan both I2C bus for devices
 *
 */
void find_modules(void)
{
	// Scan the I2C interfaces for devices
	byte error;
	uint8_t num_dev = 0;

	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, HIGH);

	Wire.begin();
	// Some modules support only 100kHz
	Wire.setClock(100000);
	for (byte address = 1; address < 127; address++)
	{
		// if ((address == 0x59 || (address == 0x73)))
		// {
		// 	MYLOG("SCAN", "Reducing speed for RAK5814");
		// 	// RAK5814 supports only low I2C speed
		// 	Wire.setClock(100000);
		// }
		// else
		// {
		// 	Wire.setClock(100000); // Wire.setClock(400000);
		// }
		if (address == 0x29)
		{
			MYLOG("SCAN", "Enable xshut for RAK12014");
			// RAK12014 has extra GPIO for power control
			// On/Off control pin
			pinMode(xshut_pin, OUTPUT);
			// Sensor on
			digitalWrite(xshut_pin, HIGH);
			// Wait for sensor wake-up
			delay(150);
		}
		else
		{
			pinMode(xshut_pin, INPUT);
		}
		if (address == 0x12)
		{
			// RAK12039 has extra GPIO for power control
			// On/Off control pin
			pinMode(WB_IO6, OUTPUT);
			// Sensor on
			digitalWrite(WB_IO6, HIGH);
			delay(500);
			time_t wait_sensor = millis();
			MYLOG("SCAN", "RAK12039 scan start %ld ms", millis());
			while (1)
			{
				delay(500);
				Wire.beginTransmission(address);
				error = Wire.endTransmission();
				if (error == 0)
				{
					MYLOG("SCAN", "RAK12039 answered at %ld ms", millis());
					break;
				}
				if ((millis() - wait_sensor) > 5000)
				{
					MYLOG("SCAN", "RAK12039 timeout after %ld ms", 5000);
					pinMode(WB_IO6, INPUT);
					break;
				}
			}
		}
		Wire.beginTransmission(address);
		error = Wire.endTransmission();
		if (error == 0)
		{
			MYLOG("SCAN", "Found sensor at I2C1 0x%02X", address);
			for (uint8_t i = 0; i < sizeof(found_sensors) / sizeof(sensors_t); i++)
			{
				if (address == found_sensors[i].i2c_addr)
				{
					found_sensors[i].found_sensor = true;
					break;
				}
			}
			num_dev++;
		}
	}

	Wire.setClock(100000); /// \todo Wire.setClock(400000);

	MYLOG("SCAN", "Found %d sensors", num_dev);
	for (uint8_t i = 0; i < sizeof(found_sensors) / sizeof(sensors_t); i++)
	{
		if (found_sensors[i].found_sensor)
		{
			MYLOG("SCAN", "ID %d addr %02X", i, found_sensors[i].i2c_addr);
		}
	}

	// Initialize the modules found
	if (found_sensors[EEPROM_ID].found_sensor)
	{
		// Check EEPROM first, it occupies multiple I2C addresses
		if (!init_rak15000())
		{
			found_sensors[EEPROM_ID].found_sensor = false;
		}
		else
		{
			// 0x51, 0x52 and 0x53 are occupied by EEPROM
			found_sensors[MQ2_ID].found_sensor = false;
			found_sensors[RTC_ID].found_sensor = false;
			found_sensors[UVL_ID].found_sensor = false;
		}
	}

	// Check if RAK15001 is available, it is a SPI device, not found by the scan
#ifndef ARDUINO_ARCH_RP2040
	if (init_rak15001())
	{
		MYLOG("SCAN", "RAK15001 found");
	}
#endif // ARDUINO_ARCH_RP2040

	if (found_sensors[TEMP_ID].found_sensor)
	{
		if (!init_rak1901())
		{
			found_sensors[TEMP_ID].found_sensor = false;
		}
	}

	if (found_sensors[PRESS_ID].found_sensor)
	{
		if (!init_rak1902())
		{
			found_sensors[PRESS_ID].found_sensor = false;
		}
	}

	if (found_sensors[LIGHT_ID].found_sensor)
	{
		if (init_rak1903())
		{
			snprintf(g_ble_dev_name, 9, "RAK_WEA");
		}
		else
		{
			found_sensors[LIGHT_ID].found_sensor = false;
		}
	}

	if (found_sensors[ACC_ID].found_sensor)
	{
		if (!init_rak1904())
		{
			found_sensors[ACC_ID].found_sensor = false;
		}
	}

	// Multiple sensors have the same I2C address, try to get the correct one
	if (found_sensors[GYRO_ID].found_sensor)
	{
		// Try the gyro sensor
		if (!init_rak12025())
		{
			found_sensors[GYRO_ID].found_sensor = false;
			// Try the 9DOF sensor MPU9250
			if (!init_rak1905())
			{
				found_sensors[MPU_ID].found_sensor = false;
				// Try the 8x8 thermal array sensor
				if (!init_rak12040())
				{
					found_sensors[TEMP_ARR_ID].found_sensor = false;
				}
				else
				{
					found_sensors[TEMP_ARR_ID].found_sensor = true;
				}
			}
			else
			{
				found_sensors[MPU_ID].found_sensor = true;
			}
		}
	}

	if (found_sensors[ENV_ID].found_sensor)
	{
		if (init_rak1906())
		{
			snprintf(g_ble_dev_name, 9, "RAK_ENV");
		}
		else
		{
			found_sensors[ENV_ID].found_sensor = false;
		}
	}

	if (found_sensors[OLED_ID].found_sensor)
	{
		if (init_rak1921())
		{
			rak1921_write_header((char *)"WisBlock Node");
		}
		else
		{
			found_sensors[OLED_ID].found_sensor = false;
		}
	}

	if (found_sensors[RTC_ID].found_sensor)
	{
		if (!init_rak12002())
		{
			found_sensors[RTC_ID].found_sensor = false;
		}
	}

	if (found_sensors[FIR_ID].found_sensor)
	{
		if (!init_rak12003())
		{
			found_sensors[FIR_ID].found_sensor = false;
		}
	}

	if (found_sensors[MQ2_ID].found_sensor)
	{
		if (init_rak12004())
		{
			snprintf(g_ble_dev_name, 9, "RAK_GAS");
		}
		else
		{
			found_sensors[MQ2_ID].found_sensor = false;
		}
	}

	if (found_sensors[SCT31_ID].found_sensor)
	{
		if (init_rak12008())
		{
			snprintf(g_ble_dev_name, 9, "RAK_GAS");
		}
		else
		{
			found_sensors[SCT31_ID].found_sensor = false;
		}
	}

	// I2C address conflict with RAK12027 Seismic Sensor and RAK12009 Gas Sensor
	if (found_sensors[MQ3_ID].found_sensor)
	{
		if (init_rak12027())
		{
			found_sensors[MQ3_ID].found_sensor = false;
			found_sensors[SEISM_ID].found_sensor = true;
		}
		// else if (init_rak12009())
		// {
		// 	snprintf(g_ble_dev_name, 9, "RAK_GAS");
		// }
		else
		{
			found_sensors[MQ3_ID].found_sensor = false;
		}
	}

	if (found_sensors[LIGHT2_ID].found_sensor)
	{
		if (init_rak12010())
		{
			snprintf(g_ble_dev_name, 9, "RAK_WEA");
		}
		else
		{
			found_sensors[LIGHT2_ID].found_sensor = false;
		}
	}

	if (found_sensors[UVL_ID].found_sensor)
	{
		if (!init_rak12019())
		{
			found_sensors[UVL_ID].found_sensor = false;
		}
	}

	if (found_sensors[TOF_ID].found_sensor)
	{
		// Try TOF sensor first
		if (!init_rak12014())
		{
			found_sensors[TOF_ID].found_sensor = false;
		}
	}

	if (found_sensors[ACC2_ID].found_sensor)
	{
		if (!init_rak12032())
		{
			found_sensors[ACC2_ID].found_sensor = false;
		}
	}

	if (found_sensors[DOF_ID].found_sensor)
	{
		if (!init_rak12034())
		{
			found_sensors[DOF_ID].found_sensor = false;
		}
	}

	if (found_sensors[SOIL_ID].found_sensor)
	{
		if (init_rak12035())
		{
			snprintf(g_ble_dev_name, 9, "RAK_SOIL");
		}
		else
		{
			found_sensors[SOIL_ID].found_sensor = false;
		}
	}

	if (found_sensors[CO2_ID].found_sensor)
	{
		if (init_rak12037())
		{
			snprintf(g_ble_dev_name, 9, "RAK_GAS");
		}
		else
		{
			found_sensors[CO2_ID].found_sensor = false;
		}
	}

	if (found_sensors[PM_ID].found_sensor)
	{
		if (init_rak12039())
		{
			snprintf(g_ble_dev_name, 9, "RAK_ENV");
		}
		else
		{
			found_sensors[PM_ID].found_sensor = false;
		}
	}

	if (found_sensors[TEMP_ARR_ID].found_sensor)
	{
		if (!init_rak12040())
		{
			found_sensors[TEMP_ARR_ID].found_sensor = false;
		}
	}

	if (found_sensors[TOUCH_ID].found_sensor)
	{
		if (!init_rak14002())
		{
			found_sensors[TOUCH_ID].found_sensor = false;
		}
	}

	if (found_sensors[BAR_ID].found_sensor)
	{
		if (!init_rak14003())
		{
			found_sensors[BAR_ID].found_sensor = false;
		}
	}

	// Problems to detect RAK14008 in I2C scan, just try if it is plugged in
	// if (found_sensors[GESTURE_ID].found_sensor)
	{
		if (!init_rak14008())
		{
			found_sensors[GESTURE_ID].found_sensor = false;
		}
		else
		{
			found_sensors[GESTURE_ID].found_sensor = true;
		}
	}

	if (found_sensors[GNSS_ID].found_sensor)
	{
		if (init_gnss())
		{
			found_sensors[GNSS_ID].found_sensor = true;
			snprintf(g_ble_dev_name, 9, "RAK_GNSS");
		}
		else
		{
			found_sensors[GNSS_ID].found_sensor = false;
			// MYLOG("APP", "GNSS failed");
		}
	}
	else
	{
		if (init_gnss())
		{
			found_sensors[GNSS_ID].found_sensor = true;
			snprintf(g_ble_dev_name, 9, "RAK_GNSS");
		}
		else
		{
			found_sensors[GNSS_ID].found_sensor = false;
			// MYLOG("APP", "GNSS failed");
		}
	}

	if (found_sensors[VOC_ID].found_sensor)
	{
		MYLOG("APP", "Initialize RAK12047");
		if (init_rak12047())
		{
			MYLOG("APP", "RAK12047 init success");
			snprintf(g_ble_dev_name, 9, "RAK_VOC");
		}
		else
		{
			found_sensors[VOC_ID].found_sensor = false;
		}
	}

	if (found_sensors[CURRENT_ID].found_sensor)
	{
		if (!init_rak16000())
		{
			found_sensors[CURRENT_ID].found_sensor = false;
		}
	}

	if ((num_dev == 0) && !found_sensors[GNSS_ID].found_sensor)
	{
		// api_deinit_gpio(WB_IO2);
		Wire.end();
#if HAS_EPD == 0
		MYLOG("APP", "Switching off 3V3_S, no modules found");
		digitalWrite(WB_IO2, LOW);
#endif
		// api_deinit_gpio(PIN_WIRE_SDA);
		// api_deinit_gpio(PIN_WIRE_SCL);
	}
}

/**
 * @brief AT command feedback about found modules
 *
 */
void announce_modules(void)
{
	if (!found_sensors[TEMP_ID].found_sensor)
	{
		// MYLOG("APP", "SHTC3 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK1901 OK\n");
		read_rak1901();
	}

	if (!found_sensors[PRESS_ID].found_sensor)
	{
		// MYLOG("APP", "LPS22HB error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK1902 OK\n");
		read_rak1902();
	}

	if (!found_sensors[LIGHT_ID].found_sensor)
	{
		// MYLOG("APP", "OPT3001 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK1903 OK\n");
		read_rak1903();
	}

	if (!found_sensors[ACC_ID].found_sensor)
	{
		// MYLOG("APP", "ACC error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK1904 OK\n");
	}

	if (!found_sensors[MPU_ID].found_sensor)
	{
		// MYLOG("APP", "MPU error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK1905 OK\n");
	}

	if (!found_sensors[ENV_ID].found_sensor)
	{
		// MYLOG("APP", "BME680 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK1906 OK\n");
	}

	if (!found_sensors[GNSS_ID].found_sensor)
	{
		// MYLOG("APP", "GNSS error");
		init_result = false;
	}
	else
	{
		if (g_gnss_option == RAK12500_GNSS)
		{
			AT_PRINTF("+EVT:RAK12500 OK\n");
		}
		else
		{
			AT_PRINTF("+EVT:RAK1910 OK\n");
		}
	}

	// Delayed sending timer is used by several modules
	if (g_lorawan_settings.send_repeat_time != 0)
	{
		// Set delay for sending to 1/2 of scheduled sending
		min_delay = g_lorawan_settings.send_repeat_time / 2;
	}
	else
	{
		// Send repeat time is 0, set delay to 30 seconds
		min_delay = 30000;
	}

// Set delayed sending to 1/2 of programmed send interval or 30 seconds
#ifdef NRF52_SERIES
	delayed_sending.begin(min_delay, send_delayed, NULL, false);
#endif
	if (!found_sensors[OLED_ID].found_sensor)
	{
		// MYLOG("APP", "OLED error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK1921 OK\n");
	}

	if (!found_sensors[RTC_ID].found_sensor)
	{
		// MYLOG("APP", "RTC error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12002 OK\n");
		read_rak12002();
	}

	if (!found_sensors[FIR_ID].found_sensor)
	{
		// MYLOG("APP", "MLX90632 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12003 OK\n");
		read_rak12003();
	}

	if (!found_sensors[MQ2_ID].found_sensor)
	{
		// MYLOG("APP", "MQ2 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12004 OK\n");
		read_rak12004();
	}

	if (!found_sensors[SCT31_ID].found_sensor)
	{
		// MYLOG("APP", "SCT31 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12008 OK\n");
		// read_rak12008();
	}

	if (!found_sensors[MQ3_ID].found_sensor)
	{
		// MYLOG("APP", "MQ3 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12009 OK\n");
		read_rak12009();
	}

	if (!found_sensors[LIGHT2_ID].found_sensor)
	{
		// MYLOG("APP", "VEML7700 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12010 OK\n");
		read_rak12010();
	}

	if (!found_sensors[TOF_ID].found_sensor)
	{
		// MYLOG("APP", "VL53L01 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12014 OK\n");
		read_rak12014();
	}

	if (!found_sensors[UVL_ID].found_sensor)
	{
		// MYLOG("APP", "LTR390 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12019 OK\n");
		read_rak12019();
	}

	if (!found_sensors[GYRO_ID].found_sensor)
	{
		// MYLOG("APP", "I3G4240D error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12025 OK\n");
		read_rak12025();
	}

	if (!found_sensors[SEISM_ID].found_sensor)
	{
		// MYLOG("APP", "D7S error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12027 OK\n");
		read_rak12027(false);
	}

	if (!found_sensors[ACC2_ID].found_sensor)
	{
		// MYLOG("APP", "ADXL313 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12032 OK\n");
	}

	if (!found_sensors[DOF_ID].found_sensor)
	{
		// MYLOG("APP", "BMX160 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12034 OK\n");
	}

	if (!found_sensors[SOIL_ID].found_sensor)
	{
		// MYLOG("APP", "Soil Sensor error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12035 OK\n");
	}

	if (!found_sensors[CO2_ID].found_sensor)
	{
		// MYLOG("APP", "SCD30 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12037 OK\n");
		read_rak12037();
	}

	if (!found_sensors[PM_ID].found_sensor)
	{
		// MYLOG("APP", "PMSA003I error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12039 OK\n");
		// read_rak12039();
	}

	if (!found_sensors[TEMP_ARR_ID].found_sensor)
	{
		// MYLOG("APP", "AMG8833 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12040 OK\n");
		read_rak12040();
	}

	if (!found_sensors[VOC_ID].found_sensor)
	{
		// MYLOG("APP", "SGP40 Sensor error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12047 OK\n");
	}

	if (!found_sensors[TOUCH_ID].found_sensor)
	{
		// MYLOG("APP", "Touch Pad error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK14002 OK\n");
	}

	if (!found_sensors[BAR_ID].found_sensor)
	{
		// MYLOG("APP", "LED_BAR error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK14003 OK\n");
		uint8_t led_stat[10] = {0};
		set_rak14003(led_stat);
	}

	if (!found_sensors[GESTURE_ID].found_sensor)
	{
		// MYLOG("APP", "PAJ7620 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK14008 OK\n");
	}

	if (!found_sensors[CURRENT_ID].found_sensor)
	{
		// MYLOG("APP", "INA219 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK16000 OK\n");
	}

	if (!found_sensors[EEPROM_ID].found_sensor)
	{
		// MYLOG("APP", "EEPROM error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK15000 OK\n");
	}
}

/**
 * @brief Read values from the found modules
 *
 */
void get_sensor_values(void)
{
	if (found_sensors[TEMP_ID].found_sensor)
	{
		// Read environment data
		read_rak1901();
	}
	// RAK1902 needs time to get correct value. Reading was already started and results will be gotten in app.cpp
	// if (found_sensors[PRESS_ID].found_sensor)
	// {
	// 	// Read environment data
	// 	read_rak1902();
	// }
	if (found_sensors[LIGHT_ID].found_sensor)
	{
		// Read environment data
		read_rak1903();
	}
	// RAK1906 needs time to get correct value. Reading was already started and results will be gotten in app.cpp
	// if (found_sensors[ENV_ID].found_sensor)
	// {
	// 	// Start reading environment data
	// 	start_rak1906();
	// }
	if (found_sensors[FIR_ID].found_sensor)
	{
		// Get the MLX90632 sensor values
		read_rak12003();
	}
	if (found_sensors[MQ2_ID].found_sensor)
	{
		// Get the MQ2 sensor values
		read_rak12004();
	}
	// RAK12008 needs readings from temperature/humidity/pressure sensors. Readings will be gotten in app.cpp
	// if (found_sensors[SCT31_ID].found_sensor)
	// {
	// 	// Get the SCT31 sensor values
	// 	read_rak12008();
	// }
	if (found_sensors[MQ3_ID].found_sensor)
	{
		// Get the MQ3 sensor values
		read_rak12009();
	}
	if (found_sensors[CO2_ID].found_sensor)
	{
		// Get the CO2 sensor values
		read_rak12037();
	}
	if (found_sensors[LIGHT2_ID].found_sensor)
	{
		// Read environment data
		read_rak12010();
	}
	if (found_sensors[TOF_ID].found_sensor)
	{
		// Get the VL53L01 sensor values
		read_rak12014();
	}
	if (found_sensors[UVL_ID].found_sensor)
	{
		// Get the LTR390 sensor values
		read_rak12019();
	}
	if (found_sensors[GYRO_ID].found_sensor)
	{
		// Get the I3G4240D sensor values
		read_rak12025();
	}
	// if (found_sensors[SEISM_ID].found_sensor)
	// {
	// 	// Get the D7S sensor values
	// 	read_rak12027(false);
	// }
	if (found_sensors[SOIL_ID].found_sensor)
	{
		// Get the soil moisture sensor values
		read_rak12035();
	}
	if (found_sensors[PM_ID].found_sensor)
	{
		// Get the particle matter sensor values
		read_rak12039();
	}
	if (found_sensors[TEMP_ARR_ID].found_sensor)
	{
		// Get the temp array sensor values
		read_rak12040();
	}
	if (found_sensors[VOC_ID].found_sensor)
	{
		// Get the voc sensor values
		read_rak12047();
	}
	if (found_sensors[TOUCH_ID].found_sensor)
	{
		// Get the touch pad status
		get_rak14002();
	}
	if (found_sensors[GESTURE_ID].found_sensor)
	{
		// Get the gesture sensor values
		read_rak14008();
	}
	if (found_sensors[CURRENT_ID].found_sensor)
	{
		// Get the gesture sensor values
		read_rak16000();
	}
}