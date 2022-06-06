/**
 * @file RAK1910-RAK12500_gnss.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief GNSS functions and task
 * @version 0.3
 * @date 2022-01-29
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include <TinyGPS++.h>
// #include <SparkFun_Ublox_Arduino_Library.h>

// /** Instance for RAK1910 GNSS sensor */
// TinyGPSPlus my_rak1910_gnss;
// /** Instance for RAK12500 GNSS sensor */
// SFE_UBLOX_GPS my_gnss;

#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

/** Instance for RAK1910 GNSS sensor */
TinyGPSPlus my_rak1910_gnss;
/** Instance for RAK12500 GNSS sensor */
SFE_UBLOX_GNSS my_gnss;

/** GNSS task handle */
TaskHandle_t gnss_task_handle;

/** GPS reading task */
void gnss_task(void *pvParameters);

/** Semaphore for GNSS aquisition task */
SemaphoreHandle_t g_gnss_sem;

/** Timer for GNSS polling */
SoftwareTimer poll_timer;

/** Limiter for GNSS polling */
uint16_t check_gnss_max_try;

/** Counter for GNSS polling */
uint16_t check_gnss_counter;

/** Semaphore for GNSS polling */
SemaphoreHandle_t g_gnss_poll;

/** GNSS polling function */
bool poll_gnss(void);

/** Flag if location was found */
volatile bool last_read_ok = false;

/** Flag if GNSS is serial or I2C */
bool i2c_gnss = false;

/** The GPS module to use */
uint8_t g_gnss_option = 0;

/** Flag if GNSS module is on Serial1 or Serial2 */
uint8_t is_serial = 0;

/** Unified Serial port (Serial1 or Serial2) */
HardwareSerial *gnssSerial;

/**
 * @brief Initialize GNSS module
 *
 * @return true if GNSS module was found
 * @return false if no GNSS module was found
 */
bool init_gnss(void)
{
	// Power on the GNSS module
	digitalWrite(WB_IO2, HIGH);

	// Give the module some time to power up
	delay(500);

	if (g_gnss_option == NO_GNSS_INIT)
	{
		if (found_sensors[GNSS_ID].found_sensor)
		{
			Wire.begin();
			if (!my_gnss.begin(Wire))
			{
				MYLOG("GNSS", "Could not initialize RAK12500 on Wire");
				i2c_gnss = false;
			}
			else
			{
				i2c_gnss = true;
			}
			if (i2c_gnss)
			{
				MYLOG("GNSS", "RAK12500 found on I2C");
				i2c_gnss = true;
				my_gnss.setI2COutput(COM_TYPE_UBX); // Set the I2C port to output UBX only (turn off NMEA noise)
				g_gnss_option = RAK12500_GNSS;

				my_gnss.setNavigationFrequency(20);

				my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_GPS);
				my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_GALILEO);
				my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_GLONASS);
				my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_SBAS);
				my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_BEIDOU);
				my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_IMES);
				my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_QZSS);

				my_gnss.saveConfiguration(); // Save the current settings to flash and BBR

				return true;
			}
		}
		// No RAK12500 found, check if RAK1910 is plugged in
		MYLOG("GNSS", "Initialize RAK1910 on Serial1");
		Serial1.begin(9600);
		delay(100);

		Serial1.print("START");

		time_t timeout = millis();
		while ((millis() - timeout) < 1000)
		{
			char gnss = Serial1.read();
			// Serial.printf("%02x\n",gnss);
			if ((gnss >= 0x20) && (gnss <= 0x7F))
			{
				g_gnss_option = RAK1910_GNSS;
				MYLOG("GNSS", "Got data from RAK1910 after %ld", (uint32_t)(millis() - timeout));
				is_serial = 1;
				gnssSerial = &Serial1;
				return true;
			}
			delay(500);
		}
		MYLOG("GNSS", "Got no data from RAK1910 on Serial1 in %ld", (uint32_t)(millis() - timeout));
		MYLOG("GNSS", "End Serial1");
		Serial1.end();

		Serial2.begin(9600);
		delay(100);

		MYLOG("GNSS", "Initialize RAK1910 on Serial2");
		Serial2.print("START");

		timeout = millis();
		while ((millis() - timeout) < 1000)
		{
			char gnss = Serial2.read();
			// Serial.printf("%02x\n", gnss);
			if ((gnss >= 0x20) && (gnss <= 0x7F))
			{
				g_gnss_option = RAK1910_GNSS;
				MYLOG("GNSS", "Got data from RAK1910 after %ld", (uint32_t)(millis() - timeout));
				is_serial = 2;
				gnssSerial = &Serial2;
				return true;
			}
			delay(500);
		}
		MYLOG("GNSS", "Got no data from RAK1910 on Serial2 in %ld", (uint32_t)(millis() - timeout));

		MYLOG("GNSS", "End Serial2");
		Serial2.end();
	}
	else
	{
		if (g_gnss_option == RAK12500_GNSS)
		{
			my_gnss.begin(Wire);

			my_gnss.setNavigationFrequency(20);

			my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_GPS);
			my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_GALILEO);
			my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_GLONASS);
			my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_SBAS);
			my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_BEIDOU);
			my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_IMES);
			my_gnss.enableGNSS(true, SFE_UBLOX_GNSS_ID_QZSS);
		}
		else
		{
			gnssSerial->begin(9600);
			while (!gnssSerial)
				;
		}
		return true;
	}
	return false;
}

