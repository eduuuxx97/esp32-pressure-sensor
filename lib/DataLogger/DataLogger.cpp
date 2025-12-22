#include "DataLogger.h"

bool DataLogger::begin() {
  if(!SD.begin(PIN_CS_SD)) return false;
  
  if(!SD.exists("/datalog.csv")) {
    File f = SD.open("/datalog.csv", FILE_WRITE);
    if(f) {
      f.println("Tempo,S1,S2,S3,S4,S5,S6");
      f.close();
    }
  }
  return true;
}

void DataLogger::log(String dados) {
  File f = SD.open("/datalog.csv", FILE_APPEND);
  if(f) {
    f.println(dados);
    f.close();
    Serial.println("SD Gravado: " + dados);
  } else {
    Serial.println("Erro ao gravar SD");
  }
}

void DataLogger::deleteLog() {
  if(SD.exists("/datalog.csv")) SD.remove("/datalog.csv");
}

File DataLogger::openForRead() {
  return SD.open("/datalog.csv");
}