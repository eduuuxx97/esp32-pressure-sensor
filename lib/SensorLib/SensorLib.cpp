#include "SensorLib.h"
#include <stdio.h>
#include <Wire.h>

void SensorLib::begin() {
  hw_status = true;

  // Inicialização dos ADCs
  if (!ads1.begin(ADS1_ADDR)) {
    Serial.println("ERRO: ADS1 (0x48)");
    hw_status = false;
  }

  if (!ads2.begin(ADS2_ADDR)) {
    Serial.println("ERRO: ADS2 (0x49)");
    hw_status = false;
  }

  ads1.setGain(GAIN_ONE);
  ads2.setGain(GAIN_ONE);

  for (int i = 0; i < NUM_SENSORES; i++) {
    adc_filtrado[i] = 0.0;
    pressoes_finais[i] = 0.0;
  }

  // Inicialização do DS18B20
  sensorsDS.begin();
  sensorsDS.setWaitForConversion(false); // Não trava o processador esperando a leitura
  sensorsDS.requestTemperatures();       // Dispara a primeira leitura

  temperaturaAtual = 0.0;
  ultimaRequisicaoTemp = 0;
}

void SensorLib::updateTemp() {
  // Solicita uma leitura e só busca o resultado 2 segundos depois.
  // Isso evita travar o processador enquanto o sensor converte a temperatura.
  if (millis() - ultimaRequisicaoTemp > 2000) {
    float tempLeitura = sensorsDS.getTempCByIndex(0);

    // Validação básica: se o sensor for desconectado, ele retorna -127 °C
    if (tempLeitura > -50.0 && tempLeitura < 150.0) {
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

  // Gera pulsos no SCL para tentar liberar algum dispositivo preso no barramento
  for (int i = 0; i < 9; i++) {
    digitalWrite(PIN_I2C_SCL, LOW);
    delayMicroseconds(5);

    digitalWrite(PIN_I2C_SCL, HIGH);
    delayMicroseconds(5);
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

  for (int i = 0; i < NUM_SENSORES; i++) {
    int16_t raw = (i < 4) ? ads1.readADC_SingleEnded(i) : ads2.readADC_SingleEnded(i - 4);

    // ADS1115 com GAIN_ONE: faixa de ±4,096 V
    float tensao = (raw * 4.096) / 32768.0;

    // Filtro exponencial simples
    adc_filtrado[i] = (ALPHA * tensao) + ((1.0 - ALPHA) * adc_filtrado[i]);

    // Conversão da tensão no shunt para corrente do loop 4-20 mA
    float corrente_mA = (adc_filtrado[i] / R_SHUNT) * 1000.0;

    // Primeiro calcula a pressão absoluta do transdutor
    float pressao_absoluta = 0.0;

    if (corrente_mA < 3.0) {
      pressao_absoluta = 0.0;
    }
    else if (corrente_mA > 21.0) {
      pressao_absoluta = PRESSAO_MAX;
    }
    else {
      pressao_absoluta = PRESSAO_MIN +
                         (corrente_mA - 4.0) *
                         (PRESSAO_MAX - PRESSAO_MIN) / 16.0;
    }

    // Remove a pressão atmosférica para obter pressão relativa,
    // equivalente ao que o manômetro gauge mostra.
    float pressao_relativa = pressao_absoluta - PRESSAO_ATMOSFERICA;

    // Evita mostrar valor negativo quando estiver aberto à atmosfera
    if (pressao_relativa < 0.0) {
      pressao_relativa = 0.0;
    }

    pressoes_finais[i] = pressao_relativa;
  }
}

void SensorLib::getJson(char* buffer, size_t n) {
  // Envia as pressões relativas e a temperatura no JSON
  snprintf(buffer, n,
           "{\"s1\":%.2f,\"s2\":%.2f,\"s3\":%.2f,\"s4\":%.2f,\"s5\":%.2f,\"s6\":%.2f,\"temp\":%.2f}",
           pressoes_finais[0], pressoes_finais[1], pressoes_finais[2],
           pressoes_finais[3], pressoes_finais[4], pressoes_finais[5],
           temperaturaAtual);
}

float SensorLib::getPressure(int index) {
  if (index >= 0 && index < NUM_SENSORES) {
    return pressoes_finais[index];
  }

  return 0.0;
}
