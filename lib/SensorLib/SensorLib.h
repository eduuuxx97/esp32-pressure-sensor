#ifndef SENSORLIB_H
#define SENSORLIB_H

#include <Arduino.h>
#include <Adafruit_ADS1X15.h> 
#include "../../include/Config.h"
#include <OneWire.h>
#include <DallasTemperature.h>

class SensorLib {
  private:
    Adafruit_ADS1115 ads1; 
    Adafruit_ADS1115 ads2; 
    float adc_filtrado[NUM_SENSORES];
    float pressoes_finais[NUM_SENSORES];
    bool hw_status; 

    // Instâncias para o sensor de temperatura
    OneWire oneWire;
    DallasTemperature sensorsDS;
    float temperaturaAtual;
    unsigned long ultimaRequisicaoTemp;

  public:
    // No construtor, ligamos a OneWire à DallasTemperature
    SensorLib() : oneWire(27), sensorsDS(&oneWire) {} 

    void begin(); 
    void update(); 
    void recuperarI2C(); 
    float getPressure(int index); 
    bool isHardwareOK() { return hw_status; }
    void getJson(char* buffer, size_t n); 
    
    void updateTemp(); 
    float getTemp() { return temperaturaAtual; } // Corrigido: adicionado '}' e ';'
}; // Corrigido: adicionado ';' após o fechamento da classe

#endif