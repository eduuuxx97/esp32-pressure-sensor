#include <Arduino.h>

/**
 * PROJETO: Leitura de 6 Sensores Industriais 4-20mA
 * AUTOR: Eduardo dos Santos Rodrigues
 * PLATAFORMA: ESP32 (PlatformIO)
 * ALGORITMO: Filtro Média Móvel Exponencial (EMA) para cada sensor
 * PINAGEM: 34, 35, 39, 36, 32, 33
 */

// --- Configurações de Hardware ---
const int NUM_SENSORES = 6;
// Ordem: Sensor 1 a 6
const int PINOS_SENSORES[NUM_SENSORES] = {34, 35, 39, 36, 32, 33};

const float R_SHUNT = 150.0; // Ohms
const float V_REF = 3.3;     
const int ADC_RES = 4095;    

// --- Configurações do Sensor ---
const float PRESSAO_MIN = 0.5; // Bar
const float PRESSAO_MAX = 3.0; // Bar

// --- Configurações do Filtro ---
const float ALPHA = 0.1; 
// Array para guardar o histórico de filtragem de CADA sensor separadamente
float adc_filtrado[NUM_SENSORES]; 

void setup() {
  Serial.begin(115200);
  analogSetAttenuation(ADC_11db);

  // Configura todos os pinos de uma vez usando um laço
  for(int i = 0; i < NUM_SENSORES; i++) {
    pinMode(PINOS_SENSORES[i], INPUT);
    // Inicializa o filtro com a primeira leitura para não começar do zero
    adc_filtrado[i] = analogRead(PINOS_SENSORES[i]);
  }
}

void loop() {
  Serial.println("-------------------------------------------------------------");
  
  // Passa por cada um dos 6 sensores
  for(int i = 0; i < NUM_SENSORES; i++) {
    
    // 1. Leitura Raw (Crua) do sensor atual (i)
    int leitura_atual = analogRead(PINOS_SENSORES[i]);

    // 2. Aplicação do Filtro EMA individual
    // Atualiza apenas o valor guardado na posição [i]
    adc_filtrado[i] = (ALPHA * leitura_atual) + ((1.0 - ALPHA) * adc_filtrado[i]);

    // 3. Conversões
    float tensao = (adc_filtrado[i] * V_REF) / ADC_RES;
    float corrente_mA = (tensao / R_SHUNT) * 1000.0;

    // 4. Cálculo da Pressão
    float pressao = 0.0;

    if (corrente_mA < 3.5) {
      pressao = 0.0; // Sensor desconectado
    } else if (corrente_mA > 21.0) {
      pressao = PRESSAO_MAX; 
    } else {
      pressao = PRESSAO_MIN + (corrente_mA - 4.0) * (PRESSAO_MAX - PRESSAO_MIN) / (16.0);
    }

    // 5. Exibição (Formato solicitado)
    // Adicionei o prefixo [S1], [S2] para você saber quem é quem
    Serial.print("[S");
    Serial.print(i + 1); // Mostra S1, S2, S3...
    Serial.print("] ");
    
    Serial.print("ADC: ");
    Serial.print((int)adc_filtrado[i]); 
    Serial.print(" | Tensão: ");
    Serial.print(tensao);
    Serial.print("V | Corrente: ");
    Serial.print(corrente_mA);
    Serial.print("mA | >> PRESSÃO: ");
    Serial.print(pressao);
    Serial.println(" bar");
  }

  // Delay aumentado para 1 segundo, senão 6 sensores imprimindo 
  // ao mesmo tempo tornam o terminal impossível de ler.
  delay(1000); 
}