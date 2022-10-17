/**
 * @file RAK1905_9dof.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Global definitions and forward declarations
 * @version 0.1
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef RAK1905_H
#define RAK1905_H
#include <Arduino.h>

//******************************************************************//
// RAK1905 INT1_PIN
//******************************************************************//
// Slot A      WB_IO1
// Slot B      WB_IO2 ( not recommended, pin conflict with IO2)
// Slot C      WB_IO3
// Slot D      WB_IO5
// Slot E      WB_IO4
// Slot F      WB_IO6
//******************************************************************//

bool init_rak1905(void);
void clear_int_rak1905(void);

#endif // RAK1905_H