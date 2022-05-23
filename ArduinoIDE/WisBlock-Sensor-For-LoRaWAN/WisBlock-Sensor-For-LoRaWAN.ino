/**
 * @file WisBlock-Sensor-For-LoRaWAN.ino
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Application specific functions. Mandatory to have init_app(),
 *        app_event_handler(), ble_data_handler(), lora_data_handler()
 *        and lora_tx_finished()
 * @version 0.2
 * @date 2022-01-30
 *
 * @copyright Copyright (c) 2022
 *
 */
/*******************************************************************/
/*******************************************************************/
/** For Arduino IDE these libraries need to be installed manually: */
/*******************************************************************/
/*******************************************************************/
// SX126x-Arduino                         //Click here to install the library => http://librarymanager/All#SX126x-Arduino
// WisBlock-API                           //Click here to install the library => http://librarymanager/All#WisBlock-API
// SparkFun SHTC3                         //Click here to install the library => http://librarymanager/All#SparkFun_SHTC3
// Adafruit LPS2X                         //Click here to install the library => http://librarymanager/All#Adafruit_LPS2X
// Adafruit BME680                        //Click here to install the library => http://librarymanager/All#Adafruit_BME680
// CayenneLPP                             //Click here to install the library => http://librarymanager/All#CayenneLPP
// SparkFun u-blox GNSS                   //Click here to install the library => http://librarymanager/All#SparkFun_u-blox_GNSS
// TinyGPSPlus                            //Click here to install the library => http://librarymanager/All#TinyGPSPlus
// Adafruit LIS3DH                        //Click here to install the library => http://librarymanager/All#Adafruit_LIS3DH
// RAK12035_SoilMoisture                  //Click here to install the library => http://librarymanager/All#RAK12035_SoilMoisture
// RAKwireless VEML Light Sensor          //Click here to install the library => http://librarymanager/All#RAKwireless_VEML_Light_Sensor
// Sensirion Core                         //Click here to install the library => http://librarymanager/All#Sensirion_Core
// Sensirion Gas Index Algorithm          //Click here to install the library => http://librarymanager/All#Sensirion_Gas_Index_Algorithm
// Sensirion I2C SGP40                    //Click here to install the library => http://librarymanager/All#Sensirion_I2C_SGP40
// RAKwireless MQx                        //Click here to install the library => http://librarymanager/All#RAKwireless_MQx
// Adafruit MCP23017                      //Click here to install the library => http://librarymanager/All#Adafruit_MCP23017
// VL53L0X                                //Click here to install the library => http://librarymanager/All#VL53L0X (CHOOSE THE ONE FROM POLOLU)
// RAK I3G4250D                           //Click here to install the library => http://librarymanager/All#RAK_I3G4250D
// RevEng PAJ7620                         //Click here to install the library => http://librarymanager/All#RevEng_PAJ7620
// nRF52_OLED                             //Click here to install the library => http://librarymanager/All#nRF52_OLED
// Melopero RV3028                        //Click here to install the library => http://librarymanager/All#Melopero_RV3028
// Coulomb Counter for 3.3V to 5V LTC2941 //Click here to install the library => http://librarymanager/All#Grove_Coulomb-Counter
// RAK12019_LTR390                        //Click here to install the library => http://librarymanager/All#RAK12019_LTR390
// INA219_WE                              //Click here to install the library => http://librarymanager/All#INA219_WE
// RAKwireless CAP1293                    //Click here to install the library => http://librarymanager/All#RAKwireless_CAP1293
// MPU9250_WE                             //Click here to install the library => http://librarymanager/All#MPU9250_WE
// ClosedCube_OPT3001                     //Click here to install the library => http://librarymanager/All#ClosedCube_OPT3001
// LPS35HW                                //Click here to install the library => http://librarymanager/All#LPS35HW (CHOOSE THE ONE FROM PAVEL SLAMA)
// Sparkfun SCD30                         //Click here to install the library => http://librarymanager/All#Sparkfun_SCD30
// Sparkfun MLX90632                      //Click here to install the library => http://librarymanager/All#Sparkfun_MLX90632
// Melopero AMG8833                       //Click here to install the library => http://librarymanager/All#Melopero_AMG8833
// SparkFun ADXL313 Arduino Library       //Click here to install the library => http://librarymanager/All#SparkFun_AMG8833
// RAKwireless Storage                    //Click here to install the library => http://librarymanager/All#RAKwireless_Storage
// ArduinoECCX08                          //Click here to install the library => http://librarymanager/All#ArduinoECCX08
// Adafruit FRAM I2C                      //Click here to install the library => http://librarymanager/All#Adafruit%20FRAM%20I2C

