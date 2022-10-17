/**
 * @file RAK1904_acc.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK1904_H
#define RAK1904_H
#include <Arduino.h>
#include <Wire.h>

//******************************************************************//
// RAK1904 INT1_PIN
//******************************************************************//
// Slot A      WB_IO1
// Slot B      WB_IO2 ( not recommended, pin conflict with IO2)
// Slot C      WB_IO3
// Slot D      WB_IO5
// Slot E      WB_IO4
// Slot F      WB_IO6
//******************************************************************//

#if BASE_BOARD == 0
#define ACC_INT_PIN WB_IO3
#else
#define ACC_INT_PIN WB_IO5
#endif

bool init_rak1904(void);
void int_assign_rak1904(uint8_t new_irq_pin);
void clear_int_rak1904(void);

#endif // RAK1904_H