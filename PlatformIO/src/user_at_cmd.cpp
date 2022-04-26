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
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>
using namespace Adafruit_LittleFS_Namespace;

/** Filename to save GPS precision setting */
static const char gnss_name[] = "GNSS";

/** Filename to save data format setting */
static const char helium_format[] = "HELIUM";

/** Filename to save GPS precision setting */
static const char batt_name[] = "BATT";

/** File to save GPS precision setting */
File gps_file(InternalFS);

/** File to save battery check status */
File batt_check(InternalFS);

/** Structure for NVRAM settings */
struct s_nvram_settings
{
	uint8_t batt_settings = 0;	 // 0 Battery check disabled, 1 Battery enabled
	uint8_t gnss_settings = 0;	 // 0 GNSS precision 4, 1 GNSS precision 6, 2 Helium GNSS format
	uint8_t reserved[255] = {0}; // Reserved for future extensions
};

/** User AT command defined settings */
s_nvram_settings g_nvram_settings;

#define AT_PRINTF(...)                  \
	Serial.printf(__VA_ARGS__);         \
	if (g_ble_uart_is_connected)        \
	{                                   \
		g_ble_uart.printf(__VA_ARGS__); \
	}

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
 * @return int 0 if the command was succesfull, 5 if the parameter was wrong
 */
static int at_exec_gnss(char *str)
{
	if (str[0] == '0')
	{
		g_is_helium = false;
		g_gps_prec_6 = false;
		save_gps_settings();
	}
	else if (str[0] == '1')
	{
		g_is_helium = false;
		g_gps_prec_6 = true;
		save_gps_settings();
	}
	else if (str[0] == '2')
	{
		g_is_helium = true;
		save_gps_settings();
	}
	else
	{
		return AT_ERRNO_PARA_VAL;
	}
	return 0;
}

/**
 * @brief Read saved setting for precision and packet format
 *
 */
void read_gps_settings(void)
{
	if (g_ext_nvram)
	{
		api_read_ext_nvram(1, (uint8_t *)&g_nvram_settings, sizeof(s_nvram_settings));
		switch (g_nvram_settings.gnss_settings)
		{
		case 0:
			g_gps_prec_6 = false;
			g_is_helium = false;
			MYLOG("USR_AT", "Set GNSS precision to low");
			break;
		case 1:
			g_gps_prec_6 = true;
			g_is_helium = false;
			MYLOG("USR_AT", "Set GNSS precision to high");
			break;
		case 2:
			g_is_helium = true;
			MYLOG("USR_AT", "Set GNSS precision to Helium");
			break;
		default:
			// not initialized, set it all to false
			g_gps_prec_6 = false;
			g_is_helium = false;
			MYLOG("USR_AT", "Error, set GNSS precision to low");
			save_gps_settings();
			break;
		}
		if (g_nvram_settings.gnss_settings == 0)
		{
			g_gps_prec_6 = false;
		}
		return;
	}
	if (InternalFS.exists(gnss_name))
	{
		g_gps_prec_6 = true;
		MYLOG("USR_AT", "File found, set precision to high");
	}
	else
	{
		g_gps_prec_6 = false;
		MYLOG("USR_AT", "File not found, set precision to low");
	}
	if (InternalFS.exists(helium_format))
	{
		g_is_helium = true;
		MYLOG("USR_AT", "File found, set Helium Mapper format");
	}
	else
	{
		g_is_helium = false;
		MYLOG("USR_AT", "File not found, set Cayenne LPP format");
	}
}

/**
 * @brief Save the GPS settings
 *
 */
void save_gps_settings(void)
{
	if (g_ext_nvram)
	{
		if (g_is_helium)
		{
			g_nvram_settings.gnss_settings = 2;
			MYLOG("USR_AT", "Set GNSS Helium format");
		}
		else if (g_gps_prec_6)
		{
			g_nvram_settings.gnss_settings = 1;
			MYLOG("USR_AT", "Set GNSS high precision");
		}
		else
		{
			g_nvram_settings.gnss_settings = 0;
			MYLOG("USR_AT", "Set GNSS low precision");
		}
		api_write_ext_nvram(1, (uint8_t *)&g_nvram_settings, sizeof(s_nvram_settings));
		return;
	}
	if (g_gps_prec_6)
	{
		gps_file.open(gnss_name, FILE_O_WRITE);
		gps_file.write("1");
		gps_file.close();
		MYLOG("USR_AT", "Created File for high precision");
	}
	else
	{
		InternalFS.remove(gnss_name);
		MYLOG("USR_AT", "Remove File for high precision");
	}
	if (g_is_helium)
	{
		gps_file.open(helium_format, FILE_O_WRITE);
		gps_file.write("1");
		gps_file.close();
		MYLOG("USR_AT", "Created File for Helium Mapper format");
	}
	else
	{
		InternalFS.remove(helium_format);
		MYLOG("USR_AT", "Remove File for Helium Mapper format");
	}
}