/**
 * @brief Check GNSS module for position
 *
 * @return true Valid position found
 * @return false No valid position
 */
bool poll_gnss(void)
{
	// MYLOG("GNSS", "poll_gnss");

	last_read_ok = false;

	int64_t latitude = 0;
	int64_t longitude = 0;
	int32_t altitude = 0;
	float accuracy = 0;
	uint8_t satellites = 0;

	bool has_pos = false;
	bool has_alt = false;

	if (g_gnss_option == RAK12500_GNSS)
	{
		if (my_gnss.getGnssFixOk())
		{
			byte fix_type = my_gnss.getFixType(); // Get the fix type
			char fix_type_str[32] = {0};
			if (fix_type == 0)
				sprintf(fix_type_str, "No Fix");
			else if (fix_type == 1)
				sprintf(fix_type_str, "Dead reckoning");
			else if (fix_type == 2)
				sprintf(fix_type_str, "Fix type 2D");
			else if (fix_type == 3)
				sprintf(fix_type_str, "Fix type 3D");
			else if (fix_type == 4)
				sprintf(fix_type_str, "GNSS fix");
			else if (fix_type == 5)
				sprintf(fix_type_str, "Time fix");

			accuracy = (float)(my_gnss.getHorizontalDOP() / 100.0);
			satellites = my_gnss.getSIV();
			MYLOG("GNSS", "HDOP: %.2f ", accuracy);
			MYLOG("GNSS", "SIV: %d ", satellites);

			if ((fix_type >= 3) && (satellites >= 5) && (accuracy <= 2.00)) /** Fix type 3D,at least 5 satellites & HDOP better than 2 */
			// if (fix_type >= 3) /** Fix type 3D */
			{
				last_read_ok = true;
				latitude = my_gnss.getLatitude();
				longitude = my_gnss.getLongitude();
				altitude = my_gnss.getAltitude();
				my_gnss.flushDOP();
				my_gnss.flushPVT();

				MYLOG("GNSS", "Fixtype: %d %s", my_gnss.getFixType(), fix_type_str);
				MYLOG("GNSS", "Lat: %.4f Lon: %.4f", latitude / 10000000.0, longitude / 10000000.0);
				MYLOG("GNSS", "Alt: %.2f", altitude / 1000.0);
				// MYLOG("GNSS", "HDOP: %d ", accuracy);
				// MYLOG("GNSS", "SIV: %d ", satellites);
			}
		}
		// }
		// else
		// {
		// 	MYLOG("GNSS", "PVT not finished");
		// }
	}
	else
	{
		while (gnssSerial->available() > 0)
		{
			// char gnss = gnssSerial->read();
			// Serial.print(gnss);
			// if (my_rak1910_gnss.encode(gnss))
			if (my_rak1910_gnss.encode(gnssSerial->read()))
			{
				if (my_rak1910_gnss.location.isUpdated() && my_rak1910_gnss.location.isValid())
				{
					MYLOG("GNSS", "Location valid");
					has_pos = true;
					latitude = (my_rak1910_gnss.location.lat() * 10000000.0);
					longitude = (my_rak1910_gnss.location.lng() * 10000000.0);
				}
				else if (my_rak1910_gnss.altitude.isUpdated() && my_rak1910_gnss.altitude.isValid())
				{
					MYLOG("GNSS", "Altitude valid");
					has_alt = true;
					altitude = (my_rak1910_gnss.altitude.meters() * 1000);
				}
				else if (my_rak1910_gnss.hdop.isUpdated() && my_rak1910_gnss.hdop.isValid())
				{
					accuracy = my_rak1910_gnss.hdop.hdop() * 100;
				}
			}
			// if (has_pos && has_alt)
			if (has_pos && has_alt)
			{
				MYLOG("GNSS", "Lat: %.4f Lon: %.4f", latitude / 10000000.0, longitude / 10000000.0);
				MYLOG("GNSS", "Alt: %.2f", altitude / 1000.0);
				MYLOG("GNSS", "Acy: %.2f ", accuracy / 100.0);
				last_read_ok = true;
				break;
			}
		}
		if (has_pos && has_alt)
		{
			last_read_ok = true;
		}
	}

	if (last_read_ok)
	{
		if ((latitude == 0) && (longitude == 0))
		{
			last_read_ok = false;
			return false;
		}
		if (!g_is_helium && !g_is_tester)
		{
			if (g_gps_prec_6)
			{
				// Save extended precision, not Cayenne LPP compatible
				g_solution_data.addGNSS_6(LPP_CHANNEL_GPS, latitude, longitude, altitude);
			}
			else
			{
				// Save default Cayenne LPP precision
				g_solution_data.addGNSS_4(LPP_CHANNEL_GPS, latitude, longitude, altitude);
			}
		}
		else
		{
			if (g_is_helium)
			{
				// Save Helium Mapper format
				g_solution_data.addGNSS_H(latitude, longitude, altitude, (int16_t)accuracy, read_batt());
			}
			if (g_is_tester)
			{
				// Save Field Tester format
				g_solution_data.addGNSS_T(latitude, longitude, altitude, accuracy, satellites);
			}
		}

		if (g_is_helium || g_is_tester)
		{
			my_gnss.setNavigationFrequency(1, 10000);
			my_gnss.powerSaveMode(true, 10000);
		}

		return true;
	}
	else
	{
		// No location found
#if FAKE_GPS > 0
		MYLOG("GNSS", "Faking GPS");
		// 14.4213730, 121.0069140, 35.000
		latitude = 144213730;
		longitude = 1210069140;
		altitude = 35000;
		accuracy = 1;
		satellites = 5;

		MYLOG("GNSS", "Lat: %.4f Lon: %.4f", latitude / 10000000.0, longitude / 10000000.0);
		MYLOG("GNSS", "Alt: %.2f", altitude / 1000.0);
		MYLOG("GNSS", "HDOP: %d ", accuracy);
		MYLOG("GNSS", "SIV: %d ", satellites);

		if (!g_is_helium && !g_is_tester)
		{
			if (g_gps_prec_6)
			{
				// Save extended precision, not Cayenne LPP compatible
				g_solution_data.addGNSS_6(LPP_CHANNEL_GPS, latitude, longitude, altitude);
			}
			else
			{
				// Save default Cayenne LPP precision
				g_solution_data.addGNSS_4(LPP_CHANNEL_GPS, latitude, longitude, altitude);
			}
		}
		else
		{
			if (g_is_helium)
			{
				// Save Helium Mapper format
				g_solution_data.addGNSS_H(latitude, longitude, altitude, (int16_t)accuracy, read_batt());
			}
			else
			{
				g_solution_data.addGNSS_T(latitude, longitude, altitude, accuracy, satellites);
			}
		}
		last_read_ok = true;
		return true;
#endif
	}

	MYLOG("GNSS", "No valid location found");
	last_read_ok = false;

	if (g_is_helium || g_is_tester)
	{
		if (g_gnss_option == RAK12500_GNSS)
		{
			my_gnss.setNavigationFrequency(1);
		}
	}

	return false;
}

