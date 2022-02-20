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
#define TOUCH_EVENT      0b0001000000000000
#define N_TOUCH_EVENT    0b1110111111111111

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
#define LPP_CHANNEL_UVI 27
#define LPP_CHANNEL_UVS 28
#define LPP_CHANNEL_CURRENT_CURRENT 29
#define LPP_CHANNEL_CURRENT_VOLTAGE 30
#define LPP_CHANNEL_CURRENT_POWER 31
#define LPP_CHANNEL_TOUCH_1 32
#define LPP_CHANNEL_TOUCH_2 33
#define LPP_CHANNEL_TOUCH_3 34

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
bool init_rak1921(void);
void rak1921_add_line(char *line);
void rak1921_show(void);
void rak1921_write_header(char *header_line);
bool init_rak12002(void);
void set_rak12002(uint16_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute);
void read_rak12002(void);
bool init_rak12004(void);
void read_rak12004(void);
bool init_rak12008(void);
void read_rak12008(void);
bool init_rak12009(void);
void read_rak12009(void);
bool init_rak12010(void);
void read_rak12010();
bool init_rak12014(void);
void read_rak12014(void);
bool init_rak12019(void);
void read_rak12019();
#define GYRO_INT_PIN WB_IO4
bool init_rak12025(void);
void read_rak12025(void);
void clear_int_rak12025(void);
bool init_rak12047(void);
void read_rak12047(void);
void do_read_rak12047(void);
#define TOUCH_INT_PIN WB_IO6
bool init_rak14002(void);
void read_rak14002(void);
void get_rak14002(void);
bool init_rak14003(void);
void set_rak14003(uint8_t *leds);
#define GESTURE_INT_PIN WB_IO4
bool init_rak14008(void);
void read_rak14008(void);
bool init_rak16000(void);
void read_rak16000(void);

void find_modules(void);
void announce_modules(void);
void get_sensor_values(void);

// Index for known I2C devices
#define ACC_ID 0	  // RAK1904 accelerometer
#define LIGHT_ID 1	  // RAK1903 light sensor
#define GNSS_ID 2	  // RAK12500 GNSS sensor
#define PRESS_ID 3	  // RAK1902 barometric pressure sensor
#define TEMP_ID 4	  // RAK1901 temperature & humidity sensor
#define ENV_ID 5	  // RAK1906 environment sensor
#define SOIL_ID 6	  // RAK12035 soil moisture sensor
#define LIGHT2_ID 7	  // RAK12010 light sensor
#define MQ2_ID 8	  // RAK12004 MQ2 CO2 gas sensor
#define MG812_ID 9	  // RAK12008 MG812 CO2 gas sensor
#define MQ3_ID 10	  // RAK12009 MQ3 Alcohol gas sensor
#define TOF_ID 11	  // RAK12014 Laser ToF sensor
#define RTC_ID 12	  // RAK12002 RTC module
#define BAR_ID 13	  // RAK14003 LED bargraph module
#define VOC_ID 14	  // RAK12047 VOC sensor
#define GYRO_ID 15	  // RAK12025 Gyroscope
#define GESTURE_ID 16 // RAK14008 Gesture sensor
#define OLED_ID 17	  // RAK1921 OLED display
#define UVL_ID 18	  // RAK12019 UV light sensor
#define TOUCH_ID 19	  // RAK14002 Touch Pad
#define CURRENT_ID 20 // RAK16000 current sensor

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