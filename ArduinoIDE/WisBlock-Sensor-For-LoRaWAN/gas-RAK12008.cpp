/**
 * @file gas-RAK12008.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Functions for RAK12008 CO2 gas sensor
 * @version 0.1
 * @date 2022-02-01
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include <ADC121C021.h>

#define MG812_ADDRESS 0x50

/** Constant to calculate PPM log(y) = constantA*log(x) + constantB,  y:sensor voltage,  x:gas concentration ppm */
float constantA = 0.027;
/** Constant to calculate PPM log(y) = constantA*log(x) + constantB,  y:sensor voltage,  x:gas concentration ppm */
float constantB = 0.4524;

/** Gas sensor instance */
ADC121C021 MG812;

/** Voltage read from sensor */
float sensorVoltage;

/**
 * @brief Initialize the gas sensor
 *
 * @return true success
 * @return false failed
 */
bool init_rak12008(void)
{
	pinMode(ALERT_PIN, INPUT);
	pinMode(EN_PIN, OUTPUT);
	digitalWrite(EN_PIN, HIGH); // power on RAK12008

	if (found_sensors[MG812_ID].i2c_num == 1)
	{
		Wire.begin();
		if (!MG812.begin(MG812_ADDRESS, Wire))
		{
			MYLOG("MG812", "MG812 not found");
			digitalWrite(EN_PIN, LOW); // power down RAK12008
			// api_deinit_gpio(EN_PIN);
			return false;
		}
	}
	else
	{
#if WIRE_INTERFACES_COUNT > 1
		Wire1.begin();
		if (!MG812.begin(MG812_ADDRESS, Wire1))
		{
			MYLOG("MG812", "MG812 not found");
			digitalWrite(EN_PIN, LOW); // power down RAK12008
			// api_deinit_gpio(EN_PIN);
			return false;
		}
#else
		return false;
#endif
	}
	return true;
}

/**
 * @brief Read data from the gas sensor
 *     Data is added to Cayenne LPP payload as channels
 *     LPP_CHANNEL_CO2 and LPP_CHANNEL_CO2_PERC
 *
 */
void read_rak12008(void)
{
	sensorVoltage = MG812.getSensorVoltage() / V_RATIO;
	double ppm_log = (constantB - sensorVoltage) / constantA;
	sensorPPM = pow(M_E, ppm_log);
	MYLOG("MG812", "MG812 sensor voltage Value is: %3.2f\r\n", sensorVoltage);
	MYLOG("MG812", "MG812 sensor PPM Value is: %3.2f\r\n", sensorPPM);
	PPMpercentage = sensorPPM / 10000;
	MYLOG("MG812", "MG812 PPM percentage Value is:%3.2f%%\r\n", PPMpercentage);

	g_solution_data.addAnalogInput(LPP_CHANNEL_CO2, sensorPPM);
	g_solution_data.addPercentage(LPP_CHANNEL_CO2_PERC, PPMpercentage);
}