/**
 * @file RAK14014_tft.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Definitions and graphics for RAK14014
 * @version 0.1
 * @date 2022-12-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _RAK14014_TFT_H_
#define _RAK14014_TFT_H_

#ifdef ST7789_DRIVER
#include "app.h"
#include <TFT_eSPI.h> // Display library
#include <SPI.h>
#include "FT6336U.h" // Touch screen library

extern const GUI_BITMAP bmUp31X31;
extern const GUI_BITMAP bmDown31X31;

void init_rak14014(void);
void wake_rak14014(void);
void clear_rak14014(void);
void refresh_rak14014(void);
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

#endif // ST7789_DRIVER
#endif // _RAK14014_TFT_H_