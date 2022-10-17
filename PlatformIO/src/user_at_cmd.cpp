/**
 * @file user_at_cmd.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Handle user defined AT commands
 * @version 0.3
 * @date 2022-01-29
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "app.h"
#ifdef NRF52_SERIES
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;

/** Filename to save GPS precision setting */
static const char gnss_name[] = "GNSS";

/** Filename to save GPS precision setting */
static const char gnss_power_name[] = "GNSS_2";

/** Filename to save Battery check setting */
static const char batt_name[] = "BATT";

/** File to save GPS precision setting */
File gps_file(InternalFS);

/** File to save GPS shutoff setting */
File gps_pwer_file(InternalFS);

/** File to save battery check status */
File batt_check(InternalFS);
#endif
#ifdef ESP32
#include <Preferences.h>
/** ESP32 preferences */
Preferences esp32_prefs;
#endif

/** Flag for sleep activated */
bool g_device_sleep = false;

/** Structure for NVRAM settings */
struct s_nvram_settings
{
	uint8_t batt_settings = 0;	 // 0 Battery check disabled, 1 Battery enabled
	uint8_t gnss_settings = 0;	 // 0 GNSS precision 4, 1 GNSS precision 6, 2 Helium GNSS format
	uint8_t reserved[255] = {0}; // Reserved for future extensions
};

/** User AT command defined settings */
s_nvram_settings g_nvram_settings;

/*****************************************
 * Query modules AT commands
 *****************************************/

/**
 * @brief Query found modules
 *
 * @return int 0
 */
int at_query_modules(void)
{
	announce_modules();
	return 0;
}

/**
 * @brief List of all available commands with short help and pointer to functions
 *
 */
atcmd_t g_user_at_cmd_list_modules[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	// Module commands
	{"+MOD", "List all connected I2C devices", at_query_modules, NULL, at_query_modules},
};

/*****************************************
 * GNSS AT commands
 *****************************************/

/**
 * @brief Returns in g_at_query_buf the current settings for the GNSS precision
 *
 * @return int always 0
 */
static int at_query_gnss()
{
	if (g_is_helium)
	{
		snprintf(g_at_query_buf, ATQUERY_SIZE, "GPS precision: 2");
	}
	else if (g_is_tester)
	{
		snprintf(g_at_query_buf, ATQUERY_SIZE, "GPS precision: 3");
	}
	else
	{
		snprintf(g_at_query_buf, ATQUERY_SIZE, "GPS precision: %d", g_gps_prec_6 ? 1 : 0);
	}
	return 0;
}

/**
 * @brief Command to set the GNSS precision
 *
 * @param str Either '0' or '1'
 *  '0' sets the precission to 4 digits
 *  '1' sets the precission to 6 digits
 *  '2' sets the dataformat to Helium Mapper
 *  '3' sets the dataformat to LoRaWAN Field Tester
 * @return int 0 if the command was succesfull, 5 if the parameter was wrong
 */
static int at_exec_gnss(char *str)
{
	if (str[0] == '0')
	{
		g_is_helium = false;
		g_gps_prec_6 = false;
		g_is_tester = false;
		save_gps_settings(0);
	}
	else if (str[0] == '1')
	{
		g_is_helium = false;
		g_gps_prec_6 = true;
		g_is_tester = false;
		save_gps_settings(0);
	}
	else if (str[0] == '2')
	{
		g_is_helium = true;
		g_gps_prec_6 = false;
		g_is_tester = false;
		save_gps_settings(0);
	}
	else if (str[0] == '3')
	{
		g_is_helium = false;
		g_gps_prec_6 = false;
		g_is_tester = true;
		save_gps_settings(0);
	}
	else
	{
		return AT_ERRNO_PARA_VAL;
	}
	return 0;
}

/**
 * @brief Read saved setting for precision, packet format and power control
 *
 * @param settings 0 = get precision and packet format
 *                 1 = get power control settings
 */
