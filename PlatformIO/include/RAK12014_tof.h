/**
 * @file RAK12014_tof.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK12014_H
#define RAK12014_H
#include <Arduino.h>

//******************************************************************//
// RAK12014 xshut_pin and interrupt guide
//******************************************************************//
// on/off control pin xshut_pin
// Slot A      WB_IO2 ( == power control )
// Slot B      WB_IO1 ( not recommended, INT pin conflict with IO2)
// Slot C      WB_IO4
// Slot D      WB_IO6
// Slot E      WB_IO3
// Slot F      WB_IO5
//******************************************************************//
// interrupt pin
// Slot A      WB_IO1
// Slot B      WB_IO2 ( not recommended, pin conflict with IO2)
// Slot C      WB_IO3
// Slot D      WB_IO5
// Slot E      WB_IO4
// Slot F      WB_IO6
//******************************************************************//

extern uint8_t xshut_pin;
bool init_rak12014(void);
void read_rak12014(void);

#endif // RAK12014_H