/*******************************************************************/

#include "app.h"

/** Timer since last position message was sent */
time_t last_pos_send = 0;
/** Timer for delayed sending to keep duty cycle */
SoftwareTimer delayed_sending;

/** Flag if delayed sending is already activated */
bool delayed_active = false;

/** Minimum delay between sending new locations, set to 45 seconds */
time_t min_delay = 45000;

/** GPS precision */
bool g_gps_prec_6 = true;

/** Switch between Cayenne LPP and Helium Mapper data packet */
bool g_is_helium = false;

/** Flag for battery protection enabled */
bool battery_check_enabled = false;

// Forward declaration
void send_delayed(TimerHandle_t unused);

/** Set the device name, max length is 10 characters */
char g_ble_dev_name[10] = "RAK-SENS";

/** Send Fail counter **/
uint8_t send_fail = 0;

/** Flag for low battery protection */
bool low_batt_protection = false;

/** LoRaWAN packet */
WisCayenne g_solution_data(255);

/** Initialization result */
bool init_result = true;

char disp_txt[64] = {0};

// /** Structure for multicast group entry */
// MulticastParams_t test_multicast;
// /** Multicast network session key, must be the same as in the Multicast group in the LNS **/
// uint8_t _mc_nwskey[] = {0xc2, 0x52, 0x19, 0xc3, 0x69, 0xae, 0x0a, 0xc4, 0xa9, 0x17, 0x61, 0xee, 0x1b, 0x8d, 0xc4, 0xc5};
// /** Multicast application session key, must be the same as in the Multicast group in the LNS **/
// uint8_t _mc_appskey[] = {0x03, 0x76, 0xc3, 0xe7, 0x99, 0x3c, 0xe1, 0xcd, 0x64, 0xa6, 0x6d, 0xa0, 0x70, 0x88, 0xcc, 0xad};
// /** Multicast device address, must be the same as in the Multicast group in the LNS **/
// uint32_t _mc_devaddr = 0x7bca00be;

/**
 * @brief Application specific setup functions
 *
 */
void setup_app(void)
{
	// Initialize Serial for debug output
	Serial.begin(115200);

	time_t serial_timeout = millis();
	// On nRF52840 the USB serial is not available immediately
	while (!Serial)
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
			digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
		}
		else
		{
			break;
		}
	}

	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, HIGH);

	delay(500);

	// Scan the I2C interfaces for devices
	find_modules();

	// Initialize the User AT command list
	init_user_at();

	// Enable BLE
	g_enable_ble = true;
}

/**
 * @brief Application specific initializations
 *
 * @return true Initialization success
 * @return false Initialization failure
 */
