/**
 * @file RAK14000_epd.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialization and functions for EPD display
 * @version 0.1
 * @date 2022-06-25
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "app.h"
#if HAS_EPD == 1
#include <Adafruit_GFX.h>
#include <Adafruit_EPD.h>

#include "RAK14000_epd.h"

#define SMALL_FONT &RAK_EPD_10pt
#define LARGE_FONT &RAK_EPD_20pt

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
} DEPG;

// DEPG  DEPG_HP = {250,122};  //use 2.13" DEPG0213RWS800F41HP as default B/W/R
// DEPG  DEPG_HP = {212,104};  //  this is for 2.13" DEPG0213BNS800F42HP B/W
DEPG DEPG_HP = {400, 300}; //  this is for 4.2" DEPG0420BNS19AF4 B/W

// 4.2" EPD with SSD1683
Adafruit_SSD1681 display(DEPG_HP.height, DEPG_HP.width, EPD_MOSI,
						 EPD_SCK, EPD_DC, EPD_RESET,
						 EPD_CS, SRAM_CS, EPD_MISO,
						 EPD_BUSY);

/** Set num_values to 1/4 of the display width */
const uint16_t num_values = 400 / 4;
uint16_t voc_values[num_values] = {0};
float temp_values[num_values] = {0.0};
float humid_values[num_values] = {0.0};
float baro_values[num_values] = {0.0};
float co2_values[num_values] = {0.0};
uint16_t pm10_values[num_values] = {0};
uint16_t pm25_values[num_values] = {0};
uint16_t pm100_values[num_values] = {0};
uint8_t voc_idx = 0;
uint8_t temp_idx = 0;
uint8_t humid_idx = 0;
uint8_t baro_idx = 0;
uint8_t co2_idx = 0;
uint8_t pm_idx = 0;

char disp_text[60];

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

// Forward declaration
void rak14000_text(int16_t x, int16_t y, char *text, uint16_t text_color, uint32_t text_size);

// For text length calculations
int16_t txt_x1;
int16_t txt_y1;
uint16_t txt_w;
uint16_t txt_w2;
uint16_t txt_h;

// For text and image placements
uint16_t x_text;
uint16_t y_text;
uint16_t s_text;
uint16_t w_text;
uint16_t h_text;
uint16_t x_graph;
uint16_t y_graph;
uint16_t h_bar;
uint16_t w_bar;
float bar_divider;
uint16_t spacer;

/**
 * @brief Initialization of RAK14000 EPD
 *
 */
void init_rak14000(void)
{
	pinMode(POWER_ENABLE, INPUT_PULLUP);
	digitalWrite(POWER_ENABLE, HIGH);

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
#endif
	{
		MYLOG("EPD", "Failed to start EPD task");
	}
	MYLOG("EPD", "Initialized 4.2\" display");
}

/**
 * @brief Wake task to handle screen updates
 *
 */
void wake_rak14000(void)
{
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
	if (text_size == 1)
	{
		display.setFont(SMALL_FONT); // Font_5x7_practical8pt7b
		y = y + 7;
	}
	else
	{
		display.setFont(LARGE_FONT);
		y = y + 12;
	}
	display.setCursor(x, y);
	display.setTextColor(text_color);
	display.setTextSize(1);
	display.setTextWrap(false);
	display.print(text);
}

/**
 * @brief Clear display content
 *
 */
void clear_rak14000(void)
{
	display.clearBuffer();
	display.fillRect(0, 0, DEPG_HP.width, DEPG_HP.height, bg_color);
}

/** Flag for first screen update */
bool first_time = true;

char *months_txt[] = {(char *)"Jan", (char *)"Feb", (char *)"Mar", (char *)"Apr", (char *)"May", (char *)"Jun", (char *)"Jul", (char *)"Aug", (char *)"Sep", (char *)"Oct", (char *)"Nov", (char *)"Dec"};

/**
 * @brief Update screen content
 *
 */
