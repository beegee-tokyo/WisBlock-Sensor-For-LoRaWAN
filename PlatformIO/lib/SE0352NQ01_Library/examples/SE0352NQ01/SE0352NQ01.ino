#include <SPI.h>
#include <SE0352NQ01.h>
#include "Ap_29demo.h"
// Include fonts from https://github.com/adafruit/Adafruit-GFX-Library/tree/master/Fonts
#include "FreeSansBold12pt7b.h"
#include "FreeSerifBoldItalic24pt7b.h"
// See README.md for Chinese fonts
#include "CJK14pt.h"
#include "CJK16ptB.h"

unsigned char frame[10800];

void partialDemo() {
  Serial.println("Partial Refresh Demo (??)");
  memset(frame, PIC_WHITE, 10800);
  SE0352.fillScreen(PIC_WHITE);
  SE0352.refresh();
  delay(100);
  SE0352.fillCircle(300, 100, 30, 0, frame);
  SE0352.send(frame);
  SE0352.refresh();
  delay(100);

  Serial.println("partialRefresh 0");
  SE0352.fillCircle(300, 200, 30, 0, frame);
  uint16_t zhongwenyekeyi[] = {0x4e2d, 0x6587, 0x4E5F, 0x53EF, 0x4EE5}; // 中文也可以
  SE0352.drawUnicode(zhongwenyekeyi, 5, 0, 20, CJK16ptBfont, CJK16ptBsparse, CJK16ptBSparseLen, CJK16ptBHeight, 0, frame);
  SE0352.drawUnicode(zhongwenyekeyi, 5, 0, 223, CJK16ptBfont, CJK16ptBsparse, CJK16ptBSparseLen, CJK16ptBHeight, 0, frame);
  for (uint8_t z = 0; z < 5; z++) {
    digitalWrite(PIN_LED2, HIGH);
    SE0352.partialRefresh(z * 16, 16, z * 16 + 15, 39, 0, frame);
    delay(100);
    digitalWrite(PIN_LED2, LOW);
  }
  delay(1000);
  Serial.println("partialRefresh 1");
  digitalWrite(PIN_LED2, HIGH);
  SE0352.partialRefresh(0, 208, 79, 239, 0, frame);
  delay(100);
  digitalWrite(PIN_LED2, LOW);
  delay(1000);

  Serial.println("partialRefresh 2");
  digitalWrite(PIN_LED2, HIGH);
  SE0352.partialRefresh(270, 170, 330, 230, 0, frame);
  delay(100);
  digitalWrite(PIN_LED2, LOW);
  Serial.println("done.");
  digitalWrite(LED_BUILTIN, LOW);
}

void textDemo() {
  Serial.print("Text Demo 1/2");
  memset(frame, PIC_WHITE, 10800);
  // Will wrap around
  SE0352.drawString((char*)"Landscape 0", 300, 30, FreeSansBold12pt7b, 0, frame);
  SE0352.drawString((char*)"Kongduino", 60, 125, FreeSerifBoldItalic24pt7b, 0, frame);
  SE0352.drawString((char*)"Landscape 2", 10, 30, FreeSansBold12pt7b, 2, frame);
  SE0352.drawString((char*)"Portrait 1", 90, 20, FreeSansBold12pt7b, 1, frame);
  SE0352.drawString((char*)"Portrait 3", 10, 20, FreeSansBold12pt7b, 3, frame);
  uint16_t zhongwenyekeyi[] = {0x4e2d, 0x6587, 0x4E5F, 0x53EF, 0x4EE5}; // 中文也可以
  Serial.write('.');
  SE0352.drawUnicode(zhongwenyekeyi, 5, 40, 200, CJK14ptfont, CJK14ptsparse, CJK14ptSparseLen, CJK14ptHeight, 0, frame);
  Serial.write('.');
  SE0352.drawUnicode(zhongwenyekeyi, 5, 40, 160, CJK16ptBfont, CJK16ptBsparse, CJK16ptBSparseLen, CJK16ptBHeight, 0, frame);
  Serial.write('.');
  SE0352.drawUnicode(zhongwenyekeyi, 5, 40, 45, CJK14ptfont, CJK14ptsparse, CJK14ptSparseLen, CJK14ptHeight, 2, frame);
  Serial.write('.');
  SE0352.drawUnicode(zhongwenyekeyi, 5, 40, 65, CJK16ptBfont, CJK16ptBsparse, CJK16ptBSparseLen, CJK16ptBHeight, 2, frame);
  SE0352.send(frame);
  SE0352.refresh();
  Serial.println(" done!");
}

