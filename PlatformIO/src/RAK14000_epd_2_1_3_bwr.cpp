/**
 * @file RAK14000_epd_2_1_3.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialization and functions for EPD display
 * @version 0.1
 * @date 2022-06-25
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#if HAS_EPD == 2 || HAS_EPD == 3

#include <Adafruit_GFX.h>
#include <Adafruit_EPD.h>
#include "RAK14000_epd_gfx.h"

#define POWER_ENABLE WB_IO2
#define EPD_MOSI MOSI
#define EPD_MISO -1 // not use
#define EPD_SCK SCK
#define EPD_CS SS
#define EPD_DC WB_IO1
#define SRAM_CS -1	 // not use
#define EPD_RESET -1 // not use
#define EPD_BUSY WB_IO4

#define LEFT_BUTTON WB_IO6
#define MIDDLE_BUTTON WB_IO5
#define RIGHT_BUTTON WB_IO3

typedef struct DEPG
{
	int width;
	int height;
	int position1_x;
	int position1_y;
	int position2_x;
	int position2_y;
	int position3_x;
	int position3_y;
	int position4_x;
	int position4_y;
} DEPG;

#if HAS_EPD == 3
DEPG DEPG_HP = {250, 122, 40, 20, 40, 30, 40, 50, 90, 40}; // this is for DEPG0213RWS800F41HP as default B/W/R
#else
DEPG DEPG_HP = {250, 122, 30, 15, 30, 25, 30, 45, 80, 30}; // this is for DEPG0213BNS800F42HP B/W
#endif
// DEPG DEPG_HP = {400, 300, 30, 15, 30, 25, 30, 45, 80, 30}; //  this is for DEPG0420BNS19AF4 B/W

// 2.13" EPD with SSD1680
Adafruit_SSD1680 display(DEPG_HP.width, DEPG_HP.height, EPD_MOSI,
						 EPD_SCK, EPD_DC, EPD_RESET,
						 EPD_CS, SRAM_CS, EPD_MISO,
						 EPD_BUSY);

enum
{
	DISP_ALL = 0,
	DISP_VOC,
	DISP_TEMP,
	DISP_HUMID,
	DISP_BARO
};

/** Set num_values to 1/4 of the display width */
const uint16_t num_values = 400 / 4;
uint16_t voc_values[num_values] = {0};
float temp_values[num_values] = {0.0};
float humid_values[num_values] = {0.0};
float baro_values[num_values] = {0.0};
float co2_values[num_values] = {0.0};
uint8_t voc_idx = 0;
uint8_t temp_idx = 0;
uint8_t humid_idx = 0;
uint8_t baro_idx = 0;
uint8_t co2_idx = 0;

char disp_text[60];

uint8_t display_content = DISP_ALL;

bool button_event = false;

bool show_status = false;

uint16_t bg_color = EPD_WHITE;
uint16_t txt_color = EPD_BLACK;

#if defined NRF52_SERIES || defined ESP32
/** EPD task handle */
TaskHandle_t epd_task_handle;

/** Semaphore for EPD display update */
SemaphoreHandle_t g_epd_sem;

/** Required for Semaphore from ISR */
static BaseType_t xHigherPriorityTaskWoken = pdTRUE;

/** Task declaration */
void epd_task(void *pvParameters);
#endif
#ifdef ARDUINO_ARCH_RP2040
/** The event handler thread */
Thread epd_task_handle(osPriorityNormal, 4096);

/** Thread id for lora event thread */
osThreadId epd_task_id = NULL;

/** Task declaration */
void epd_task(void);
#endif

char *bws[] = {(char *)"125", (char *)"250", (char *)"500", (char *)"062", (char *)"041", (char *)"031", (char *)"020", (char *)"015", (char *)"010", (char *)"007"};
char *regions[] = {(char *)"AS923", (char *)"AU915", (char *)"CN470", (char *)"CN779",
				   (char *)"EU433", (char *)"EU868", (char *)"KR920", (char *)"IN865",
				   (char *)"US915", (char *)"AS923-2", (char *)"AS923-3", (char *)"AS923-4", (char *)"RU864"};