/**
 * @brief Task to read from GNSS module without stopping the loop
 *
 * @param pvParameters unused
 */
void gnss_task(void *pvParameters)
{
	MYLOG("GNSS", "GNSS Task started");

	poll_timer.begin(5000, wake_poll, NULL, true);

	if (!g_is_helium && !g_is_tester)
	{
		// Power down the module
		digitalWrite(WB_IO2, LOW);
		delay(100);
	}

	while (1)
	{
		if (xSemaphoreTake(g_gnss_sem, portMAX_DELAY) == pdTRUE)
		{
			if (!g_is_helium && !g_is_tester)
			{
				// Startup GNSS module
				init_gnss();
			}

			if (g_is_helium || g_is_tester)
			{
				g_solution_data.reset();
			}
			MYLOG("GNSS", "GNSS Task wake up");
			AT_PRINTF("+EVT:START_LOCATION\n");

			// Start polling the GNSS module
			check_gnss_counter = 0;
			if (g_lorawan_settings.send_repeat_time != 0)
			{
				check_gnss_max_try = g_lorawan_settings.send_repeat_time / 2 / 5000;
			}
			else
			{
				check_gnss_max_try = 10;
			}
			poll_timer.start();

			bool got_location = false;

			// Make the first poll immediate
			xSemaphoreGive(g_gnss_poll);

			while (check_gnss_counter < check_gnss_max_try)
			{
				// MYLOG("GNSS", "GNSS Wait for semaphore");
				check_gnss_counter++;
				if ((xSemaphoreTake(g_gnss_poll, portMAX_DELAY) == pdTRUE))
				{
					digitalWrite(LED_BLUE, HIGH);
					// MYLOG("GNSS", "GNSS polling");
					// Get location
					got_location = poll_gnss();

					digitalWrite(LED_BLUE, LOW);
					if (got_location)
					{
						// Found location, finish polling
						check_gnss_counter = check_gnss_max_try + 1;
						// break;
					}
				}
			}
			poll_timer.stop();

			AT_PRINTF("+EVT:LOCATION %s\n", got_location ? "FIX" : "NOFIX");

			// if ((g_task_sem != NULL) && got_location)
			if (g_is_helium && !got_location)
			{
				MYLOG("GNSS", "Helium and no location, skip sending");
			}
			else if (g_is_tester && !got_location)
			{
				MYLOG("GNSS", "Field tester and no location, skip sending");
			}
			else
			{
				if (g_task_sem != NULL)
				{
					api_wake_loop(GNSS_FIN);
				}
			}

			if (!g_is_helium && !g_is_tester)
			{
				// Power down the module
				digitalWrite(WB_IO2, LOW);
				delay(100);
			}

			MYLOG("GNSS", "GNSS Task finished");
		}
	}
}