void refresh_rak14000(void)
{
	// Clear display buffer
	clear_rak14000();

	voc_rak14000();
	co2_rak14000(found_sensors[PM_ID].found_sensor);
	temp_rak14000(found_sensors[PM_ID].found_sensor);
	humid_rak14000(found_sensors[PM_ID].found_sensor);
	baro_rak14000(found_sensors[PM_ID].found_sensor);

	display.setFont(SMALL_FONT);
	display.setTextSize(1);
	if (found_sensors[RTC_ID].found_sensor)
	{
		read_rak12002();

		if ((found_sensors[PM_ID].found_sensor) || (found_sensors[CO2_ID].found_sensor))
		{
			snprintf(disp_text, 59, "RAK10702   %s %d %d %02d:%02d",
					 months_txt[g_date_time.month - 1], g_date_time.date, g_date_time.year,
					 g_date_time.hour, g_date_time.minute);
		}
		else
		{
			snprintf(disp_text, 59, "RAK10702   %s %d %d %02d:%02d Batt: %.2f V",
					 months_txt[g_date_time.month - 1], g_date_time.date, g_date_time.year,
					 g_date_time.hour, g_date_time.minute,
					 read_batt() / 1000.0);
		}
	}
	else
	{
		if ((found_sensors[PM_ID].found_sensor) || (found_sensors[CO2_ID].found_sensor))
		{
			snprintf(disp_text, 59, "RAK10702 Air Quality");
		}
		else
		{
			snprintf(disp_text, 59, "RAK10702 Air Quality Batt: %.2f V",
					 read_batt() / 1000.0);
		}
	}

	display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	rak14000_text((DEPG_HP.width / 2) - (txt_w / 2), 1, disp_text, (uint16_t)txt_color, 1);

	if (found_sensors[PM_ID].found_sensor)
	{
		pm_rak14000();
		display.drawLine(0, DEPG_HP.height / 2 + 3, DEPG_HP.width / 2 + 50, DEPG_HP.height / 2 + 3, (uint16_t)txt_color);
		display.drawLine(DEPG_HP.width / 2 + 50, DEPG_HP.height / 5, DEPG_HP.width, DEPG_HP.height / 5, (uint16_t)txt_color);
		display.drawLine(DEPG_HP.width / 2 + 50, 10, DEPG_HP.width / 2 + 50, DEPG_HP.height, (uint16_t)txt_color);
	}
	else
	{
		display.drawLine(DEPG_HP.width / 2 + 50, 10, DEPG_HP.width / 2 + 50, DEPG_HP.height, (uint16_t)txt_color);
		display.drawLine(DEPG_HP.width / 2 + 50, DEPG_HP.height / 3, DEPG_HP.width, DEPG_HP.height / 3, (uint16_t)txt_color);
		display.drawLine(DEPG_HP.width / 2 + 50, DEPG_HP.height / 3 * 2, DEPG_HP.width, DEPG_HP.height / 3 * 2, (uint16_t)txt_color);
	}
	// For partial update only
	// if (first_time)
	// {
	// 	first_time = false;
	// 	display.display(true);
	// }
	// return;
	delay(100);
	display.display();
	delay(100);
}

/**
 * @brief Add VOC value to buffer
 *
 * @param voc_value new VOC value
 */
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

/**
 * @brief Add temperature value to buffer
 *
 * @param temp_value new temperature value
 */
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

/**
 * @brief Add humidity value to buffer
 *
 * @param humid_value new humidity value
 */
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

/**
 * @brief Add CO2 value to buffer
 *
 * @param co2_value new CO2 value
 */
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

/**
 * @brief Add barometric pressure to buffer
 *
 * @param baro_value new barometric pressure
 */
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

/**
 * @brief Add PM values to array
 *
 * @param pm10_env new PM 1.0 value
 * @param pm25_env new PM 2.5 value
 * @param pm100_env new PM 10 value
 */