void rak14000_text(int16_t x, int16_t y, char *text, uint16_t text_color, uint32_t text_size);

void butt_left_int(void)
{
	detachInterrupt(LEFT_BUTTON);
	MYLOG("EPD", "Left Button");
	uint16_t switch_color = bg_color;
	bg_color = txt_color;
	txt_color = switch_color;
	button_event = true;

	wake_rak14000();
}

void butt_mid_int(void)
{
	detachInterrupt(MIDDLE_BUTTON);
	MYLOG("EPD", "Middle Button");
	if (display_content == DISP_ALL)
	{
		show_status = true;
	}
	else
	{
		display_content = DISP_ALL;
	}
	button_event = true;

	wake_rak14000();
}

void butt_right_int(void)
{
	detachInterrupt(RIGHT_BUTTON);
	MYLOG("EPD", "Right Button");
	if (display_content == DISP_BARO)
	{
		display_content = DISP_ALL;
	}
	else
	{
		display_content++;
	}
	button_event = true;

	wake_rak14000();
}

void init_rak14000(void)
{
	pinMode(POWER_ENABLE, INPUT_PULLUP);
	digitalWrite(POWER_ENABLE, HIGH);

	// set left button interrupt
	pinMode(LEFT_BUTTON, INPUT);
	attachInterrupt(LEFT_BUTTON, butt_left_int, FALLING);

	// set middle button interrupt
	pinMode(MIDDLE_BUTTON, INPUT);
	attachInterrupt(MIDDLE_BUTTON, butt_mid_int, FALLING);

	// set right button interrupt
	pinMode(RIGHT_BUTTON, INPUT);
	attachInterrupt(RIGHT_BUTTON, butt_right_int, FALLING);

#if defined NRF52_SERIES || defined ESP32
	// Create the EPD event semaphore
	g_epd_sem = xSemaphoreCreateBinary();
	// Initialize semaphore
	xSemaphoreGive(g_epd_sem);
	// Take semaphore
	xSemaphoreTake(g_epd_sem, 10);
#endif

#ifdef ARDUINO_ARCH_RP2040
	epd_task_handle.start(epd_task);
	epd_task_handle.set_priority(osPriorityNormal);
#endif
#if defined NRF52_SERIES || defined ESP32
	if (!xTaskCreate(epd_task, "EPD", 4096, NULL, TASK_PRIO_LOW, &epd_task_handle))
	{
		MYLOG("EPD", "Failed to start EPD task");
	}
#endif
	MYLOG("EPD", "Initialized 2.13\" display");
}

void wake_rak14000(void)
{
	digitalWrite(POWER_ENABLE, HIGH);

#if defined NRF52_SERIES || defined ESP32
	xSemaphoreGiveFromISR(g_epd_sem, &xHigherPriorityTaskWoken);
#endif
#ifdef ARDUINO_ARCH_RP2040
	if (epd_task_id != NULL)
	{
		osSignalSet(epd_task_id, 0x1);
	}
#endif
}

/**
   @brief Write a text on the display
   @param x x position to start
   @param y y position to start
   @param text text to write
   @param text_color color of text
   @param text_size size of text
*/
void rak14000_text(int16_t x, int16_t y, char *text, uint16_t text_color, uint32_t text_size)
{
	display.setCursor(x, y);
	display.setTextColor(text_color);
	display.setTextSize(text_size);
	display.setTextWrap(false);
	display.print(text);
}

void clear_rak14000(void)
{
	display.clearBuffer();
	display.fillRect(0, 0, DEPG_HP.width, DEPG_HP.height, bg_color);
}

bool first_time = true;

