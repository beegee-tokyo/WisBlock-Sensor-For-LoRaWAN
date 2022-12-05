#include "SE0352NQ01.h"

/*
  @brief Initializes the EPD
  @param None
  @return nothing
*/
SE0352NQ01::SE0352NQ01() {
  pinMode(EPD_EN, OUTPUT);
  digitalWrite(EPD_EN, HIGH);
  pinMode(BUSY_Pin, INPUT);
  pinMode(RES_Pin, OUTPUT);
  pinMode(DC_Pin, OUTPUT);
  pinMode(CS_Pin, OUTPUT);
  pinMode(SCK_Pin, OUTPUT);
  pinMode(SDI_Pin, OUTPUT);
  EPD_Reset();
  EPD_init(); // EPD init
}

/*
  @brief De-initializes the EPD. In reality does nothing for now.
  @param None
  @return nothing
*/
SE0352NQ01::~SE0352NQ01() {
}

/*
  @brief Returns the width, adjusted to orientation.
  @param orientation
  @return width
*/
uint16_t SE0352NQ01::width(uint8_t orientation) {
  if(orientation == 0 || orientation == 2) return 360;
  return 240;
}

/*
  @brief Returns the height, adjusted to orientation.
  @param orientation
  @return nothing
*/
uint16_t SE0352NQ01::height(uint8_t orientation) {
  if(orientation == 0 || orientation == 2) return 240;
  return 360;
}

/*
  @brief Delay. Manufacturer-supplied method.
  @param xus  number of microseconds
  @return nothing
*/
void SE0352NQ01::driver_delay_us(unsigned int xus) {
  // 1 µs
  for (; xus > 1; xus--);
}

/*
  @brief Delay. Manufacturer-supplied method.
  @param xus  number of milliseconds
  @return nothing
*/
void SE0352NQ01::driver_delay_xms(unsigned long xms) {
  // 1 ms
  unsigned long i = 0, j = 0;
  for (j = 0; j < xms; j++) {
    for (i = 0; i < 256; i++);
  }
}

/*
  @brief Delay. Manufacturer-supplied method.
  @param xus  number of seconds
  @return nothing
*/
void SE0352NQ01::DELAY_S(unsigned int delaytime) {
  // 1s
  int i, j, k;
  for (i = 0; i < delaytime; i++) {
    for (j = 0; j < 4000; j++) {
      for (k = 0; k < 222; k++);
    }
  }
}

/*
  @brief Delay. Manufacturer-supplied method.
  @param xus  number of minutes
  @return nothing
*/
void SE0352NQ01::DELAY_M(unsigned int delaytime) {
  // 1 mn
  int i;
  for (i = 0; i < delaytime; i++)
    DELAY_S(60);
}

/*
  @brief Initializes the EPD
  @param None
  @return nothing
*/
void SE0352NQ01::EPD_init(void) {
  LUT_Flag = 0;
#if 1
  EPD_W21_WriteCMD(0x00); // panel setting PSR
  EPD_W21_WriteDATA(0xFF); // RES1 RES0 REG KW/R UD SHL SHD_N RST_N
  EPD_W21_WriteDATA(0x01); // x x x VCMZ TS_AUTO TIGE NORG VC_LUTZ
  EPD_W21_WriteCMD(0x01); // POWER SETTING PWR
  EPD_W21_WriteDATA(0x03); // x x x x x x VDS_EN VDG_EN
  EPD_W21_WriteDATA(0x10); // x x x VCOM_SLWE VGH[3:0] VGH=20V, VGL=-20V
  EPD_W21_WriteDATA(0x3F); // x x VSH[5:0] VSH = 15V
  EPD_W21_WriteDATA(0x3F); // x x VSL[5:0] VSL=-15V
  EPD_W21_WriteDATA(0x03); // OPTEN VDHR[6:0] VHDR=6.4V
  // T_VDS_OFF[1:0] 00=1 frame; 01=2 frame; 10=3 frame; 11=4 frame
  EPD_W21_WriteCMD(0x06); // booster soft start BTST
  EPD_W21_WriteDATA(0x37); // BT_PHA[7:0]
  EPD_W21_WriteDATA(0x3D); // BT_PHB[7:0]
  EPD_W21_WriteDATA(0x3D); // x x BT_PHC[5:0]
  EPD_W21_WriteCMD(0x60); // TCON setting TCON
  EPD_W21_WriteDATA(0x22); // S2G[3:0] G2S[3:0] non-overlap = 12
  EPD_W21_WriteCMD(0x82); // VCOM_DC setting VDCS
  EPD_W21_WriteDATA(0x07); // x VDCS[6:0] VCOM_DC value= -1.9v 00~3f, 0x12=-1.9v
  EPD_W21_WriteCMD(0x30);
  EPD_W21_WriteDATA(0x09);
  EPD_W21_WriteCMD(0xe3); // power saving PWS
  EPD_W21_WriteDATA(0x88); // VCOM_W[3:0] SD_W[3:0]
  EPD_W21_WriteCMD(0x61); // resoultion setting
  EPD_W21_WriteDATA(0xf0); // HRES[7:3] 0 0 0
  EPD_W21_WriteDATA(0x01); // x x x x x x x VRES[8]
  EPD_W21_WriteDATA(0x68); // VRES[7:0]
  EPD_W21_WriteCMD(0X50);
  EPD_W21_WriteDATA(0xB7); // Border
  EPD_W21_WriteCMD(0x50);
  EPD_W21_WriteDATA(0xD7);
#endif
}

