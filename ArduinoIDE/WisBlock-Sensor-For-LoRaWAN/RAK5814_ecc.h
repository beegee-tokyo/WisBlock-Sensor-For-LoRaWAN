/**
 * @file RAK5814_ecc.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK5814_H
#define RAK5814_H
#include <Arduino.h>

bool init_rak5814(void);
uint16_t random_num_rak5814(uint16_t min, uint16_t max);
bool sha256_rak5814(byte *data, uint32_t length, byte *result);

#endif // RAK5814_H