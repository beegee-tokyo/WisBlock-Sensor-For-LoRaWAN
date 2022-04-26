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
	{0x18, 0, false}, //  0 ✔ RAK1904 accelerometer
	{0x44, 0, false}, //  1 ✔ RAK1903 light sensor
	{0x42, 0, false}, //  2 ✔ RAK12500 GNSS sensor
	{0x5c, 0, false}, //  3 ✔ RAK1902 barometric pressure sensor
	{0x70, 0, false}, //  4 ✔ RAK1901 temperature & humidity sensor
	{0x76, 0, false}, //  5 ✔ RAK1906 environment sensor
	{0x20, 0, false}, //  6 ✔ RAK12035 soil moisture sensor !! address conflict with RAK13003
	{0x10, 0, false}, //  7 ✔ RAK12010 light sensor
	{0x51, 0, false}, //  8 ✔ RAK12004 MQ2 CO2 gas sensor
	{0x50, 0, false}, //  9 ✔ RAK12008 MG812 CO2 gas sensor
	{0x55, 0, false}, // 10 ✔ RAK12009 MQ3 Alcohol gas sensor
	{0x52, 0, false}, // 11 ✔ RAK12014 Laser ToF sensor !! conflict with RAK12002
	{0x52, 0, false}, // 12 ✔ RAK12002 RTC module !! conflict with RAK12014
	{0x04, 0, false}, // 13 ✔ RAK14003 LED bargraph module
	{0x59, 0, false}, // 14 ✔ RAK12047 VOC sensor !! conflict with RAK13600, RAK13003, RAK5814
	{0x68, 0, false}, // 15 ✔ RAK12025 Gyroscope address !! conflict with RAK1905
	{0x73, 0, false}, // 16 ✔ RAK14008 Gesture sensor
	{0x3C, 0, false}, // 17 ✔ RAK1921 OLED display
	{0x53, 0, false}, // 18 ✔ RAK12019 LTR390 light sensor
	{0x28, 0, false}, // 19 ✔ RAK14002 Touch Button module
	{0x41, 0, false}, // 20 ✔ RAK16000 DC current sensor
	{0x68, 0, false}, // 21 ✔ RAK1905 MPU9250 9DOF sensor !! conflict with RAK12025
	{0x61, 0, false}, // 22 ✔ RAK12037 CO2 sensor !! conflict with RAK16001
	{0x3A, 0, false}, // 23 ✔ RAK12003 IR temperature sensor
	{0x68, 0, false}, // 24 ✔ RAK12040 AMG8833 temperature array sensor
	{0x69, 0, false}, // 25 ✔ RAK12034 BMX160 9DOF sensor
	{0x1D, 0, false}, // 26 ✔ RAK12032 ADXL313 accelerometer
	{0x59, 0, false}, // 26 ✔ RAK5814 ACC608 encryption module (limited I2C speed 100000) !! conflict with RAK12047, RAK13600, RAK13003
	{0x57, 0, false}, // 27 RAK12012 MAX30102 heart rate sensor
	{0x54, 0, false}, // 28 RAK12016 Flex sensor
	{0x47, 0, false}, // 29 RAK13004 PWM expander module
	{0x38, 0, false}, // 30 RAK14001 RGB LED module
	{0x5F, 0, false}, // 31 RAK14004 Keypad interface
	{0x61, 0, false}, // 32 RAK16001 ADC sensor !! conflict with RAK12037
	{0x59, 0, false}, // 33 RAK13600 NFC !! conflict with RAK12047, RAK13600, RAK5814
	{0x59, 0, false}, // 34 RAK16002 Coulomb sensor !! conflict with RAK13600, RAK12047, RAK5814
	{0x20, 0, false}, // 35 RAK13003 IO expander module !! conflict with RAK12035
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

	digitalWrite(WB_IO2, HIGH);

	Wire.begin();
	Wire.setClock(100000);
	for (byte address = 1; address < 127; address++)
	{
		Wire.beginTransmission(address);
		error = Wire.endTransmission();
		if (error == 0)
		{
			MYLOG("SCAN", "Found sensor at I2C1 0x%02X\n", address);
			for (uint8_t i = 0; i < sizeof(found_sensors) / sizeof(sensors_t); i++)
			{
				if (address == found_sensors[i].i2c_addr)
				{
					found_sensors[i].i2c_num = 1;
					found_sensors[i].found_sensor = true;

					if (address == 0x52)
					{
						found_sensors[i + 1].i2c_num = 1;
						found_sensors[i + 1].found_sensor = true;
					}
					break;
				}
			}
			num_dev++;
		}
	}

