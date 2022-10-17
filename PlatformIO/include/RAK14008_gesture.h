/**
 * @file RAK14008_gesture.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-24
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK14008_H
#define RAK14008_H
#include <Arduino.h>

#define GESTURE_INT_PIN WB_IO6
bool init_rak14008(void);
void read_rak14008(void);

#endif // RAK14008_H