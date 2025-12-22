#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "../../include/Config.h"

class DataLogger {
  public:
    bool begin();
    void log(String dados); // Salva uma linha
    void deleteLog();
    bool exists();
    File openForRead(); // Para download
};

#endif