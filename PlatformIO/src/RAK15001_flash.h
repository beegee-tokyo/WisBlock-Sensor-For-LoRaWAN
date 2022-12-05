/**
 * @file RAK15001_flash.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-24
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef ARDUINO_ARCH_RP2040
#ifndef RAK15001_H
#define RAK15001_H
#include <Arduino.h>

bool init_rak15001(void);
bool read_rak15001(uint16_t address, uint8_t *buffer, uint16_t size);
bool write_rak15001(uint16_t address, uint8_t *buffer, uint16_t size);
extern bool g_has_rak15001;

#endif // RAK15001_H
#endif // ARDUINO_ARCH_RP2040