/*
  @brief Refreshes the EPD
  @param None
  @return nothing
*/
void SE0352NQ01::refresh(void) {
  EPD_W21_WriteCMD(0x17); // DISPLAY REFRESH
  EPD_W21_WriteDATA(0xA5);
  lcd_chkstatus();
}

/*
  @brief Sleeps the EPD
  @param None
  @return nothing
*/
void SE0352NQ01::sleep(void) {
  EPD_W21_WriteCMD(0X07); // deep sleep
  EPD_W21_WriteDATA(0xA5);
}

/*
  @brief Sends the 5S LUT to the EPD
  @param None
  @return nothing
*/
void SE0352NQ01::lut_5S(void) {
  unsigned int count;
  EPD_W21_WriteCMD(0x20); // vcom
  for (count = 0; count < 56; count++) {
    EPD_W21_WriteDATA(lut_vcom[count]);
  }
  EPD_W21_WriteCMD(0x21); // red not used
  for (count = 0; count < 42; count++) {
    EPD_W21_WriteDATA(lut_ww[count]);
  }
  EPD_W21_WriteCMD(0x24); // wb w
  for (count = 0; count < 56; count++) {
    EPD_W21_WriteDATA(lut_bb[count]);
  }
  if (LUT_Flag == 0) {
    EPD_W21_WriteCMD(0x22); // bb b
    for (count = 0; count < 56; count++) {
      EPD_W21_WriteDATA(lut_bw[count]);
    }
    EPD_W21_WriteCMD(0x23); // bw r
    for (count = 0; count < 56; count++) {
      EPD_W21_WriteDATA(lut_wb[count]);
    }
    LUT_Flag = 1;
  } else {
    EPD_W21_WriteCMD(0x23); // bb b
    for (count = 0; count < 56; count++) {
      EPD_W21_WriteDATA(lut_bw[count]);
    }
    EPD_W21_WriteCMD(0x22); // bw r
    for (count = 0; count < 56; count++) {
      EPD_W21_WriteDATA(lut_wb[count]);
    }
    LUT_Flag = 0;
  }
}

/*
  @brief Sends the GC LUT to the EPD. This is the preferred LUT.
  @param None
  @return nothing
*/
void SE0352NQ01::lut_GC(void) {
  /*
      It is recommended to call GC waveform to refresh the screen for normal use.
  */
  unsigned int count;
  EPD_W21_WriteCMD(0x20); // vcom
  for (count = 0; count < 56; count++) {
    EPD_W21_WriteDATA(lut_R20_GC[count]);
  }
  EPD_W21_WriteCMD(0x21); // red not use
  for (count = 0; count < 42; count++) {
    EPD_W21_WriteDATA(lut_R21_GC[count]);
  }
  EPD_W21_WriteCMD(0x24); // bb b
  for (count = 0; count < 56; count++) {
    EPD_W21_WriteDATA(lut_R24_GC[count]);
  }
  if (LUT_Flag == 0) {
    EPD_W21_WriteCMD(0x22); // bw r
    for (count = 0; count < 56; count++) {
      EPD_W21_WriteDATA(lut_R22_GC[count]);
    }
    EPD_W21_WriteCMD(0x23); // wb w
    for (count = 0; count < 56; count++) {
      EPD_W21_WriteDATA(lut_R23_GC[count]);
    }
    LUT_Flag = 1;
  } else {
    EPD_W21_WriteCMD(0x22); // bw r
    for (count = 0; count < 56; count++) {
      EPD_W21_WriteDATA(lut_R23_GC[count]);
    }
    EPD_W21_WriteCMD(0x23); // wb w
    for (count = 0; count < 56; count++) {
      EPD_W21_WriteDATA(lut_R22_GC[count]);
    }
    LUT_Flag = 0;
  }
}

/*
  @brief Sends the DU LUT to the EPD. Faster refresh.
  @param None
  @return nothing
*/
void SE0352NQ01::lut_DU(void) {
  /*
      If you use DU waveform to refresh the screen too many times, there will be low shadows.
      It is recommended to use GC waveform to refresh the screen every 5~10 times after calling DU waveform.
  */
  unsigned int count;
  EPD_W21_WriteCMD(0x20); // vcom
  for (count = 0; count < 56; count++) {
    EPD_W21_WriteDATA(lut_R20_DU[count]);
  }
  EPD_W21_WriteCMD(0x21); // red not use
  for (count = 0; count < 42; count++) {
    EPD_W21_WriteDATA(lut_R21_DU[count]);
  }
  EPD_W21_WriteCMD(0x24); // bb b
  for (count = 0; count < 56; count++) {
    EPD_W21_WriteDATA(lut_R24_DU[count]);
  }
  if (LUT_Flag == 0) {
    EPD_W21_WriteCMD(0x22); // bw r
    for (count = 0; count < 56; count++) {
      EPD_W21_WriteDATA(lut_R22_DU[count]);
    }
    EPD_W21_WriteCMD(0x23); // wb w
    for (count = 0; count < 56; count++) {
      EPD_W21_WriteDATA(lut_R23_DU[count]);
    }
    LUT_Flag = 1;
  } else {
    EPD_W21_WriteCMD(0x22); // bw r
    for (count = 0; count < 56; count++) {
      EPD_W21_WriteDATA(lut_R23_DU[count]);
    }
    EPD_W21_WriteCMD(0x23); // wb w
    for (count = 0; count < 56; count++) {
      EPD_W21_WriteDATA(lut_R22_DU[count]);
    }
    LUT_Flag = 0;
  }
}