bool init_app(void)
{
	MYLOG("APP", "init_app");

	api_set_version(SW_VERSION_1, SW_VERSION_2, SW_VERSION_3);

	// Get the battery check setting
	read_batt_settings();

	if (found_sensors[GNSS_ID].found_sensor)
	{
		// Get precision settings
		read_gps_settings();
	}

	AT_PRINTF("============================\n");
	if (found_sensors[SOIL_ID].found_sensor)
	{
		AT_PRINTF("Soil Moisture Solution\n");
	}
	else if (found_sensors[GNSS_ID].found_sensor)
	{
		if (g_is_helium)
		{
			AT_PRINTF("Helium Mapper Solution\n");
		}
		else
		{
			AT_PRINTF("LPWAN Tracker Solution\n");
		}
	}
	else if (found_sensors[ENV_ID].found_sensor)
	{
		AT_PRINTF("LPWAN Environment Solution\n");
	}
	else if (found_sensors[PRESS_ID].found_sensor)
	{
		AT_PRINTF("LPWAN Weather Sensor\n");
	}
	else if (found_sensors[VOC_ID].found_sensor)
	{
		AT_PRINTF("LPWAN VOC Sensor\n");
	}
	else
	{
		AT_PRINTF("LPWAN WisBlock Node\n");
	}
	AT_PRINTF("Built with RAK's WisBlock\n");
	AT_PRINTF("SW Version %d.%d.%d\n", g_sw_ver_1, g_sw_ver_2, g_sw_ver_3);
	AT_PRINTF("LoRa(R) is a registered trademark or service mark of Semtech Corporation or its affiliates.\nLoRaWAN(R) is a licensed mark.\n");
	AT_PRINTF("============================\n");
	api_log_settings();

	// Announce found modules with +EVT: over Serial
	announce_modules();

	AT_PRINTF("============================\n");

	Serial.flush();
	// Reset the packet
	g_solution_data.reset();

	if (found_sensors[OLED_ID].found_sensor)
	{
		if (found_sensors[RTC_ID].found_sensor)
		{
			read_rak12002();
			snprintf(disp_txt, 64, "%d:%02d Init finished", g_date_time.hour, g_date_time.minute);
		}
		else
		{
			snprintf(disp_txt, 64, "Init finished");
		}
		rak1921_add_line(disp_txt);
	}
	return true;
}

/**
 * @brief Application specific event handler
 *        Requires as minimum the handling of STATUS event
 *        Here you handle as well your application specific events
 */
