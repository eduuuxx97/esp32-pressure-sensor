#include <Arduino.h>

/**
 * PROJETO: Leitura de Sensor Industrial 4-20mA (Versão Estável)
 * PLATAFORMA: ESP32 (PlatformIO)
 * ALGORITMO: Filtro Média Móvel Exponencial (EMA)
 */

// --- Configurações de Hardware ---
const int PIN_SENSOR = 34;   
const float R_SHUNT = 150.0; // Ohms
const float V_REF = 3.3;     
const int ADC_RES = 4095;    

// --- Configurações do Sensor ---
const float PRESSAO_MIN = 0.5; // Bar
const float PRESSAO_MAX = 3.0; // Bar

// --- Configurações do Filtro (O Segredo) ---
// Alpha define a suavidade (0.01 a 1.0).
// 0.05 = Muito lento/suave (parece câmera lenta)
// 0.10 = Equilíbrio ideal (Recomendado)
// 0.50 = Rápido, mas com algum ruído
const float ALPHA = 0.1; 

float adc_filtrado = 0.0; // Guarda o histórico

void setup() {
  Serial.begin(115200);
  analogSetAttenuation(ADC_11db);
  pinMode(PIN_SENSOR, INPUT);

  // Inicializa o filtro com a primeira leitura para não começar do zero
  adc_filtrado = analogRead(PIN_SENSOR);
}

void loop() {
  // 1. Leitura Raw (Crua)
  int leitura_atual = analogRead(PIN_SENSOR);

  // 2. Aplicação do Filtro EMA
  // Fórmula: NovoValor = (Alpha * Atual) + ((1 - Alpha) * Anterior)
  adc_filtrado = (ALPHA * leitura_atual) + ((1.0 - ALPHA) * adc_filtrado);

  // 3. Conversões (Usando o valor filtrado agora)
  float tensao = (adc_filtrado * V_REF) / ADC_RES;
  float corrente_mA = (tensao / R_SHUNT) * 1000.0;

  // 4. Cálculo da Pressão
  float pressao = 0.0;

  if (corrente_mA < 3.5) {
    pressao = 0.0; // Sensor desconectado/erro
  } else if (corrente_mA > 21.0) {
    pressao = PRESSAO_MAX; 
  } else {
    pressao = PRESSAO_MIN + (corrente_mA - 4.0) * (PRESSAO_MAX - PRESSAO_MIN) / (16.0);
  }

  // 5. Exibição
  // Mostro o RAW vs Filtrado para você ver a diferença
  Serial.printf("Raw: %d | Filtrado: %.1f | >> PRESSÃO: %.3f bar\n", 
                leitura_atual, adc_filtrado, pressao);

  delay(100); // 10 leituras por segundo (mais rápido que antes)
}