/*
  @brief Sends a full buffer to the EPD, then does a full refresh.
  @param picData, 10,800 bytes
  @return nothing
*/
void SE0352NQ01::send(uint8_t* picData) {
  PIC_display1(picData);
  lut_GC();
}

/*
  @brief Sends a full buffer to the EPD, then does a faster refresh.
  @param picData, 10,800 bytes
  @return nothing
*/
void SE0352NQ01::send_DU(uint8_t* picData) {
  PIC_display1(picData);
  lut_DU();
}

/*
  @brief Sends a full buffer to the EPD.
  @param picData, 10,800 bytes
  @return nothing
*/
void SE0352NQ01::PIC_display1(uint8_t* picData) {
  unsigned int i;
  EPD_W21_WriteCMD(0x13); // Transfer new data
  for (i = 0; i < (Gate_Pixel * Source_Pixel / 8); i++) {
    EPD_W21_WriteDATA(*picData);
    picData++;
  }
}

/*
  @brief Fills the screen with the same color, then does a full refresh.
  @param NUM: PIC_WHITE or PIC_BLACK
  @return nothing
*/
void SE0352NQ01::fillScreen(uint8_t NUM) {
  PIC_display(NUM);
  lut_GC();
}

/*
  @brief Sends a full screen's worth of a single color to the EPD.
  @param NUM: PIC_WHITE or PIC_BLACK
  @return nothing
*/
void SE0352NQ01::PIC_display(uint8_t NUM) {
  unsigned int row, column;
  EPD_W21_WriteCMD(0x13); // Transfer new data
  for (column = 0; column < Gate_Pixel; column++) {
    for (row = 0; row < Source_Pixel / 8; row++) {
      switch (NUM) {
        case PIC_WHITE:
          EPD_W21_WriteDATA(0xFF);
          break;
        case PIC_BLACK:
          EPD_W21_WriteDATA(0x00);
          break;
        default:
          break;
      }
    }
  }
}

/*
  @brief Waits while EPD is busy.
  @param none
  @return nothing
*/
void SE0352NQ01::lcd_chkstatus(void) {
  while (isEPD_W21_BUSY == 0);
}

/*
  @brief Waits for Key input. Not used here.
  @param none
  @return nothing
*/
void SE0352NQ01::KEY_Scan(void) {
  // uint8_t KEY;
  // do {
  // KEY = KEY0;
  // driver_delay_xms(2);
  // }
  // while(KEY);
  // driver_delay_xms(20);
}

/*
  @brief Resets the EPD.
  @param none
  @return nothing
*/
void SE0352NQ01::EPD_Reset(void) {
  EPD_W21_RST_1;
  driver_delay_xms(10); // At least 10 ms delay
  EPD_W21_RST_0; // Module reset
  driver_delay_xms(100); // At least 100 ms delay
  EPD_W21_RST_1;
  driver_delay_xms(100); // At least 10 ms delay
}

/*
  @brief Writes a single byte to the EPD via SPI.
  @param value
  @return nothing
*/
void SE0352NQ01::SPI_Write(uint8_t value) {
  uint8_t i;
  for (i = 0; i < 8; i++) {
    EPD_W21_CLK_0;
    if (value & 0x80) EPD_W21_MOSI_1;
    else EPD_W21_MOSI_0;
    value = (value << 1);
    EPD_W21_CLK_1;
  }
}

/*
  @brief Sends a command to the EPD via SPI.
  @param value
  @return nothing
*/
void SE0352NQ01::EPD_W21_WriteCMD(uint8_t command) {
  EPD_W21_CS_0;
  EPD_W21_DC_0; // command write
  SPI_Write(command);
  EPD_W21_CS_1;
  EPD_W21_DC_1;
}

/*
  @brief Sends data to the EPD via SPI.
  @param data
  @return nothing
*/
void SE0352NQ01::EPD_W21_WriteDATA(uint8_t data) {
  EPD_W21_MOSI_0;
  EPD_W21_CS_0;
  EPD_W21_DC_1; // data write
  SPI_Write(data);
  EPD_W21_CS_1;
  EPD_W21_DC_1;
  EPD_W21_MOSI_0;
}

/*
  @brief Draws a horizontal line.
  @param x0 start, x position
  @param y0 start, y position
  @param x1 end, x position
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return nothing
*/
void SE0352NQ01::drawHLine(uint16_t x0, uint16_t y0, uint16_t x1, uint8_t rotation, uint8_t *buffer) {
  uint16_t x2, x3;
  if (x0 > x1) {
    x2 = x1; x3 = x0 + 1;
  } else {
    x2 = x0; x3 = x1 + 1;
  }
  for (int16_t x = x2; x < x3; x++) {
    setPixel(x, y0, rotation, buffer);
  }
}

