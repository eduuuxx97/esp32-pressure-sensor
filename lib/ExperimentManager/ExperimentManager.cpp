#include "ExperimentManager.h"

ExperimentManager::ExperimentManager() {
    this->emAndamento = false;
    this->pausado = false;
    this->unixFim = 0;
    this->unixPausa = 0;
    this->intervaloLog = 2000;
    this->contadorLeituras = 0;
}

void ExperimentManager::start(long minutos, uint32_t unixAgora) {
    this->unixFim = unixAgora + (minutos * 60);
    this->emAndamento = true;
    this->pausado = false;
    this->contadorLeituras = 0;
}

void ExperimentManager::stop() {
    this->emAndamento = false;
    this->pausado = false;
}

void ExperimentManager::pause(uint32_t unixAgora) {
    if (emAndamento && !pausado) {
        pausado = true;
        unixPausa = unixAgora;
    }
}

void ExperimentManager::resume(uint32_t unixAgora) {
    if (emAndamento && pausado) {
        pausado = false;
        // O tempo que ficou pausado é adicionado ao tempo de fim original
        uint32_t tempoPausado = unixAgora - unixPausa;
        this->unixFim += tempoPausado;
    }
}

String ExperimentManager::getTempoRestante(uint32_t unixAgora) {
    if (!emAndamento) return "00:00:00";
    
    uint32_t agoraReferencia = pausado ? unixPausa : unixAgora;

    if (agoraReferencia >= unixFim) {
        emAndamento = false;
        return "00:00:00";
    }

    uint32_t restanteSeg = unixFim - agoraReferencia;
    int h = restanteSeg / 3600;
    int m = (restanteSeg % 3600) / 60;
    int s = restanteSeg % 60;

    char buffer[15];
    sprintf(buffer, "%02d:%02d:%02d", h, m, s);
    return String(buffer);
}