void refresh_rak14000(void)
{
	digitalWrite(POWER_ENABLE, HIGH);
	delay(250);

	if (show_status)
	{
		show_status = false;

		// Clear display buffer
		clear_rak14000();

		status_rak14000();
		display.display(true);

		button_event = false;
		attachInterrupt(MIDDLE_BUTTON, butt_mid_int, FALLING);
		return;
	}

	if (found_sensors[VOC_ID].found_sensor)
	{
		if (voc_idx == 0)
		{
			display.fillRect(0, 0, DEPG_HP.width, DEPG_HP.height, bg_color);
			display.drawBitmap(DEPG_HP.position1_x, DEPG_HP.position1_y, rak_img, 150, 56, txt_color);
			rak14000_text(DEPG_HP.position1_x, DEPG_HP.position1_y + 50, (char *)"IoT Made Easy", txt_color, 2);
			display.display(true);

			button_event = false;
			attachInterrupt(LEFT_BUTTON, butt_left_int, FALLING);
			attachInterrupt(MIDDLE_BUTTON, butt_mid_int, FALLING);
			attachInterrupt(RIGHT_BUTTON, butt_right_int, FALLING);

			digitalWrite(POWER_ENABLE, LOW);

			return;
		}
	}
	// Clear display buffer
	clear_rak14000();

	switch (display_content)
	{
	case DISP_ALL:
		display.clearBuffer();
		voc_rak14000();
		co2_rak14000(false);
		temp_rak14000(false);
		humid_rak14000(false);
		baro_rak14000(false);
		status_general_rak14000(false);
		display.drawLine(DEPG_HP.width / 2 + 10, 0, DEPG_HP.width / 2 + 10, DEPG_HP.height, txt_color);
		display.drawLine(0, DEPG_HP.height / 3, DEPG_HP.width, DEPG_HP.height / 3, txt_color);
		display.drawLine(0, DEPG_HP.height / 3 * 2, DEPG_HP.width, DEPG_HP.height / 3 * 2, txt_color);
		break;
	case DISP_VOC:
		voc_rak14000();
		break;
	case DISP_TEMP:
		temp_rak14000(true);
		break;
	case DISP_HUMID:
		humid_rak14000(true);
		break;
	case DISP_BARO:
		baro_rak14000(true);
		break;
	}
	display.display(true);

	if (button_event)
	{
		button_event = false;
		attachInterrupt(LEFT_BUTTON, butt_left_int, FALLING);
		attachInterrupt(MIDDLE_BUTTON, butt_mid_int, FALLING);
		attachInterrupt(RIGHT_BUTTON, butt_right_int, FALLING);
	}

	digitalWrite(POWER_ENABLE, LOW);
}

void set_voc_rak14000(uint16_t voc_value)
{
	MYLOG("EPD", "VOC set to %d at index %d", voc_value, voc_idx);
	// Shift values if necessary
	if (voc_idx == num_values)
	{
		for (int idx = 0; idx < (num_values - 1); idx++)
		{
			voc_values[idx] = voc_values[idx + 1];
		}
		voc_idx = (num_values - 1);
	}

	// Fill VOC array
	voc_values[voc_idx] = voc_value;

	// Increase index
	voc_idx++;
}

void set_temp_rak14000(float temp_value)
{
	MYLOG("EPD", "Temp set to %.2f at index %d", temp_value, temp_idx);
	// Shift values if necessary
	if (temp_idx == num_values)
	{
		for (int idx = 0; idx < (num_values - 1); idx++)
		{
			temp_values[idx] = temp_values[idx + 1];
		}
		temp_idx = (num_values - 1);
	}

	// Fill Temperature array
	temp_values[temp_idx] = temp_value;

	// Increase index
	temp_idx++;
}

void set_humid_rak14000(float humid_value)
{
	MYLOG("EPD", "Humid set to %.2f at index %d", humid_value, humid_idx);
	// Shift values if necessary
	if (humid_idx == num_values)
	{
		for (int idx = 0; idx < (num_values - 1); idx++)
		{
			humid_values[idx] = humid_values[idx + 1];
		}
		humid_idx = (num_values - 1);
	}

	// Fill VOC array
	humid_values[humid_idx] = humid_value;

	// Increase index
	humid_idx++;
}