/*
  @brief Draws a vertical line.
  @param x0 start, x position
  @param y0 start, y position
  @param y1 end, y position
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return nothing
*/
void SE0352NQ01::drawVLine(uint16_t x0, uint16_t y0, uint16_t y1, uint8_t rotation, uint8_t *buffer) {
  uint16_t y2, y3;
  if (y0 > y1) {
    y2 = y1; y3 = y0 + 1;
  } else {
    y2 = y0; y3 = y1 + 1;
  }
  for (int16_t y = y2; y < y3; y++) {
    setPixel(x0, y, rotation, buffer);
  }
}

/*
  @brief Draws a line.
  @param x0 start, x position
  @param y0 start, y position
  @param x1 end, x position
  @param y1 end, y position
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return nothing
*/
void SE0352NQ01::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t rotation, uint8_t *buffer) {
  if (x0 == x1 && y0 == y1) {
    // one pixel
    setPixel(x0, y0, rotation, buffer);
    return;
  }
  if (x0 == x1) {
    // vertical line
    drawVLine(x0, y0, y1, rotation, buffer);
    return;
  }
  if (y0 == y1) {
    // horizontal line
    drawVLine(x0, y0, x1, rotation, buffer);
    return;
  }
  uint16_t x2, x3, y2, y3;
  if (x0 > x1) {
    x2 = x1; x3 = x0 + 1;
    y2 = y1; y3 = y0 + 1;
  } else {
    x2 = x0; x3 = x1 + 1;
    y2 = y0; y3 = y1 + 1;
  }
  int16_t dx = x3 - x2;
  int16_t dy = y3 - y2;
  int16_t yi = 1;
  if (dy < 0) {
    yi = -1;
    dy = -dy;
  }
  int16_t D = (2 * dy) - dx;
  int16_t y = y2;
  x3 += 1;
  for (int16_t x = x2; x < x3; x++) {
    setPixel(x, y, rotation, buffer);
    if (D > 0) {
      y = y + yi;
      D = D + (2 * (dy - dx));
    } else D = D + 2 * dy;
  }
}

void SE0352NQ01::drawCirclePoints(uint16_t xc, uint16_t yc, uint16_t x, uint16_t y, uint8_t rotation, uint8_t *buffer) {
  setPixel(xc + x, yc + y, rotation, buffer);
  setPixel(xc - x, yc + y, rotation, buffer);
  setPixel(xc + x, yc - y, rotation, buffer);
  setPixel(xc - x, yc - y, rotation, buffer);
  setPixel(xc + y, yc + x, rotation, buffer);
  setPixel(xc - y, yc + x, rotation, buffer);
  setPixel(xc + y, yc - x, rotation, buffer);
  setPixel(xc - y, yc - x, rotation, buffer);
}

void SE0352NQ01::drawFillCircle(uint16_t xc, uint16_t yc, uint16_t r, uint8_t rotation, uint8_t *buffer, uint8_t fill) {
  int16_t x = 0, y = r;
  int16_t d = 3 - 2 * r;
  if (fill == 0) drawCirclePoints(xc, yc, x, y, rotation, buffer);
  else fillCirclePoints(xc, yc, x, y, rotation, buffer);
  while (y >= x) {
    x++;
    // check for decision parameter
    // and correspondingly update d, x, y
    if (d > 0) {
      y--;
      d = d + 4 * (x - y) + 10;
    } else d = d + 4 * x + 6;
    if (fill == 0) drawCirclePoints(xc, yc, x, y, rotation, buffer);
    else fillCirclePoints(xc, yc, x, y, rotation, buffer);
  }
}

void SE0352NQ01::fillCirclePoints(uint16_t xc, uint16_t yc, uint16_t x, uint16_t y, uint8_t rotation, uint8_t *buffer) {
  drawHLine(xc - x, yc + y, xc + x, rotation, buffer);
  drawHLine(xc - x, yc - y, xc + x, rotation, buffer);
  drawHLine(xc - y, yc + x, xc + y, rotation, buffer);
  drawHLine(xc - y, yc - x, xc + y, rotation, buffer);
}

/*
  @brief Fills a circle
  @param xc center point, x position
  @param yc center point, y position
  @param r radius
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return nothing
*/
void SE0352NQ01::fillCircle(uint16_t xc, uint16_t yc, uint16_t r, uint8_t rotation, uint8_t *buffer) {
  drawFillCircle(xc, yc, r, rotation, buffer, 1);
}

/*
  @brief Draws a circle
  @param xc center point, x position
  @param yc center point, y position
  @param r radius
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return nothing
*/
void SE0352NQ01::drawCircle(uint16_t xc, uint16_t yc, uint16_t r, uint8_t rotation, uint8_t *buffer) {
  drawFillCircle(xc, yc, r, rotation, buffer, 0);
}

/*
  @brief Draws a rectangle
  @param x0 start, x position
  @param y0 start, y position
  @param x1 end, x position
  @param y1 end, y position
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return nothing
*/
void SE0352NQ01::drawRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t rotation, uint8_t *buffer) {
  uint16_t fx0, fx1, fy0, fy1;
  if (x1 > x0) {
    fx0 = x1; fx1 = x0;
  } else {
    fx1 = x1; fx0 = x0;
  }
  if (y1 > y0) {
    fy0 = y1; fy1 = y0;
  } else {
    fy1 = y1; fy0 = y0;
  }
  drawHLine(fx0, fy0, fx1, rotation, buffer);
  drawHLine(fx0, fy1, fx1, rotation, buffer);
  drawVLine(fx0, fy0, fy1, rotation, buffer);
  drawVLine(fx1, fy0, fy1, rotation, buffer);
}

