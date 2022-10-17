/**
 * @file app.cpp
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

#include "app.h"

/** Timer since last position message was sent */
time_t last_pos_send = 0;
/** Timer for delayed sending to keep duty cycle */
#ifdef NRF52_SERIES
SoftwareTimer delayed_sending;
#endif
#ifdef ESP32
Ticker delayed_sending;
#endif
#ifdef ARDUINO_ARCH_RP2040
mbed::Ticker delayed_sending;
#endif

/** Flag if delayed sending is already activated */
bool delayed_active = false;

/** Minimum delay between sending new locations, set to 30 seconds */
time_t min_delay = 30000;

/** GPS precision */
bool g_gps_prec_6 = true;

/** Switch between Cayenne LPP and Helium Mapper data packet */
bool g_is_helium = false;

/** Switch to Field Tester data packet */
bool g_is_tester = false;

/** Switch to enable/disable GNSS module power */
bool g_gnss_power_off = false;

/** Flag for battery protection enabled */
bool battery_check_enabled = false;

/** Set the device name, max length is 10 characters */
char g_ble_dev_name[10] = "RAK-SENS";

/** Send Fail counter **/
uint8_t join_send_fail = 0;

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

#if HAS_EPD > 0
	MYLOG("APP", "Init RAK14000");
	init_rak14000();
#endif

	delay(500);

	// Scan the I2C interfaces for devices
	find_modules();

	// Initialize the User AT command list
	init_user_at();

#if defined NRF52_SERIES || defined ESP32
#ifdef BLE_OFF
	// Enable BLE
	g_enable_ble = false;
#else
	// Enable BLE
	g_enable_ble = true;
#endif
#endif
}

/**
 * @brief Application specific initializations
 *
 * @return true Initialization success
 * @return false Initialization failure
 */
