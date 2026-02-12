#ifndef SENSORLIB_H
#define SENSORLIB_H

#include <Arduino.h>
#include <Adafruit_ADS1X15.h> 
#include "../../include/Config.h"

class SensorLib {
  private:
    Adafruit_ADS1115 ads1; // Módulo 1 (Endereço 0x48)
    Adafruit_ADS1115 ads2; // Módulo 2 (Endereço 0x49)
    
    float adc_filtrado[NUM_SENSORES];
    float pressoes_finais[NUM_SENSORES];

  public:
    void begin(); 
    void update(); 
    float getPressure(int index); 
    
    // Atualizado: agora aceita o buffer e o tamanho, sem retornar String
    void getJson(char* buffer, size_t n); 
};

#endif