void set_pm_rak14000(uint16_t pm10_env, uint16_t pm25_env, uint16_t pm100_env)
{
	MYLOG("EPD", "PM set to %d %d %d  at index %d", pm10_env, pm25_env, pm100_env, pm_idx);
	// Shift values if necessary
	if (pm_idx == num_values)
	{
		for (int idx = 0; idx < (num_values - 1); idx++)
		{
			pm10_values[idx] = pm10_values[idx + 1];
			pm25_values[idx] = pm25_values[idx + 1];
			pm100_values[idx] = pm100_values[idx + 1];
		}
		pm_idx = (num_values - 1);
	}

	// Fill PM array
	pm10_values[pm_idx] = pm10_env;
	pm25_values[pm_idx] = pm25_env;
	pm100_values[pm_idx] = pm100_env;

	// Increase index
	pm_idx++;
}

/**
 * @brief Update display for VOC values
 *
 * @param full unused
 */
void voc_rak14000(void)
{
	x_text = 2;
	y_text = 10;
	s_text = 2;
	x_graph = 0;
	y_graph = 60;
	h_bar = DEPG_HP.height / 2 - 60;
	w_bar = 2;
	bar_divider = 500.0 / h_bar;

	// Write value
	display.drawBitmap(x_text, y_text, voc_img, 32, 32, txt_color);

	if (!voc_valid)
	{
		snprintf(disp_text, 29, "VOC na");
	}
	else
	{
		if (voc_values[voc_idx - 1] > 400)
		{
			snprintf(disp_text, 29, " !!  VOC %d", voc_values[voc_idx - 1]);
		}
		else if (voc_values[voc_idx - 1] > 250)
		{
			snprintf(disp_text, 29, " !  VOC %d", voc_values[voc_idx - 1]);
		}
		else
		{
			snprintf(disp_text, 29, "VOC %d", voc_values[voc_idx - 1]);
		}
	}
	rak14000_text(x_text + 40, y_text + 20, disp_text, txt_color, s_text);

	rak14000_text(DEPG_HP.width / 2 + 15, y_graph + h_bar - 7, (char *)"0", txt_color, 1);
	rak14000_text(DEPG_HP.width / 2 + 15, y_graph - 7, (char *)"500", txt_color, 1);

	display.drawLine(DEPG_HP.width / 2 + 10, y_graph + h_bar, DEPG_HP.width / 2 + 10, y_graph, (uint16_t)txt_color);
	display.drawLine(DEPG_HP.width / 2 + 5, y_graph + h_bar, DEPG_HP.width / 2 + 10, y_graph + h_bar, (uint16_t)txt_color);
	display.drawLine(DEPG_HP.width / 2 + 5, y_graph, DEPG_HP.width / 2 + 10, y_graph, (uint16_t)txt_color);

	// Draw VOC values
	for (int idx = 0; idx < num_values; idx++)
	{
		display.drawLine((int16_t)(x_graph + (idx * w_bar)),
						 (int16_t)(y_graph + ((h_bar) - (voc_values[idx] / bar_divider))),
						 (int16_t)(x_graph + (idx * w_bar)),
						 (int16_t)(y_graph + h_bar),
						 txt_color);
	}
	display.drawLine(x_graph, y_graph + h_bar, x_graph + DEPG_HP.width / 2, y_graph + h_bar, (uint16_t)txt_color);

	// For partial update only
	// MYLOG("EPD", "Updating x1 %d y1 %d x2 %d y2 %d", x_text, y_text, x_text + w_text, y_text + h_text);
	// display.displayPartial(x_text, y_text, x_text + w_text, y_text + h_text);
}

/**
 * @brief Update display for CO2 values
 *
 * @param full unused
 */
