#include "ExperimentManager.h"

ExperimentManager::ExperimentManager() {
  this->emAndamento = false;
  this->pausado = false;
  this->duracaoTotal = 0;
  this->intervaloLog = 2000;
  this->contadorLeituras = 0;
}

void ExperimentManager::start(long minutos) {
  this->emAndamento = true;
  this->pausado = false;
  this->inicio = millis();
  this->contadorLeituras = 0; // Reseta ao iniciar novo teste
  this->duracaoTotal = (minutos > 0) ? (unsigned long)minutos * 60 * 1000 : 0;
}

void ExperimentManager::stop() { this->emAndamento = false; this->pausado = false; }

void ExperimentManager::pause() {
  if (emAndamento && !pausado) {
    this->pausado = true;
    this->inicioPausa = millis();
  }
}

void ExperimentManager::resume() {
  if (emAndamento && pausado) {
    this->pausado = false;
    this->inicio += (millis() - inicioPausa);
  }
}

bool ExperimentManager::isRunning() {
  if (!emAndamento || pausado) return false;
  if (duracaoTotal > 0 && (millis() - inicio >= duracaoTotal)) {
    stop();
    return false;
  }
  return true;
}

bool ExperimentManager::isPaused() { return (emAndamento && pausado); }

String ExperimentManager::getTempoRestante() {
  if (!emAndamento) return "PARADO";
  if (pausado) return "PAUSADO";
  if (duracaoTotal == 0) return "LIVRE (REC)";

  unsigned long decorrido = millis() - inicio;
  if (decorrido > duracaoTotal) return "00:00:00";
  
  unsigned long falta = duracaoTotal - decorrido;
  unsigned long segs = falta / 1000;
  unsigned long d = segs / 86400; segs %= 86400;
  unsigned long h = segs / 3600; segs %= 3600;
  unsigned long m = segs / 60; segs %= 60;
  
  char buffer[30];
  if(d > 0) sprintf(buffer, "%ldd %02ld:%02ld:%02ld", d, h, m, segs);
  else sprintf(buffer, "%02ld:%02ld:%02ld", h, m, segs);
  return String(buffer);
}