void read_gps_settings(uint8_t settings) // Read saved setting for precision and packet format
{
	g_gps_prec_6 = false;
	g_is_helium = false;
	g_is_tester = false;
	bool found_prefs = false;
	char data[3] = {'0'};

	if (settings == 0)
	{
#ifdef NRF52_SERIES
		if (InternalFS.exists(gnss_name))
		{
			gps_file.open(gnss_name, FILE_O_READ);
			// int read (void *buf, uint16_t nbyte);
			gps_file.read(data, 1);
			gps_file.close();
			found_prefs = true;
		}
#endif
#ifdef ESP32
		esp32_prefs.begin("gnss", false);
		data[0] = esp32_prefs.getShort("fmt", 0);
		esp32_prefs.end();
#endif
		if (found_prefs)
		{
			MYLOG("USR_AT", "File found, read %c", data[0]);
			if (data[0] == '1')
			{
				g_gps_prec_6 = true;
				MYLOG("USR_AT", "File found, set format to 6 digit");
			}
			else if (data[0] == '2')
			{
				g_is_helium = true;
				MYLOG("USR_AT", "File found, set format to Helium");
			}
			else if (data[0] == '3')
			{
				g_is_tester = true;
				MYLOG("USR_AT", "File found, set format to Tester");
			}
		}
		else
		{
			MYLOG("USR_AT", "File not found, set format to Tester");
			g_is_tester = true;
		}
	}
	else if (settings == 1)
	{
#ifdef NRF52_SERIES
		if (InternalFS.exists(gnss_power_name))
		{
			gps_file.open(gnss_power_name, FILE_O_READ);
			// int read (void *buf, uint16_t nbyte);
			gps_file.read(data, 1);
			gps_file.close();
			found_prefs = true;
		}
#endif
#ifdef ESP32
		esp32_prefs.begin("gnss", false);
		data[0] = esp32_prefs.getShort("pwr", 0);
		esp32_prefs.end();
#endif
		if (found_prefs)
		{
			MYLOG("USR_AT", "File found, read %c", data[0]);
			if (data[0] == '0')
			{
				g_gnss_power_off = true;
				MYLOG("USR_AT", "File found, enable GNSS power off");
			}
			else if (data[0] == '1')
			{
				g_gnss_power_off = false;
				MYLOG("USR_AT", "File found, disable GNSS power off");
			}
		}
		else
		{
			MYLOG("USR_AT", "File not found, enable GNSS power off");
			g_gnss_power_off = false;
		}
	}
}

/**
 * @brief Save the GPS settings
 *
 */
void save_gps_settings(uint8_t settings)
{
	if (settings == 0)
	{
#ifdef NRF52_SERIES
			InternalFS.remove(gnss_name);
		gps_file.open(gnss_name, FILE_O_WRITE);
		if (g_gps_prec_6)
		{
			MYLOG("USR_AT", "Saved high precision");
			gps_file.write("1");
		}
		else if (g_is_helium)
		{
			MYLOG("USR_AT", "Saved Helium format");
			gps_file.write("2");
		}
		else if (g_is_tester)
		{
			MYLOG("USR_AT", "Saved Tester format");
			gps_file.write("3");
		}
		else
		{
			MYLOG("USR_AT", "Saved low precision");
			gps_file.write("0");
		}
		gps_file.close();
#endif
#ifdef ESP32
		esp32_prefs.begin("gnss", false);
		if (g_gps_prec_6)
		{
			MYLOG("USR_AT", "Saved high precision");
			esp32_prefs.putShort("fmt", 1);
		}
		else if (g_is_helium)
		{
			MYLOG("USR_AT", "Saved Helium format");
			esp32_prefs.putShort("fmt", 2);
		}
		else if (g_is_tester)
		{
			MYLOG("USR_AT", "Saved Tester format");
			esp32_prefs.putShort("fmt", 3);
		}
		else
		{
			MYLOG("USR_AT", "Saved low precision");
			esp32_prefs.putShort("fmt", 0);
		}
		esp32_prefs.end();
#endif
	}
	else if (settings == 1)
	{
#ifdef NRF52_SERIES
		InternalFS.remove(gnss_power_name);
		gps_file.open(gnss_power_name, FILE_O_WRITE);
		if (g_gnss_power_off)
		{
			MYLOG("USR_AT", "Saved power off enabled");
			gps_file.write("0");
		}
		else 
		{
			MYLOG("USR_AT", "Saved power off disabled");
			gps_file.write("1");
		}
		gps_file.close();
#endif
#ifdef ESP32
		esp32_prefs.begin("gnss", false);
		if (g_gnss_power_off)
		{
			MYLOG("USR_AT", "Saved power off enabled");
			esp32_prefs.putShort("pwr", 0);
		}
		else
		{
			MYLOG("USR_AT", "Saved power off disabled");
			esp32_prefs.putShort("pwr", 1);
		}
		esp32_prefs.end();
#endif
	}
}

