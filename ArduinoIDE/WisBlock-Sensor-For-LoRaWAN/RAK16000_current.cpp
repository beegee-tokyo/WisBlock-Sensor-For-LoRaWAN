/**
 * @file RAK16000_current.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Functions for RAK16000 current sensor
 * @version 0.1
 * @date 2022-02-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include "INA219_WE.h"

#define I2C_ADDRESS 0x41

/** Current sensor instance using Wire */
INA219_WE ina219_1 = (Wire, I2C_ADDRESS);
#if WIRE_INTERFACES_COUNT > 1
/** Current sensor instance using Wire1 */
INA219_WE ina219_2 = (Wire1, I2C_ADDRESS);
#endif
/** Pointer to used instance */
INA219_WE *ina219;

/** Default ADC mode */
INA219_ADC_MODE adc_mode = SAMPLE_MODE_128;

/** Default measurement mode */
INA219_MEASURE_MODE measure_mode = CONTINUOUS;

/** Default gain */
INA219_PGAIN gain_mode = PG_320;

/** Default bus range */
INA219_BUS_RANGE bus_range = BRNG_32;

/**
 * @brief Initialize current sensor
 *
 * @return true success
 * @return false failed
 */
bool init_rak16000(void)
{
	if (found_sensors[CURRENT_ID].i2c_num == 1)
	{
		Wire.begin();
		ina219 = &ina219_1;
	}
	else
	{
#if WIRE_INTERFACES_COUNT > 1
		Wire1.begin();
		ina219 = &ina219_2;
#else
		return false;
#endif
	}

	if (!ina219->init())
	{
		MYLOG("INA", "INA219 not found");
		return false;
	}

	/* Set ADC Mode for Bus and ShuntVoltage
	  * Mode *            * Res / Samples *       * Conversion Time *
	  BIT_MODE_9        9 Bit Resolution             84 µs
	  BIT_MODE_10       10 Bit Resolution            148 µs
	  BIT_MODE_11       11 Bit Resolution            276 µs
	  BIT_MODE_12       12 Bit Resolution            532 µs  (DEFAULT)
	  SAMPLE_MODE_2     Mean Value 2 samples         1.06 ms
	  SAMPLE_MODE_4     Mean Value 4 samples         2.13 ms
	  SAMPLE_MODE_8     Mean Value 8 samples         4.26 ms
	  SAMPLE_MODE_16    Mean Value 16 samples        8.51 ms
	  SAMPLE_MODE_32    Mean Value 32 samples        17.02 ms
	  SAMPLE_MODE_64    Mean Value 64 samples        34.05 ms
	  SAMPLE_MODE_128   Mean Value 128 samples       68.10 ms
	  */
	ina219->setADCMode(adc_mode); // choose mode and uncomment for change of default

	/* Set measure mode
	POWER_DOWN - INA219 switched off
	TRIGGERED  - measurement on demand
	ADC_OFF    - Analog/Digital Converter switched off
	CONTINUOUS  - Continuous measurements (DEFAULT)
	*/
	ina219->setMeasureMode(measure_mode); // choose mode and uncomment for change of default

	/* Set PGain
	* Gain *  * Shunt Voltage Range *   * Max Current (if shunt is 0.1 ohms) *
	 PG_40       40 mV                    0.4 A
	 PG_80       80 mV                    0.8 A
	 PG_160      160 mV                   1.6 A
	 PG_320      320 mV                   3.2 A (DEFAULT)
	*/
	ina219->setPGain(gain_mode); // choose gain and uncomment for change of default

	/* Set Bus Voltage Range
	 BRNG_16   -> 16 V
	 BRNG_32   -> 32 V (DEFAULT)
	*/
	ina219->setBusRange(bus_range);	 // choose range and uncomment for change of default
	ina219->setShuntSizeInOhms(0.1); // we use 100ohm

	/* If the current values delivered by the INA219 differ by a constant factor
	   from values obtained with calibrated equipment you can define a correction factor.
	   Correction factor = current delivered from calibrated equipment / current delivered by INA219
	*/
	ina219->setCorrectionFactor(0.99); // insert your correction factor if necessary
	return true;
}

/**
 * @brief Read value from current, voltage and power sensor
 *     Data is added to Cayenne LPP payload as channel
 *     LPP_CHANNEL_CURRENT_CURRENT, LPP_CHANNEL_CURRENT_VOLTAGE,
 *     and LPP_CHANNEL_CURRENT_POWER
 *
 */
void read_rak16000(void)
{
	float shuntVoltage_mV = 0.0;
	float busVoltage_V = 0.0;
	float current_mA = 0.0;
	float power_mW = 0.0;

	shuntVoltage_mV = ina219->getShuntVoltage_mV();
	busVoltage_V = ina219->getBusVoltage_V();
	// here we use the I=U/R to calculate, here the Resistor is 100mΩ, accuracy can reach to 0.5%.
	current_mA = shuntVoltage_mV / 0.1;
	power_mW = ina219->getBusPower();

	if (ina219->getOverflow())
	{
		MYLOG("INA", "INA219 overflow");
		g_solution_data.addAnalogInput(LPP_CHANNEL_CURRENT_CURRENT, 0.0);
		g_solution_data.addAnalogInput(LPP_CHANNEL_CURRENT_VOLTAGE, 0.0);
		g_solution_data.addAnalogInput(LPP_CHANNEL_CURRENT_POWER, 0.0);
	}
	else
	{
		g_solution_data.addAnalogInput(LPP_CHANNEL_CURRENT_CURRENT, current_mA);
		g_solution_data.addAnalogInput(LPP_CHANNEL_CURRENT_VOLTAGE, busVoltage_V);
		g_solution_data.addAnalogInput(LPP_CHANNEL_CURRENT_POWER, power_mW);
	}
}