void set_co2_rak14000(float co2_value)
{
	MYLOG("EPD", "CO2 set to %.2f at index %d", co2_value, co2_idx);
	// Shift values if necessary
	if (co2_idx == num_values)
	{
		for (int idx = 0; idx < (num_values - 1); idx++)
		{
			co2_values[idx] = co2_values[idx + 1];
		}
		co2_idx = (num_values - 1);
	}

	// Fill VOC array
	co2_values[co2_idx] = co2_value;

	// Increase index
	co2_idx++;
}

void set_baro_rak14000(float baro_value)
{
	MYLOG("EPD", "Baro set to %.2f at index %d", baro_value, baro_idx);
	// Shift values if necessary
	if (baro_idx == num_values)
	{
		for (int idx = 0; idx < (num_values - 1); idx++)
		{
			baro_values[idx] = baro_values[idx + 1];
		}
		baro_idx = (num_values - 1);
	}

	// Fill Barometer array
	baro_values[baro_idx] = baro_value;

	// Increase index
	baro_idx++;
}

void voc_rak14000(void)
{
	uint16_t x_text = 2;
	uint16_t y_text = 1;
	uint16_t s_text = 2;
	uint16_t w_text = DEPG_HP.width / 2;
	uint16_t h_text = DEPG_HP.height / 2;
	uint16_t h_bar = DEPG_HP.height / 2 - 40;

	uint16_t use_txt_color = txt_color;
#if HAS_EPD == 3
	if (voc_values[voc_idx - 1] > 250)
	{
		use_txt_color = EPD_RED;
	}
#endif
	// Write value
	display.fillRect(x_text, y_text, w_text, h_text, bg_color);
	if (found_sensors[VOC_ID].found_sensor)
	{
		snprintf(disp_text, 29, "VOC Index");
	}
	else if (found_sensors[ENV_ID].found_sensor)
	{
		snprintf(disp_text, 29, "IAQ Index");
	}
	else
	{
		snprintf(disp_text, 29, "-----");
	}
	rak14000_text(x_text, y_text, disp_text, use_txt_color, s_text);
	snprintf(disp_text, 29, "%d", voc_values[voc_idx - 1]);
	rak14000_text(x_text, y_text + 20, disp_text, use_txt_color, s_text);
}

void co2_rak14000(bool has_pm)
{
	uint16_t x_text = DEPG_HP.width / 2 + 13;
	uint16_t y_text = 1;
	uint16_t s_text = 2;
	uint16_t w_text = DEPG_HP.width / 2;
	uint16_t h_text = DEPG_HP.height / 2;
	uint16_t h_bar = DEPG_HP.height / 2 - 40;

	uint16_t use_txt_color = txt_color;
#if HAS_EPD == 3
	if (co2_values[co2_idx - 1] > 2.0)
	{
		use_txt_color = EPD_RED;
	}
#endif
	// Write value
	display.fillRect(x_text, y_text, w_text, h_text, bg_color);
	if (found_sensors[CO2_ID].found_sensor)
	{
		snprintf(disp_text, 29, "CO2");
	}
	else
	{
		snprintf(disp_text, 29, "RAK10703");
	}
	rak14000_text(x_text, y_text, disp_text, use_txt_color, s_text);
	if (found_sensors[CO2_ID].found_sensor)
	{
		snprintf(disp_text, 29, "%.2f %%", co2_values[co2_idx - 1]);
		rak14000_text(x_text, y_text + 20, disp_text, use_txt_color, s_text);
	}
}