/**
 * @brief Timer callback, used to wakeup GNSS task
 * to check if location is acquired
 *
 * @param unused
 */
void wake_poll(TimerHandle_t unused)
{
	// MYLOG("GNSS", "Enable poll");
	xSemaphoreGive(g_gnss_poll);
}

/********************************************************************************************/

// #include <SparkFun_u-blox_GNSS_Arduino_Library.h>

// /** Instance for RAK1910 GNSS sensor */
// TinyGPSPlus my_rak1910_gnss;
// /** Instance for RAK12500 GNSS sensor */
// SFE_UBLOX_GNSS my_gnss;

// /** GNSS task handle */
// TaskHandle_t gnss_task_handle;

// /** GPS reading task */
// void gnss_task(void *pvParameters);

// /** Semaphore for GNSS aquisition task */
// SemaphoreHandle_t g_gnss_sem;

// /** GNSS polling function */
// bool poll_gnss(void);

// /** Flag if location was found */
// volatile bool last_read_ok = false;

// /** Flag if GNSS is serial or I2C */
// bool i2c_gnss = false;

// /** The GPS module to use */
// uint8_t g_gnss_option = 0;

// /**
//  * @brief Initialize GNSS module
//  *
//  * @return true if GNSS module was found
//  * @return false if no GNSS module was found
//  */
// bool init_gnss(void)
// {
// 	// Power on the GNSS module
// 	digitalWrite(WB_IO2, HIGH);

// 	// Give the module some time to power up
// 	delay(500);

