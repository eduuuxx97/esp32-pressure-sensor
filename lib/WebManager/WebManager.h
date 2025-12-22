#ifndef WEBMANAGER_H
#define WEBMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "../../include/Config.h"
#include "../../include/WebPage.h"
#include "../SensorLib/SensorLib.h"   // Precisa conhecer os sensores
#include "../DataLogger/DataLogger.h" // Precisa conhecer o logger

class WebManager {
  private:
    WebServer server;
    SensorLib* sensores;   // Ponteiro (link) para os sensores
    DataLogger* logger;    // Ponteiro (link) para o logger

    void setupRoutes();    // Função interna para organizar as rotas

  public:
    WebManager();          // Construtor
    void begin(SensorLib* s, DataLogger* l); // Inicializa recebendo os links
    void handle();         // Fica no loop
};

#endif