/**
 * @brief Goto sleep
 *
 * @return int 0
 */
static int at_sleep(void)
{
	// Switch off module power
	digitalWrite(WB_IO2, LOW);

	// Cancel automatic sending
	api_timer_stop();

	// Put radio into sleep
	Radio.Sleep();

	// set sleep flag
	g_device_sleep = true;
	return 0;
}

/**
 * @brief Wakeup
 *
 * @return int 0
 */
int at_wake(void)
{
	MYLOG("USR_AT", "Wakeup");
	// Switch off module power
	digitalWrite(WB_IO2, HIGH);

	// Cancel automatic sending
	api_timer_restart(g_lorawan_settings.send_repeat_time);

	if (found_sensors[GNSS_ID].found_sensor)
	{
		init_gnss();
	}

	// remove sleep flag
	g_device_sleep = false;
	return 0;
}

static int at_query_shutoff()
{
	if (g_gnss_power_off)
	{
		snprintf(g_at_query_buf, ATQUERY_SIZE, "GNSS power off enabled");
	}
	else 
	{
		snprintf(g_at_query_buf, ATQUERY_SIZE, "GNSS power off disabled");
	}
	return 0;
}

static int at_exec_shutoff(char *str)
{
	if (str[0] == '0')
	{
		g_gnss_power_off = true;
		save_gps_settings(1);
	}
	else if (str[0] == '1')
	{
		g_gnss_power_off = false;
		save_gps_settings(1);
	}
	else
	{
		return AT_ERRNO_PARA_VAL;
	}
	return 0;
}

/**
 * @brief List of all available commands with short help and pointer to functions
 *
 */
atcmd_t g_user_at_cmd_list_gps[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	// GNSS commands
	{"+GNSS", "Get/Set the GNSS precision and format 0 = 4 digit, 1 = 6 digit, 2 = Helium Mapper, 3 = Field Tester", at_query_gnss, at_exec_gnss, at_query_gnss},
	{"+GNSSSLEEP", "Enable/Disable GNSS module power off 0 = power off, 1 = keep power on", at_query_shutoff, at_exec_shutoff, at_query_shutoff},
	{"+SLEEP", "Put device into sleep", NULL, NULL, at_sleep},
};

/*****************************************
 * Soil moisture sensor AT commands
 *****************************************/

/**
 * @brief Start dry calibration
 *
 * @return int 0
 */
static int at_exec_dry()
{
	// Dry calibration requested
	AT_PRINTF("Start Dry Calibration\n");
	uint16_t new_val = start_calib_rak12035(true);
	if (new_val == 0xFFFF)
	{
		AT_PRINTF("Calibration failed, please try again");
	}
	else
	{
		AT_PRINTF("New Dry Calibration Value: %d", new_val);
	}
	return 0;
}

static int at_set_dry(char *str)
{
	long dry_val = strtol(str, NULL, 0);
	if ((dry_val < 0) || (dry_val > 1000))
	{
		return AT_ERRNO_PARA_VAL;
	}
	set_calib_rak12035(true, dry_val);
	return 0;
}

/**
 * @brief Query dry calibration value
 *
 * @return int 0
 */
static int at_query_dry()
{
	// Dry calibration value query
	AT_PRINTF("Dry Calibration Value: %d", get_calib_rak12035(true));
	return 0;
}

/**
 * @brief Start wet calibration
 *
 * @return int 0
 */
static int at_exec_wet()
{
	// Dry calibration requested
	AT_PRINTF("Start Wet Calibration\n");
	uint16_t new_val = start_calib_rak12035(false);
	if (new_val == 0xFFFF)
	{
		AT_PRINTF("Calibration failed, please try again");
	}
	else
	{
		AT_PRINTF("New Wet Calibration Value: %d", new_val);
	}
	return 0;
}

static int at_set_wet(char *str)
{
	long wet_val = strtol(str, NULL, 0);
	if ((wet_val < 0) || (wet_val > 1000))
	{
		return AT_ERRNO_PARA_VAL;
	}
	set_calib_rak12035(false, wet_val);
	return 0;
}

