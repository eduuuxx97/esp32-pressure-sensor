#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <Arduino.h>
#include <RTClib.h> // Biblioteca para o DS3231

class TimeManager {
  private:
    RTC_DS3231 rtc;

  public:
    bool begin();
    
    // Sincroniza o RTC com os dados vindos da interface Web
    void ajustarHora(int ano, int mes, int dia, int h, int m, int s);
    
    // Retorna string formatada para a linha do CSV: "DD/MM/AAAA;HH:MM:SS"
    String getDataHoraCSV();
    
    // NOVO: Gera nome de arquivo válido para o SD (ex: /L021758.csv)
    String getNomeArquivoFormatado();
};

#endif