/*
  @brief Like fillRect, but in white
  @param x0 start, x position
  @param y0 start, y position
  @param x1 end, x position
  @param y1 end, y position
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return nothing
*/
void SE0352NQ01::clearRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t rotation, uint8_t *buffer) {
  uint16_t fx0, fx1, fy0, fy1, x, y;
  if (x1 < x0) {
    fx0 = x1; fx1 = x0 + 1;
  } else {
    fx1 = x1 + 1; fx0 = x0;
  }
  if (y1 < y0) {
    fy0 = y1; fy1 = y0 + 1;
  } else {
    fy1 = y1 + 1; fy0 = y0;
  }
  for (y = fy0; y < fy1; y++) {
    for (x = fx0; x < fx1; x++) {
      clearPixel(x, y, rotation, buffer);
    }
  }
}

/*
  @brief Fills a rectangle
  @param x0 start, x position
  @param y0 start, y position
  @param x1 end, x position
  @param y1 end, y position
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return nothing
*/
void SE0352NQ01::fillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t rotation, uint8_t *buffer) {
  uint16_t fx0, fx1, fy0, fy1, x, y;
  if (x1 < x0) {
    fx0 = x1; fx1 = x0 + 1;
  } else {
    fx1 = x1 + 1; fx0 = x0;
  }
  if (y1 < y0) {
    fy0 = y1; fy1 = y0 + 1;
  } else {
    fy1 = y1 + 1; fy0 = y0;
  }
  for (y = fy0; y < fy1; y++) {
    drawHLine(fx0, y, fx1, rotation, buffer);
  }
}

/*
  @brief Draws a polygon from x,y pairs
  @param points array of x,y points
  @param len length of the array
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return nothing
*/
void SE0352NQ01::drawPolygon(uint16_t *points, uint16_t len, uint8_t rotation, uint8_t *buffer) {
  for (uint16_t x = 0; x < len - 1; x++) {
    drawLine(points[x * 2], points[x * 2 + 1], points[x * 2 + 2], points[x * 2 + 3], rotation, buffer);
  }
}

/*
  @brief Clears (ie set to white) a pixel
  @param x x-position
  @param y y-position
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return nothing
*/
void SE0352NQ01::clearPixel(uint16_t x, uint16_t y, uint8_t rotation, uint8_t *buffer) {
  uint16_t x0, y0;
  if (rotation == 0) {
    x0 = y;
    y0 = 359 - x;
  } else if (rotation == 2) {
    x0 = 239 - y;
    y0 = x;
  } else if (rotation == 1) {
    x0 = x;
    y0 = y;
  } else if (rotation == 3) {
    x0 = 239 - x;
    y0 = 359 - y;
  }
  uint16_t bytePos = y0 * 30 + x0 / 8;
  uint8_t n = (x0 % 8); // (7 - (x % 8));
  uint8_t bf = buffer[bytePos];
  buffer[bytePos] = bf | (1 << (7 - n));
}

/*
  @brief Sets (ie to black) a pixel
  @param x x-position
  @param y y-position
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return nothing
*/
void SE0352NQ01::setPixel(uint16_t x, uint16_t y, uint8_t rotation, uint8_t *buffer) {
  uint8_t anders[8] = {
    0b01111111, 0b10111111, 0b11011111, 0b11101111,
    0b11110111, 0b11111011, 0b11111101, 0b11111110
  };
  uint16_t x0, y0;
  if (rotation == 0) {
    x0 = y;
    y0 = 359 - x;
  } else if (rotation == 2) {
    x0 = 239 - y;
    y0 = x;
  } else if (rotation == 1) {
    x0 = x;
    y0 = y;
  } else if (rotation == 3) {
    x0 = 239 - x;
    y0 = 359 - y;
  }
  uint16_t bytePos = y0 * 30 + x0 / 8;
  uint8_t n = (x0 % 8); // (7 - (x % 8));
  uint8_t bf = buffer[bytePos];
  uint8_t af = bf & anders[n];
  buffer[bytePos] = af;
}

/*
  @brief returns the colour of a pixel
  @param x x-position
  @param y y-position
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return colour
*/

uint8_t SE0352NQ01::getPixel(uint16_t x, uint16_t y, uint8_t rotation, uint8_t *buffer) {
  uint8_t anders[8] = {
    0b10000000, 0b01000000, 0b00100000, 0b00010000,
    0b00001000, 0b00000100, 0b00000010, 0b00000001
  };
  uint16_t x0, y0;
  if (rotation == 0) {
    x0 = y;
    y0 = 359 - x;
  } else if (rotation == 2) {
    x0 = 239 - y;
    y0 = x;
  } else if (rotation == 1) {
    x0 = x;
    y0 = y;
  } else if (rotation == 3) {
    x0 = 239 - x;
    y0 = 359 - y;
  }
  uint16_t bytePos = y0 * 30 + x0 / 8;
  uint8_t n = (x0 % 8); // (7 - (x % 8));
  uint8_t bf = buffer[bytePos];
  uint8_t af = bf & anders[n];
  if (af == 0) return PIC_BLACK;
  else return PIC_WHITE;
}

