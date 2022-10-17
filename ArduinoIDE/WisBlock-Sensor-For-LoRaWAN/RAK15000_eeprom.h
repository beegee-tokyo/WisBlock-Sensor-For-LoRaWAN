/**
 * @file RAK15000_eeprom.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-24
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK15000_H
#define RAK15000_H
#include <Arduino.h>

/** Default I2C address */
#define EEPROM_ADDR 0x50
/** Max address of EEPROM */
#define MAXADD 262143

bool init_rak15000(void);
bool read_rak15000(uint16_t addr, uint8_t *buffer, uint16_t num);
bool write_rak15000(uint16_t addr, uint8_t *buffer, uint16_t num);

#endif // RAK15000_H