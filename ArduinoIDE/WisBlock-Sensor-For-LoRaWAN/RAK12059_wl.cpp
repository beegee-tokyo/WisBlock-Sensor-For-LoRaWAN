/**
 * @file RAK12059_wl.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Init and value reading for RAK12059 water level sensor
 * @version 0.1
 * @date 2023-03-18
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "app.h"

#include <ADC_SGM58031.h> //http://librarymanager/All#RAKwireless_ADC_SGM58031_library

/** Sensor instance */
RAK_ADC_SGM58031 sgm58031(SGM58031_SDA_ADDRESS);

/************************************************/
/* Select Sensor length                         */
/************************************************/
#if SENSOR_LEN == 8
/** Use for 8 inch sensor */
float s_len = 7;
#elif SENSOR_LEN == 12
/** Use for 12 inch sensor */
float s_len = 11;
#else
/** Use for 24 inch sensor */
float s_len = 23;
#endif

/************************************************/
/* Value for max level from calibration         */
/************************************************/
float g_v_high = 2.2245; // calibration value at 12 inch
/************************************************/
/* Value for min level from calibration         */
/************************************************/
float g_v_low = 1.3256; // calibration value at 1 inch
/** Voltage difference between min and max levels */
float v_diff = (g_v_high - g_v_low);
/** Measured depth in inch */
float distance_inch = 0.0;
/** Measured depth in cm */
float distance_cm = 0.0;
/** Voltage level of sensor */
float sensor_voltage = 0.0f;

/** Low level threshold */
uint16_t g_low_level = 0x8000;
/** High level treshold */
uint16_t g_high_level = 0x7FFF;
/** Flag for interrupt */
bool interrupt_flag = false;

/** Configuration */
uint16_t config_value = 0xC2E0;
// 1100001011100000
// 1                 Single Start conversion
//  100              AINp = AIN0, AINn = GND
//     001           FS 4.096V
//        0          0 Continuous conversion mode 1 Power Down single-shot mode
//         111       Data rate 800/960 Hz
//            0      Traditional Comparator
//             0     Polarity of Alert active low
//              0    Non latching comparator
//               00  Assert after one conversion

/** Configuration 1 */
uint16_t config1_value = 0x0180;
//  0000000110000000
//  0000000           N/A
//         1          Power down
//          1         Set datarate to 960 Hz
//           0        No current source
//            0       Do not use external diode as ADC input
//             0      Disable leakage blocking circuit
//              0     Do not use external reference
//               000  N/A

bool init_rak12059(void)
{
	// Initialize sensor
	sgm58031.begin();
	MYLOG("WL", "RAK12059 Init");
	if (sgm58031.getChipID() != DEVICE_ID)
	{
		MYLOG("WL", "No CHIP found ... please check your connection");
		return false;
	}
	else
	{
		MYLOG("WL", "Found SGM58031 Chip");
	}

	// Prepare interrupt pin
	pinMode(ALERT_WL_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(ALERT_WL_PIN), int_rak10259, FALLING);

	// Get saved calibration values
	read_wl_calibration();
	v_diff = (g_v_high - g_v_low);

	// Set thresholds
	g_low_level = (uint16_t)((g_v_low + 0.05) * 0x7FFF / 3.3f);
	g_high_level = (uint16_t)((g_v_high)*0x7FFF / 3.3f);

	// Set interrupts
	set_threshold_rak12059();

	// Configure sensor
	sgm58031.setConfig1(config1_value);

	sgm58031.setConfig(config_value);

	sgm58031.setVoltageResolution(3.3); // Set resolution to 3.3 Volt

	delay(1000);
	return true;
}

