/**
 * @file RAK14003_bar.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-24
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK14003_H
#define RAK14003_H
#include <Arduino.h>

bool init_rak14003(void);
void set_rak14003(uint8_t *leds);

#endif // RAK14003_H