/*
  @brief flood fill
  @param iXseed start x-position
  @param iYseed start y-position
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return nothing
*/
void SE0352NQ01::fillContour(uint16_t iXseed, uint16_t iYseed, uint8_t rotation, uint8_t *buffer) {
  /*
      https://www.rosettacode.org/wiki/Bitmap/Flood_fill#Simple_and_complete_example_in_C89
      fills contour with black border using seed point inside contour and horizontal lines.
      it starts from seed point, saves max right(iXmaxLocal) and max left (iXminLocal) interior points of horizontal line,
      in new line (iY+1 or iY-1) it computes new interior point: iXmidLocal=iXminLocal + (iXmaxLocal-iXminLocal)/2;
      result is stored in _data array : 1D array of 1-bit colors (shades of gray);
      it does not check if index of _data array is good so memory error is possible
  */
  // iYmax depends on rotation: landscape = 240, portrait = 360
  uint16_t iYmax = 240;
  if (rotation == 1 || rotation == 3) iYmax = 360;
  uint16_t iX, /* seed integer coordinate */
           iY = iYseed,
           /* most interior point of line iY */
           iXmidLocal = iXseed,
           /* min and max of interior points of horizontal line iY */
           iXminLocal,
           iXmaxLocal;
  uint16_t i ; /* index of _data array */;
  /* --------- move up --------------- */
  do {
    iX = iXmidLocal;
    /* move to right */
    while (getPixel(iX, iY, rotation, buffer) == PIC_WHITE) {
      setPixel(iX, iY, rotation, buffer);
      iX += 1;
    }
    iXmaxLocal = iX - 1;
    /* move to left */
    iX = iXmidLocal - 1;
    while (getPixel(iX, iY, rotation, buffer) == PIC_WHITE) {
      setPixel(iX, iY, rotation, buffer);
      iX -= 1;
    }
    iXminLocal = iX + 1;
    iY += 1; /* move up */
    iXmidLocal = iXminLocal + (iXmaxLocal - iXminLocal) / 2; /* new iX inside contour */
    if (getPixel(iXmidLocal, iY, rotation, buffer) == PIC_BLACK) break; /* it should not cross the border */
  } while (iY < iYmax);
  /* ------ move down ----------------- */
  iXmidLocal = iXseed;
  iY = iYseed - 1;
  do {
    iX = iXmidLocal;
    /* move to right */
    while (getPixel(iX, iY, rotation, buffer) == PIC_WHITE) {
      setPixel(iX, iY, rotation, buffer);
      iX += 1;
    }
    iXmaxLocal = iX - 1;
    /* move to left */
    iX = iXmidLocal - 1;
    while (getPixel(iX, iY, rotation, buffer) == PIC_WHITE) {
      setPixel(iX, iY, rotation, buffer);
      iX -= 1;
    }
    iXminLocal = iX + 1;
    iY -= 1; /* move down */
    iXmidLocal = iXminLocal + (iXmaxLocal - iXminLocal) / 2; /* new iX inside contour */
    if (getPixel(iXmidLocal, iY, rotation, buffer) == PIC_BLACK) break; /* it should not cross the border */
  } while (0 < iY);
}

/*
  @brief draws a partial image inside the buffer
  @param width image width
  @param height image height
  @param posX top left corner x-position
  @param posY top left corner y-position
  @param buffer the 10,800-byte buffer you are drawing to
  @param bitmap the buffer to the image you are drawing
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @return nothing
*/
void SE0352NQ01::drawBitmap(
  uint8_t width, uint8_t height, uint16_t posX, uint16_t posY,
  uint8_t *buffer, uint8_t *bitmap, uint8_t rotation
) {
  drawBitmap(width, height, posX, posY, 0, 0, 0, buffer, bitmap, rotation);
}

/*
  @brief draws a partial image inside the buffer
  @param width image width
  @param height image height
  @param posX top left corner x-position
  @param posY top left corner y-position
  @param xOffset For fonts. 0 for images.
  @param yOffset For fonts. 0 for images.
  @param bitmapOffset For fonts. 0 for images.
  @param buffer the 10,800-byte buffer you are drawing to
  @param bitmap the buffer to the image you are drawing
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @return nothing
*/
void SE0352NQ01::drawBitmap(
  uint8_t width, uint8_t height, uint16_t posX, uint16_t posY,
  int8_t xOffset, int8_t yOffset, uint16_t bitmapOffset,
  uint8_t *buffer, uint8_t *bitmap, uint8_t rotation
) {
  uint8_t anders[8] = {
    0b01111111, 0b10111111, 0b11011111, 0b11101111,
    0b11110111, 0b11111011, 0b11111101, 0b11111110
  };
  for (uint16_t y = 0; y < height; y++) {
    for (uint16_t x = 0; x < width; x++) {
      uint16_t bitIndex = x + width * y;
      uint16_t myByte = bitIndex >> 3;
      uint16_t bitMask = 0x80 >> (bitIndex & 7);
      uint8_t c = bitmap[myByte + bitmapOffset];
      if (c & bitMask) {
        // Set bit
        uint16_t x0, y0;
        if (rotation == 0) {
          x0 = (posY + yOffset + y);
          y0 = 359 - (posX + x + xOffset);
        } else if (rotation == 2) {
          x0 = 239 - (posY + yOffset + y);
          y0 = (posX + x + xOffset);
        } else if (rotation == 1) {
          x0 = (posX + x + xOffset);
          y0 = (posY + yOffset + y);
        } else if (rotation == 3) {
          x0 = 239 - (posX + x + xOffset);
          y0 = 359 - (posY + yOffset + y);
        }
        uint16_t bytePos = y0 * 30 + x0 / 8;
        uint8_t n = (x0 % 8); // (7 - (x % 8));
        uint8_t bf = buffer[bytePos];
        uint8_t af = bf & anders[n];
        buffer[bytePos] = af;
      }
    }
  }
}

