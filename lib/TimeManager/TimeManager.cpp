#include "TimeManager.h"
#include <Arduino.h> // Essencial para reconhecer o tipo String

bool TimeManager::begin() {
    if (!rtc.begin()) return false;
    return true;
}

void TimeManager::ajustarHora(int ano, int mes, int dia, int h, int m, int s) {
    rtc.adjust(DateTime(ano, mes, dia, h, m, s));
}

String TimeManager::getDataHoraCSV() {
    DateTime agora = rtc.now();
    char buffer[25];
    // O ";" separa a coluna Data da coluna Hora no Excel
    sprintf(buffer, "%02d/%02d/%04d;%02d:%02d:%02d", 
            agora.day(), agora.month(), agora.year(), 
            agora.hour(), agora.minute(), agora.second());
    return String(buffer);
}

String TimeManager::getNomeArquivoFormatado() {
    DateTime agora = rtc.now();
    char buffer[15];
    // Nome curto 8.3 para evitar erros no sistema de arquivos do SD
    sprintf(buffer, "/L%02d%02d%02d.csv", 
            agora.day(), agora.hour(), agora.minute());
    return String(buffer);
}