// 	if (g_gnss_option == NO_GNSS_INIT)
// 	{
// 		if (found_sensors[GNSS_ID].found_sensor)
// 		{
// 			if (found_sensors[GNSS_ID].i2c_num == 1)
// 			{
// 				Wire.begin();
// 				if (!my_gnss.begin(Wire))
// 				{
// 					MYLOG("GNSS", "Could not initialize RAK12500 on Wire");
// 					i2c_gnss = false;
// 				}
// 				else
// 				{
// 					i2c_gnss = true;
// 				}
// 			}
// 			else
// 			{
// #if WIRE_INTERFACES_COUNT > 1
// 				Wire1.begin();
// 				if (!my_gnss.begin(Wire1))
// 				{
// 					MYLOG("GNSS", "Could not initialize RAK12500 on Wire1");
// 					i2c_gnss = false;
// 				}
// 				else
// 				{
// 					i2c_gnss = true;
// 				}
// #else
// 				return false;
// #endif
// 			}
// 			if (i2c_gnss)
// 			{
// 				MYLOG("GNSS", "RAK12500 found on I2C");
// 				i2c_gnss = true;
// 				my_gnss.setI2COutput(COM_TYPE_UBX); // Set the I2C port to output UBX only (turn off NMEA noise)
// 				g_gnss_option = RAK12500_GNSS;

// 				my_gnss.saveConfiguration(); // Save the current settings to flash and BBR

// 				my_gnss.setMeasurementRate(500);

// 				return true;
// 			}
// 		}
// 		// No RAK12500 found, check if RAK1910 is plugged in
// 		MYLOG("GNSS", "Initialize RAK1910 on Serial1");
// 		Serial1.begin(9600);
// 		delay(100);

// 		Serial1.print("START");

// 		time_t timeout = millis();
// 		while ((millis() - timeout) < 1000)
// 		{
// 			char gnss = Serial1.read();
// 			// Serial.printf("%02x\n",gnss);
// 			if ((gnss >= 0x20) && (gnss <= 0x7F))
// 			{
// 				g_gnss_option = RAK1910_GNSS;
// 				MYLOG("GNSS", "Got data from RAK1910 after %ld", (uint32_t)(millis() - timeout));
// 				return true;
// 			}
// 			delay(500);
// 		}
// 		MYLOG("GNSS", "Got no data from RAK1910 on Serial1 in %ld", (uint32_t)(millis() - timeout));

// 		Serial1.end();

// 		Serial2.begin(9600);
// 		delay(100);

// 		MYLOG("GNSS", "Initialize RAK1910 on Serial2");
// 		Serial2.print("START");

// 		timeout = millis();
// 		while ((millis() - timeout) < 1000)
// 		{
// 			char gnss = Serial2.read();
// 			// Serial.printf("%02x\n", gnss);
// 			if ((gnss >= 0x20) && (gnss <= 0x7F))
// 			{
// 				g_gnss_option = RAK1910_GNSS;
// 				MYLOG("GNSS", "Got data from RAK1910 after %ld", (uint32_t)(millis() - timeout));
// 				return true;
// 			}
// 			delay(500);
// 		}
// 		MYLOG("GNSS", "Got no data from RAK1910 on Serial2 in %ld", (uint32_t)(millis() - timeout));

// 		Serial2.end();
// 	}
// 	else
// 	{
// 		if (g_gnss_option == RAK12500_GNSS)
// 		{
// 			if (found_sensors[GNSS_ID].i2c_num == 1)
// 			{
// 				my_gnss.begin(Wire);
// 			}
// 			else
// 			{
// #if WIRE_INTERFACES_COUNT > 1
// 				my_gnss.begin(Wire1);
// #else
// 				return false;
// #endif
// 			}
// 		}
// 		else
// 		{
// 			Serial1.begin(9600);
// 			while (!Serial1)
// 				;
// 		}
// 		return true;
// 	}
// 	return false;
// }

// /**
//  * @brief Check GNSS module for position
//  *
//  * @return true Valid position found
//  * @return false No valid position
//  */
// bool poll_gnss(void)
// {
// 	MYLOG("GNSS", "poll_gnss");

// 	last_read_ok = false;

// 	// if (!g_is_helium)
// 	// {
// 	// 	// Startup GNSS module
// 	// 	init_gnss();
// 	// }

// 	time_t time_out = millis();
// 	int64_t latitude = 0;
// 	int64_t longitude = 0;
// 	int32_t altitude = 0;
// 	int32_t accuracy = 0;

// 	time_t check_limit = 90000;