/*
  @brief Draws a string
  @param myStr String as character array
  @param posX Start x position
  @param posY Start y position
  @param myFont GFX font to use
  @param rotation Screen rotation to use
  @param buffer Buffer to draw string to
  @return uint16_t Length of string in pixel. Optional.
*/
uint16_t SE0352NQ01::drawString(char *myStr, uint16_t posX, uint16_t posY, GFXfont myFont, uint8_t rotation, uint8_t* buffer) {
  uint8_t ln = strlen(myStr);
  uint16_t strLen = 0;
  uint16_t right, bottom;
  if (rotation == 0 || rotation == 2) {
    right = 359;
    bottom = 239;
  } else {
    right = 239;
    bottom = 359;
  }
  for (uint8_t i = 0; i < ln; i++) {
    uint8_t c = myStr[i] - 32;
    GFXglyph glyph = myFont.glyph[c];
    uint8_t width = glyph.width;
    uint8_t height = glyph.height;
    int8_t xOffset = glyph.xOffset;
    int8_t yOffset = glyph.yOffset;
    uint16_t bitmapOffset = glyph.bitmapOffset;
    uint8_t xAdvance = glyph.xAdvance;
    uint8_t nb = height * width / 8;
    strLen += xAdvance;
    if (posX + width > right) {
      // overflows the screen --> wrap around
      posX = 0;
      posY += myFont.yAdvance;
      if (posY + yOffset + height > bottom) return strLen; // Stop if we are outside the screen
    }
    if (nb * 8 < height * width) nb += 1;
    drawBitmap(width, height, posX, posY, xOffset, yOffset, bitmapOffset, buffer, myFont.bitmap, rotation);
    posX += xAdvance;
  }
  return strLen;
}

/*
  @brief Calculates a string's width in pixels
  @param myStr String as character array
  @param myFont GFX font to use
  @return uint16_t Length of string in pixel.
*/
uint16_t SE0352NQ01::strWidth(char *myStr, GFXfont myFont) {
  uint8_t ln = strlen(myStr);
  uint16_t strLen = 0;
  for (uint8_t i = 0; i < ln; i++) {
    uint8_t c = myStr[i] - 32;
    GFXglyph glyph = myFont.glyph[c];
    strLen += glyph.xAdvance;
  }
  return strLen;
}

/*
  @brief Draws a Unicode string. Mostly Chinese for now.
  @param myStr String as character array
  @param posX Start x position
  @param posY Start y position
  @param myFont font to use
  @param myIndex sparse index buffer with the glyphs data
  @param myIndexLen Length of the sparse index buffer data
  @param charHeight Character height in pixel
  @param rotation Screen rotation to use
  @param buffer Buffer to draw string to
  @return nothing
*/
void SE0352NQ01::drawUnicode(
  uint16_t *myStr, uint8_t len,
  uint16_t posX, uint16_t posY,
  uint8_t *myFont, uint8_t *myIndex,
  uint16_t myIndexLen, uint8_t charHeight,
  uint8_t rotation, uint8_t* buffer) {
  for (uint8_t zw = 0; zw < len; zw++) {
    get_ch2(myStr[zw], myIndex, myIndexLen, myFont, charHeight);
    uint8_t ln = (next_offs - doff);
    uint8_t w = ln * 8 / myHeight;
    drawBitmap(w, myHeight, posX, posY, 0, 0, doff, buffer, myFont, rotation);
    posX += w;
#ifdef SHOW_OFF_SE0352
    uint8_t lCount = 0;
    for (uint16_t i = doff; i < next_offs; i++) {
      uint8_t c = myFont[i], mask = 0b10000000;
      for (uint8_t x = 0; x < 8; x++) {
        uint8_t mask = 1 << (7 - x);
        uint8_t rslt = c & mask;
        if (rslt != 0) Serial.write('*');
        else Serial.write(' ');
        lCount += 1;
        mask = mask >> 1;
        if (lCount == w) {
          Serial.write('\n');
          lCount = 0;
        }
      }
    }
    Serial.write('\n');
#endif
  }
}

uint16_t SE0352NQ01::bs(const uint8_t *lst, uint16_t sparseLen, uint16_t val) {
  uint16_t low = 0;
  uint16_t high = (sparseLen / 4);
  uint8_t count = 0;
  while (count < 30) {
    uint16_t m = (high - low) / 2 + low;
    uint16_t pos = m * 4;
    uint16_t v = lst[pos] | (lst[pos + 1] << 8);
    if (v == val) {
      v = lst[pos + 2] | (lst[pos + 3] << 8);
      return v;
    }
    if (low > high) {
      Serial.println("Stopping here...");
      return 0;
    }
    if (v < val) {
      low = m + 1;
    } else {
      high = m - 1;
    }
    count += 1;
  }
  return 0;
}

