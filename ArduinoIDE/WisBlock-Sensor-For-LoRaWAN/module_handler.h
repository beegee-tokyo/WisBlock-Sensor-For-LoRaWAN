/**
 * @file module_handler.h
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Globals and defines for module handling
 * @version 0.1
 * @date 2022-02-15
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <Arduino.h>

#ifndef MODULE_HANDLER_H
#define MODULE_HANDLER_H

extern bool init_result;
extern time_t min_delay;
extern time_t last_pos_send;
extern SoftwareTimer delayed_sending;
void send_delayed(TimerHandle_t unused);

/** Wakeup triggers for application events */
#define MOTION_TRIGGER 0b1000000000000000
#define N_MOTION_TRIGGER 0b0111111111111111
#define GNSS_FIN 0b0100000000000000
#define N_GNSS_FIN 0b1011111111111111
#define VOC_REQ 0b0010000000000000
#define N_VOC_REQ 0b1101111111111111

typedef struct sensors_s
{
	uint8_t i2c_addr;  // I2C address
	uint8_t i2c_num;   // I2C port
	bool found_sensor; // Flag if sensor is present
} sensors_t;

extern sensors_t found_sensors[];

// LoRaWAN stuff
#include "wisblock_cayenne.h"
// Cayenne LPP Channel numbers per sensor value
#define LPP_CHANNEL_BATT 1
#define LPP_CHANNEL_HUMID 2
#define LPP_CHANNEL_TEMP 3
#define LPP_CHANNEL_PRESS 4
#define LPP_CHANNEL_LIGHT 5
#define LPP_CHANNEL_HUMID_2 6
#define LPP_CHANNEL_TEMP_2 7
#define LPP_CHANNEL_PRESS_2 8
#define LPP_CHANNEL_GAS_2 9
#define LPP_CHANNEL_GPS 10
#define LPP_CHANNEL_SOIL_TEMP 11
#define LPP_CHANNEL_SOIL_HUMID 12
#define LPP_CHANNEL_SOIL_HUMID_RAW 13
#define LPP_CHANNEL_SOIL_VALID 14
#define LPP_CHANNEL_LIGHT2 15
#define LPP_CHANNEL_VOC 16
#define LPP_CHANNEL_GAS 17
#define LPP_CHANNEL_GAS_PERC 18
#define LPP_CHANNEL_CO2 19
#define LPP_CHANNEL_CO2_PERC 20
#define LPP_CHANNEL_ALC 21
#define LPP_CHANNEL_ALC_PERC 22
#define LPP_CHANNEL_TOF 23
#define LPP_CHANNEL_TOF_VALID 24
#define LPP_CHANNEL_GYRO 25
#define LPP_CHANNEL_GESTURE 26

extern WisCayenne g_solution_data;

/** Sensor functions */
bool init_rak1901(void);
void read_rak1901(void);
void get_rak1901_values(float *values);
bool init_rak1902(void);
void read_rak1902(void);
bool init_rak1903(void);
void read_rak1903();
#define ACC_INT_PIN WB_IO3
bool init_rak1904(void);
void clear_int_rak1904(void);
bool init_rak1906(void);
void start_rak1906(void);
bool read_rak1906(void);
bool init_rak12010(void);
void read_rak12010();
bool init_rak12047(void);
void read_rak12047(void);
void do_read_rak12047(void);
bool init_rak12004(void);
void read_rak12004(void);
bool init_rak12008(void);
void read_rak12008(void);
bool init_rak12009(void);
void read_rak12009(void);
bool init_rak12014(void);
void read_rak12014(void);
#define GYRO_INT_PIN WB_IO4
bool init_rak12025(void);
void read_rak12025(void);
void clear_int_rak12025(void);
bool init_rak14003(void);
void set_rak14003(uint8_t *leds);
bool init_rak14008(void);
void read_rak14008(void);
#define GESTURE_INT_PIN WB_IO4

void find_modules(void);
void announce_modules(void);
void get_sensor_values(void);

// Flags for sensors found */
extern bool has_rak1906;
extern bool has_rak1901;
extern bool has_rak1902;
extern bool has_rak1903;
extern bool has_rak1904;
extern bool has_rak1910_rak12500;
extern bool has_rak12004;
extern bool has_rak12008;
extern bool has_rak12009;
extern bool has_rak12035;
extern bool has_rak12010;
extern bool has_rak12014;
extern bool has_rak12025;
extern bool has_rak12047;
extern bool has_rak14003;
extern bool has_rak14008;

