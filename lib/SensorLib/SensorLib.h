#ifndef SENSORLIB_H
#define SENSORLIB_H

#include <Arduino.h>
#include <Adafruit_ADS1X15.h> 
#include "../../include/Config.h"

class SensorLib {
  private:
    Adafruit_ADS1115 ads1; 
    Adafruit_ADS1115 ads2; 
    float adc_filtrado[NUM_SENSORES];
    float pressoes_finais[NUM_SENSORES];
    bool hw_status; // Status de saúde do I2C

  public:
    void begin(); 
    void update(); 
    void recuperarI2C(); // Camada 2: Força a limpeza do barramento
    float getPressure(int index); 
    bool isHardwareOK() { return hw_status; }
    void getJson(char* buffer, size_t n); 
};

#endif