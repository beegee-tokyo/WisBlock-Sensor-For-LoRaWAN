#include "Rak_BMX160.h"


void RAK_BMX160::InterruptConfig(uint8_t interrupt_enable, uint8_t interrupt_high_th)
{
  // set low Interrupt
  writeBmxReg(BMX160_INT_ENABLE_1_ADDR, interrupt_enable); // The register controls which interrupt engines are enabled

  writeBmxReg(BMX160_INT_OUT_CTRL_ADDR, 0xA0); // The register contains the behavioral configuration (electrical definition of the interrupt pins

  //  writeBmxReg(BMX160_INT_MAP_0_ADDR, 0x01); //

  writeBmxReg(BMX160_INT_MAP_2_ADDR, 0x02); //

  //  writeBmxReg(BMX160_INT_LOWHIGH_0_ADDR, 0x07); // INT_LOWHIGH[0] contains the delay time definition for the low-g interrupt([int_low_dur<7:0> + 1] • 2.5 ms)
  //
  //  writeBmxReg(BMX160_INT_LOWHIGH_1_ADDR, 0x80); // INT_LOWHIGH[1] contains the threshold definition for the low-g interrupt(Val(int_low_th<7:0>) • 7.81 mg)

  writeBmxReg(BMX160_INT_LOWHIGH_2_ADDR, 0x81); // INT_LOWHIGH[2] contains the low-g interrupt mode selection, the low-g interrupt hysteresis setting, and the high-g interrupt hysteresis setting.

  // set high interrupt
  writeBmxReg(BMX160_INT_LOWHIGH_3_ADDR, 0x0B); //  INT_LOWHIGH[3] contains the delay time definition for the high-g interrupt([int_high_dur<7:0>  + 1] • 2.5 ms)

  writeBmxReg(BMX160_INT_LOWHIGH_4_ADDR, interrupt_high_th); //  INT_LOWHIGH[4] contains the threshold definition for the high-g interrupt((int_high_th<7:0>) · 7.81 mg)
}

int32_t RAK_BMX160::getTemperature(float *temp)
{
  uint8_t data[2];
  uint16_t rawTemp;
  uint8_t temAddress = 0x20;
  readReg(temAddress, data, 2);

  rawTemp = ((data[1] << 8) | data[0]);
  if (rawTemp & 0x8000)
  {
    *temp = (23.0F - ((0x10000 - rawTemp) / 512.0F));
  }
  else
  {
    *temp = ((rawTemp / 512.0F) + 23.0F);
  }
  return 0;
}


void RAK_BMX160::ODR_Config(uint8_t accelCfg_odr, uint8_t gyroCfg_odr)
{

  accelCfg_odr |= 0x20;
  gyroCfg_odr  |= 0x20;

  writeBmxReg(BMX160_ACCEL_CONFIG_ADDR, accelCfg_odr); //default:0x28

  writeBmxReg(BMX160_GYRO_CONFIG_ADDR, gyroCfg_odr);  //default:0x28
}


void RAK_BMX160::get_ORD_Config(float *accelCfg_odr, float *gyroCfg_odr)
{
  uint8_t Data[2] = {0};
  readReg(BMX160_ACCEL_CONFIG_ADDR, &Data[0], 1);
  readReg(BMX160_GYRO_CONFIG_ADDR, &Data[1], 1);
  
  int16_t odr[2] = {0};
  odr[0] = 8 - (Data[0] & 0x0F);

  *accelCfg_odr = 100 / pow(2, odr[0]);

  odr[1] = 8 - (Data[1] & 0x0F);
  *gyroCfg_odr = 100 / pow(2, odr[1]);
}
