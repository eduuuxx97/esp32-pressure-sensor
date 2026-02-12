#include "DataLogger.h"
#include <Arduino.h>

bool DataLogger::begin() {
  if (!SD.begin(5)) { // Usando o pino 5 que validamos
    Serial.println("Erro critico: SD nao montado no pino 5");
    return false;
  }
  return true;
}

void DataLogger::log(String nomeArquivo, String dados) {
  // Tenta abrir em modo APPEND (adicionar ao final)
  File dataFile = SD.open(nomeArquivo, FILE_APPEND);
  
  // WATCHDOG DE PERIFÉRICO: Se falhar (mau contato), tenta reiniciar o chip do SD
  if (!dataFile) {
    Serial.println("Watchdog SD: Falha de escrita. Reiniciando hardware...");
    SD.end(); 
    delay(100);
    if (SD.begin(5)) {
      dataFile = SD.open(nomeArquivo, FILE_APPEND);
    }
  }

  if (dataFile) {
    dataFile.println(dados); 
    dataFile.flush(); // Força a escrita física no disco
    dataFile.close();
  } else {
    Serial.println("Watchdog SD: Erro persistente de hardware.");
  }
}