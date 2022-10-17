/**
 * @file RAK1921_oled.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK1921_H
#define RAK1921_H
#include <Arduino.h>

#ifdef ESP32
#define PIN_WIRE_SDA SDA
#define PIN_WIRE_SCL SCL
#endif

bool init_rak1921(void);
void rak1921_add_line(char *line);
void rak1921_show(void);
void rak1921_write_header(char *header_line);

#endif // RAK1921_H