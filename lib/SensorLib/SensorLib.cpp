#include "SensorLib.h"

void SensorLib::begin() {
  analogSetAttenuation(ADC_11db);
  for(int i=0; i<NUM_SENSORES; i++) {
    pinMode(PINOS_SENSORES[i], INPUT);
    adc_filtrado[i] = analogRead(PINOS_SENSORES[i]); // Valor inicial
  }
}

void SensorLib::update() {
  for(int i=0; i<NUM_SENSORES; i++) {
    int raw = analogRead(PINOS_SENSORES[i]);
    // Filtro EMA
    adc_filtrado[i] = (ALPHA * raw) + ((1.0 - ALPHA) * adc_filtrado[i]);

    // Conversão
    float tensao = (adc_filtrado[i] * V_REF) / ADC_RES;
    float corrente = (tensao / R_SHUNT) * 1000.0;
    
    if(corrente < 3.5) pressoes_finais[i] = 0.0;
    else if(corrente > 21.0) pressoes_finais[i] = PRESSAO_MAX;
    else pressoes_finais[i] = PRESSAO_MIN + (corrente - 4.0) * (PRESSAO_MAX - PRESSAO_MIN) / 16.0;
  }
}

float SensorLib::getPressure(int index) {
  if(index >= 0 && index < NUM_SENSORES) return pressoes_finais[index];
  return 0.0;
}

String SensorLib::getJson() {
  String json = "{";
  for(int i=0; i<NUM_SENSORES; i++) {
    json += "\"s" + String(i+1) + "\": \"" + String(pressoes_finais[i], 2) + "\"";
    if(i < NUM_SENSORES - 1) json += ", ";
  }
  json += "}";
  return json;
}