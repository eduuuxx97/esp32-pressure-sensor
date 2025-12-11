/*
 * Leitura de Sensor Industrial 4-20mA com ESP32
 * Range de Pressão: 0.5 bar a 3.0 bar
 * Hardware: Resistor Shunt de 150 Ohms entre o pino de leitura e GND.
 */
#include <Arduino.h>
const int PIN_SENSOR = 34; // Use pinos ADC1 (ex: 32, 33, 34, 35) se usar Wi-Fi

// Configurações do Hardware
const float R_SHUNT = 150.0; // Valor do resistor em Ohms
const float V_REF = 3.3;     // Tensão de referência do ESP32 (pode calibrar medindo o pino 3V3)
const int ADC_RES = 4095;    // Resolução do ADC (12 bits)

// Configurações do Sensor
const float PRESSAO_MIN = 0.5; // Pressão em 4mA (Bar)
const float PRESSAO_MAX = 3.0; // Pressão em 20mA (Bar)

void setup() {
  Serial.begin(115200);
  
  // Configura a atenuação do ADC para ler até ~3.3V
  analogSetAttenuation(ADC_11db); 
  pinMode(PIN_SENSOR, INPUT);
}

void loop() {
  // 1. Ler o valor cru do ADC (0 a 4095)
  // Fazemos uma média de 10 leituras para estabilizar o sinal
  long soma = 0;
  for(int i=0; i<10; i++){
    soma += analogRead(PIN_SENSOR);
    delay(5);
  }
  int adc_raw = soma / 10;

  // 2. Converter ADC para Tensão (Volts)
  float tensao = (adc_raw * V_REF) / ADC_RES;

  // 3. Converter Tensão para Corrente (mA) usando Lei de Ohm (I = V / R)
  // Multiplicamos por 1000 para ver em mA
  float corrente_mA = (tensao / R_SHUNT) * 1000.0;

  // 4. Calcular a Pressão (Bar)
  // Mapeamento linear: Se corrente < 4mA, assume min; se > 20mA, assume max.
  float pressao = 0.0;

  if (corrente_mA < 4.0) {
    pressao = PRESSAO_MIN; // Ou tratar como erro/sensor desconectado
  } else if (corrente_mA > 20.0) {
    pressao = PRESSAO_MAX;
  } else {
    // Fórmula de interpolação linear
    pressao = PRESSAO_MIN + (corrente_mA - 4.0) * (PRESSAO_MAX - PRESSAO_MIN) / (20.0 - 4.0);
  }

  // Mostra os resultados no Serial Monitor
  Serial.print("ADC: ");
  Serial.print(adc_raw);
  Serial.print(" | Tensão: ");
  Serial.print(tensao);
  Serial.print("V | Corrente: ");
  Serial.print(corrente_mA);
  Serial.print("mA | >> PRESSÃO: ");
  Serial.print(pressao);
  Serial.println(" bar");

  delay(1000);
}