/**
 * @file RAK12003_fir.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK12003_H
#define RAK12003_H
#include <Arduino.h>

bool init_rak12003(void);
void read_rak12003(void);

#endif // RAK12003_H