void temp_rak14000(bool has_pm)
{
	uint16_t x_text = 2;
	uint16_t y_text = DEPG_HP.height / 3 + 3;
	uint16_t s_text = 2;
	uint16_t w_text = DEPG_HP.width / 2;
	uint16_t h_text = DEPG_HP.height / 3;

	uint16_t use_txt_color = txt_color;
#if HAS_EPD == 3
	if (temp_values[temp_idx - 1] > 40.0)
	{
		use_txt_color = EPD_RED;
	}
#endif
	// Write value
	display.fillRect(x_text, y_text, w_text, h_text, bg_color);
	snprintf(disp_text, 29, "Temperature");
	rak14000_text(x_text, y_text, disp_text, use_txt_color, s_text);
	snprintf(disp_text, 29, "%.2f %cC", temp_values[temp_idx - 1], (char)247);
	rak14000_text(x_text, y_text + 20, disp_text, use_txt_color, s_text);
}

void humid_rak14000(bool has_pm)
{
	uint16_t x_text = DEPG_HP.width / 2 + 13;
	uint16_t y_text = DEPG_HP.height / 3 + 3;
	uint16_t s_text = 2;
	uint16_t w_text = DEPG_HP.width / 2;
	uint16_t h_text = DEPG_HP.height / 3;
	uint16_t h_bar = DEPG_HP.height / 3 - 40;

	uint16_t use_txt_color = txt_color;
#if HAS_EPD == 3
	if (humid_values[humid_idx - 1] > 60.0)
	{
		use_txt_color = EPD_RED;
	}
#endif

	// Write value
	display.fillRect(x_text, y_text, w_text, h_text, bg_color);
	snprintf(disp_text, 29, "Humidity");
	rak14000_text(x_text, y_text, disp_text, use_txt_color, s_text);
	snprintf(disp_text, 29, "%.2f %%RH", humid_values[humid_idx - 1]);
	rak14000_text(x_text, y_text + 20, disp_text, use_txt_color, s_text);
}

void baro_rak14000(bool has_pm)
{
	uint16_t x_text = 2;
	uint16_t y_text = DEPG_HP.height / 3 * 2 + 3;
	uint16_t s_text = 2;
	uint16_t w_text = DEPG_HP.width / 2;
	uint16_t h_text = DEPG_HP.height / 3;

	// Write value
	display.fillRect(x_text, y_text, w_text, h_text, bg_color);
	snprintf(disp_text, 29, "Barometer");
	rak14000_text(x_text, y_text, disp_text, txt_color, s_text);
	snprintf(disp_text, 29, "%.0f mBar", baro_values[baro_idx - 1]);
	rak14000_text(x_text, y_text + 20, disp_text, txt_color, s_text);
}

void status_general_rak14000(bool has_pm)
{
	uint16_t x_pos = DEPG_HP.width / 2 + 13;
	uint16_t y_pos = DEPG_HP.height / 3 * 2 + 3;

	double batt_val = read_batt() / 1000.0;
	for (int idx = 0; idx < 5; idx++)
	{
		batt_val += read_batt() / 1000.0;
		batt_val = batt_val / 2;
	}

	uint16_t use_txt_color = txt_color;
#if HAS_EPD == 3
	if (batt_val < 3.6)
	{
		use_txt_color = EPD_RED;
	}
#endif

	if (found_sensors[RTC_ID].found_sensor)
	{
		snprintf(disp_text, 59, "Battery: %.2f V", batt_val);
		rak14000_text(x_pos, y_pos, disp_text, use_txt_color, 1);
		y_pos = y_pos + 10;
		read_rak12002();

		snprintf(disp_text, 59, "%d/%d/%d %02d:%02d", g_date_time.date, g_date_time.month, g_date_time.year,
				 g_date_time.hour, g_date_time.minute);
		rak14000_text(x_pos, y_pos, disp_text, use_txt_color, 1);
	}
	else
	{
		snprintf(disp_text, 59, "Battery");
		rak14000_text(x_pos, y_pos, disp_text, use_txt_color, 2);
		y_pos = y_pos + 20;
		snprintf(disp_text, 59, "%.2f V", batt_val);
		rak14000_text(x_pos, y_pos, disp_text, use_txt_color, 2);
	}
	return;
}