void co2_rak14000(bool has_pm)
{
	if (has_pm)
	{
		x_text = DEPG_HP.width / 2 + 53;
		y_text = 15;
		s_text = 2;
		spacer = 20;

		// Write value
		display.drawBitmap(x_text, y_text, co2_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "ppm");
		display.setFont(SMALL_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		rak14000_text(DEPG_HP.width - txt_w - 1, y_text + spacer + 4, disp_text, (uint16_t)txt_color, 1);

		if (co2_values[co2_idx - 1] > 1500)
		{
			snprintf(disp_text, 29, "!! %.0f", co2_values[co2_idx - 1]);
		}
		else if (co2_values[co2_idx - 1] > 1000)
		{
			snprintf(disp_text, 29, "! %.0f", co2_values[co2_idx - 1]);
		}
		else
		{
			snprintf(disp_text, 29, "%.0f", co2_values[co2_idx - 1]);
		}

		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w2, &txt_h);

		rak14000_text(DEPG_HP.width - txt_w - txt_w2 - 4, y_text + spacer, disp_text, (uint16_t)txt_color, s_text);
	}
	else
	{
		x_text = 2;
		y_text = DEPG_HP.height / 2;
		s_text = 2;
		x_graph = 0;
		y_graph = DEPG_HP.height / 2 + 60;
		h_bar = DEPG_HP.height / 2 - 62;
		w_bar = 2;
		bar_divider = 2500 / h_bar;

		// Get min and max values => maybe adjust graph to the min and max values
		int fmin = 2500;
		int fmax = 0;
		for (int idx = 0; idx < co2_idx; idx++)
		{
			if (co2_values[idx] <= fmin)
			{
				fmin = co2_values[idx];
			}
			if (co2_values[idx] >= fmax)
			{
				fmax = co2_values[idx];
			}
		}
		// give some margin at the top
		fmax += 50;

		// give some margin at the bottom
		if (fmin > 50)
		{
			fmin -= 50;
		}
		// make it an even number
		fmax = ((fmax / 100) + 1) * 100;
		bar_divider = fmax / h_bar;

		MYLOG("EPD", "CO2 min %d max %d", fmin, fmax);

		// Write value
		display.drawBitmap(x_text, y_text, co2_img, 32, 32, txt_color);

		if (co2_values[co2_idx - 1] > 1500)
		{
			snprintf(disp_text, 29, "!!  %.0f", co2_values[co2_idx - 1]);
		}
		else if (co2_values[co2_idx - 1] > 1000)
		{
			snprintf(disp_text, 29, "!  %.0f", co2_values[co2_idx - 1]);
		}
		else
		{
			snprintf(disp_text, 29, "%.0f", co2_values[co2_idx - 1]);
		}
		rak14000_text(x_text + 40, y_text + 20, disp_text, txt_color, s_text);
		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w2, &txt_h);

		rak14000_text(x_text + 40 + txt_w2 + 3, y_text + 24, (char *)"ppm", txt_color, 1);

		sprintf(disp_text, "%d", fmax);
		rak14000_text(DEPG_HP.width / 2 + 15, y_graph + h_bar - 17, (char *)"200", txt_color, 1);
		rak14000_text(DEPG_HP.width / 2 + 15, y_graph + h_bar - 7, (char *)"ppm", txt_color, 1);
		// rak14000_text(DEPG_HP.width / 2 + 15, y_graph + h_bar - 7, (char *)"0ppm", txt_color, 1);
		rak14000_text(DEPG_HP.width / 2 + 15, y_graph - 7, disp_text, txt_color, 1);
		rak14000_text(DEPG_HP.width / 2 + 15, y_graph + 3, (char *)"ppm", txt_color, 1);

		display.drawLine(DEPG_HP.width / 2 + 10, y_graph + h_bar, DEPG_HP.width / 2 + 10, y_graph, (uint16_t)txt_color);
		display.drawLine(DEPG_HP.width / 2 + 5, y_graph + h_bar, DEPG_HP.width / 2 + 10, y_graph + h_bar, (uint16_t)txt_color);
		display.drawLine(DEPG_HP.width / 2 + 5, y_graph, DEPG_HP.width / 2 + 10, y_graph, (uint16_t)txt_color);

		// Draw CO2 values
		for (int idx = 0; idx < num_values; idx++)
		{
			// if (co2_values[idx] != 0.0)
			if (co2_values[idx] >= 200.0)
			{
				display.drawLine((int16_t)(x_graph + (idx * w_bar)),
								//  (int16_t)(y_graph + ((h_bar) - (co2_values[idx] / bar_divider))),
								 (int16_t)(y_graph + ((h_bar) - ((co2_values[idx] - 200) / bar_divider))),
								 (int16_t)(x_graph + (idx * w_bar)),
								 (int16_t)(y_graph + h_bar),
								 txt_color);
			}
		}
		display.drawLine(x_graph, y_graph + h_bar, x_graph + DEPG_HP.width / 2, y_graph + h_bar, (uint16_t)txt_color);

		// For partial update only
		// MYLOG("EPD", "Updating x1 %d y1 %d x2 %d y2 %d", x_text, y_text, x_text + w_text, y_text + h_text);
		// display.displayPartial(x_text, y_text, x_text + w_text, y_text + h_text);
	}
}

