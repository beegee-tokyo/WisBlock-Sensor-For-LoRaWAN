/**
 * @file RAK1910-RAK12500_gnss.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK1910_RAK12500_H
#define RAK1910_RAK12500_H
#include <Arduino.h>

#ifndef TASK_PRIO_LOW
#define TASK_PRIO_LOW 1
#endif

// GNSS functions
#define NO_GNSS_INIT 0
#define RAK1910_GNSS 1
#define RAK12500_GNSS 2
bool init_gnss(void);
bool poll_gnss(void);

void read_gps_settings(uint8_t settings);
void save_gps_settings(uint8_t settings);

#if defined NRF52_SERIES || defined ESP32
extern SemaphoreHandle_t g_gnss_sem;
extern SemaphoreHandle_t g_gnss_poll;
extern TaskHandle_t gnss_task_handle;
void gnss_task(void *pvParameters);
#endif
#ifdef ARDUINO_ARCH_RP2040
extern Thread gnss_task_handle; // (osPriorityNormal, 4096);
extern osThreadId gnss_task_id;
void gnss_task(void);
#endif

extern volatile bool last_read_ok;
extern uint8_t g_gnss_option;

extern bool g_gps_prec_6;
extern bool g_is_helium;
extern bool g_is_tester;

extern volatile bool gnss_active;
extern bool g_gnss_power_off;

extern time_t min_delay;
extern time_t last_pos_send;

/** Latitude/Longitude value union */
union latLong_s
{
	uint32_t val32;
	uint8_t val8[4];
};

#endif // RAK1910_RAK12500_H