void status_rak14000(void)
{
	uint16_t y_pos = 0;
	uint16_t x_pos = 1;

	rak14000_text(x_pos, y_pos, (char *)"RAK10702 Air Quality", txt_color, 2);
	y_pos = y_pos + 20;
#ifdef NRF52_SERIES
	snprintf(disp_text, 59, (char *)"Model RAK4631");
#endif
#ifdef ESP32
	snprintf(disp_text, 59, (char *)"Model RAK11200");
#endif
#ifdef ARDUINO_ARCH_RP2040
	snprintf(disp_text, 59, (char *)"Model RAK11310");
#endif
	rak14000_text(x_pos, y_pos, disp_text, txt_color, 1);
	y_pos = y_pos + 10;
	snprintf(disp_text, 59, "Battery Level: %.2f", read_batt() / 1000.0);
	rak14000_text(x_pos, y_pos, disp_text, txt_color, 1);
	y_pos = y_pos + 10;
	snprintf(disp_text, 59, "Send Int: %ld s", g_lorawan_settings.send_repeat_time / 1000);
	rak14000_text(x_pos, y_pos, disp_text, txt_color, 1);
	y_pos = y_pos + 15;
	if (g_lorawan_settings.lorawan_enable)
	{
		rak14000_text(x_pos, y_pos, (char *)"Mode: LoRaWAN", txt_color, 1);
		snprintf(disp_text, 59, "Region: %s", regions[g_lorawan_settings.lora_region]);
		rak14000_text(x_pos + DEPG_HP.width / 2, y_pos, disp_text, txt_color, 1);
		y_pos = y_pos + 10;
		snprintf(disp_text, 59, "DR: %d", g_lorawan_settings.data_rate);
		rak14000_text(x_pos, y_pos, disp_text, txt_color, 1);
		snprintf(disp_text, 59, "Join mode: %s", g_lorawan_settings.otaa_enabled ? "OTAA" : "ABP");
		rak14000_text(x_pos + DEPG_HP.width / 2, y_pos, disp_text, txt_color, 1);
		y_pos = y_pos + 10;
		snprintf(disp_text, 59, "Confirmed mode: %s", g_lorawan_settings.confirmed_msg_enabled == LMH_CONFIRMED_MSG ? "ON" : "OFF");
		rak14000_text(x_pos, y_pos, disp_text, txt_color, 1);
		snprintf(disp_text, 59, "Join status: %s", g_lpwan_has_joined ? "Joined" : "Not Joined");
		rak14000_text(x_pos + DEPG_HP.width / 2, y_pos, disp_text, txt_color, 1);
		y_pos = y_pos + 10;
	}
	else
	{
		rak14000_text(x_pos, y_pos, (char *)"Mode: LoRa P2P", txt_color, 1);
		snprintf(disp_text, 59, "Frequency: %ld Hz", g_lorawan_settings.p2p_frequency);
		rak14000_text(x_pos + DEPG_HP.width / 2, y_pos, disp_text, txt_color, 1);
		y_pos = y_pos + 10;
		snprintf(disp_text, 59, "Bandwidth: %s khZ", bws[g_lorawan_settings.p2p_bandwidth]);
		rak14000_text(x_pos, y_pos, disp_text, txt_color, 1);
		snprintf(disp_text, 59, "SF: %d", g_lorawan_settings.p2p_sf);
		rak14000_text(x_pos + DEPG_HP.width / 2, y_pos, disp_text, txt_color, 1);
		y_pos = y_pos + 10;
	}
	y_pos = y_pos + 5;
	rak14000_text(x_pos, y_pos, (char *)"Sensors", txt_color, 1);
	y_pos = y_pos + 10;
	if (found_sensors[ENV_ID].found_sensor)
	{
		rak14000_text(x_pos, y_pos, (char *)"RAK1906 Sensor", txt_color, 1);
		if (x_pos < DEPG_HP.width / 2)
		{
			x_pos = DEPG_HP.width / 2;
		}
		else
		{
			x_pos = 1;
			y_pos = y_pos + 10;
		}
	}
	if (found_sensors[TEMP_ID].found_sensor)
	{
		rak14000_text(x_pos, y_pos, (char *)"RAK1901 Sensor", txt_color, 1);
		if (x_pos < DEPG_HP.width / 2)
		{
			x_pos = DEPG_HP.width / 2;
		}
		else
		{
			x_pos = 1;
			y_pos = y_pos + 10;
		}
	}
	if (found_sensors[VOC_ID].found_sensor)
	{
		rak14000_text(x_pos, y_pos, (char *)"RAK12047 Sensor", txt_color, 1);
		if (x_pos < DEPG_HP.width / 2)
		{
			x_pos = DEPG_HP.width / 2;
		}
		else
		{
			x_pos = 1;
			y_pos = y_pos + 10;
		}
	}
	if (found_sensors[CO2_ID].found_sensor)
	{
		rak14000_text(x_pos, y_pos, (char *)"RAK12037 Sensor", txt_color, 1);
		if (x_pos < DEPG_HP.width / 2)
		{
			x_pos = DEPG_HP.width / 2;
		}
		else
		{
			x_pos = 1;
			y_pos = y_pos + 10;
		}
	}
	if (found_sensors[SCT31_ID].found_sensor)
	{
		rak14000_text(x_pos, y_pos, (char *)"RAK12008 Sensor", txt_color, 1);
		if (x_pos < DEPG_HP.width / 2)
		{
			x_pos = DEPG_HP.width / 2;
		}
		else
		{
			x_pos = 1;
			y_pos = y_pos + 10;
		}
	}
}

