/**
 * @file RAK12040_temp_array.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK12040_H
#define RAK12040_H
#include <Arduino.h>

bool init_rak12040(void);
void read_rak12040(void);

#endif // RAK12040_H