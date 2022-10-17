/**
 * @file RAK12025_gyro.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK12025_H
#define RAK12025_H
#include <Arduino.h>

//******************************************************************//
// RAK12025 interrupt guide
//******************************************************************//
// INT1
// Slot A      WB_IO2 ( == power control )
// Slot B      WB_IO1 ( not recommended, INT pin conflict with IO2)
// Slot C      WB_IO4
// Slot D      WB_IO6
// Slot E      WB_IO3
// Slot F      WB_IO5
//******************************************************************//
// INT2
// Slot A      WB_IO1
// Slot B      WB_IO2 ( not recommended, pin conflict with IO2)
// Slot C      WB_IO3
// Slot D      WB_IO5
// Slot E      WB_IO4
// Slot F      WB_IO6
//******************************************************************//

#define GYRO_INT_PIN WB_IO4
bool init_rak12025(void);
void read_rak12025(void);
void clear_int_rak12025(void);

#endif // RAK12025_H