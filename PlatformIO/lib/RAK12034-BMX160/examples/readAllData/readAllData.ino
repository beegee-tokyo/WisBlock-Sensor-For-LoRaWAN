/*!
   @file readAllData.ino
   @author rakwireless.com
   @brief Through the example, you can get the sensor data by using getSensorData:
   @n     get all data of magnetometer, gyroscope, accelerometer.
   @n     With the rotation of the sensor, data changes are visible.
   @version 0.1
   @date 2022-01-04
   @copyright Copyright (c) 2022
*/

#include "Rak_BMX160.h"

#define  INT1_PIN   WB_IO1 //if use SLOT A INT1_PIN use WB_IO1,if use SLOT B,the INT1_PIN use WB_IO2. if use SLOT C,the INT1_PIN use WB_IO3.  if use SLOT D,the INT1_PIN use WB_IO5

#define  HIGH_G_INT 0x01    //0x01-accelerometer(X轴高门限中断，0x02-y轴高门限中断，0x04-z轴高门限中断)
#define  HIGH_G_THRESHOLD 0x80 //threshold = (0x80*7.81) mg(2g range), (0x80*15.63 )mg (4g range), (0x80*31.25)mg (8g range),(0x80* 62.5)mg (16g range) 

RAK_BMX160 bmx160;

uint8_t INT1_Flag = 0;

void setup() {
  //Sensor power switch
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  delay(300);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
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

  pinMode(INT1_PIN, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(INT1_PIN), To_INT1_Interrupt, RISING);

  //init the hardware bmx160
  if (bmx160.begin() != true) {
    Serial.println("bmx160 init false");
    while (1) delay(100);
  }

  Serial.println("RAK12034 Test!!");
  //  bmx160.setLowPower();   //disable the gyroscope and accelerometer sensor
  bmx160.wakeUp();        //enable the gyroscope and accelerometer sensor
  //bmx160.softReset();     //reset the sensor

  uint8_t PMU_Status = 0;
  bmx160.readReg(0x03, &PMU_Status, 1);
  Serial.printf("PMU_Status=%x\r\n", PMU_Status);


  bmx160.InterruptConfig(HIGH_G_INT, HIGH_G_THRESHOLD);//Enable HIGH_G_Interrupt ands et the accelerometer threshold

  bmx160.ODR_Config(BMX160_ACCEL_ODR_200HZ, BMX160_GYRO_ODR_200HZ); //set output data rate

  float OrdBuf[2] = {0};
  bmx160.get_ORD_Config(&OrdBuf[0], &OrdBuf[1]);
  Serial.printf("OrdBuf[0]=%f,OrdBuf[1]=%f\r\n", OrdBuf[0], OrdBuf[1]);

  /**
     enum{eGyroRange_2000DPS,
           eGyroRange_1000DPS,
           eGyroRange_500DPS,
           eGyroRange_250DPS,
           eGyroRange_125DPS
           }eGyroRange_t;
   **/
  bmx160.setGyroRange(eGyroRange_500DPS);

  /**
      enum{eAccelRange_2G,
           eAccelRange_4G,
           eAccelRange_8G,
           eAccelRange_16G
           }eAccelRange_t;
  */
  bmx160.setAccelRange(eAccelRange_2G);
  delay(100);
}

void loop() {
  sBmx160SensorData_t Omagn, Ogyro, Oaccel;

  float Temp = 0;
  bmx160.getTemperature(&Temp);
  Serial.print("Temperature:");
  Serial.println(Temp);

  /* Get a new sensor event */
  bmx160.getAllData(&Omagn, &Ogyro, &Oaccel);
  /* Display the magnetometer results (magn is magnetometer in uTesla) */
  Serial.print("M ");
  Serial.print("X: "); Serial.print(Omagn.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(Omagn.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(Omagn.z); Serial.print("  ");
  Serial.println("uT");

  /* Display the gyroscope results (gyroscope data is in °/s) */
  Serial.print("G ");
  Serial.print("X: "); Serial.print(Ogyro.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(Ogyro.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(Ogyro.z); Serial.print("  ");
  Serial.println("°/s");

  /* Display the accelerometer results (accelerometer data is in m/s^2) */
  Serial.print("A ");
  Serial.print("X: "); Serial.print(Oaccel.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(Oaccel.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(Oaccel.z); Serial.print("  ");
  Serial.println("m/s^2");
  Serial.println("");


  if (INT1_Flag)
  {
    Serial.println("INT1 High Interrupt");
    INT1_Flag = 0;
  }

  delay(500);
}

void To_INT1_Interrupt(void)
{
  // Serial.println("INT1 High Interrupt");
  INT1_Flag = 1;
}
