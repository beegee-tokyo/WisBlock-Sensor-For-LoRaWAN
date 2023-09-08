/**
 * @file RAK12059_wl.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief 
 * @version 0.1
 * @date 2023-03-18
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef RAK12059_H
#define RAK12059_H
#include <Arduino.h>

//******************************************************************//
// RAK12059 alert pin guide
//******************************************************************//
// Slot A      WB_IO1 
// Slot B      WB_IO2 ( not recommended, INT pin conflict with IO2)
// Slot C      WB_IO3
// Slot D      WB_IO5
// Slot E      WB_IO4
// Slot F      WB_IO6
//******************************************************************//
#define ALERT_WL_PIN WB_IO1 // Slot A installation

//******************************************************************//
// RAK12059 sensor length (active area is from 1 inch to sensor length)
//******************************************************************//
// 8 inch sensor, max distance 7 inch
// 12 inch sensor, max distance 11 inch
// 24 inch sensor, max distance 23 inch
//******************************************************************//
#define SENSOR_LEN 12

// Forward declarations
bool init_rak12059(void);
bool read_rak12059(void);
void int_rak10259(void);
void set_threshold_rak12059(void);
void reset_int_rak12059(void);
void start_calib_rak12059(bool low_high);
uint16_t calc_thres_rak12059(uint16_t thres_request);
float get_thres_cm_rak12059(uint16_t threshold);

// Water Level AT commands
void save_wl_calibration(void);
void read_wl_calibration(void);

extern float g_v_low;
extern float g_v_high;
extern uint16_t g_low_level;
extern uint16_t g_high_level;

#endif // RAK12059_H