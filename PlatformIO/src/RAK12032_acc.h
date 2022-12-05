/**
 * @file RAK12032_acc.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK12032_H
#define RAK12032_H
#include <Arduino.h>

//******************************************************************//
// RAK12032 INT1_PIN
//******************************************************************//
// Slot A      WB_IO1
// Slot B      WB_IO2 ( not recommended, pin conflict with IO2)
// Slot C      WB_IO3
// Slot D      WB_IO5
// Slot E      WB_IO4
// Slot F      WB_IO6
//******************************************************************//
//******************************************************************//
// RAK12032 INT2_PIN
//******************************************************************//
// Slot A      WB_IO2 ( not recommended, pin conflict with IO2)
// Slot B      WB_IO1
// Slot C      WB_IO4
// Slot D      WB_IO6
// Slot E      WB_IO3
// Slot F      WB_IO5
//******************************************************************//

bool init_rak12032(void);
void int_assign_rak12032(uint8_t new_irq_pin);
void clear_int_rak12032(void);

#endif // RAK12032_H