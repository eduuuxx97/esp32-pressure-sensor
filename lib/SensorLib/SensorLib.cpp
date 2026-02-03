#include "SensorLib.h"

void SensorLib::begin() {
  // Inicializa Módulo 1 (S0, S1, S2, S3)
  if (!ads1.begin(ADS1_ADDR)) {
    Serial.println("ERRO: ADS1 (GND/0x48) nao encontrado!");
  } else {
    Serial.println("ADS1 OK!");
  }

  // Inicializa Módulo 2 (S4, S5)
  if (!ads2.begin(ADS2_ADDR)) {
    Serial.println("ERRO: ADS2 (VCC/0x49) nao encontrado!");
  } else {
    Serial.println("ADS2 OK!");
  }

  // Configura Ganho 1x (+/- 4.096V)
  // Perfeito para ler até 3.3V com precisão máxima
  ads1.setGain(GAIN_ONE);
  ads2.setGain(GAIN_ONE);

  // Zera as variáveis
  for(int i=0; i<NUM_SENSORES; i++) {
    adc_filtrado[i] = 0;
    pressoes_finais[i] = 0;
  }
}

void SensorLib::update() {
  // Loop pelos 6 sensores
  for(int i=0; i<NUM_SENSORES; i++) {
    int16_t raw = 0;
    float tensao = 0.0;

    // --- ROTEAMENTO (Mapeamento que você pediu) ---
    
    if (i < 4) {
      // Índices 0, 1, 2, 3 (Seus S0-S3)
      // Lê do Módulo 1 nas portas A0, A1, A2, A3
      raw = ads1.readADC_SingleEnded(i);
      tensao = ads1.computeVolts(raw);
    } 
    else {
      // Índices 4, 5 (Seus S4-S5)
      // Lê do Módulo 2. 
      // Quando i=4 -> lê porta 0 (A0)
      // Quando i=5 -> lê porta 1 (A1)
      raw = ads2.readADC_SingleEnded(i - 4);
      tensao = ads2.computeVolts(raw);
    }

    // --- FILTRAGEM E CONVERSÃO ---
    
    // Inicializa filtro se for a primeira vez
    if (adc_filtrado[i] == 0) adc_filtrado[i] = tensao;
    
    // Filtro Exponencial (Suaviza o ruído)
    adc_filtrado[i] = (ALPHA * tensao) + ((1.0 - ALPHA) * adc_filtrado[i]);

    // Calcula Corrente (mA) = Tensão / Resistor Shunt (150R) * 1000
    float corrente_mA = (adc_filtrado[i] / R_SHUNT) * 1000.0;
    
    // Converte Corrente 4-20mA para Pressão 0.5-3.0 Bar
    if(corrente_mA < 3.0) {
      pressoes_finais[i] = 0.0; // Cabo desconectado ou sensor desligado
    } else if(corrente_mA > 21.0) {
      pressoes_finais[i] = PRESSAO_MAX;
    } else {
      pressoes_finais[i] = PRESSAO_MIN + (corrente_mA - 4.0) * (PRESSAO_MAX - PRESSAO_MIN) / 16.0;
    }
  }
}

float SensorLib::getPressure(int index) {
  if(index >= 0 && index < NUM_SENSORES) return pressoes_finais[index];
  return 0.0;
}

String SensorLib::getJson() {
  String json = "{";
  for(int i=0; i<NUM_SENSORES; i++) {
    // Retorna json formatado: "s1": "1.20", "s2": "0.50"...
    json += "\"s" + String(i+1) + "\": \"" + String(pressoes_finais[i], 2) + "\"";
    if(i < NUM_SENSORES - 1) json += ", ";
  }
  json += "}";
  return json;
}