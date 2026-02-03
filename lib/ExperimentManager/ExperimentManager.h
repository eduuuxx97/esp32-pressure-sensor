#ifndef EXPERIMENTMANAGER_H
#define EXPERIMENTMANAGER_H

#include <Arduino.h>

class ExperimentManager {
  private:
    bool emAndamento;
    bool pausado;
    unsigned long inicio;
    unsigned long duracaoTotal;
    unsigned long inicioPausa;
    unsigned long intervaloLog;
    unsigned long contadorLeituras; // Contador único para cada gravação

  public:
    ExperimentManager();
    void start(long minutos);
    void stop();
    void pause();
    void resume();
    
    bool isRunning(); 
    bool isPaused();
    
    void setIntervalo(unsigned long segundos) { 
        this->intervaloLog = (segundos > 0) ? segundos * 1000 : 2000; 
    }
    unsigned long getIntervalo() { return this->intervaloLog; }
    
    // Funções do Contador (Trava para o Python)
    void incrementarContador() { this->contadorLeituras++; }
    unsigned long getContador() { return this->contadorLeituras; }
    
    String getTempoRestante();
};

#endif