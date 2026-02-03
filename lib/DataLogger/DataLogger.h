#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

class DataLogger {
  public:
    bool begin();
    // Agora recebe o nome do arquivo e os dados
    void log(String nomeArquivo, String dados); 
    void deleteLog(String nomeArquivo);
    File openForRead(String nomeArquivo); 
};

#endif