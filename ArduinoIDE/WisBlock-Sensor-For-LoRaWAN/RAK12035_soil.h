/**
 * @file RAK12035_soil.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK12035_H
#define RAK12035_H
#include <Arduino.h>

/** Soil sensor stuff */
bool init_rak12035(void);
void read_rak12035(void);
uint16_t start_calib_rak12035(bool is_dry);
uint16_t get_calib_rak12035(bool is_dry);
uint16_t set_calib_rak12035(bool is_dry, uint16_t calib_val);

#endif // RAK12035_H