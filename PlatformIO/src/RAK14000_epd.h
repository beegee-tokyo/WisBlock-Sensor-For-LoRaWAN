/**
 * @file RAK14000_epd.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations, Images for the EPD display
 * @version 0.1
 * @date 2022-06-25
 *
 * @copyright Copyright (c) 2022
 * Images cortesy of <a href="https://www.flaticon.com/free-icons" title="Freepik - Flaticon">Icons created by Freepik - Flaticon</a>
 */
#ifndef RAK14000_H
#define RAK14000_H
#include <Arduino.h>

// RAK14000 EPD stuff
void init_rak14000(void);
void wake_rak14000(void);
void clear_rak14000(void);
void refresh_rak14000(void);
void set_voc_rak14000(uint16_t voc_value);
extern bool voc_valid;
void set_temp_rak14000(float temp_value);
void set_humid_rak14000(float humid_value);
void set_baro_rak14000(float baro_value);
void set_co2_rak14000(float co2_value);
void set_pm_rak14000(uint16_t pm10_env, uint16_t pm25_env, uint16_t pm100_env);
void voc_rak14000(void);
void temp_rak14000(bool has_pm);
void humid_rak14000(bool has_pm);
void baro_rak14000(bool has_pm);
void co2_rak14000(bool has_pm);
void pm_rak14000(void);
void status_general_rak14000(bool has_pm);
void status_rak14000(void);

#endif // RAK14000_H