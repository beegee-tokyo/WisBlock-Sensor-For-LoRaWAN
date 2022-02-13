/**
 * @file voc-RAK12047.cpp
 * @author Bernd Giesecke (bernd.giesecke@rakwireless.com)
 * @brief Read values from the RAK12047 VOC sensor
 *        The VOC algorithm requires a reading every one second.
 *        This code uses a timer to set the VOC_REQ every one second
 *        and wake up the loop to perform the readings
 * @date 2022-02-05
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#include <SensirionI2CSgp40.h>
#include <VOCGasIndexAlgorithm.h>

/** Instance for the VOC sensor */
SensirionI2CSgp40 sgp40;
/** Instance for the VOC algorithm */
VOCGasIndexAlgorithm voc_algorithm = VOCGasIndexAlgorithm();

/** Timer for VOC measurement */
SoftwareTimer voc_read_timer;

/** Calculated VOC index */
int32_t voc_index = 0;

/** Flag if the VOC index is valid */
bool voc_valid = false;

/** Buffer for debug output */
char errorMessage[256];

/** Counter to discard the first 100 readings */
uint16_t discard_counter = 0;

/**
 * @brief Timer callback to wakeup the loop with the VOC_REQ event
 *
 * @param unused
 */
void voc_read_wakeup(TimerHandle_t unused)
{
	api_wake_loop(VOC_REQ);
}

/**
 * @brief Initialize the sensor
 *
 * @return true success
 * @return false failed
 */
bool init_rak12047(void)
{
	sgp40.begin(Wire);

	uint16_t serialNumber[3];
	uint8_t serialNumberSize = 3;

	uint16_t error = sgp40.getSerialNumber(serialNumber, serialNumberSize);

	if (error)
	{
		errorToString(error, errorMessage, 256);
		MYLOG("VOC", "Error trying to execute getSerialNumber() %s", errorMessage);
		return false;
	}
	else
	{
#if MY_DEBUG > 0
		Serial.print("SerialNumber:");
		Serial.print("0x");
		for (size_t i = 0; i < serialNumberSize; i++)
		{
			uint16_t value = serialNumber[i];
			Serial.print(value < 4096 ? "0" : "");
			Serial.print(value < 256 ? "0" : "");
			Serial.print(value < 16 ? "0" : "");
			Serial.print(value, HEX);
		}
		Serial.println();
#endif
	}

	uint16_t testResult;
	error = sgp40.executeSelfTest(testResult);
	if (error)
	{
		errorToString(error, errorMessage, 256);
		MYLOG("VOC", "Error trying to execute executeSelfTest() %s", errorMessage);
		return false;
	}
	else if (testResult != 0xD400)
	{
		MYLOG("VOC", "executeSelfTest failed with error %d", testResult);
		return false;
	}

	int32_t index_offset;
	int32_t learning_time_offset_hours;
	int32_t learning_time_gain_hours;
	int32_t gating_max_duration_minutes;
	int32_t std_initial;
	int32_t gain_factor;
	voc_algorithm.get_tuning_parameters(
		index_offset, learning_time_offset_hours, learning_time_gain_hours,
		gating_max_duration_minutes, std_initial, gain_factor);

	// Reset discard counter
	discard_counter = 0;

	// Set delayed sending to 1/2 of programmed send interval or 30 seconds
	voc_read_timer.begin(1000, voc_read_wakeup, NULL, true);
	voc_read_timer.start();

	return true;
}

/**
 * @brief Read the last VOC index
 *     Data is added to Cayenne LPP payload as channel
 *     LPP_CHANNEL_VOC
 *
 */
void read_rak12047(void)
{
	MYLOG("VOC", "Get VOC");
	if (voc_valid)
	{
		AT_PRINTF("+EVT:GET_VOC\n");
		MYLOG("VOC", "VOC Index: %ld", voc_index);

		g_solution_data.addVoc_index(LPP_CHANNEL_VOC, voc_index);
	}
	else
	{
		AT_PRINTF("+EVT:VOC_ERROR\n");
		MYLOG("VOC", "No valid VOC available");
	}
}

/**
 * @brief Read the current VOC and feed it to the
 *        VOC algorithm
 *        Called every 1 second
 *
 */
void do_read_rak12047(void)
{
#if MY_DEBUG > 0
	digitalWrite(LED_BLUE, HIGH);
#endif
	uint16_t error;
	float humidity = 0;	   // %RH
	float temperature = 0; // degreeC
	uint16_t srawVoc = 0;
	uint16_t defaultRh = 0x8000;
	uint16_t defaultT = 0x6666;
	float rak1901_values[2] = {0.0};

	if (has_rak1901)
	{
		get_rak1901_values(rak1901_values);
		// MYLOG("VOC", "Rh: %.2f T: %.2f", humidity, temperature);

		if ((temperature != 0.0) && (temperature != 0.0))
		{
			defaultRh = (uint16_t)(humidity * 65535 / 100);
			defaultT = (uint16_t)((temperature + 45) * 65535 / 175);
		}
	}

	// 2. Measure SGP4x signals
	error = sgp40.measureRawSignal(defaultRh, defaultT,
								   srawVoc);
	// MYLOG("VOC", "VOC: %d", srawVoc);

	// 3. Process raw signals by Gas Index Algorithm to get the VOC index values
	if (error)
	{
		errorToString(error, errorMessage, 256);
		MYLOG("VOC", "SGP40 - Error trying to execute measureRawSignals(): %s", errorMessage);
	}
	else
	{
		if (discard_counter <= 100)
		{
			// Discard the first 100 readings
			voc_algorithm.process(srawVoc);
			discard_counter++;
			MYLOG("VOC", "Discard reading %d", discard_counter);
		}
		else if (discard_counter == 101)
		{
			// First accepted reading
			voc_index = voc_algorithm.process(srawVoc);
			discard_counter++;
			MYLOG("VOC", "First good reading: %ld", voc_index);
		}
		else
		{
			uint32_t new_voc_index = voc_algorithm.process(srawVoc);
			voc_index = ((voc_index + new_voc_index) / 2);
		}
		// MYLOG("VOC", "VOC Index: %ld", voc_index);
		voc_valid = true;
	}

#if MY_DEBUG > 0
	digitalWrite(LED_BLUE, LOW);
#endif
}
