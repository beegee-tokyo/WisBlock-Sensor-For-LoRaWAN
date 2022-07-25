/**
 * @file RAK14000_epd.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Images for the EPD display
 * @version 0.1
 * @date 2022-06-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */

const unsigned char rak_img[] = {
	// 'RAK-logo, 150x56px
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff,
	0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x07, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x80, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
	0xfc, 0x1f, 0x80, 0x00, 0x00, 0x1f, 0xff, 0xff, 0x80, 0x00, 0x7f, 0xc0, 0x07, 0xf8, 0x01, 0xff,
	0x80, 0x00, 0x03, 0xf8, 0xff, 0xc0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xe0, 0x00, 0x7f, 0xc0, 0x07,
	0xf8, 0x03, 0xff, 0x00, 0x00, 0x03, 0xf3, 0xff, 0xc0, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xf0, 0x00,
	0xff, 0xe0, 0x07, 0xf8, 0x07, 0xfe, 0x00, 0x00, 0x07, 0xe7, 0xff, 0xc8, 0x00, 0x00, 0x1f, 0xff,
	0xff, 0xf0, 0x00, 0xff, 0xe0, 0x07, 0xf8, 0x0f, 0xfc, 0x00, 0x00, 0x07, 0xcf, 0xff, 0xce, 0x00,
	0x00, 0x1f, 0xff, 0xff, 0xf8, 0x00, 0xff, 0xe0, 0x07, 0xf8, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x02,
	0x00, 0x1f, 0x80, 0x00, 0x1f, 0xff, 0xff, 0xf8, 0x01, 0xff, 0xf0, 0x07, 0xf8, 0x3f, 0xf8, 0x00,
	0x00, 0x1c, 0x30, 0x1e, 0x1f, 0xc0, 0x00, 0x1f, 0xe0, 0x0f, 0xf8, 0x01, 0xff, 0xf0, 0x07, 0xf8,
	0x7f, 0xf0, 0x00, 0x0f, 0x3e, 0x79, 0xff, 0xcf, 0xe0, 0x00, 0x1f, 0xe0, 0x07, 0xf8, 0x01, 0xff,
	0xf0, 0x07, 0xf8, 0x7f, 0xe0, 0x00, 0x0f, 0x3e, 0x7d, 0xff, 0xc7, 0xf0, 0x00, 0x1f, 0xe0, 0x03,
	0xfc, 0x03, 0xff, 0xf8, 0x07, 0xf8, 0xff, 0xc0, 0x00, 0x1f, 0x3e, 0xfd, 0xff, 0xc3, 0xf0, 0x00,
	0x1f, 0xe0, 0x03, 0xf8, 0x03, 0xfb, 0xf8, 0x07, 0xf9, 0xff, 0x80, 0x00, 0x1f, 0x3e, 0xfd, 0xff,
	0xc9, 0xf8, 0x00, 0x1f, 0xe0, 0x03, 0xf8, 0x07, 0xfb, 0xfc, 0x07, 0xfb, 0xff, 0x00, 0x00, 0x1f,
	0x3e, 0xfd, 0xff, 0x99, 0xf8, 0x00, 0x1f, 0xe0, 0x07, 0xf8, 0x07, 0xfb, 0xfc, 0x07, 0xff, 0xfe,
	0x00, 0x00, 0x1f, 0x3e, 0xfc, 0x00, 0x1c, 0xfc, 0x00, 0x1f, 0xe0, 0x0f, 0xf8, 0x07, 0xf1, 0xfc,
	0x07, 0xff, 0xfc, 0x00, 0x00, 0x1f, 0xbe, 0xfc, 0x03, 0x9e, 0x7c, 0x00, 0x1f, 0xff, 0xff, 0xf0,
	0x0f, 0xf1, 0xfe, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x0f, 0x9e, 0x7c, 0x07, 0xde, 0x7e, 0x00, 0x1f,
	0xff, 0xff, 0xf0, 0x0f, 0xf1, 0xfe, 0x07, 0xff, 0xff, 0x00, 0x00, 0x0f, 0x9e, 0x7c, 0x07, 0xcf,
	0x7e, 0x00, 0x1f, 0xff, 0xff, 0xe0, 0x0f, 0xe0, 0xfe, 0x07, 0xff, 0xff, 0x00, 0x00, 0x0f, 0xde,
	0x7c, 0x07, 0xcf, 0x3e, 0x00, 0x1f, 0xff, 0xff, 0xc0, 0x1f, 0xe0, 0xff, 0x07, 0xff, 0xff, 0x80,
	0x00, 0x0f, 0xcf, 0x00, 0x07, 0xcf, 0x3e, 0x00, 0x1f, 0xff, 0xff, 0xe0, 0x1f, 0xe0, 0xff, 0x07,
	0xff, 0xff, 0xc0, 0x00, 0x07, 0xe7, 0x00, 0x07, 0xcf, 0x3e, 0x00, 0x1f, 0xff, 0xff, 0xf0, 0x3f,
	0xff, 0xff, 0x87, 0xff, 0x7f, 0xc0, 0x00, 0x07, 0xe6, 0x7f, 0xe7, 0xcf, 0x3e, 0x00, 0x1f, 0xe0,
	0x1f, 0xf8, 0x3f, 0xff, 0xff, 0x87, 0xfe, 0x7f, 0xe0, 0x00, 0x03, 0xf2, 0x7f, 0xe7, 0xcf, 0x3e,
	0x00, 0x1f, 0xe0, 0x0f, 0xf8, 0x3f, 0xff, 0xff, 0x87, 0xfc, 0x3f, 0xf0, 0x00, 0x01, 0xf8, 0xff,
	0xe7, 0xcf, 0x3e, 0x00, 0x1f, 0xe0, 0x07, 0xf8, 0x7f, 0xff, 0xff, 0xc7, 0xf8, 0x1f, 0xf0, 0x00,
	0x01, 0xfc, 0xff, 0xe7, 0xcf, 0x3e, 0x00, 0x1f, 0xe0, 0x07, 0xf8, 0x7f, 0xff, 0xff, 0xc7, 0xf8,
	0x1f, 0xf8, 0x00, 0x00, 0xfe, 0x7f, 0xe7, 0x8f, 0x1c, 0x00, 0x1f, 0xe0, 0x03, 0xf8, 0x7f, 0xff,
	0xff, 0xc7, 0xf8, 0x0f, 0xfc, 0x00, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x03,
	0xf8, 0xff, 0x00, 0x1f, 0xe7, 0xf8, 0x07, 0xfe, 0x00, 0x00, 0x3e, 0x00, 0x38, 0x30, 0x00, 0x00,
	0x1f, 0xe0, 0x03, 0xf8, 0xff, 0x00, 0x1f, 0xe7, 0xf8, 0x07, 0xfe, 0x00, 0x00, 0x0e, 0x7f, 0xfc,
	0xf8, 0x00, 0x00, 0x1f, 0xe0, 0x03, 0xf9, 0xfe, 0x00, 0x1f, 0xf7, 0xf8, 0x03, 0xff, 0x00, 0x00,
	0x00, 0xff, 0xf9, 0xf8, 0x00, 0x00, 0x1f, 0xe0, 0x03, 0xfd, 0xfe, 0x00, 0x0f, 0xf7, 0xf8, 0x01,
	0xff, 0x80, 0x00, 0x00, 0xff, 0xf3, 0xf0, 0x00, 0x00, 0x1f, 0xe0, 0x03, 0xfd, 0xfe, 0x00, 0x0f,
	0xf7, 0xf8, 0x01, 0xff, 0x80, 0x00, 0x00, 0x7f, 0xc7, 0xf0, 0x00, 0x00, 0x1f, 0xe0, 0x03, 0xff,
	0xfc, 0x00, 0x0f, 0xff, 0xf8, 0x00, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x1f,
	0xe0, 0x01, 0xff, 0xfc, 0x00, 0x07, 0xff, 0xf8, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x00, 0x7f, 0xc0,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x3f, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const unsigned char lora_img[] = {
	// 'features_lora_02, 60x40px
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x0f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x38, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x10, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x07, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x20, 0x78, 0x01, 0xfe, 0x0f, 0xff, 0x00, 0x00, 0x20,
	0x78, 0x03, 0x03, 0x8f, 0xff, 0x80, 0x00, 0x00, 0x78, 0x00, 0x00, 0x0f, 0xff, 0xc0, 0x00, 0x00,
	0x78, 0x00, 0x78, 0x0f, 0x03, 0xc1, 0xe0, 0x00, 0x78, 0x01, 0xfe, 0x0f, 0x03, 0xc7, 0xfc, 0x00,
	0x78, 0x03, 0xff, 0x0f, 0x03, 0xcf, 0xfc, 0x00, 0x78, 0x07, 0xc7, 0x8f, 0x07, 0xcf, 0x1c, 0x00,
	0x78, 0x07, 0x83, 0x8f, 0xff, 0x80, 0x1c, 0x00, 0x78, 0x07, 0x03, 0xcf, 0xff, 0x01, 0xfc, 0x00,
	0x78, 0x07, 0x03, 0xcf, 0xfe, 0x07, 0xfc, 0x00, 0x78, 0x07, 0x03, 0xcf, 0x1e, 0x0f, 0x9c, 0x00,
	0x78, 0x07, 0x83, 0x8f, 0x0f, 0x0e, 0x1c, 0x00, 0x7f, 0xe7, 0x87, 0x8f, 0x0f, 0x8e, 0x3c, 0x00,
	0x7f, 0xf3, 0xff, 0x8f, 0x07, 0x8f, 0xfc, 0x00, 0x7f, 0xf1, 0xff, 0x0f, 0x07, 0xcf, 0xfe, 0x00,
	0x7f, 0xe0, 0x7c, 0x0f, 0x03, 0xc3, 0xcc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x0f, 0xc7, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x38, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x01, 0xf8, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x0f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
