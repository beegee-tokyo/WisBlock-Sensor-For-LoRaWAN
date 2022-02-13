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
SoftwareTimer delayed_sending;

/** Flag if delayed sending is already activated */
bool delayed_active = false;

/** Minimum delay between sending new locations, set to 45 seconds */
time_t min_delay = 45000;

/** GPS precision */
bool g_gps_prec_6 = true;

/** Switch between Cayenne LPP and Helium Mapper data packet */
bool g_is_helium = false;

// Forward declaration
void send_delayed(TimerHandle_t unused);
void at_settings(void);

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
	pinMode(EN_PIN, OUTPUT);
	digitalWrite(EN_PIN, HIGH); // power on RAK12004

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

	// Get precision settings
	read_gps_settings();

	AT_PRINTF("============================\n");
	if (has_soil)
	{
		AT_PRINTF("Soil Moisture Solution\n");
	}
	else if (has_gnss)
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
	else if (has_rak1906)
	{
		AT_PRINTF("LPWAN Environment Solution\n");
	}
	else if (has_rak1902)
	{
		AT_PRINTF("LPWAN Weather Sensor\n");
	}
	else if (has_rak12047)
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
	at_settings();

	// Announce found modules with +EVT: over Serial
	announce_modules();

	AT_PRINTF("============================\n");

	Serial.flush();
	// Reset the packet
	g_solution_data.reset();

	return init_result;
}

/**
 * @brief Application specific event handler
 *        Requires as minimum the handling of STATUS event
 *        Here you handle as well your application specific events
 */
void app_event_handler(void)
{
	// Timer triggered event
	if ((g_task_event_type & STATUS) == STATUS)
	{
		if (has_rak1906 && !g_is_helium)
		{
			// Startup the BME680
			start_rak1906();
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
			if (has_gnss)
			{
				// Start the GNSS location tracking
				xSemaphoreGive(g_gnss_sem);
			}
		}

		// Get battery level
		float batt_level_f = read_batt();
		g_solution_data.addVoltage(LPP_CHANNEL_BATT, batt_level_f / 1000.0);

		// Protection against battery drain
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

		if (!has_gnss)
		{
			if (has_rak1906)
			{
				// Read environment data
				read_rak1906();
			}

			MYLOG("APP", "Packetsize %d", g_solution_data.getSize());
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
	}

	// VOC read request event
	if ((g_task_event_type & VOC_REQ) == VOC_REQ)
	{
		g_task_event_type &= N_VOC_REQ;

		do_read_rak12047();
	}

	// ACC trigger event
	if ((g_task_event_type & ACC_TRIGGER) == ACC_TRIGGER)
	{
		g_task_event_type &= N_ACC_TRIGGER;
		MYLOG("APP", "ACC triggered");
		clear_int_rak1904();

		if (has_soil && g_enable_ble)
		{
			// If BLE is enabled, restart Advertising
			restart_advertising(15);
			return;
		}

		if (has_gnss)
		{
			// If GNSS solution, check if new location data can be sent
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
					// Remember last send time
					last_pos_send = millis();

					// Trigger a GNSS reading and packet sending
					g_task_event_type |= STATUS;
				}

				// Reset the standard timer
				if (g_lorawan_settings.send_repeat_time != 0)
				{
					g_task_wakeup_timer.reset();
				}
			}
		}
	}

	// GNSS location search finished
	if ((g_task_event_type & GNSS_FIN) == GNSS_FIN)
	{
		g_task_event_type &= N_GNSS_FIN;

		if (!g_is_helium)
		{ 
			// Get Environment data
			read_rak1906();
		}
		// Remember last time sending
		last_pos_send = millis();
		// Just in case
		delayed_active = false;

#if MY_DEBUG == 1
		uint8_t *packet_buff = g_solution_data.getBuffer();
		for (int idx = 0; idx < g_solution_data.getSize(); idx++)
		{
			Serial.printf("%02X", packet_buff[idx]);
		}
		Serial.println("");
		Serial.printf("Packetsize %d\n", g_solution_data.getSize());
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
			MYLOG("APP", "Successfully joined network");
			AT_PRINTF("+EVT:JOINED\n");
			last_pos_send = millis();
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

		MYLOG("APP", "LPWAN TX cycle %s", g_rx_fin_result ? "finished ACK" : "failed NAK");

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