/**
 * @brief Update display with particle matter values
 *
 * @param full unused
 */
void pm_rak14000(void)
{
	x_text = DEPG_HP.width / 2 + 53;
	y_text = DEPG_HP.height / 4;
	s_text = 2;

	// Write value
	display.drawBitmap(x_text, y_text, pm_img, 32, 32, txt_color);

	snprintf(disp_text, 29, "PM");
	rak14000_text(x_text + 40, y_text + 20, disp_text, txt_color, s_text);

	// PM 1.0 levels
	if (pm10_values[pm_idx - 1] > 75)
	{
		snprintf(disp_text, 29, "1.0: !!");
	}
	else if (pm10_values[pm_idx - 1] > 35)
	{
		snprintf(disp_text, 29, "1.0: !");
	}
	else
	{
		snprintf(disp_text, 29, "1.0:");
	}
	rak14000_text(x_text, y_text + 60, disp_text, txt_color, s_text);

	snprintf(disp_text, 29, "%d", pm10_values[pm_idx - 1]);
	display.setFont(LARGE_FONT);
	display.setTextSize(1);
	display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	rak14000_text(DEPG_HP.width - txt_w - 45, y_text + 60, disp_text, txt_color, s_text);
	snprintf(disp_text, 29, "%cg/m%c", 0x7F, 0x80);
	rak14000_text(DEPG_HP.width - 38, y_text + 65, disp_text, txt_color, 1);

	// PM 2.5 levels
	if (pm25_values[pm_idx - 1] > 75)
	{
		snprintf(disp_text, 29, "2.5: !!");
	}
	else if (pm25_values[pm_idx - 1] > 35)
	{
		snprintf(disp_text, 29, "2.5: !");
	}
	else
	{
		snprintf(disp_text, 29, "2.5:");
	}
	rak14000_text(x_text, y_text + 120, disp_text, txt_color, s_text);

	snprintf(disp_text, 29, "%d", pm25_values[pm_idx - 1]);
	display.setFont(LARGE_FONT);
	display.setTextSize(1);
	display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	rak14000_text(DEPG_HP.width - txt_w - 45, y_text + 120, disp_text, txt_color, s_text);
	snprintf(disp_text, 29, "%cg/m%c", 0x7F, 0x80);
	rak14000_text(DEPG_HP.width - 38, y_text + 125, disp_text, txt_color, 1);

	// PM 10 levels
	if (pm100_values[pm_idx - 1] > 199)
	{
		snprintf(disp_text, 29, "10: !!");
	}
	else if (pm100_values[pm_idx - 1] > 150)
	{
		snprintf(disp_text, 29, "10: !");
	}
	else
	{
		snprintf(disp_text, 29, "10:");
	}
	rak14000_text(x_text, y_text + 180, disp_text, txt_color, s_text);

	snprintf(disp_text, 29, "%d", pm100_values[pm_idx - 1]);
	display.setFont(LARGE_FONT);
	display.setTextSize(1);
	display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	rak14000_text(DEPG_HP.width - txt_w - 45, y_text + 180, disp_text, txt_color, s_text);
	snprintf(disp_text, 29, "%cg/m%c", 0x7F, 0x80);
	rak14000_text(DEPG_HP.width - 38, y_text + 185, disp_text, txt_color, 1);
}

/**
 * @brief Update display for temperature values
 *
 * @param full unused
 */