#define ACC_ID 0
#define LIGHT_ID 1
#define GNSS_ID 2
#define PRESS_ID 3
#define TEMP_ID 4
#define ENV_ID 5
#define SOIL_ID 6
#define LIGHT2_ID 7
#define VOC_ID 22
#define MQ2_ID 8
#define MG812_ID 9
#define MQ3_ID 10
#define BAR_ID 18
#define TOF_ID 12
#define GYRO_ID 23
#define GESTURE_ID 24

/**
	{0x18, 0, false}, //  0 RAK1904 accelerometer
	{0x44, 0, false}, //  1 RAK1903 light sensor
	{0x42, 0, false}, //  2 RAK12500 GNSS sensor
	{0x5c, 0, false}, //  3 RAK1902 barometric pressure sensor
	{0x70, 0, false}, //  4 RAK1901 temperature & humidity sensor
	{0x76, 0, false}, //  5 RAK1906 environment sensor
	{0x20, 0, false}, //  6 RAK12035 soil moisture sensor
	{0x10, 0, false}, //  7 RAK12010 light sensor
	{0x51, 0, false}, //  8 RAK12004 MQ2 gas sensor
	{0x52, 0, false}, //  9 RAK12008 MG812 gas sensor
	{0x55, 0, false}, // 10 RAK12009 Alcohol gas sensor
	{0x57, 0, false}, // 11 RAK12012 MAX30102 heart rate sensor
	{0x52, 0, false}, // 12 RAK12014 Laser ToF sensor
	{0x54, 0, false}, // 13 RAK12016 Flex sensor
	{0x53, 0, false}, // 14 RAK12019 LTR390 light sensor
	{0x47, 0, false}, // 15 RAK13004 PWM expander module
	{0x38, 0, false}, // 16 RAK14001 RGB LED module
	{0x50, 0, false}, // 17 RAK14002 Touch Button module
	{0x24, 0, false}, // 18 RAK14003 LED bargraph module
	{0x5F, 0, false}, // 19 RAK14004 Keypad interface
	{0x60, 0, false}, // 20 RAK16000 DC current sensor
	{0x61, 0, false}, // 21 RAK16001 ADC sensor
	{0x59, 0, false}, // 22 RAK12047 VOC sensor
	{0x59, 0, false}, // 23 RAK12025 Gyroscope
	{0x59, 0, false}, // 24 RAK14008 Gesture sensor
	{0x59, 0, false}, // 25 RAK12001 Fingerprint sensor
	{0x59, 0, false}, // 26 RAK12032 T-Fork sensor
	{0x59, 0, false}, // 27 RAK13600 NFC
	{0x59, 0, false}, // 28 RAK16002 Coulomb sensor
*/

/** Gas Sensor stuff RAK12004, RAK12008 and RAK12009 */
/** Logic high enables the device. Logic low disables the device */
#define EN_PIN WB_IO6
/** Alarm trigger, a high indicates that the respective limit has been violated. */
#define ALERT_PIN WB_IO5
/** Clean Air ratio. RS / R0 = 1.0 ppm */
#define RatioGasCleanAir (1.0)
/** RL on the modules, RL = 10KΩ  can adjust */
#define Gas_RL (10.0)
/** Voltage amplification factor */
#define V_RATIO 3.0

extern float sensorPPM;
extern float PPMpercentage;

/** Soil sensor stuff */
bool init_rak12035(void);
void read_rak12035(void);
uint16_t start_calib_rak12035(bool is_dry);
uint16_t get_calib_rak12035(bool is_dry);
uint16_t set_calib_rak12035(bool is_dry, uint16_t calib_val);

// GNSS functions
#define NO_GNSS_INIT 0
#define RAK1910_GNSS 1
#define RAK12500_GNSS 2
bool init_gnss(void);
bool poll_gnss(void);
void gnss_task(void *pvParameters);
extern SemaphoreHandle_t g_gnss_sem;
extern TaskHandle_t gnss_task_handle;
extern volatile bool last_read_ok;
extern uint8_t g_gnss_option;

extern bool g_gps_prec_6;
extern bool g_is_helium;

void read_gps_settings(void);
void save_gps_settings(void);

/** Latitude/Longitude value union */
union latLong_s
{
	uint32_t val32;
	uint8_t val8[4];
};

#endif