#if WIRE_INTERFACES_COUNT > 1
	Wire1.begin();
	// Wire1.setClock(400000);
	for (byte address = 1; address < 127; address++)
	{
		Wire1.beginTransmission(address);
		error = Wire1.endTransmission();
		if (error == 0)
		{
			// MYLOG("SCAN", "Found sensor at I2C2 %02X", address);
			for (uint8_t i = 0; i < sizeof(found_sensors) / sizeof(sensors_t); i++)
			{
				if (address == found_sensors[i].i2c_addr)
				{
					found_sensors[i].i2c_num = 2;
					found_sensors[i].found_sensor = true;

					if (address == 0x52)
					{
						found_sensors[i + 1].i2c_num = 1;
						found_sensors[i + 1].found_sensor = true;
					}
					break;
				}
			}
			num_dev++;
		}
	}
#endif

	Wire.setClock(400000);

	MYLOG("SCAN", "Found %d sensors", num_dev);
	for (uint8_t i = 0; i < sizeof(found_sensors) / sizeof(sensors_t); i++)
	{
		if (found_sensors[i].found_sensor)
		{
			MYLOG("SCAN", "ID %d addr %02X", i, found_sensors[i].i2c_addr);
		}
	}

	// Initialize the modules found
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

	if (found_sensors[MG812_ID].found_sensor)
	{
		if (init_rak12008())
		{
			snprintf(g_ble_dev_name, 9, "RAK_GAS");
		}
		else
		{
			found_sensors[MG812_ID].found_sensor = false;
		}
	}

	if (found_sensors[MQ3_ID].found_sensor)
	{
		if (init_rak12009())
		{
			snprintf(g_ble_dev_name, 9, "RAK_GAS");
		}
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

	// Two sensor modules with I2C addr 0x52, TOF sensor and RTC clock
	if (found_sensors[TOF_ID].found_sensor)
	{
		// Try TOF sensor first
		if (!init_rak12014())
		{
			// No ToF found, try RTC clock
			found_sensors[TOF_ID].found_sensor = false;
			if (init_rak12002())
			{
				found_sensors[RTC_ID].found_sensor = true;
			}
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

	if (found_sensors[GESTURE_ID].found_sensor)
	{
		if (!init_rak14008())
		{
			found_sensors[GESTURE_ID].found_sensor = false;
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

	// Multiple sensors have the same I2C address, try to get the correct oneif
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
		// api_deinit_gpio(PIN_WIRE_SDA);
		// api_deinit_gpio(PIN_WIRE_SCL);
#if WIRE_INTERFACES_COUNT > 1
		Wire1.end();
// api_deinit_gpio(PIN_WIRE1_SDA);
// api_deinit_gpio(PIN_WIRE1_SCL);
#endif
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
		// Prepare GNSS task
		// Create the GNSS event semaphore
		g_gnss_sem = xSemaphoreCreateBinary();
		// Initialize semaphore
		xSemaphoreGive(g_gnss_sem);
		// Take semaphore
		xSemaphoreTake(g_gnss_sem, 10);
		if (!xTaskCreate(gnss_task, "LORA", 4096, NULL, TASK_PRIO_LOW, &gnss_task_handle))
		{
			MYLOG("APP", "Failed to start GNSS task");
		}
		last_pos_send = millis();

		char mode_text[64] = {0};
		if (g_gps_prec_6 && !g_is_helium)
		{
			snprintf(mode_text, 64, "+EVT:PREC_6_DIG\n");
		}
		else if (!g_gps_prec_6 && !g_is_helium)
		{
			snprintf(mode_text, 64, "+EVT:PREC_4_DIG\n");
		}
		else
		{
			snprintf(mode_text, 64, "+EVT:HELIUM_MAPPER\n");
		}
		AT_PRINTF(mode_text);
		AT_PRINTF("============================\n");
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
	delayed_sending.begin(min_delay, send_delayed, NULL, false);
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

	if (!found_sensors[MG812_ID].found_sensor)
	{
		// MYLOG("APP", "MG812 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12008 OK\n");
		read_rak12008();
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
	if (found_sensors[MG812_ID].found_sensor)
	{
		// Get the MG812 sensor values
		read_rak12008();
	}
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
	if (found_sensors[SOIL_ID].found_sensor)
	{
		// Get the soil moisture sensor values
		read_rak12035();
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