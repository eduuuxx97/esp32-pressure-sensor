#include "SensorLib.h"
#include <stdio.h> // Necessário para snprintf

void SensorLib::begin() {
  // Inicializa Módulo 1 (S0, S1, S2, S3) no endereço 0x48
  if (!ads1.begin(ADS1_ADDR)) {
    Serial.println("ERRO: ADS1 (0x48) nao encontrado!");
  } else {
    Serial.println("ADS1 OK!");
  }

  // Inicializa Módulo 2 (S4, S5) no endereço 0x49
  if (!ads2.begin(ADS2_ADDR)) {
    Serial.println("ERRO: ADS2 (0x49) nao encontrado!");
  } else {
    Serial.println("ADS2 OK!");
  }

  // Configura Ganho 1x (+/- 4.096V) - Ideal para sinais ate 3.3V
  ads1.setGain(GAIN_ONE);
  ads2.setGain(GAIN_ONE);

  // Zera as variaveis de controle e filtros
  for(int i=0; i<NUM_SENSORES; i++) {
    adc_filtrado[i] = 0;
    pressoes_finais[i] = 0;
  }
}

void SensorLib::update() {
  for(int i=0; i<NUM_SENSORES; i++) {
    int16_t raw = 0;
    float tensao = 0.0;

    // Roteamento dos canais entre os dois modulos ADS1115
    if (i < 4) {
      // Sensores S1 a S4 no primeiro modulo (Portas A0-A3)
      raw = ads1.readADC_SingleEnded(i);
      tensao = ads1.computeVolts(raw);
    } 
    else {
      // Sensores S5 e S6 no segundo modulo (Portas A0-A1)
      raw = ads2.readADC_SingleEnded(i - 4);
      tensao = ads2.computeVolts(raw);
    }

    // Inicializa filtro na primeira leitura para evitar rampa de subida
    if (adc_filtrado[i] == 0) adc_filtrado[i] = tensao;
    
    // Filtro Exponencial: suaviza ruidos eletricos da sala de experimento
    // Ponto de melhoria futuro: diminuir ALPHA para 0.05 se houver muito ruido
    adc_filtrado[i] = (ALPHA * tensao) + ((1.0 - ALPHA) * adc_filtrado[i]);

    // Conversao: Tensao -> Corrente (mA) usando Resistor Shunt de 150R
    float corrente_mA = (adc_filtrado[i] / R_SHUNT) * 1000.0;
    
    // Conversao Industrial: 4-20mA para 0.5-3.0 Bar
    if(corrente_mA < 3.0) {
      pressoes_finais[i] = 0.0; // Identifica sensor desligado ou cabo rompido
    } else if(corrente_mA > 21.0) {
      pressoes_finais[i] = PRESSAO_MAX; // Saturacao positiva
    } else {
      // Formula da reta: P = Pmin + (I - 4) * (Pmax - Pmin) / 16
      pressoes_finais[i] = PRESSAO_MIN + (corrente_mA - 4.0) * (PRESSAO_MAX - PRESSAO_MIN) / 16.0;
    }
  }
}

float SensorLib::getPressure(int index) {
  if(index >= 0 && index < NUM_SENSORES) return pressoes_finais[index];
  return 0.0;
}

/**
 * @brief Gera o JSON de dados em um buffer fixo para evitar fragmentacao de RAM.
 * @param buffer Ponteiro para o array de char onde o JSON sera escrito.
 * @param n Tamanho maximo do buffer.
 */
void SensorLib::getJson(char* buffer, size_t n) {
  // Substitui a criacao de String dinamica por formatacao estatica
  snprintf(buffer, n, 
    "{\"s1\":\"%.2f\",\"s2\":\"%.2f\",\"s3\":\"%.2f\",\"s4\":\"%.2f\",\"s5\":\"%.2f\",\"s6\":\"%.2f\"}",
    pressoes_finais[0], pressoes_finais[1], pressoes_finais[2], 
    pressoes_finais[3], pressoes_finais[4], pressoes_finais[5]);
}