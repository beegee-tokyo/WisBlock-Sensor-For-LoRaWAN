#include <SPI.h>
#include <SE0352NQ01.h>
#include "CJK16ptB.h"
unsigned char frame[10800];

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
  Serial.println("=====================================");
  Serial.println("Partial Refresh Demo (??)");
  memset(frame, PIC_WHITE, 10800);
  SE0352.fillScreen(PIC_WHITE);
  SE0352.refresh();
  delay(1000);
  SE0352.fillCircle(300, 100, 30, 0, frame);
  SE0352.send(frame);
  SE0352.refresh();
  delay(1000);

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

void loop() {
}

void hexDump(unsigned char *buf, uint16_t len) {
  char alphabet[17] = "0123456789abcdef";
  Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
  Serial.print(F("   |.0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .a .b .c .d .e .f | |      ASCII     |\n"));
  for (uint16_t i = 0; i < len; i += 16) {
    if (i % 128 == 0)
      Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
    char s[] = "|                                                | |                |\n";
    uint8_t ix = 1, iy = 52;
    for (uint8_t j = 0; j < 16; j++) {
      if (i + j < len) {
        uint8_t c = buf[i + j];
        s[ix++] = alphabet[(c >> 4) & 0x0F];
        s[ix++] = alphabet[c & 0x0F];
        ix++;
        if (c > 31 && c < 128) s[iy++] = c;
        else s[iy++] = '.';
      }
    }
    uint8_t index = i / 16;
    if (i < 256) Serial.write(' ');
    Serial.print(index, HEX); Serial.write('.');
    Serial.print(s);
  }
  Serial.print(F("   +------------------------------------------------+ +----------------+\n"));
}