void app_event_handler(void)
{
#ifdef NRF52_SERIES
// #if MY_DEBUG > 0
// 	// dbgMemInfo();
// 	if ((g_task_event_type & STATUS) == STATUS)
// 	{
// 		MYLOG("APP", "STATUS WAKEUP");
// 	}
// 	if ((g_task_event_type & VOC_REQ) == VOC_REQ)
// 	{
// 		MYLOG("APP", "VOC_REQ WAKEUP");
// 	}
// 	if ((g_task_event_type & MOTION_TRIGGER) == MOTION_TRIGGER)
// 	{
// 		MYLOG("APP", "MOTION_TRIGGER WAKEUP");
// 	}
// 	if ((g_task_event_type & TOUCH_EVENT) == TOUCH_EVENT)
// 	{
// 		MYLOG("APP", "TOUCH_EVENT WAKEUP");
// 	}
// 	if ((g_task_event_type & GNSS_FIN) == GNSS_FIN)
// 	{
// 		MYLOG("APP", "GNSS_FIN WAKEUP");
// 	}
// 	if ((g_task_event_type & BLE_CONFIG) == BLE_CONFIG)
// 	{
// 		MYLOG("APP", "BLE_CONFIG WAKEUP");
// 	}
// 	if ((g_task_event_type & BLE_DATA) == BLE_DATA)
// 	{
// 		MYLOG("APP", "BLE_DATA WAKEUP");
// 	}
// 	if ((g_task_event_type & LORA_DATA) == LORA_DATA)
// 	{
// 		MYLOG("APP", "LORA_DATA WAKEUP");
// 	}
// 	if ((g_task_event_type & LORA_TX_FIN) == LORA_TX_FIN)
// 	{
// 		MYLOG("APP", "LORA_TX_FIN WAKEUP");
// 	}
// 	if ((g_task_event_type & AT_CMD) == AT_CMD)
// 	{
// 		MYLOG("APP", "AT_CMD WAKEUP");
// 	}
// 	if ((g_task_event_type & LORA_JOIN_FIN) == LORA_JOIN_FIN)
// 	{
// 		MYLOG("APP", "LORA_JOIN_FIN WAKEUP");
// 	}
// 	char buffer[64] = {0};
// 	itoa(g_task_event_type, buffer, 2);
// 	MYLOG("APP", "Wakeup Flag %s", buffer);
// #endif
#endif

	// Timer triggered event
	if ((g_task_event_type & STATUS) == STATUS)
	{
		if (found_sensors[ENV_ID].found_sensor && !g_is_helium)
		{
			// Startup the BME680
			start_rak1906();
		}
		if (found_sensors[PRESS_ID].found_sensor && !g_is_helium)
		{
			// Startup the LPS22HB
			start_rak1902();
		}
		g_task_event_type &= N_STATUS;
		MYLOG("APP", "Timer wakeup");

		// If BLE is enabled, restart Advertising
		if (g_enable_ble)
		{
			restart_advertising(15);
		}

		// Reset the packet
		g_solution_data.reset();

		if (!low_batt_protection)
		{
			if (!g_is_helium)
			{
				// Get values from the connected modules
				get_sensor_values();
			}
			if (found_sensors[GNSS_ID].found_sensor)
			{
				// Start the GNSS location tracking
				xSemaphoreGive(g_gnss_sem);
			}
		}

		// Get battery level
		float batt_level_f = read_batt();
		g_solution_data.addVoltage(LPP_CHANNEL_BATT, batt_level_f / 1000.0);

		if (found_sensors[OLED_ID].found_sensor)
		{
			if (found_sensors[RTC_ID].found_sensor)
			{
				read_rak12002();
				snprintf(disp_txt, 64, "%d:%02d Bat %.3fV", g_date_time.hour, g_date_time.minute, batt_level_f / 1000);
			}
			else
			{
				snprintf(disp_txt, 64, "Battery %.3fV", batt_level_f / 1000);
			}
			rak1921_add_line(disp_txt);
		}

		// Protection against battery drain if battery check is enabled
		if (battery_check_enabled)
		{
			if (batt_level_f < 2900)
			{
				// Battery is very low, change send time to 1 hour to protect battery
				low_batt_protection = true; // Set low_batt_protection active
				api_timer_restart(1 * 60 * 60 * 1000);
				MYLOG("APP", "Battery protection activated");
			}
			else if ((batt_level_f > 4100) && low_batt_protection)
			{
				// Battery is higher than 4V, change send time back to original setting
				low_batt_protection = false;
				api_timer_restart(g_lorawan_settings.send_repeat_time);
				MYLOG("APP", "Battery protection deactivated");
			}
		}

		// Just as an example, RAK14008 is used to display the status of the battery
		uint8_t led_status[10] = {0};

		for (int idx = 9, lev = 1; idx >= 0; idx--, lev++)
		{
			if (batt_level_f > (4200 - (lev * 420)))
			{
				led_status[idx] = 1;
			}
		}
		set_rak14003(led_status);

		if (!found_sensors[GNSS_ID].found_sensor)
		{
			if (found_sensors[ENV_ID].found_sensor)
			{
				// Read environment data
				read_rak1906();
			}
			if (found_sensors[PRESS_ID].found_sensor)
			{
				// Read environment data
				read_rak1902();
			}

			MYLOG("APP", "Packetsize %d", g_solution_data.getSize());
			if (g_lorawan_settings.lorawan_enable)
			{
				lmh_error_status result = send_lora_packet(g_solution_data.getBuffer(), g_solution_data.getSize());
				switch (result)
				{
				case LMH_SUCCESS:
					if (found_sensors[OLED_ID].found_sensor)
					{
						if (found_sensors[RTC_ID].found_sensor)
						{
							read_rak12002();
							snprintf(disp_txt, 64, "%d:%02d Pkg %d b", g_date_time.hour, g_date_time.minute, g_solution_data.getSize());
						}
						else
						{
							snprintf(disp_txt, 64, "Packet sent %d b", g_solution_data.getSize());
						}
						rak1921_add_line(disp_txt);
					}
					MYLOG("APP", "Packet enqueued");
					break;
				case LMH_BUSY:
					MYLOG("APP", "LoRa transceiver is busy");
					AT_PRINTF("+EVT:BUSY\n");
					break;
				case LMH_ERROR:
					AT_PRINTF("+EVT:SIZE_ERROR\n");
					MYLOG("APP", "Packet error, too big to send with current DR");
					break;
				}
			}
			else
			{
				// Send packet over LoRa
				if (send_p2p_packet(g_solution_data.getBuffer(), g_solution_data.getSize()))
				{
					if (found_sensors[OLED_ID].found_sensor)
					{
						if (found_sensors[RTC_ID].found_sensor)
						{
							read_rak12002();
							snprintf(disp_txt, 64, "%d:%02d Pkg %d b", g_date_time.hour, g_date_time.minute, g_solution_data.getSize());
						}
						else
						{
							snprintf(disp_txt, 64, "Packet sent %d b", g_solution_data.getSize());
						}
						rak1921_add_line(disp_txt);
					}
					MYLOG("APP", "Packet enqueued");
				}
				else
				{
					AT_PRINTF("+EVT:SIZE_ERROR\n");
					MYLOG("APP", "Packet too big");
				}
			}
			// Reset the packet
			g_solution_data.reset();
		}
	}

	// VOC read request event
	if ((g_task_event_type & VOC_REQ) == VOC_REQ)
	{
		g_task_event_type &= N_VOC_REQ;

		do_read_rak12047();
	}

	// ACC trigger event
	if ((g_task_event_type & MOTION_TRIGGER) == MOTION_TRIGGER)
	{
		g_task_event_type &= N_MOTION_TRIGGER;

		if ((g_task_event_type & TOUCH_EVENT) == TOUCH_EVENT)
		{
			g_task_event_type &= N_TOUCH_EVENT;
			if (found_sensors[TOUCH_ID].found_sensor)
			{
				MYLOG("APP", "TOUCH triggered");
				read_rak14002();
			}
		}
		else
		{
			if (found_sensors[ACC_ID].found_sensor)
			{
				MYLOG("APP", "ACC triggered");
				clear_int_rak1904();
			}
			if (found_sensors[GYRO_ID].found_sensor)
			{
				MYLOG("APP", "Gyro triggered");
				clear_int_rak12025();
			}
			if (found_sensors[MPU_ID].found_sensor)
			{
				MYLOG("APP", "MPU triggered");
				clear_int_rak1905();
			}
			if (found_sensors[ACC2_ID].found_sensor)
			{
				MYLOG("APP", "ACC triggered");
				clear_int_rak12032();
			}
			if (found_sensors[DOF_ID].found_sensor)
			{
				MYLOG("APP", "9DOF triggered");
				clear_int_rak12034();
			}
			if (found_sensors[GESTURE_ID].found_sensor)
			{
				MYLOG("APP", "Gesture triggered");
				read_rak14008();
			}

			// If BLE is enabled, restart Advertising
			if (g_enable_ble)
			{
				restart_advertising(15);
			}

			// If it is the soil moisture sensor, just switch on BLE and do nothing else
			if (found_sensors[SOIL_ID].found_sensor)
			{
				char buffer[64] = {0};
				itoa(g_task_event_type, buffer, 2);
				// MYLOG("APP", "Leaving app handler - Wakeup Flag %s", buffer);
				return;
			}
		}

		MYLOG("APP", "Check send time delay");
		// Check if new data can be sent
		if (g_lpwan_has_joined)
		{
			// Check time since last send
			bool send_now = true;
			if (g_lorawan_settings.send_repeat_time != 0)
			{
				if ((millis() - last_pos_send) < min_delay)
				{
					send_now = false;
					if (!delayed_active)
					{
						delayed_sending.stop();
						MYLOG("APP", "Expired time %d", (int)(millis() - last_pos_send));
						MYLOG("APP", "Max delay time %d", (int)min_delay);
						time_t wait_time = abs(min_delay - (millis() - last_pos_send) >= 0) ? (min_delay - (millis() - last_pos_send)) : min_delay;
						MYLOG("APP", "Wait time %ld", (long)wait_time);

						MYLOG("APP", "Only %lds since last position message, send delayed in %lds", (long)((millis() - last_pos_send) / 1000), (long)(wait_time / 1000));
						delayed_sending.setPeriod(wait_time);
						delayed_sending.start();
						delayed_active = true;
					}
				}
			}
			if (send_now)
			{
				MYLOG("APP", "Send now");
				// Remember last send time
				last_pos_send = millis();

				// Trigger a data reading and packet sending
				g_task_event_type |= STATUS;
			}
			else
			{
				MYLOG("APP", "Send delayed");
			}

			// Reset the standard timer
			if (g_lorawan_settings.send_repeat_time != 0)
			{
				MYLOG("APP", "Timer restarted");
				api_timer_restart(g_lorawan_settings.send_repeat_time);
			}
		}
	}

	// GNSS location search finished
	if ((g_task_event_type & GNSS_FIN) == GNSS_FIN)
	{
		g_task_event_type &= N_GNSS_FIN;

		if (!g_is_helium && found_sensors[ENV_ID].found_sensor)
		{
			// Get Environment data
			read_rak1906();

			// Get battery level
			float batt_level_f = read_batt();
			g_solution_data.addVoltage(LPP_CHANNEL_BATT, batt_level_f / 1000.0);
		}
		// Remember last time sending
		last_pos_send = millis();
		// Just in case
		delayed_active = false;

#if MY_DEBUG == 1
		uint8_t *packet_buff = g_solution_data.getBuffer();
		char ble_out[256] = {0};
		for (int idx = 0; idx < g_solution_data.getSize(); idx++)
		{
			// Serial.printf("%02X", packet_buff[idx]);
			sprintf(&ble_out[idx * 2], "%02X", packet_buff[idx]);
		}
		// Serial.println("");
		// Serial.printf("Packetsize %d\n", g_solution_data.getSize());
		MYLOG("APP", "Size %d - Pckg: %s", g_solution_data.getSize(), ble_out);
#endif

		if (g_lorawan_settings.lorawan_enable)
		{
			lmh_error_status result = send_lora_packet(g_solution_data.getBuffer(), g_solution_data.getSize());
			switch (result)
			{
			case LMH_SUCCESS:
				MYLOG("APP", "Packet enqueued");
				break;
			case LMH_BUSY:
				MYLOG("APP", "LoRa transceiver is busy");
				AT_PRINTF("+EVT:BUSY\n");
				break;
			case LMH_ERROR:
				AT_PRINTF("+EVT:SIZE_ERROR\n");
				MYLOG("APP", "Packet error, too big to send with current DR");
				break;
			}
		}
		else
		{
			// Send packet over LoRa
			if (send_p2p_packet(g_solution_data.getBuffer(), g_solution_data.getSize()))
			{
				MYLOG("APP", "Packet enqueued");
			}
			else
			{
				AT_PRINTF("+EVT:SIZE_ERROR\n");
				MYLOG("APP", "Packet too big");
			}
		}
		// Reset the packet
		g_solution_data.reset();
	}

	{
		char buffer[64] = {0};
		itoa(g_task_event_type, buffer, 2);
		// MYLOG("APP", "Leaving app handler - Wakeup Flag %s", buffer);
	}
}