bool read_rak12059(void)
{
	bool low_alert = false;
	bool high_alert = false;

	// Check if interrupt was set
	// if (interrupt_flag) // Interrupts are not working atm, just check the levels
	{
		interrupt_flag = false;
		if (sgm58031.getAdcValue() > g_high_level)
		{
			MYLOG("WL", "High threshold triggered");
			high_alert = true;
		}
		else if (sgm58031.getAdcValue() < g_low_level)
		{
			MYLOG("WL", "Low threshold triggered");
			low_alert = true;
		}
		// Reset interrupts
		reset_int_rak12059();
	}

	// Read sensor
	// Get voltage
	sensor_voltage = sgm58031.getVoltage();
	MYLOG("WL", "sensor_voltage= %.6f\n", sensor_voltage);
	MYLOG("WL", "ADC value = %04X\n", sgm58031.getAdcValue());

	// Calculate water level
	distance_inch = ((sensor_voltage - g_v_low) * s_len / v_diff) + 1;
	distance_cm = distance_inch * 2.54;
	MYLOG("WL", "Distance = %.4f\n", distance_cm);

	/** distance_cm = (((sensor_voltage - g_v_low) * s_len / v_diff) + 1) * 2.54
	 * (distance_cm / 2.54) = ((sensor_voltage - g_v_low) * s_len / v_diff) + 1
	 * (distance_cm / 2.54) - 1 = ((sensor_voltage - g_v_low) * s_len / v_diff
	 * ((distance_cm / 2.54) - 1) * v_diff / slen = sensor_voltage - g_v_low
	 * (((distance_cm / 2.54) - 1) * v_diff / slen) + g_v_low = sensor_voltage
	 *
	 *
	 * g_low_level = (uint16_t)(((((distance_cm / 2.54) - 1) * v_diff / slen) + g_v_low)*0x7FFF / 3.3f);
	 *
	 * g_low_level * 3.3f = (((((distance_cm / 2.54) - 1) * v_diff / slen) + g_v_low)*0x7FFF);
	 * g_low_level / 0x7FFF * 3.3f = (((distance_cm / 2.54) - 1) * v_diff / slen) + g_v_low;
	 * (g_low_level / 0x7FFF * 3.3f) - g_v_low = (((distance_cm / 2.54) - 1) * v_diff / slen);
	 * ((g_low_level / 0x7FFF * 3.3f) - g_v_low) * slen = ((distance_cm / 2.54) - 1) * v_diff
	 * ((g_low_level / 0x7FFF * 3.3f) - g_v_low) * slen / v_diff = (distance_cm / 2.54) - 1
	 * (((g_low_level / 0x7FFF * 3.3f) - g_v_low) * slen / v_diff) + 1 = distance_cm / 2.54
	 * ((((g_low_level / 0x7FFF * 3.3f) - g_v_low) * slen / v_diff) + 1) * 2.54 = distance_cm
	 */
	// Add level to the payload
	g_solution_data.addAnalogInput(LPP_CHANNEL_WLEVEL, distance_cm);
	g_solution_data.addPresence(LPP_CHANNEL_WL_LOW, low_alert);
	g_solution_data.addPresence(LPP_CHANNEL_WL_HIGH, high_alert);
}

/**
 * @brief Interrupt handler
 *
 */
void int_rak10259(void)
{
	interrupt_flag = true;
}

/**
 * @brief Calculate the threshold as ADC value
 * 		from requested level given as 100 * cm
 *
 * @param thres_request
 * @return uint16_t
 */
uint16_t calc_thres_rak12059(uint16_t thres_request)
{
	return (uint16_t)(((((thres_request / 100.0 / 2.54) - 1) * v_diff / s_len) + g_v_low) * 32767.0 / 3.3f);
}

/**
 * @brief Get the threshold in cm
 *
 * @param threshold as ADC value
 * @return float threshold in cm
 */
float get_thres_cm_rak12059(uint16_t threshold)
{
	return (((((float)threshold * 3.3 / 32767.0) - g_v_low) * s_len / v_diff) + 1) * 2.54;
}

/**
 * @brief Set and reset interrupt registers
 *
 */
void set_threshold_rak12059(void)
{
	// Set low threshold
	MYLOG("WL", "Alert Low %04X\n", g_low_level);
	sgm58031.setAlertLowThreshold(g_low_level); // Write  threshold  to Lo_Thresh

	// Set low threshold
	MYLOG("WL", "Alert High %04X\n", g_high_level);
	sgm58031.setAlertHighThreshold(g_high_level); // Write threshold to Hi_Thresh

	// Calculate difference between full and empty
	v_diff = (g_v_high - g_v_low);

	// MYLOG("WL", "Calib Low %.4f\n", g_v_low);
	// MYLOG("WL", "Calib High %.4f\n", g_v_high);
	// MYLOG("WL", "V diff %.4f\n", v_diff);
}

/**
 * @brief Reset interrupt registers
 *
 */
void reset_int_rak12059(void)
{
	set_threshold_rak12059();
	sgm58031.setConfig(0xC2E0);
}

/**
 * @brief Collect 10 values for calibration of the water level sensor
 *
 * @param high_low if true, low level calibration, else high level calibration
 */
void start_calib_rak12059(bool low_high)
{
	double collection = sgm58031.getVoltage();
	for (int idx = 0; idx < 100; idx++)
	{
		collection = (collection + sgm58031.getVoltage()) / 2;
		delay(250);
	}

	if (low_high)
	{
		// Low level collection
		g_v_low = collection;
	}
	else
	{
		// High level calibration
		g_v_high = collection;
	}
	v_diff = (g_v_high - g_v_low);
}
