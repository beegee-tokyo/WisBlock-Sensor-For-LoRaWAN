/**
 * @file RAK14002_touch.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-24
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK14002_H
#define RAK14002_H
#include <Arduino.h>

#define TOUCH_INT_PIN WB_IO6
bool init_rak14002(void);
void read_rak14002(void);
void get_rak14002(void);

#endif // RAK14002_H