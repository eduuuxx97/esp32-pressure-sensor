#include "TimeManager.h"

bool TimeManager::begin() {
    if (!rtc.begin()) return false;
    return true;
}

void TimeManager::ajustarHora(int ano, int mes, int dia, int h, int m, int s) {
    rtc.adjust(DateTime(ano, mes, dia, h, m, s));
}

uint32_t TimeManager::getUnixAgora() {
    return rtc.now().unixtime();
}

String TimeManager::getDataHoraCSV() {
    DateTime agora = rtc.now();
    char buffer[25];
    sprintf(buffer, "%02d/%02d/%04d;%02d:%02d:%02d", 
            agora.day(), agora.month(), agora.year(), 
            agora.hour(), agora.minute(), agora.second());
    return String(buffer);
}

String TimeManager::getNomeArquivoFormatado() {
    DateTime agora = rtc.now();
    char buffer[15];
    sprintf(buffer, "/L%02d%02d%02d.csv", 
            agora.day(), agora.hour(), agora.minute());
    return String(buffer);
}