void SE0352NQ01::get_ch2(uint16_t ch, const uint8_t* sparse, uint16_t sparseLen, uint8_t* myFont, uint8_t fHeight) {
  myWidth = 0;
  next_offs = 0;
  next_offs = 0;
  myHeight = 0;
  doff = bs(sparse, sparseLen, ch);
  if (doff == 0) {
    Serial.println("doff is null. Aborting...");
    return;
  }
  myWidth = myFont[doff] | (myFont[doff + 1] << 8);
  doff += 2;
  next_offs = doff + ((myWidth - 1) / 8 + 1) * fHeight;
  myHeight = fHeight;
}

/*
  @brief Refreshes partially the EPD within the rectangle passed as coordinates.
  @param xStart start x-position
  @param yStart start y-position
  @param xEnd end x-position
  @param yEnd end y-position
  @param rotation 0 / 2 landscape, 1 / 3 portrait
  @param buffer the 10,800-byte buffer you are drawing to
  @return nothing
*/
void SE0352NQ01::partialRefresh(
  uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd,
  uint8_t rotation, uint8_t* buffer
) {
  // Serial.printf("Original coordinates: %d:%d to %d:%d\n", xStart, yStart, xEnd, yEnd);
  uint16_t x0, x1, temp, y0, y1;
/*
  Rotating the coordinates to match the buffer, which is in Portrait 1 mode,
  Then enforcing the X axis constraints – X axis of the buffer, not your orientation of choice.
  HRST[7:3] HRED[7:3]
  Horizontal 8-pixel channel bank. (value 00h~1Dh)
  30 possibilities * 8 (last three bits @ 0 = X << 3)
  for HRED, the last 3 bits are set to 1 to match the last bit of the last byte.

  For instance if in rotation mode 0 you want to partial refresh from 10,20 to 109,119,
  you are really refreshing from 20,10.
  But 20 isn't a multiple of 8. So the coordinates that get passed after rotating x and y are 16,349 to 119,250:

    20 & 0b11111000 ==> 16
    10 ==> 359 - 10 ==> 349 (top becomes bottom)
    119 & 0b11111000 = 112 | 0b00000111 ==> 119
    109 ==> 359 - 109 ==> 250 (top becomes bottom)
*/

  if (rotation == 0) {
    // X and Y axis are switched
    // The Y axis is also inverted – as we are rotating 90° CW
    x0 = yStart & 0b111111000;
    x1 = (yEnd & 0b111111000) | 7;
    y0 = (359 - xEnd);
    y1 = (359 - xStart);
  } else if (rotation == 2) {
    // X and Y axis are switched
    // The X axis is also inverted – as we are rotating 90° CCW
    x0 = (239 - yEnd) & 0b111111000;
    x1 = ((239 - yStart) & 0b111111000) | 7;
    y0 = xStart;
    y1 = xEnd;
  } else if (rotation == 1) {
    // Nothing to do except enforce HRST HRED constraints
    x0 = xStart & 0b111111000;
    x1 = (xEnd & 0b111111000) | 0b111;
    y0 = yStart;
    y1 = yEnd;
  } else if (rotation == 3) {
    // Enforce HRST HRED constraints after inverting the X and Y axis,
    // as we are rotating 180°
    x0 = (239 - xEnd) & 0b111111000;
    x1 = ((239 - xStart) & 0b111111000) | 0b111;
    y1 = 359 - yStart;
    y0 = 359 - yEnd;
  }
  // Serial.printf("Coordinates: %d:%d to %d:%d\n\n", x0, y0, x1, y1);
  uint8_t py00, py01, py10, py11;
  py00 = y0 >> 8;
  py01 = y0 & 0xFF;
  py10 = y1 >> 8;
  py11 = y1 & 0xFF;
  SE0352.EPD_W21_WriteCMD(0x91); // Enter partial refresh mode
  SE0352.EPD_W21_WriteCMD(0x90); // Partial refresh data
  SE0352.EPD_W21_WriteDATA((uint8_t)x0); // HRST
  SE0352.EPD_W21_WriteDATA((uint8_t)x1); // HRED
  SE0352.EPD_W21_WriteDATA(py00);
  SE0352.EPD_W21_WriteDATA(py01); // VRST
  SE0352.EPD_W21_WriteDATA(py10);
  SE0352.EPD_W21_WriteDATA(py11); // VRED
  SE0352.EPD_W21_WriteDATA(0x01);
  SE0352.EPD_W21_WriteCMD(0x13);
  for (uint16_t y = y0; y <= y1; y++) {
    // rows
    for (uint16_t x = x0; x <= x1; x += 8) {
      // cols / 8 bits
      uint16_t btPos = y * 30 + (x / 8);
      SE0352.EPD_W21_WriteDATA(buffer[btPos]);
    }
  }
  SE0352.lut_GC();
  SE0352.refresh();
  SE0352.EPD_W21_WriteCMD(0x92); // Exit partial refresh mode
}

SE0352NQ01 SE0352;