/**
 * @brief List of all available commands with short help and pointer to functions
 *
 */
atcmd_t g_user_at_cmd_list_gps[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	// GNSS commands
	{"+GNSS", "Get/Set the GNSS precision and format 0 = 4 digit, 1 = 6 digit, 2 = Helium Mapper", at_query_gnss, at_exec_gnss, at_query_gnss},
};

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
	// GNSS commands
	{"+DRY", "Get/Set dry calibration value", at_query_dry, at_set_dry, at_exec_dry},
	{"+WET", "Get/Set wet calibration value", at_query_wet, at_set_wet, at_exec_wet},
};

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
	AT_PRINTF("%d.%d.%d %d:%d:%d", g_date_time.year, g_date_time.month, g_date_time.date, g_date_time.hour, g_date_time.minute, g_date_time.second);
	return 0;
}

atcmd_t g_user_at_cmd_list_rtc[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	// GNSS commands
	{"+RTC", "Get/Set RTC time and date", at_query_rtc, at_set_rtc, at_query_rtc},
};

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
	if (g_ext_nvram)
	{
		api_read_ext_nvram(1, (uint8_t *)&g_nvram_settings, sizeof(s_nvram_settings));
		switch (g_nvram_settings.batt_settings)
		{
		case 0:
			battery_check_enabled = false;
			MYLOG("USR_AT", "Disabled battery check");
			break;
		case 1:
			battery_check_enabled = true;
			MYLOG("USR_AT", "Enabled battery check");
			break;
		default:
			battery_check_enabled = false;
			MYLOG("USR_AT", "Error, disabled battery check");
			save_batt_settings(battery_check_enabled);
			break;
		}
		return;
	}
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
	save_batt_settings(battery_check_enabled);
}

/**
 * @brief Save the GPS settings
 *
 */
void save_batt_settings(bool check_batt_enables)
{
	if (g_ext_nvram)
	{
		if (check_batt_enables)
		{
			g_nvram_settings.batt_settings = 1;
			MYLOG("USR_AT", "Save enabled battery check");
		}
		else
		{
			g_nvram_settings.batt_settings = 0;
			MYLOG("USR_AT", "Save disabled battery check");
		}
		api_write_ext_nvram(1, (uint8_t *)&g_nvram_settings, sizeof(s_nvram_settings));
		return;
	}
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
}

atcmd_t g_user_at_cmd_list_batt[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	// Battery check commands
	{"+BATCHK", "Enable/Disable the battery charge check", at_query_batt_check, at_set_batt_check, at_query_batt_check},
};

static int at_query_nvram(void)
{
	if (g_ext_nvram)
	{
		api_read_ext_nvram(1, (uint8_t *)&g_nvram_settings, sizeof(s_nvram_settings));
		AT_PRINTF("GNSS: %02X Batt Check: %02X",g_nvram_settings.gnss_settings, g_nvram_settings.batt_settings);
		return 0;
	}
	return AT_ERRNO_NOSUPP;
}

atcmd_t g_user_at_cmd_list_nvram[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	// Battery check commands
	{"+NVRAM", "Show content of NVRAM", at_query_nvram, NULL, at_query_nvram},
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

	// Get required size of structure
	if (g_ext_nvram)
	{
		required_structure_size += sizeof(g_user_at_cmd_list_nvram);

		MYLOG("USR_AT", "Structure size %d NVRAM", required_structure_size);
	}
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
	if (g_ext_nvram)
	{
		MYLOG("USR_AT", "Adding NVRAM user AT commands");
		g_user_at_cmd_num += sizeof(g_user_at_cmd_list_nvram) / sizeof(atcmd_t);
		memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_nvram, sizeof(g_user_at_cmd_list_nvram));
		index_next_cmds += sizeof(g_user_at_cmd_list_nvram) / sizeof(atcmd_t);
		MYLOG("USR_AT", "Index after adding NVRAM %d", index_next_cmds);
	}
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
		MYLOG("USR_AT", "Index after adding soil %d", index_next_cmds);
	}
	if (found_sensors[RTC_ID].found_sensor)
	{
		MYLOG("USR_AT", "Adding RTC user AT commands");
		g_user_at_cmd_num += sizeof(g_user_at_cmd_list_rtc) / sizeof(atcmd_t);
		memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_rtc, sizeof(g_user_at_cmd_list_rtc));
		index_next_cmds += sizeof(g_user_at_cmd_list_rtc) / sizeof(atcmd_t);
		MYLOG("USR_AT", "Index after adding soil %d", index_next_cmds);
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
