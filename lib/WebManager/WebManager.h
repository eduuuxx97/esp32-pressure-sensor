#ifndef WEBMANAGER_H
#define WEBMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "../../include/Config.h"
#include "../../include/WebPage.h"
#include "../SensorLib/SensorLib.h"   
#include "../DataLogger/DataLogger.h" 
#include "../ExperimentManager/ExperimentManager.h"
#include "../TimeManager/TimeManager.h" // <--- INCLUIR AQUI

class WebManager {
  private:
    WebServer server;
    SensorLib* sensores;   
    DataLogger* logger;    
    ExperimentManager* experimento;
    TimeManager* relogio; // <--- NOVO PONTEIRO

    void setupRoutes();    

  public:
    WebManager();          
    
    // Atualizamos a função begin para receber o relógio também
    void begin(SensorLib* s, DataLogger* l, ExperimentManager* e, TimeManager* t);
    
    void handle();         
};

#endif