void temp_rak14000(bool has_pm)
{
	x_text = 25;
	y_text = DEPG_HP.height / 2 + 10;
	s_text = 2;
	spacer = 60;

	// If PM sensor is not available, position is different
	if (!has_pm)
	{
		x_text = DEPG_HP.width / 2 + 53;
		y_text = 12;
		s_text = 2;
		spacer = 50;

		// Write value
		display.drawBitmap(DEPG_HP.width - (DEPG_HP.width / 4 - 16), y_text, celsius_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "~C");
		display.setFont(SMALL_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		rak14000_text(DEPG_HP.width - txt_w - 3, y_text + spacer + 4, disp_text, (uint16_t)txt_color, 1);

		snprintf(disp_text, 29, "%.2f ", temp_values[temp_idx - 1]);
		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w2, &txt_h);

		rak14000_text(DEPG_HP.width - txt_w - txt_w2 - 6, y_text + spacer, disp_text, (uint16_t)txt_color, s_text);
	}
	else
	{
		// Write value
		display.drawBitmap(x_text, y_text, celsius_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "%.2f", temp_values[temp_idx - 1]);

		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		rak14000_text(x_text + spacer, y_text + 16, disp_text, (uint16_t)txt_color, s_text);

		snprintf(disp_text, 29, "~C");
		rak14000_text(x_text + spacer + txt_w + 4, y_text + 16 + 4, disp_text, (uint16_t)txt_color, 1);
	}
}

/**
 * @brief Update display for humidity values
 *
 * @param full unused
 */
void humid_rak14000(bool has_pm)
{
	uint16_t x_text = 25;
	uint16_t y_text = DEPG_HP.height / 2 + (DEPG_HP.height / 2 / 3) + 10;
	uint16_t s_text = 2;
	uint16_t spacer = 60;

	// If PM sensor is not available, position is different
	if (!has_pm)
	{
		x_text = DEPG_HP.width / 2 + 53;
		y_text = DEPG_HP.height / 3 + 15;
		s_text = 2;
		spacer = 50;

		// Write value
		display.drawBitmap(DEPG_HP.width - (DEPG_HP.width / 4 - 16), y_text, humidity_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "%%RH");
		display.setFont(SMALL_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		rak14000_text(DEPG_HP.width - txt_w - 3, y_text + spacer + 4, disp_text, (uint16_t)txt_color, 1);

		snprintf(disp_text, 29, "%.2f ", humid_values[humid_idx - 1]);
		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w2, &txt_h);

		rak14000_text(DEPG_HP.width - txt_w - txt_w2 - 6, y_text + spacer, disp_text, (uint16_t)txt_color, s_text);
	}
	else
	{
		// Write value
		display.drawBitmap(x_text, y_text, humidity_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "%.2f", humid_values[humid_idx - 1]);

		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		rak14000_text(x_text + spacer, y_text + 16, disp_text, (uint16_t)txt_color, s_text);

		snprintf(disp_text, 29, "%%RH");
		rak14000_text(x_text + spacer + txt_w + 4, y_text + 16 + 4, disp_text, (uint16_t)txt_color, 1);
	}
}

/**
 * @brief Update display for barometric pressure
 *
 * @param full unused
 */
