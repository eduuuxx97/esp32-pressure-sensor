#ifndef SENSORLIB_H
#define SENSORLIB_H

#include <Arduino.h>
#include "../../include/Config.h" // Importa as configurações

class SensorLib {
  private:
    float adc_filtrado[NUM_SENSORES];
    float pressoes_finais[NUM_SENSORES];

  public:
    void begin(); // Inicializa pinos
    void update(); // Faz a leitura e o filtro
    float getPressure(int index); // Retorna valor pronto
    String getJson(); // Retorna JSON pronto para o site
};

#endif