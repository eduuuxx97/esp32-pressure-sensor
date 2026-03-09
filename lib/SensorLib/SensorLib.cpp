#include "SensorLib.h"
#include <stdio.h>
#include <Wire.h>

void SensorLib::begin() {
  hw_status = true;
  // Inicialização dos ADCs
  if (!ads1.begin(ADS1_ADDR)) { Serial.println("ERRO: ADS1 (0x48)"); hw_status = false; }
  if (!ads2.begin(ADS2_ADDR)) { Serial.println("ERRO: ADS2 (0x49)"); hw_status = false; }
  
  ads1.setGain(GAIN_ONE);
  ads2.setGain(GAIN_ONE);

  for(int i=0; i<NUM_SENSORES; i++) {
    adc_filtrado[i] = 0; pressoes_finais[i] = 0;
  }

  // --- Inicialização do DS18B20 ---
  sensorsDS.begin(); 
  sensorsDS.setWaitForConversion(false); // Não trava o processador esperando a leitura
  temperaturaAtual = 0.0;
  ultimaRequisicaoTemp = 0;
}

void SensorLib::updateTemp() {
  // Solicita uma leitura e só busca o resultado 2 segundos depois
  // Isso evita o erro de "OFFLINE" que você teve, pois o Wi-Fi continua livre
  if (millis() - ultimaRequisicaoTemp > 2000) {
    float tempLeitura = sensorsDS.getTempCByIndex(0);
    
    // Validação básica: se o sensor for desconectado, ele retorna -127
    if (tempLeitura > -50 && tempLeitura < 150) {
      temperaturaAtual = tempLeitura;
    }
    
    sensorsDS.requestTemperatures(); // Dispara o pedido para a próxima leitura
    ultimaRequisicaoTemp = millis();
  }
}

void SensorLib::recuperarI2C() {
  Serial.println("Watchdog I2C: Barramento travado! Recuperando...");
  Wire.end();
  pinMode(PIN_I2C_SCL, OUTPUT);
  pinMode(PIN_I2C_SDA, OUTPUT);

  for (int i = 0; i < 9; i++) {
    digitalWrite(PIN_I2C_SCL, LOW); delayMicroseconds(5);
    digitalWrite(PIN_I2C_SCL, HIGH); delayMicroseconds(5);
  }
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  this->begin(); 
}

void SensorLib::update() {
  Wire.beginTransmission(ADS1_ADDR);
  if (Wire.endTransmission() != 0) {
    hw_status = false;
    recuperarI2C();
    return;
  }
  hw_status = true;

  for(int i=0; i<NUM_SENSORES; i++) {
    int16_t raw = (i < 4) ? ads1.readADC_SingleEnded(i) : ads2.readADC_SingleEnded(i - 4);
    float tensao = (raw * 4.096) / 32768.0;

    adc_filtrado[i] = (ALPHA * tensao) + ((1.0 - ALPHA) * adc_filtrado[i]);
    float corrente_mA = (adc_filtrado[i] / R_SHUNT) * 1000.0;
    
    if(corrente_mA < 3.0) pressoes_finais[i] = 0.0;
    else if(corrente_mA > 21.0) pressoes_finais[i] = PRESSAO_MAX;
    else pressoes_finais[i] = PRESSAO_MIN + (corrente_mA - 4.0) * (PRESSAO_MAX - PRESSAO_MIN) / 16.0;
  }
}

void SensorLib::getJson(char* buffer, size_t n) {
  // Adicionado a chave "temp" ao final do JSON
  snprintf(buffer, n, "{\"s1\":%.2f,\"s2\":%.2f,\"s3\":%.2f,\"s4\":%.2f,\"s5\":%.2f,\"s6\":%.2f,\"temp\":%.2f}",
           pressoes_finais[0], pressoes_finais[1], pressoes_finais[2],
           pressoes_finais[3], pressoes_finais[4], pressoes_finais[5],
           temperaturaAtual);
}

float SensorLib::getPressure(int index) {
  if(index >= 0 && index < NUM_SENSORES) return pressoes_finais[index];
  return 0.0;
}