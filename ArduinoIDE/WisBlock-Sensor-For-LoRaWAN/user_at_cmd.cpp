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
static int at_query_wet()
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

/** Number of user defined AT commands */
uint8_t g_user_at_cmd_num = 0;

atcmd_t g_user_at_cmd_list[] = {
	/*|    CMD    |     AT+CMD?      |    AT+CMD=?    |  AT+CMD=value |  AT+CMD  |*/
	{"+dummy", "No function", NULL, NULL, NULL},
	{"+dummy", "No function", NULL, NULL, NULL},
	{"+dummy", "No function", NULL, NULL, NULL},
	{"+dummy", "No function", NULL, NULL, NULL},
	{"+dummy", "No function", NULL, NULL, NULL},
	{"+dummy", "No function", NULL, NULL, NULL},
	{"+dummy", "No function", NULL, NULL, NULL},
	{"+dummy", "No function", NULL, NULL, NULL},
	{"+dummy", "No function", NULL, NULL, NULL},
	{"+dummy", "No function", NULL, NULL, NULL},
};

void init_user_at(void)
{
	if (has_rak12035)
	{
		MYLOG("AT", "Adding Soil Sensor user AT commands");
		g_user_at_cmd_num = sizeof(g_user_at_cmd_list_soil) / sizeof(atcmd_t);
		memcpy((void *)g_user_at_cmd_list, (void *)g_user_at_cmd_list_soil, sizeof(g_user_at_cmd_list_soil));
	}
	else if (has_rak1910_rak12500)
	{
		MYLOG("AT", "Adding GNSS user AT commands");
		g_user_at_cmd_num = sizeof(g_user_at_cmd_list_gps) / sizeof(atcmd_t);
		memcpy((void *)g_user_at_cmd_list, (void *)g_user_at_cmd_list_gps, sizeof(g_user_at_cmd_list_gps));
	}
	else
	{
		g_user_at_cmd_num = 0;
	}
	if (has_rak12035 && has_rak1910_rak12500)
	{
		MYLOG("AT", "Adding GNSS && Soil Sensor user AT commands");
		g_user_at_cmd_num = sizeof(g_user_at_cmd_list_gps) / sizeof(atcmd_t) + sizeof(g_user_at_cmd_list_soil) / sizeof(atcmd_t);
		memcpy((void *)&g_user_at_cmd_list[0], (void *)g_user_at_cmd_list_gps, sizeof(g_user_at_cmd_list_gps));
		memcpy((void *)&g_user_at_cmd_list[sizeof(g_user_at_cmd_list_gps) / sizeof(atcmd_t)], (void *)g_user_at_cmd_list_soil, sizeof(g_user_at_cmd_list_soil));
	}
}
