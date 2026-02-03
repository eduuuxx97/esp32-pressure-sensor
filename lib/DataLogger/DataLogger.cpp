#include "DataLogger.h"
#include <Arduino.h>

bool DataLogger::begin() {
  // Inicialização do barramento SPI para o cartão SD
  // Retorna true se o cartão for montado com sucesso
  if (!SD.begin()) return false;
  return true;
}

void DataLogger::log(String nomeArquivo, String dados) {
  // MUDANÇA CRÍTICA: Use FILE_APPEND em vez de FILE_WRITE
  File dataFile = SD.open(nomeArquivo, FILE_APPEND); 
  
  if (dataFile) {
    dataFile.println(dados); // Grava a linha e pula para a próxima
    dataFile.flush();        // Garante que o dado saia do buffer para o chip
    dataFile.close();        // Fecha o arquivo com segurança
  } else {
    Serial.println("Erro ao abrir arquivo para APPEND no SD!");
  }
}

File DataLogger::openForRead(String nomeArquivo) {
  return SD.open(nomeArquivo, FILE_READ);
}