bool init_app(void)
{
	/** Set permanent RX mode for LoRa P2P */
	g_lora_p2p_rx_mode = RX_MODE_RX;

	MYLOG("APP", "init_app");

	api_set_version(SW_VERSION_1, SW_VERSION_2, SW_VERSION_3);

	// Get the battery check setting
	read_batt_settings();

	if (found_sensors[GNSS_ID].found_sensor)
	{
		// Get precision settings
		read_gps_settings(0);

		// Get GNSS power settings
		read_gps_settings(1);

		if (g_is_tester)
		{
			g_lorawan_settings.app_port = 1;
		}
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
		else if (g_is_tester)
		{
			AT_PRINTF("LPWAN Tester Solution\n");
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
	// Handle wake up call
	if ((g_task_event_type & AT_CMD) == AT_CMD)
	{
		if (g_device_sleep)
		{
			at_wake();
		}
		return;
	}

	// Timer triggered event
	if ((g_task_event_type & STATUS) == STATUS)
	{
		g_task_event_type &= N_STATUS;
		MYLOG("APP", "Timer wakeup");

		if (found_sensors[ENV_ID].found_sensor && !g_is_helium && !g_is_tester)
		{
			// Startup the BME680
			start_rak1906();
		}
		if (found_sensors[PRESS_ID].found_sensor && !g_is_helium && !g_is_tester)
		{
			// Startup the LPS22HB
			start_rak1902();
		}

#if defined NRF52_SERIES || defined ESP32
		// If BLE is enabled, restart Advertising
		if (g_enable_ble)
		{
			restart_advertising(15);
		}
#endif

		// Reset the packet
		g_solution_data.reset();

		if (!low_batt_protection)
		{
			if (!g_is_helium && !g_is_tester)
			{
				// Get values from the connected modules
				get_sensor_values();
			}
			if (found_sensors[GNSS_ID].found_sensor)
			{
				MYLOG("APP", "Start GNSS");
				// Set activity flag
				gnss_active = true;
				// Start the GNSS location tracking
#if defined NRF52_SERIES || defined ESP32
				xSemaphoreGive(g_gnss_sem);
#endif
#ifdef ARDUINO_ARCH_RP2040
				if (gnss_task_id != NULL)
				{
					osSignalSet(gnss_task_id, 0x1);
				}
#endif
			}
		}

		// Get battery level
		float batt_level_f = read_batt();
		g_solution_data.addVoltage(LPP_CHANNEL_BATT, batt_level_f / 1000.0);

		if ((found_sensors[OLED_ID].found_sensor) && !g_is_tester)
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
			// Get data from the slower sensors
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
			// Read the CO2 sensor last, it needs temperature and humidity values first
			if (found_sensors[SCT31_ID].found_sensor)
			{
				// Read CO2 data
				read_rak12008();
			}

			if (found_sensors[SEISM_ID].found_sensor)
			{
				if ((earthquake_end) && !(g_task_event_type & SEISMIC_EVENT) && !(g_task_event_type & SEISMIC_ALERT))
				{
					g_solution_data.addPresence(LPP_CHANNEL_EQ_EVENT, false);
				}
			}
			// Handle Seismic Events
			if ((g_task_event_type & SEISMIC_EVENT) == SEISMIC_EVENT)
			{
				MYLOG("APP", "Earthquake event");
				g_task_event_type &= N_SEISMIC_EVENT;
				switch (check_event_rak12027(false))
				{
				case 4:
					// Earthquake start
					MYLOG("APP", "Earthquake start alert!");
					read_rak12027(false);
					earthquake_end = false;
					g_solution_data.addPresence(LPP_CHANNEL_EQ_EVENT, true);
					break;
				case 5:
					// Earthquake end
					MYLOG("APP", "Earthquake end alert!");
					read_rak12027(true);
					earthquake_end = true;
					g_solution_data.addPresence(LPP_CHANNEL_EQ_SHUTOFF, shutoff_alert);
					shutoff_alert = false;

					g_solution_data.addPresence(LPP_CHANNEL_EQ_COLLAPSE, collapse_alert);
					collapse_alert = false;

					// Reset flags
					shutoff_alert = false;
					collapse_alert = false;
					// Send another packet in 30 seconds
#ifdef NRF52_SERIES
					delayed_sending.setPeriod(30000);
					delayed_sending.start();
#endif
#ifdef ESP32
					delayed_sending.attach_ms(2000, send_delayed);

#endif
					break;
				default:
					// False alert
					MYLOG("APP", "Earthquake false alert!");
					return;
					break;
				}
			}

			if ((g_task_event_type & SEISMIC_ALERT) == SEISMIC_ALERT)
			{
				g_task_event_type &= N_SEISMIC_ALERT;
				switch (check_event_rak12027(true))
				{
				case 1:
					// Collapse alert
					collapse_alert = true;
					MYLOG("APP", "Earthquake collapse alert!");
					break;
				case 2:
					// ShutDown alert
					shutoff_alert = true;
					MYLOG("APP", "Earthquake shutoff alert!");
					break;
				case 3:
					// Collapse & ShutDown alert
					collapse_alert = true;
					shutoff_alert = true;
					MYLOG("APP", "Earthquake collapse & shutoff alert!");
					break;
				default:
					// False alert
					MYLOG("APP", "Earthquake false alert!");
					break;
				}
				return;
			}

			MYLOG("APP", "Packetsize %d", g_solution_data.getSize());
			if (g_lorawan_settings.lorawan_enable)
			{
				lmh_error_status result = send_lora_packet(g_solution_data.getBuffer(), g_solution_data.getSize());
				switch (result)
				{
				case LMH_SUCCESS:
					if ((found_sensors[OLED_ID].found_sensor) && !g_is_tester)
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

#if HAS_EPD > 0
			// Refresh display
			MYLOG("APP", "Refresh RAK14000");
			wake_rak14000();
			// refresh_rak14000();
#endif
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
				MYLOG("APP", "RAK14002 triggered");
				read_rak14002();
			}
		}
		else
		{
			if (found_sensors[ACC_ID].found_sensor)
			{
				MYLOG("APP", "RAK1904 triggered");
				clear_int_rak1904();
			}
			if (found_sensors[GYRO_ID].found_sensor)
			{
				MYLOG("APP", "RAK12025 triggered");
				clear_int_rak12025();
			}
			if (found_sensors[MPU_ID].found_sensor)
			{
				MYLOG("APP", "RAK1905 triggered");
				clear_int_rak1905();
			}
			if (found_sensors[ACC2_ID].found_sensor)
			{
				MYLOG("APP", "RAK12032 triggered");
				clear_int_rak12032();
			}
			if (found_sensors[DOF_ID].found_sensor)
			{
				MYLOG("APP", "RAK12034 triggered");
				clear_int_rak12034();
			}
			if (found_sensors[GESTURE_ID].found_sensor)
			{
				MYLOG("APP", "RAK14008 triggered");
				read_rak14008();
			}

			if (gnss_active)
			{
				// GNSS is already running
				return;
			}

#if defined NRF52_SERIES || defined ESP32
			// If BLE is enabled, restart Advertising
			if (g_enable_ble)
			{
				restart_advertising(15);
			}
#endif

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
#ifdef NRF52_SERIES
						delayed_sending.stop();
#endif
#ifdef ESP32
						delayed_sending.detach();
#endif
						MYLOG("APP", "Expired time %d", (int)(millis() - last_pos_send));
						MYLOG("APP", "Max delay time %d", (int)min_delay);
						time_t wait_time = abs(min_delay - (millis() - last_pos_send) >= 0) ? (min_delay - (millis() - last_pos_send)) : min_delay;
						MYLOG("APP", "Wait time %ld", (long)wait_time);

						MYLOG("APP", "Only %lds since last position message, send delayed in %lds", (long)((millis() - last_pos_send) / 1000), (long)(wait_time / 1000));
#ifdef NRF52_SERIES
						delayed_sending.setPeriod(wait_time);
						delayed_sending.start();
#endif
#ifdef ESP32
						delayed_sending.attach_ms(wait_time, send_delayed);

#endif
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

		if (!g_is_helium && !g_is_tester)
		{
			// Get data from slower sensors
			if (found_sensors[ENV_ID].found_sensor)
			{
				// Get Environment data
				read_rak1906();
			}
			if (found_sensors[PRESS_ID].found_sensor)
			{
				// Get Environment data
				read_rak1902();
			}
			if (found_sensors[SCT31_ID].found_sensor)
			{
				// Read CO2 data
				read_rak12008();
			}

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
			sprintf(&ble_out[idx * 2], "%02X", packet_buff[idx]);
		}
		MYLOG("APP", "Size %d - Pckg: %s", g_solution_data.getSize(), ble_out);
#endif

		if (g_solution_data.getSize() > 0)
		{
			if (g_lorawan_settings.lorawan_enable)
			{
				uint8_t use_port = g_lorawan_settings.app_port;
				if (g_is_tester)
				{
					use_port = 1;
				}
				lmh_error_status result = send_lora_packet(g_solution_data.getBuffer(), g_solution_data.getSize(), use_port);
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
		}

		// Reset activity flag
		gnss_active = true;

		// Reset the packet
		g_solution_data.reset();
	}
}

// ESP32 is handling the received BLE UART data different, this works only for nRF52
#if defined NRF52_SERIES
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

			if (g_device_sleep)
			{
				at_wake();
			}

			while (g_ble_uart.available() > 0)
			{
				at_serial_input(uint8_t(g_ble_uart.read()));
				delay(5);
			}
			at_serial_input(uint8_t('\n'));
		}
	}
}
#endif

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

			// Reset join failed counter
			join_send_fail = 0;

			if (!found_sensors[GNSS_ID].found_sensor)
			{
				// Force a sensor reading in 10 seconds
#ifdef NRF52_SERIES
				delayed_sending.setPeriod(10000);
				delayed_sending.start();
#endif
#ifdef ESP32
				delayed_sending.attach_ms(10000, send_delayed);

#endif
			}
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

#if defined NRF52_SERIES || defined ESP32
			// If BLE is enabled, restart Advertising
			if (g_enable_ble)
			{
				restart_advertising(15);
			}
#endif

			join_send_fail++;
			if (join_send_fail == 10)
			{
				// Too many failed join requests, reset node and try to rejoin
				delay(100);
				api_reset();
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
			join_send_fail++;

			if (join_send_fail == 10)
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
		g_task_event_type &= N_LORA_DATA;
		MYLOG("APP", "Received package over LoRa");
		if (g_is_tester)
		{
			// uint8_t sequence = g_rx_lora_data[0];
			int16_t min_rssi = g_rx_lora_data[1] - 200;
			int16_t max_rssi = g_rx_lora_data[2] - 200;
			int16_t min_distance = g_rx_lora_data[3] * 250;
			int16_t max_distance = g_rx_lora_data[4] * 250;
			int8_t num_gateways = g_rx_lora_data[5];
			AT_PRINTF("+EVT:RX_1, RSSI %d, SNR %d\n", g_last_rssi, g_last_snr);
			AT_PRINTF("+EVT:%d:\n", g_last_fport);
			AT_PRINTF("+EVT:FieldTester %d gateways\n", num_gateways);
			AT_PRINTF("+EVT:RSSI min %d max %d\n", min_rssi, max_rssi);
			AT_PRINTF("+EVT:Distance min %d max %d\n", min_distance, max_distance);

			Serial.printf("+EVT:Distance min %d max %d\n", min_distance, max_distance);
			if (found_sensors[OLED_ID].found_sensor)
			{
				char disp_txt[64] = {0};
				snprintf(disp_txt, 64, "FieldTester %d gateways\n", num_gateways);
				rak1921_add_line(disp_txt);
				snprintf(disp_txt, 64, "RSSI min %d max %d\n", min_rssi, max_rssi);
				rak1921_add_line(disp_txt);
				snprintf(disp_txt, 64, "Distance min %d max %d\n", min_distance, max_distance);
				rak1921_add_line(disp_txt);
				// Get battery level
				float batt_level_f = read_batt();
				snprintf(disp_txt, 64, "Battery %.3fV", batt_level_f / 1000);
				rak1921_add_line(disp_txt);
			}
		}
		else
		{
			// Check if uplink was a send frequency change command
			if ((g_last_fport == 3) && (g_rx_data_len == 6))
			{
				if (g_rx_lora_data[0] == 0xAA)
				{
					if (g_rx_lora_data[1] == 0x55)
					{
						uint32_t new_send_frequency = 0;
						new_send_frequency |= (uint32_t)(g_rx_lora_data[2]) << 24;
						new_send_frequency |= (uint32_t)(g_rx_lora_data[3]) << 16;
						new_send_frequency |= (uint32_t)(g_rx_lora_data[4]) << 8;
						new_send_frequency |= (uint32_t)(g_rx_lora_data[5]);

						MYLOG("APP", "Received new send frequency %ld s\n", new_send_frequency);
						// Save the new send frequency
						g_lorawan_settings.send_repeat_time = new_send_frequency * 1000;

						// Set the timer to the new send frequency
						api_timer_restart(g_lorawan_settings.send_repeat_time);
						// Save the new send frequency
						save_settings();
					}
				}
			}

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
}

/**
 * @brief Timer function used to avoid sending packages too often.
 * 			Delays the next package by 10 seconds
 *
 * @param unused
 * 			Timer handle, not used
 */
#ifdef NRF52_SERIES
void send_delayed(TimerHandle_t unused)
{
	api_wake_loop(STATUS);
	delayed_sending.stop();
}
#elif defined ESP32 || defined ARDUINO_ARCH_RP2040
void send_delayed(void)
{
	api_wake_loop(STATUS);
	delayed_sending.detach();
}
#endif

// #ifdef NRF52_SERIES
// 						delayed_sending.stop();
// #endif
// #ifdef ESP32
// 						delayed_sending.detach();
// #endif