/**
 * @brief Task to update the display
 *
 * @param pvParameters unused
 */
#if defined NRF52_SERIES || defined ESP32
void epd_task(void *pvParameters)
#endif
#ifdef ARDUINO_ARCH_RP2040
	void epd_task(void)
#endif
{
	MYLOG("EPD", "EPD Task started");

#ifdef ARDUINO_ARCH_RP2040
	epd_task_id = osThreadGetId();
#endif

	digitalWrite(POWER_ENABLE, HIGH);
	delay(250);

	display.begin();

	display.setRotation(0);
	MYLOG("EPD", "Rotation %d", display.getRotation());

	// Clear display
	display.clearBuffer();

	int16_t txt_x1;
	int16_t txt_y1;
	uint16_t txt_w;
	uint16_t txt_h;

	// Draw Welcome Logo
	display.fillRect(0, 0, DEPG_HP.width, DEPG_HP.height, bg_color);
	display.drawBitmap(DEPG_HP.width / 2 - 75, 5, rak_img, 150, 56, txt_color); // 150x56px

	display.setTextSize(2);
	display.getTextBounds((char *)"IoT Made Easy", 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	rak14000_text(DEPG_HP.width / 2 - (txt_w / 2), 60, (char *)"IoT Made Easy", (uint16_t)txt_color, 2);

	display.setTextSize(1);
	display.getTextBounds((char *)"Wait for connect", 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	rak14000_text(DEPG_HP.width / 2 - (txt_w / 2), 80, (char *)"Wait for connect", (uint16_t)txt_color, 1);

	display.display(false);

	while (1)
	{
#ifdef ARDUINO_ARCH_RP2040
		// Wait for event
		osSignalWait(0x01, osWaitForever);
#endif
#if defined NRF52_SERIES || defined ESP32
		if (xSemaphoreTake(g_epd_sem, portMAX_DELAY) == pdTRUE)
#endif
		{
			digitalWrite(POWER_ENABLE, HIGH);
			delay(250);
			refresh_rak14000();

			digitalWrite(POWER_ENABLE, LOW);
			delay(250);
		}
	}
}
#endif