/**
 * @brief Query wet calibration value
 *
 * @return int 0
 */
static int at_query_wet(void)
{
	// Wet calibration value query
	AT_PRINTF("Wet Calibration Value: %d", get_calib_rak12035(false));
	return 0;
}

atcmd_t g_user_at_cmd_list_soil[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	// Soil Sensor commands
	{"+DRY", "Get/Set dry calibration value", at_query_dry, at_set_dry, at_exec_dry},
	{"+WET", "Get/Set wet calibration value", at_query_wet, at_set_wet, at_exec_wet},
};

/*****************************************
 * RTC AT commands
 *****************************************/

/**
 * @brief Set RTC time
 *
 * @param str time as string, format <year>:<month>:<date>:<hour>:<minute>
 * @return int 0 if successful, otherwise error value
 */
static int at_set_rtc(char *str)
{
	uint16_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hour;
	uint8_t minute;

	char *param;

	param = strtok(str, ":");

	// year:month:date:hour:minute

	if (param != NULL)
	{
		/* Check year */
		year = strtoul(param, NULL, 0);

		if (year > 3000)
		{
			return AT_ERRNO_PARA_VAL;
		}

		/* Check month */
		param = strtok(NULL, ":");
		if (param != NULL)
		{
			month = strtoul(param, NULL, 0);

			if ((month < 1) || (month > 12))
			{
				return AT_ERRNO_PARA_VAL;
			}

			// Check day
			param = strtok(NULL, ":");
			if (param != NULL)
			{
				date = strtoul(param, NULL, 0);

				if ((date < 1) || (date > 31))
				{
					return AT_ERRNO_PARA_VAL;
				}

				// Check hour
				param = strtok(NULL, ":");
				if (param != NULL)
				{
					hour = strtoul(param, NULL, 0);

					if (hour > 24)
					{
						return AT_ERRNO_PARA_VAL;
					}

					// Check minute
					param = strtok(NULL, ":");
					if (param != NULL)
					{
						minute = strtoul(param, NULL, 0);

						if (minute > 59)
						{
							return AT_ERRNO_PARA_VAL;
						}

						set_rak12002(year, month, date, hour, minute);

						return 0;
					}
				}
			}
		}
	}
	return AT_ERRNO_PARA_NUM;
}

/**
 * @brief Get RTC time
 *
 * @return int 0
 */
static int at_query_rtc(void)
{
	// Get date/time from the RTC
	read_rak12002();
	AT_PRINTF("%d.%02d.%02d %d:%02d:%02d", g_date_time.year, g_date_time.month, g_date_time.date, g_date_time.hour, g_date_time.minute, g_date_time.second);
	return 0;
}

atcmd_t g_user_at_cmd_list_rtc[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	// RTC commands
	{"+RTC", "Get/Set RTC time and date", at_query_rtc, at_set_rtc, at_query_rtc},
};

/*****************************************
 * Altitude AT commands
 *****************************************/

/**
 * @brief Get altitude
 * @author kongduino
 *
 * @return int 0
 */
static int at_query_alt()
{
	uint16_t result;
	if (found_sensors[PRESS_ID].found_sensor)
	{
		result = get_alt_rak1902();
		if (result == 0xFFFF)
		{
			return AT_ERRNO_EXEC_FAIL;
		}
		AT_PRINTF("Altitude RAK1902: %d cm\r\n", result);
	}
	if (found_sensors[ENV_ID].found_sensor)
	{
		result = get_alt_rak1906();
		if (result == 0xFFFF)
		{
			return AT_ERRNO_EXEC_FAIL;
		}
		AT_PRINTF("Altitude RAK1906: %d cm\r\n", result);
	}
	return 0;
}

/** Mean Sea Level Pressure */
float at_MSL = 1013.25;

/**
 * @brief Query the current MSL value
 *
 * @return int 0
 */
static int at_query_msl()
{
	AT_PRINTF("MSL: %d\r\n", at_MSL * 100);
	return 0;
}

/**
 * @brief Set MSL
 * @author kongduino
 *
 * @return int 0
 */
static int at_set_msl(char *str)
{
	long v = strtol(str, NULL, 0);
	if ((v < 84000) || (v > 105000))
	{
		// in Pa, ie default 101325
		return AT_ERRNO_PARA_VAL;
	}
	at_MSL = v / 100.0;
	return 0;
}

