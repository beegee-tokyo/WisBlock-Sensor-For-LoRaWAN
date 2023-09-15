/**
 * @file RAK12052_temp_array.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2023-09-15
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef RAK12052_H
#define RAK12052_H
#include <Arduino.h>

bool init_rak12052(void);
void read_rak12052(void);

#endif // RAK12052_H