/**
 * @brief Handle BLE UART data
 *
 */
void ble_data_handler(void)
{
	if (g_enable_ble)
	{
		// BLE UART data handling
		if ((g_task_event_type & BLE_DATA) == BLE_DATA)
		{
			MYLOG("AT", "RECEIVED BLE");
			/** BLE UART data arrived */
			g_task_event_type &= N_BLE_DATA;

			while (g_ble_uart.available() > 0)
			{
				at_serial_input(uint8_t(g_ble_uart.read()));
				delay(5);
			}
			at_serial_input(uint8_t('\n'));
		}
	}
}

/**
 * @brief Handle received LoRa Data
 *
 */
void lora_data_handler(void)
{

	// LoRa Join finished handling
	if ((g_task_event_type & LORA_JOIN_FIN) == LORA_JOIN_FIN)
	{
		g_task_event_type &= N_LORA_JOIN_FIN;
		if (g_join_result)
		{
			if (found_sensors[OLED_ID].found_sensor)
			{
				if (found_sensors[RTC_ID].found_sensor)
				{
					read_rak12002();
					snprintf(disp_txt, 64, "%d:%02d Join success", g_date_time.hour, g_date_time.minute);
				}
				else
				{
					snprintf(disp_txt, 64, "Join success");
				}
				rak1921_add_line(disp_txt);
			}
			MYLOG("APP", "Successfully joined network");
			AT_PRINTF("+EVT:JOINED\n");
			last_pos_send = millis();

			// // Add Multicast support
			// test_multicast.Address = _mc_devaddr;
			// memcpy(test_multicast.NwkSKey, _mc_nwskey, 16);
			// memcpy(test_multicast.AppSKey, _mc_appskey, 16);
			// LoRaMacStatus_t result = LoRaMacMulticastChannelLink(&test_multicast);
			// MYLOG("APP", "MC setup result = %d", result);
		}
		else
		{
			MYLOG("APP", "Join network failed");
			AT_PRINTF("+EVT:JOIN FAILED\n");
			/// \todo here join could be restarted.
			lmh_join();

			// If BLE is enabled, restart Advertising
			if (g_enable_ble)
			{
				restart_advertising(15);
			}
		}
	}

	// LoRa TX finished handling
	if ((g_task_event_type & LORA_TX_FIN) == LORA_TX_FIN)
	{
		g_task_event_type &= N_LORA_TX_FIN;

		MYLOG("APP", "LoRa TX cycle %s", g_rx_fin_result ? "finished ACK" : "failed NAK");

		if ((g_lorawan_settings.confirmed_msg_enabled) && (g_lorawan_settings.lorawan_enable))
		{
			AT_PRINTF("+EVT:SEND CONFIRMED %s\n", g_rx_fin_result ? "SUCCESS" : "FAIL");
		}
		else
		{
			AT_PRINTF("+EVT:SEND OK\n");
		}

		if (!g_rx_fin_result)
		{
			// Increase fail send counter
			send_fail++;

			if (send_fail == 10)
			{
				// Too many failed sendings, reset node and try to rejoin
				delay(100);
				api_reset();
			}
		}
	}

	// LoRa data handling
	if ((g_task_event_type & LORA_DATA) == LORA_DATA)
	{
		/**************************************************************/
		/**************************************************************/
		/// \todo LoRa data arrived
		/// \todo parse them here
		/**************************************************************/
		/**************************************************************/
		g_task_event_type &= N_LORA_DATA;
		MYLOG("APP", "Received package over LoRa");

		if (g_lorawan_settings.lorawan_enable)
		{
			AT_PRINTF("+EVT:RX_1, RSSI %d, SNR %d\n", g_last_rssi, g_last_snr);
			AT_PRINTF("+EVT:%d:", g_last_fport);
			for (int idx = 0; idx < g_rx_data_len; idx++)
			{
				AT_PRINTF("%02X", g_rx_lora_data[idx]);
			}
			AT_PRINTF("\n");
		}
		else
		{
			AT_PRINTF("+EVT:RXP2P, RSSI %d, SNR %d\n", g_last_rssi, g_last_snr);
			AT_PRINTF("+EVT:");
			for (int idx = 0; idx < g_rx_data_len; idx++)
			{
				AT_PRINTF("%02X", g_rx_lora_data[idx]);
			}
			AT_PRINTF("\n");
		}
	}
}

/**
 * @brief Timer function used to avoid sending packages too often.
 * 			Delays the next package by 10 seconds
 *
 * @param unused
 * 			Timer handle, not used
 */
void send_delayed(TimerHandle_t unused)
{
	api_wake_loop(STATUS);
}
