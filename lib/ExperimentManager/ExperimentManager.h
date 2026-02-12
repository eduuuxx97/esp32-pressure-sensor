#ifndef EXPERIMENTMANAGER_H
#define EXPERIMENTMANAGER_H
#include <Arduino.h>

class ExperimentManager {
  private:
    bool emAndamento;
    bool pausado;
    uint32_t unixFim;        // Timestamp de término (independente de quedas)
    uint32_t unixPausa;      
    unsigned long intervaloLog;
    unsigned long contadorLeituras;

  public:
    ExperimentManager();
    void start(long minutos, uint32_t unixAgora);
    void stop();
    void pause(uint32_t unixAgora);
    void resume(uint32_t unixAgora);
    
    bool isRunning() { return this->emAndamento; } 
    bool isPaused() { return this->pausado; }

    // Funções de Recuperação
    void setRunning(bool estado) { this->emAndamento = estado; }
    void setPausado(bool estado) { this->pausado = estado; }
    void setContador(unsigned long valor) { this->contadorLeituras = valor; }
    void setUnixFim(uint32_t unixFimSalvo) { this->unixFim = unixFimSalvo; }
    uint32_t getUnixFim() { return this->unixFim; }

    void setIntervalo(unsigned long segundos) { this->intervaloLog = (segundos > 0) ? segundos * 1000 : 2000; }
    unsigned long getIntervalo() { return this->intervaloLog; }
    void incrementarContador() { this->contadorLeituras++; }
    unsigned long getContador() { return this->contadorLeituras; }
    
    String getTempoRestante(uint32_t unixAgora);
};
#endif