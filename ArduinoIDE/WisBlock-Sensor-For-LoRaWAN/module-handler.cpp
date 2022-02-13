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
	{0x20, 0, false}, //  6 ✔ RAK12035 soil moisture sensor
	{0x10, 0, false}, //  7 ✔ RAK12010 light sensor
	{0x51, 0, false}, //  8 ✔ RAK12004 MQ2 CO2 gas sensor
	{0x50, 0, false}, //  9 ✔ RAK12008 MG812 CO2 gas sensor
	{0x55, 0, false}, // 10 ✔ RAK12009 MQ3 Alcohol gas sensor
	{0x57, 0, false}, // 11 RAK12012 MAX30102 heart rate sensor
	{0x52, 0, false}, // 12 RAK12014 Laser ToF sensor
	{0x54, 0, false}, // 13 RAK12016 Flex sensor
	{0x53, 0, false}, // 14 RAK12019 LTR390 light sensor
	{0x47, 0, false}, // 15 RAK13004 PWM expander module
	{0x38, 0, false}, // 16 RAK14001 RGB LED module
	{0x28, 0, false}, // 17 RAK14002 Touch Button module
	{0x04, 0, false}, // 18 RAK14003 LED bargraph module
	{0x5F, 0, false}, // 19 RAK14004 Keypad interface
	{0x60, 0, false}, // 20 RAK16000 DC current sensor
	{0x61, 0, false}, // 21 RAK16001 ADC sensor
	{0x59, 0, false}, // 22 ✔ RAK12047 VOC sensor
					  //  {0x20, 0, false}, // RAK13003 IO expander module address conflict with RAK12035
};

bool has_rak1906 = false;
bool has_rak1901 = false;
bool has_rak1902 = false;
bool has_rak1903 = false;
bool has_gnss = false;
bool has_rak1904 = false;
bool has_soil = false;
bool has_rak12004 = false;
bool has_rak12008 = false;
bool has_rak12009 = false;
bool has_rak12010 = false;
bool has_rak12047 = false;
bool has_rak14003 = false;

extern bool init_result;
extern time_t min_delay;
extern time_t last_pos_send;
extern SoftwareTimer delayed_sending;
void send_delayed(TimerHandle_t unused);

/**
 * @brief Scan both I2C bus for devices
 *
 */