void graphicDemo() {
  Serial.println("Graphic Demo");
  memset(frame, PIC_WHITE, 10800);
  uint16_t px = 2, py = 2;
  Serial.print(" . Thermometer");
  SE0352.drawBitmap(gThermometer_width, gThermometer_height, px, py, 0, 0, 0, frame, gThermometer, 0);
  // In FreeSansBold12pt7b, ` has been replaced with an o, with a bigger yOffset, to make a ° sign cheaply :-)
  Serial.println(" 27*");
  SE0352.drawString((char*)"27`", px + gThermometer_width + 5, (py + gThermometer_height) / 2 + 12, FreeSansBold12pt7b, 0, frame);
  py += gThermometer_height + 10;
  Serial.print(" . Humidity");
  SE0352.drawBitmap(gHumidity_width, gHumidity_height, px, py, 0, 0, 0, frame, gHumidity, 0);
  Serial.println(" 58%");
  SE0352.drawString((char*)"58%", px + gHumidity_width + 5, (py + gHumidity_height) / 1.2, FreeSansBold12pt7b, 0, frame);

  uint16_t x = 120;
  Serial.println(" . Lines");
  SE0352.drawRect(240, 0, 359, 119, 0, frame);
  for (uint16_t x = 3; x < 120; x += 3) {
    SE0352.drawLine(x + 240, 0, 358, x, 0, frame);
    SE0352.drawLine(240, 119 - x, 359 - x, 119, 0, frame);
  }

  Serial.println(" . drawRect");
  SE0352.drawRect(150, py, 190, py + 40, 0, frame);
  Serial.println(" . fillRect");
  SE0352.fillRect(155, py + 5, 185, py + 35, 0, frame);
  Serial.println(" . drawCircle");
  SE0352.drawCircle(60, 189, 50, 0, frame);
  Serial.println(" . drawCircle");
  SE0352.drawCircle(60, 189, 45, 0, frame);
  Serial.println(" . drawCircle");
  SE0352.drawCircle(60, 189, 40, 0, frame);
  Serial.println(" . floodFill");
  SE0352.fillContour(16, 189, 0, frame);
  Serial.println(" . floodFill");
  SE0352.fillContour(104, 189, 0, frame);
  Serial.println(" . fillCircle");
  SE0352.fillCircle(60, 189, 25, 0, frame);

  Serial.println(" . VLines 120 to 220 [139:239] step 2");
  for (x = 120; x < 220; x += 2) SE0352.drawVLine(x, 139, 239, 0, frame);
  Serial.println(" . HLines [229:349] 139 to 239 step 2");
  for (uint16_t y = 139; y < 240; y += 2) SE0352.drawHLine(229, y, 349, 0, frame);

  uint16_t points[] = {
    110, 0, 140, 25,
    140, 50, 110, 75,
    85, 50, 85, 25, 110, 0
  };
  Serial.println(" . drawPolygon");
  SE0352.drawPolygon(points, 7, 0, frame);
  SE0352.send(frame);
  SE0352.refresh();
  delay(1000);
  Serial.println(" . floodFill");
  SE0352.fillContour(110, 50, 0, frame);
  SE0352.partialRefresh(85, 0, 140, 75, 0, frame);
  Serial.println(" done!");
}

void setup() {
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  digitalWrite(PIN_LED1, HIGH);
  // Initialize Serial for debug output
  time_t timeout = millis();
  Serial.begin(115200);
  while (!Serial) {
    if ((millis() - timeout) < 5000) {
      delay(100);
    } else {
      break;
    }
  }
  delay(500);
  delay(500);
  Serial.println("EPD TEST");
  digitalWrite(PIN_LED1, LOW);
}

void loop() {
  digitalWrite(PIN_LED2, HIGH);
  memset(frame, PIC_WHITE, 10800);
  SE0352.fillScreen(PIC_WHITE);
  SE0352.refresh();
  Serial.println("gImage_1");
  SE0352.send(gImage_1);
  SE0352.refresh();
  digitalWrite(PIN_LED2, LOW);
  delay(5000);
  digitalWrite(PIN_LED2, HIGH);
  Serial.println("gImage_Kongduino");
  SE0352.send(gImage_Kongduino);
  SE0352.refresh();
  digitalWrite(PIN_LED2, LOW);
  delay(5000);
  digitalWrite(PIN_LED2, HIGH);
  Serial.println("textDemo");
  textDemo();
  digitalWrite(PIN_LED2, LOW);
  delay(5000);
  digitalWrite(PIN_LED2, HIGH);
  partialDemo();
  digitalWrite(PIN_LED2, LOW);
  delay(5000);
  digitalWrite(PIN_LED2, HIGH);
  graphicDemo();
  digitalWrite(PIN_LED2, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  delay(30000);
}
