#ifndef RAK_BMX160_H
#define RAK_BMX160_H

#include "DFRobot_BMX160.h"

class RAK_BMX160 : public DFRobot_BMX160
{
  public:
    RAK_BMX160(TwoWire * rWire = & Wire) : DFRobot_BMX160(rWire) {};
    ~RAK_BMX160() {};

    /**
       @fn getTemperature
       @brief get BMX160 Temperature
       @param temp
       @return Temperature data
    */
    int32_t getTemperature(float *temp);

    /**
       @fn InterruptConfig
       @brief set BMX160 Interrupt
       @param interrupt_enable: Enable HIGH_G_Interrupt
       @param interrupt_high_th: set the accelerometer threshold
    */
    void InterruptConfig(uint8_t interrupt_enable, uint8_t interrupt_high_th);

    /**
       @fn ODR_Config
       @brief set BMX160 Output data rate
       @param accelCfg_odr: set accelerometer output data rate
       @param gyroCfg_odr: set  gyroscope output data rate
    */
    void ODR_Config(uint8_t accelCfg_odr, uint8_t gyroCfg_odr);

    /**
       @fn ODR_Config
       @brief get BMX160 Output data rate
       @param accelCfg_odr: get accelerometer output data rate
       @param gyroCfg_odr:  get  gyroscope output data rate
    */
    void get_ORD_Config(float *accelCfg_odr, float *gyroCfg_odr);
};

#endif
