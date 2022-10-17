/**
 * @file RAK16000_current.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-24
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK16000_H
#define RAK16000_H
#include <Arduino.h>

bool init_rak16000(void);
void read_rak16000(void);

#endif // RAK16000_H