void baro_rak14000(bool has_pm)
{
	x_text = 25;
	y_text = DEPG_HP.height / 2 + (DEPG_HP.height / 2 / 3 * 2) + 10;
	s_text = 2;
	spacer = 60;

	// If PM sensor is not available, position is different
	if (!has_pm)
	{
		x_text = DEPG_HP.width / 2 + 53;
		y_text = DEPG_HP.height / 3 * 2 + 15;
		s_text = 2;
		spacer = 50;

		// Write value
		display.drawBitmap(DEPG_HP.width - (DEPG_HP.width / 4 - 16), y_text, barometer_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "mBar");
		display.setFont(SMALL_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		rak14000_text(DEPG_HP.width - txt_w - 3, y_text + spacer + 4, disp_text, (uint16_t)txt_color, 1);

		snprintf(disp_text, 29, "%.1f ", baro_values[baro_idx - 1]);
		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w2, &txt_h);

		rak14000_text(DEPG_HP.width - txt_w - txt_w2 - 6, y_text + spacer, disp_text, (uint16_t)txt_color, s_text);
	}
	else
	{
		// Write value
		display.drawBitmap(x_text, y_text, barometer_img, 32, 32, txt_color);

		snprintf(disp_text, 29, "%.2f", baro_values[baro_idx - 1]);

		display.setFont(LARGE_FONT);
		display.setTextSize(1);
		display.getTextBounds(disp_text, 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);

		rak14000_text(x_text + spacer, y_text + 16, disp_text, (uint16_t)txt_color, s_text);

		snprintf(disp_text, 29, "mBar");
		rak14000_text(x_text + spacer + txt_w + 4, y_text + 16 + 4, disp_text, (uint16_t)txt_color, 1);
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

	display.begin();

	display.setRotation(3);
	MYLOG("EPD", "Rotation %d", display.getRotation());

	// Clear display
	display.clearBuffer();

	// Draw Welcome Logo
	display.fillRect(0, 0, DEPG_HP.width, DEPG_HP.height, bg_color);
	display.drawBitmap(DEPG_HP.width / 2 - 75, 50, rak_img, 150, 56, txt_color);

	// If RTC is available, write the date
	if (found_sensors[RTC_ID].found_sensor)
	{
		read_rak12002();
		snprintf(disp_text, 59, "%d/%d/%d %d:%d", g_date_time.date, g_date_time.month, g_date_time.year,
				 g_date_time.hour, g_date_time.minute);
		rak14000_text(0, 0, disp_text, (uint16_t)txt_color, 2);
	}

	display.setFont(LARGE_FONT);
	display.setTextSize(1);
	display.getTextBounds((char *)"IoT Made Easy", 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	rak14000_text(DEPG_HP.width / 2 - (txt_w / 2), 110, (char *)"IoT Made Easy", (uint16_t)txt_color, 2);

	display.setFont(LARGE_FONT);
	display.setTextSize(1);
	display.getTextBounds((char *)"RAK10702 Air Quality", 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	rak14000_text(DEPG_HP.width / 2 - (txt_w / 2), 150, (char *)"RAK10702 Air Quality", (uint16_t)txt_color, 2);

	display.drawBitmap(DEPG_HP.width / 2 - 63, 190, built_img, 126, 66, txt_color);

	display.setFont(SMALL_FONT);
	display.setTextSize(1);
	display.getTextBounds((char *)"Wait for connect", 0, 0, &txt_x1, &txt_y1, &txt_w, &txt_h);
	rak14000_text(DEPG_HP.width / 2 - (txt_w / 2), 260, (char *)"Wait for connect", (uint16_t)txt_color, 1);

	display.display(false);

	// For partial update only
	// uint16_t counter = 0;
	while (1)
	{
		// For partial update only
		// display.clearBuffer();
		// display.setTextSize(4);
		// display.setTextColor(EPD_BLACK);
		// display.setCursor(32, 32);
		// display.print((counter / 1000) % 10);
		// display.print((counter / 100) % 10);
		// display.print((counter / 10) % 10);
		// display.print(counter % 10);

		// if ((counter % 10) == 0)
		// {
		// 	MYLOG("EPD", "Full update");
		// 	display.display(false);
		// }
		// else
		// {
		// 	MYLOG("EPD", "Partial update");
		// 	// redraw only 4th digit
		// 	display.displayPartial(32 + (24 * 3), 32, 32 + (24 * 4), 32 + (4 * 8));
		// }

		// MYLOG("EPD", "Counter = %d", counter);
		// counter++;
		// delay(5000);

#ifdef ARDUINO_ARCH_RP2040
		// Wait for event
		osSignalWait(0x01, osWaitForever);
#endif
#if defined NRF52_SERIES || defined ESP32
		if (xSemaphoreTake(g_epd_sem, portMAX_DELAY) == pdTRUE)
#endif
		{
			refresh_rak14000();
			delay(1000);
		}
	}
}
#endif
