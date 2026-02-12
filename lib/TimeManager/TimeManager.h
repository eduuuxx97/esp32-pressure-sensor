#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H
#include <Arduino.h>
#include <RTClib.h> 

class TimeManager {
  private:
    RTC_DS3231 rtc;
  public:
    bool begin();
    void ajustarHora(int ano, int mes, int dia, int h, int m, int s);
    String getDataHoraCSV();
    uint32_t getUnixAgora(); // Retorna segundos absolutos
    String getNomeArquivoFormatado();
};
#endif