/**
 * @brief Structure for environment AT commands
 * @author kongduino
 *
 */
atcmd_t g_user_at_cmd_list_env[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	// ENV commands
	{"+ALT", "Get Altitude", at_query_alt, NULL, at_query_alt},
	{"+MSL", "Get/Set MSL value", at_query_msl, at_set_msl, at_query_msl},
};

/*****************************************
 * Battery check AT commands
 *****************************************/

/**
 * @brief Enable/Disable battery check
 *
 * @param str
 * @return int
 */
static int at_set_batt_check(char *str)
{
	long check_bat_request = strtol(str, NULL, 0);
	if (check_bat_request == 1)
	{
		battery_check_enabled = true;
		save_batt_settings(battery_check_enabled);
	}
	else if (check_bat_request == 0)
	{
		battery_check_enabled = false;
		save_batt_settings(battery_check_enabled);
	}
	else
	{
		return AT_ERRNO_PARA_VAL;
	}
	return 0;
}

/**
 * @brief Enable/Disable battery check
 *
 * @return int 0
 */
static int at_query_batt_check(void)
{
	// Wet calibration value query
	AT_PRINTF("Battery check is %s", battery_check_enabled ? "enabled" : "disabled");
	return 0;
}

/**
 * @brief Read saved setting for precision and packet format
 *
 */
void read_batt_settings(void)
{
#ifdef NRF52_SERIES
	if (InternalFS.exists(batt_name))
	{
		battery_check_enabled = true;
		MYLOG("USR_AT", "File found, enable battery check");
	}
	else
	{
		battery_check_enabled = false;
		MYLOG("USR_AT", "File not found, disable battery check");
	}
#endif
#ifdef ESP32
	esp32_prefs.begin("bat", false);
	battery_check_enabled = esp32_prefs.getBool("bat", false);
	esp32_prefs.end();
#endif

	save_batt_settings(battery_check_enabled);
}

/**
 * @brief Save the GPS settings
 *
 */
void save_batt_settings(bool check_batt_enables)
{
#ifdef NRF52_SERIES
	if (check_batt_enables)
	{
		batt_check.open(batt_name, FILE_O_WRITE);
		batt_check.write("1");
		batt_check.close();
		MYLOG("USR_AT", "Created File for battery protection enabled");
	}
	else
	{
		InternalFS.remove(batt_name);
		MYLOG("USR_AT", "Remove File for battery protection enabled");
	}
#endif
#ifdef ESP32
	esp32_prefs.begin("bat", false);
	esp32_prefs.putBool("bat", battery_check_enabled);
	esp32_prefs.end();
#endif
}

atcmd_t g_user_at_cmd_list_batt[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	// Battery check commands
	{"+BATCHK", "Enable/Disable the battery charge check", at_query_batt_check, at_set_batt_check, at_query_batt_check},
};

/** Number of user defined AT commands */
uint8_t g_user_at_cmd_num = 0;

/** Pointer to the combined user AT command structure */
atcmd_t *g_user_at_cmd_list;

#define TEST_ALL_CMDS 0

/**
 * @brief Initialize the user defined AT command list
 *
 */
