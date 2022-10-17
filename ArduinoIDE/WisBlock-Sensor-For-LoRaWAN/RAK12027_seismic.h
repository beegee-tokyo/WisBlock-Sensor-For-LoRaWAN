/**
 * @file RAK12027_seismic.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK12027_H
#define RAK12027_H
#include <Arduino.h>
#include <Wire.h>

//******************************************************************//
// RAK12027 INT1_PIN
//******************************************************************//
// Slot A      WB_IO1
// Slot B      WB_IO2 ( not recommended, pin conflict with IO2)
// Slot C      WB_IO3
// Slot D      WB_IO5
// Slot E      WB_IO4
// Slot F      WB_IO6
//******************************************************************//
//******************************************************************//
// RAK12027 INT2_PIN
//******************************************************************//
// Slot A      WB_IO2 ( not recommended, pin conflict with IO2)
// Slot B      WB_IO1
// Slot C      WB_IO4
// Slot D      WB_IO6
// Slot E      WB_IO3
// Slot F      WB_IO5
//******************************************************************//

/** Interrupt pin, depends on slot */
#define INT1_PIN WB_IO3 // interrupt pin INT1 of D7S attached to WB_IO5 of WisBlock Base Board Slot D
#define INT2_PIN WB_IO4 // interrupt pin INT2 of D7S attached to WB_IO6 of WisBlock Base Board Slot D

bool init_rak12027(void);
bool calib_rak12027(void);
void read_rak12027(bool add_values);
uint8_t check_event_rak12027(bool is_int1);
extern bool shutoff_alert;
extern bool collapse_alert;
extern bool earthquake_end;

#endif // RAK12027_H