// 	if (g_lorawan_settings.send_repeat_time == 0)
// 	{
// 		check_limit = 90000;
// 	}
// 	else if (g_lorawan_settings.send_repeat_time <= 90000)
// 	{
// 		check_limit = g_lorawan_settings.send_repeat_time / 2;
// 	}
// 	else
// 	{
// 		check_limit = 90000;
// 	}

// 	MYLOG("GNSS", "GNSS timeout %ld", (long int)check_limit);

// 	MYLOG("GNSS", "Using %s", g_gnss_option == RAK12500_GNSS ? "RAK12500" : "RAK1910");

// 	bool has_pos = false;
// 	bool has_alt = false;

// #if FAKE_GPS > 0
// 	check_limit = 1000;
// #endif
// 	while ((millis() - time_out) < check_limit)
// 	{
// 		if (g_gnss_option == RAK12500_GNSS)
// 		{
// 			if (my_gnss.getGnssFixOk())
// 			{
// 				byte fix_type = my_gnss.getFixType(); // Get the fix type
// 				char fix_type_str[32] = {0};
// 				if (fix_type == 0)
// 					sprintf(fix_type_str, "No Fix");
// 				else if (fix_type == 1)
// 					sprintf(fix_type_str, "Dead reckoning");
// 				else if (fix_type == 2)
// 					sprintf(fix_type_str, "Fix type 2D");
// 				else if (fix_type == 3)
// 					sprintf(fix_type_str, "Fix type 3D");
// 				else if (fix_type == 4)
// 					sprintf(fix_type_str, "GNSS fix");
// 				else if (fix_type == 5)
// 					sprintf(fix_type_str, "Time fix");

// 				// if ((fix_type >= 3) && (my_gnss.getSIV() >= 5)) /** Fix type 3D and at least 5 satellites */
// 				if (fix_type >= 3) /** Fix type 3D */
// 				{
// 					last_read_ok = true;
// 					latitude = my_gnss.getLatitude();
// 					longitude = my_gnss.getLongitude();
// 					altitude = my_gnss.getAltitude();
// 					accuracy = my_gnss.getHorizontalDOP();

// 					MYLOG("GNSS", "Fixtype: %d %s", my_gnss.getFixType(), fix_type_str);
// 					MYLOG("GNSS", "Lat: %.4f Lon: %.4f", latitude / 10000000.0, longitude / 10000000.0);
// 					MYLOG("GNSS", "Alt: %.2f", altitude / 1000.0);
// 					MYLOG("GNSS", "Acy: %.2f ", accuracy / 100.0);

// 					// Break the while()
// 					break;
// 				}
// 			}
// 			else
// 			{
// 				delay(1000);
// 			}
// 		}
// 		else
// 		{
// 			while (Serial1.available() > 0)
// 			{
// 				// char gnss = Serial1.read();
// 				// Serial.print(gnss);
// 				// if (my_rak1910_gnss.encode(gnss))
// 				if (my_rak1910_gnss.encode(Serial1.read()))
// 				{
// 					if (my_rak1910_gnss.location.isUpdated() && my_rak1910_gnss.location.isValid())
// 					{
// 						MYLOG("GNSS", "Location valid");
// 						has_pos = true;
// 						latitude = (my_rak1910_gnss.location.lat() * 10000000.0);
// 						longitude = (my_rak1910_gnss.location.lng() * 10000000.0);
// 					}
// 					else if (my_rak1910_gnss.altitude.isUpdated() && my_rak1910_gnss.altitude.isValid())
// 					{
// 						MYLOG("GNSS", "Altitude valid");
// 						has_alt = true;
// 						altitude = (my_rak1910_gnss.altitude.meters() * 1000);
// 					}
// 					else if (my_rak1910_gnss.hdop.isUpdated() && my_rak1910_gnss.hdop.isValid())
// 					{
// 						accuracy = my_rak1910_gnss.hdop.hdop() * 100;
// 					}
// 				}
// 				// if (has_pos && has_alt)
// 				if (has_pos && has_alt)
// 				{
// 					MYLOG("GNSS", "Lat: %.4f Lon: %.4f", latitude / 10000000.0, longitude / 10000000.0);
// 					MYLOG("GNSS", "Alt: %.2f", altitude / 1000.0);
// 					MYLOG("GNSS", "Acy: %.2f ", accuracy / 100.0);
// 					last_read_ok = true;
// 					break;
// 				}
// 			}
// 			if (has_pos && has_alt)
// 			{
// 				last_read_ok = true;
// 				break;
// 			}
// 		}
// 	}