void init_user_at(void)
{
#if TEST_ALL_CMDS == 1
	bool _has_rak12035 = found_sensors[SOIL_ID].found_sensor;
	bool _has_rak1910_rak12500 = found_sensors[GNSS_ID].found_sensor;
	bool _has_rak12002 = found_sensors[RTC_ID].found_sensor;
	found_sensors[SOIL_ID].found_sensor = true;
	found_sensors[GNSS_ID].found_sensor = true;
	found_sensors[RTC_ID].found_sensor = true;
#endif

	uint16_t index_next_cmds = 0;
	uint16_t required_structure_size = sizeof(g_user_at_cmd_list_batt);
	MYLOG("USR_AT", "Structure size %d Battery", required_structure_size);
	required_structure_size += sizeof(g_user_at_cmd_list_modules);
	MYLOG("USR_AT", "Structure size %d Modules", required_structure_size);

	// Get required size of structure
	if (found_sensors[SOIL_ID].found_sensor)
	{
		required_structure_size += sizeof(g_user_at_cmd_list_soil);

		MYLOG("USR_AT", "Structure size %d Soil", required_structure_size);
	}
	if (found_sensors[GNSS_ID].found_sensor)
	{
		required_structure_size += sizeof(g_user_at_cmd_list_gps);

		MYLOG("USR_AT", "Structure size %d GNSS", required_structure_size);
	}
	if (found_sensors[RTC_ID].found_sensor)
	{
		required_structure_size += sizeof(g_user_at_cmd_list_rtc);

		MYLOG("USR_AT", "Structure size %d RTC", required_structure_size);
	}
	if ((found_sensors[ENV_ID].found_sensor) || (found_sensors[PRESS_ID].found_sensor))
	{
		required_structure_size += sizeof(g_user_at_cmd_list_env);
		MYLOG("USR_AT", "Structure size %d ENV/Pressure", required_structure_size);
	}

	// Reserve memory for the structure
	g_user_at_cmd_list = (atcmd_t *)malloc(required_structure_size);

	// Add AT commands to structure
	MYLOG("USR_AT", "Adding battery check AT commands");
	g_user_at_cmd_num += sizeof(g_user_at_cmd_list_batt) / sizeof(atcmd_t);
	memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_batt, sizeof(g_user_at_cmd_list_batt));
	index_next_cmds += sizeof(g_user_at_cmd_list_batt) / sizeof(atcmd_t);
	MYLOG("USR_AT", "Index after adding battery check %d", index_next_cmds);

	MYLOG("USR_AT", "Adding module AT commands");
	g_user_at_cmd_num += sizeof(g_user_at_cmd_list_modules) / sizeof(atcmd_t);
	memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_modules, sizeof(g_user_at_cmd_list_modules));
	index_next_cmds += sizeof(g_user_at_cmd_list_modules) / sizeof(atcmd_t);
	MYLOG("USR_AT", "Index after adding modules %d", index_next_cmds);

	if (found_sensors[SOIL_ID].found_sensor)
	{
		MYLOG("USR_AT", "Adding Soil Sensor user AT commands");
		g_user_at_cmd_num += sizeof(g_user_at_cmd_list_soil) / sizeof(atcmd_t);
		memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_soil, sizeof(g_user_at_cmd_list_soil));
		index_next_cmds += sizeof(g_user_at_cmd_list_soil) / sizeof(atcmd_t);
		MYLOG("USR_AT", "Index after adding soil %d", index_next_cmds);
	}
	if (found_sensors[GNSS_ID].found_sensor)
	{
		MYLOG("USR_AT", "Adding GNSS user AT commands");
		g_user_at_cmd_num += sizeof(g_user_at_cmd_list_gps) / sizeof(atcmd_t);
		memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_gps, sizeof(g_user_at_cmd_list_gps));
		index_next_cmds += sizeof(g_user_at_cmd_list_gps) / sizeof(atcmd_t);
		MYLOG("USR_AT", "Index after adding GNSS %d", index_next_cmds);
	}
	if (found_sensors[RTC_ID].found_sensor)
	{
		MYLOG("USR_AT", "Adding RTC user AT commands");
		g_user_at_cmd_num += sizeof(g_user_at_cmd_list_rtc) / sizeof(atcmd_t);
		memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_rtc, sizeof(g_user_at_cmd_list_rtc));
		index_next_cmds += sizeof(g_user_at_cmd_list_rtc) / sizeof(atcmd_t);
		MYLOG("USR_AT", "Index after adding RTC %d", index_next_cmds);
	}
	if ((found_sensors[ENV_ID].found_sensor) || (found_sensors[PRESS_ID].found_sensor))
	{
		MYLOG("USR_AT", "Adding ENV user AT commands");
		g_user_at_cmd_num += sizeof(g_user_at_cmd_list_env) / sizeof(atcmd_t);
		memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_env, sizeof(g_user_at_cmd_list_env));
		index_next_cmds += sizeof(g_user_at_cmd_list_env) / sizeof(atcmd_t);
		MYLOG("USR_AT", "Index after adding env %d", index_next_cmds);
	}

#if TEST_ALL_CMDS == 1
	found_sensors[SOIL_ID].found_sensor = _has_rak12035;
	found_sensors[GNSS_ID].found_sensor = _has_rak1910_rak12500;
	found_sensors[RTC_ID].found_sensor = _has_rak12002;
#endif
}
