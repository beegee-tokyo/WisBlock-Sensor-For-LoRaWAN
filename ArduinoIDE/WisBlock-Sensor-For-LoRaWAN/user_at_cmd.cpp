/**
 * @file user_at.cpp
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

/** File to save GPS precision setting */
File gps_file(InternalFS);

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
	{"+GNSS", "Get/Set the GNSS precision and format 0 = 4 digit, 1 = 6 digit, 2 = Helium Mapper", at_query_gnss, at_exec_gnss, NULL},
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
	{"+RTC", "Get/Set RTC time and date", at_query_rtc, at_set_rtc, NULL},
};

/**
 * @brief Get altitude
 * @author kongduino
 *
 * @return int 0
 */
static int at_query_alt()
{
	if (found_sensors[PRESS_ID].found_sensor)
	{
		AT_PRINTF("Altitude RAK1902: %d cm\r\n", get_alt_rak1902());
	}
	if (found_sensors[ENV_ID].found_sensor)
	{
		AT_PRINTF("Altitude RAK1906: %d cm\r\n", get_alt_rak1906());
	}
	return 0;
}

/** Mean Sea Level Pressure */
float at_MSL = 1013.25;

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
	{"+ALT", "Get Altitude", at_query_alt, NULL, NULL},
	{"+MSL", "Set MSL value", NULL, at_set_msl, NULL},
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
	uint16_t required_structure_size = 0;
	// Get required size of structure
	if (found_sensors[SOIL_ID].found_sensor)
	{
		required_structure_size += sizeof(g_user_at_cmd_list_soil);

		MYLOG("AT", "Structure size %d", required_structure_size);
	}
	if (found_sensors[GNSS_ID].found_sensor)
	{
		required_structure_size += sizeof(g_user_at_cmd_list_gps);

		MYLOG("AT", "Structure size %d", required_structure_size);
	}
	if (found_sensors[RTC_ID].found_sensor)
	{
		required_structure_size += sizeof(g_user_at_cmd_list_rtc);

		MYLOG("AT", "Structure size %d", required_structure_size);
	}
	if ((found_sensors[ENV_ID].found_sensor) || (found_sensors[PRESS_ID].found_sensor))
	{
		required_structure_size += sizeof(g_user_at_cmd_list_env);
		MYLOG("AT", "Structure size %d", required_structure_size);
	}

	// Reserve memory for the structure
	g_user_at_cmd_list = (atcmd_t *)malloc(required_structure_size);

	// Add AT commands to structure
	if (found_sensors[SOIL_ID].found_sensor)
	{
		MYLOG("AT", "Adding Soil Sensor user AT commands");
		g_user_at_cmd_num += sizeof(g_user_at_cmd_list_soil) / sizeof(atcmd_t);
		memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_soil, sizeof(g_user_at_cmd_list_soil));
		index_next_cmds += sizeof(g_user_at_cmd_list_soil) / sizeof(atcmd_t);
		MYLOG("AT", "Index after adding soil %d", index_next_cmds);
	}
	if (found_sensors[GNSS_ID].found_sensor)
	{
		MYLOG("AT", "Adding GNSS user AT commands");
		g_user_at_cmd_num += sizeof(g_user_at_cmd_list_gps) / sizeof(atcmd_t);
		memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_gps, sizeof(g_user_at_cmd_list_gps));
		index_next_cmds += sizeof(g_user_at_cmd_list_gps) / sizeof(atcmd_t);
		MYLOG("AT", "Index after adding soil %d", index_next_cmds);
	}
	if (found_sensors[RTC_ID].found_sensor)
	{
		MYLOG("AT", "Adding RTC user AT commands");
		g_user_at_cmd_num += sizeof(g_user_at_cmd_list_rtc) / sizeof(atcmd_t);
		memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_rtc, sizeof(g_user_at_cmd_list_rtc));
		index_next_cmds += sizeof(g_user_at_cmd_list_rtc) / sizeof(atcmd_t);
		MYLOG("AT", "Index after adding soil %d", index_next_cmds);
	}
	if (found_sensors[ENV_ID].found_sensor)
	{
		MYLOG("AT", "Adding ENV user AT commands");
		AT_PRINTF("\nAdding ENV user AT commands\n");
		g_user_at_cmd_num += sizeof(g_user_at_cmd_list_env) / sizeof(atcmd_t);
		memcpy((void *)&g_user_at_cmd_list[index_next_cmds], (void *)g_user_at_cmd_list_env, sizeof(g_user_at_cmd_list_env));
		index_next_cmds += sizeof(g_user_at_cmd_list_env) / sizeof(atcmd_t);
		MYLOG("AT", "Index after adding env %d", index_next_cmds);
	}

#if TEST_ALL_CMDS == 1
	found_sensors[SOIL_ID].found_sensor = _has_rak12035;
	found_sensors[GNSS_ID].found_sensor = _has_rak1910_rak12500;
	found_sensors[RTC_ID].found_sensor = _has_rak12002;
#endif
}