// 	if (!g_is_helium)
// 	{
// 		// Power down the module
// 		digitalWrite(WB_IO2, LOW);
// 		delay(100);
// 	}

// 	if (last_read_ok)
// 	{
// 		if ((latitude == 0) && (longitude == 0))
// 		{
// 			last_read_ok = false;
// 			return false;
// 		}
// 		if (!g_is_helium)
// 		{
// 			if (g_gps_prec_6)
// 			{
// 				// Save extended precision, not Cayenne LPP compatible
// 				g_solution_data.addGNSS_6(LPP_CHANNEL_GPS, latitude, longitude, altitude);
// 			}
// 			else
// 			{
// 				// Save default Cayenne LPP precision
// 				g_solution_data.addGNSS_4(LPP_CHANNEL_GPS, latitude, longitude, altitude);
// 			}
// 		}
// 		else
// 		{
// 			// Save default Cayenne LPP precision
// 			g_solution_data.addGNSS_H(latitude, longitude, altitude, accuracy, read_batt());
// 		}

// 		if (g_is_helium)
// 		{
// 			my_gnss.setMeasurementRate(10000);
// 			my_gnss.setNavigationFrequency(1, 10000);
// 			my_gnss.powerSaveMode(true, 10000);
// 		}

// 		return true;
// 	}
// 	else
// 	{
// 		// No location found
// #if FAKE_GPS > 0
// 		MYLOG("GNSS", "Faking GPS");
// 		// 14.4213730, 121.0069140, 35.000
// 		latitude = 144213730;
// 		longitude = 1210069140;
// 		altitude = 35000;
// 		accuracy = 100;

// 		if (!g_is_helium)
// 		{
// 			if (g_gps_prec_6)
// 			{
// 				// Save extended precision, not Cayenne LPP compatible
// 				g_solution_data.addGNSS_6(LPP_CHANNEL_GPS, latitude, longitude, altitude);
// 			}
// 			else
// 			{
// 				// Save default Cayenne LPP precision
// 				g_solution_data.addGNSS_4(LPP_CHANNEL_GPS, latitude, longitude, altitude);
// 			}
// 		}
// 		else
// 		{
// 			// Save default Cayenne LPP precision
// 			g_solution_data.addGNSS_H(latitude, longitude, altitude, accuracy, read_batt());
// 		}
// 		last_read_ok = true;
// 		return true;
// #endif
// 	}

// 	MYLOG("GNSS", "No valid location found");
// 	last_read_ok = false;

// 	if (g_is_helium)
// 	{
// 		if (g_gnss_option == RAK12500_GNSS)
// 		{
// 			my_gnss.setMeasurementRate(1000);
// 		}
// 	}

// 	return false;
// }

// /**
//  * @brief Task to read from GNSS module without stopping the loop
//  *
//  * @param pvParameters unused
//  */
// void gnss_task(void *pvParameters)
// {
// 	MYLOG("GNSS", "GNSS Task started");

// 	if (!g_is_helium)
// 	{
// 		// Power down the module
// 		digitalWrite(WB_IO2, LOW);
// 		delay(100);
// 	}

// 	while (1)
// 	{
// 		if (xSemaphoreTake(g_gnss_sem, portMAX_DELAY) == pdTRUE)
// 		{
// 			if (!g_is_helium)
// 			{
// 				// Startup GNSS module
// 				init_gnss();
// 			}

// 			if (g_is_helium)
// 			{
// 				g_solution_data.reset();
// 			}
// 			MYLOG("GNSS", "GNSS Task wake up");
// 			AT_PRINTF("+EVT:START_LOCATION\n");

// 			// Get location
// 			bool got_location = poll_gnss();
// 			AT_PRINTF("+EVT:LOCATION %s\n", got_location ? "FIX" : "NOFIX");

// 			// if ((g_task_sem != NULL) && got_location)
// 			if (g_is_helium && !got_location)
// 			{
// 				MYLOG("GNSS", "Helium and no location, skip sending");
// 			}
// 			else
// 			{
// 				if (g_task_sem != NULL)
// 				{
// 					api_wake_loop(GNSS_FIN);
// 				}
// 			}
// 			MYLOG("GNSS", "GNSS Task finished");
// 		}
// 	}
// }
