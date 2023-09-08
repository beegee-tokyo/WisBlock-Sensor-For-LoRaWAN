/**
   @file RAK_SGM58031.ino
   @author rakwireless.com
   @brief This code is designed to config SGM58031 ADC device and handle the data
   @version 1.0
   @date 2022-01-19
   @copyright Copyright (c) 2022
*/

#include "ADC_SGM58031.h"

//RAK_ADC_SGM58031 sgm58031;
RAK_ADC_SGM58031 sgm58031(SGM58031_SDA_ADDRESS);
// RAK_ADC_SGM58031 sgm58031(Wire);
// RAK_ADC_SGM58031 sgm58031(Wire,SGM58031_DEFAULT_ADDRESS);

#define ALERT_PIN     WB_IO1  //SlotA installation, please do not use it on SLOTB
//#define ALERT_PIN   WB_IO3  //SlotC installation.
//#define ALERT_PIN   WB_IO5  //SlotD installation.

bool  interrupt_flag = false;
void setup()
{
  // put your setup code here, to run once:
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  delay(300);
  time_t timeout = millis();
  Serial.begin(115200);
  while (!Serial)
  {
    if ((millis() - timeout) < 5000)
    {
      delay(100);
    }
    else
    {
      break;
    }
  }
  sgm58031.begin();
  Serial.println("ADC_SGM58031 TEST");
  if (sgm58031.getChipID() != DEVICE_ID)
  {
    Serial.println("No CHIP found ... please check your connection");
    while (1)
    { 
      delay(100);
    }
  }
  else
  {
    Serial.println("Found SGM58031 Chip");
  }

  pinMode(ALERT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(ALERT_PIN), threshold_interrupt, FALLING);

  sgm58031.setAlertLowThreshold(0x0000);  // Write  0x0000 to Lo_Thresh
  sgm58031.setAlertHighThreshold(0x7FFF); // Write 0x8000 to Hi_Thresh
  sgm58031.setConfig(0xC2E0);             // Write config, OS=1, AIN0 to GND, G=(+/-4.096V input range)
  sgm58031.setVoltageResolution(SGM58031_FS_4_096);
  //continuous mode conversion, DR=800, others default
  //COMP_QUE = 00
  //sgm58031.setConfig1(0x0008);//write Config1, use external Reference
  delay(1000);
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (interrupt_flag)
  {
    interrupt_flag = false;
    Serial.println("The voltage exceeds the preset threshold. Please check");
  }
  uint16_t adcVlaue = sgm58031.getAdcValue();
  Serial.print("adcVlaue=");
  Serial.println(adcVlaue);
  float gVoltage = sgm58031.getVoltage();
  Serial.print(F("gVoltage="));
  Serial.print(gVoltage);
  Serial.println("V");
  Serial.println("");
  delay(1000);
}

void threshold_interrupt(void)
{
  interrupt_flag = true;
}