void find_modules(void)
{
	// Scan the I2C interfaces for devices
	byte error;
	uint8_t num_dev = 0;

	Wire.begin();
	Wire.setClock(400000);
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
					break;
				}
			}
			num_dev++;
		}
	}
	Wire1.begin();
	Wire1.setClock(400000);
	for (byte address = 1; address < 127; address++)
	{
		Wire1.beginTransmission(address);
		error = Wire1.endTransmission();
		if (error == 0)
		{
			MYLOG("SCAN", "Found sensor at I2C2 %02X", address);
			for (uint8_t i = 0; i < sizeof(found_sensors) / sizeof(sensors_t); i++)
			{
				if (address == found_sensors[i].i2c_addr)
				{
					found_sensors[i].i2c_num = 2;
					found_sensors[i].found_sensor = true;
					break;
				}
			}
			num_dev++;
		}
	}
	MYLOG("SCAN", "Found %d sensors", num_dev);

	// Initialize the modules found
	if (found_sensors[TEMP_ID].found_sensor == true)
	{
		if (init_rak1901())
		{
			has_rak1901 = true;
		}
	}

	if (found_sensors[PRESS_ID].found_sensor == true)
	{
		if (init_rak1902())
		{
			has_rak1902 = true;
		}
	}

	if (found_sensors[LIGHT_ID].found_sensor == true)
	{
		if (init_rak1903())
		{
			has_rak1903 = true;
			snprintf(g_ble_dev_name, 9, "RAK_WEA");
		}
	}

	if (found_sensors[ACC_ID].found_sensor == true)
	{
		if (init_rak1904())
		{
			has_rak1904 = true;
		}
	}

	if (found_sensors[ENV_ID].found_sensor == true)
	{
		if (init_rak1906())
		{
			has_rak1906 = true;
			snprintf(g_ble_dev_name, 9, "RAK_ENV");
		}
	}

	if (found_sensors[MQ2_ID].found_sensor == true)
	{
		if (init_rak12004())
		{
			has_rak12004 = true;
			snprintf(g_ble_dev_name, 9, "RAK_GAS");
		}
	}

	if (found_sensors[MG812_ID].found_sensor == true)
	{
		if (init_rak12008())
		{
			has_rak12008 = true;
			snprintf(g_ble_dev_name, 9, "RAK_GAS");
		}
	}

	if (found_sensors[MQ3_ID].found_sensor == true)
	{
		if (init_rak12009())
		{
			has_rak12009 = true;
			snprintf(g_ble_dev_name, 9, "RAK_GAS");
		}
	}

	if (found_sensors[LIGHT2_ID].found_sensor == true)
	{
		if (init_rak12010())
		{
			has_rak12010 = true;
			snprintf(g_ble_dev_name, 9, "RAK_WEA");
		}
	}

	if (found_sensors[SOIL_ID].found_sensor == true)
	{
		if (init_rak12035())
		{
			has_soil = true;
			snprintf(g_ble_dev_name, 9, "RAK_SOIL");
		}
	}

	if (found_sensors[BAR_ID].found_sensor == true)
	{
		if (init_rak14003())
		{
			has_rak14003 = true;
		}
	}

	if (init_gnss())
	{
		has_gnss = true;
		snprintf(g_ble_dev_name, 9, "RAK_GNSS");
	}
	else
	{
		MYLOG("APP", "GNSS failed");
	}

	if (found_sensors[VOC_ID].found_sensor == true)
	{
		MYLOG("APP", "Initialize RAK12047");
		if (init_rak12047())
		{
			MYLOG("APP", "RAK12047 init success");
			has_rak12047 = true;
			snprintf(g_ble_dev_name, 9, "RAK_VOC");
		}
	}
}

void announce_modules(void)
{
	if (!has_rak1901)
	{
		MYLOG("APP", "SHTC3 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK1901 OK\n");
		read_rak1901();
	}

	if (!has_rak1902)
	{
		MYLOG("APP", "LPS22HB error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK1902 OK\n");
		read_rak1902();
	}

	if (!has_rak1903)
	{
		MYLOG("APP", "OPT3001 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK1903 OK\n");
		read_rak1903();
	}

	if (!has_rak1904)
	{
		MYLOG("APP", "ACC error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK1904 OK\n");
	}

	if (!has_rak1906)
	{
		MYLOG("APP", "BME680 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK1906 OK\n");
	}

	if (!has_rak12004)
	{
		MYLOG("APP", "MQ2 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12004 OK\n");
		read_rak12004();
	}

	if (!has_rak12008)
	{
		MYLOG("APP", "MG812 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12008 OK\n");
		read_rak12008();
	}

	if (!has_rak12009)
	{
		MYLOG("APP", "MQ3 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12009 OK\n");
		read_rak12009();
	}

	if (!has_rak12010)
	{
		MYLOG("APP", "VEML7700 error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12010 OK\n");
		read_rak12010();
	}

	if (!has_rak14003)
	{
		MYLOG("APP", "LED_BAR error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK14003 OK\n");
		uint8_t led_stat[10] = {0};
		set_rak14003(led_stat);
	}

	if (!has_gnss)
	{
		MYLOG("APP", "GNSS error");
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
	}

	if (!has_soil)
	{
		MYLOG("APP", "Soil Sensor error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12035 OK\n");
	}

	if (!has_rak12047)
	{
		MYLOG("APP", "SGP40 Sensor error");
		init_result = false;
	}
	else
	{
		AT_PRINTF("+EVT:RAK12047 OK\n");
	}
}