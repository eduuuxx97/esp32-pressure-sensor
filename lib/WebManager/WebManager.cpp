#include "WebManager.h"

WebManager::WebManager() : server(80) {}

void WebManager::begin(SensorLib* s, DataLogger* l, ExperimentManager* e, TimeManager* t) {
  this->sensores = s; this->logger = l; this->experimento = e; this->relogio = t;
  WiFi.softAP(WIFI_SSID, WIFI_PASS); 
  setupRoutes();
  server.begin();
}

void WebManager::handle() { server.handleClient(); }

void WebManager::setupRoutes() {
  server.on("/", [this](){ server.send(200, "text/html", index_html); });

  server.on("/dados", [this](){
      char tempBuffer[256]; 
      this->sensores->getJson(tempBuffer, sizeof(tempBuffer));
      
      String json = String(tempBuffer);
      if (json.endsWith("}")) json.remove(json.length() - 1);
      
      bool sdOk = (SD.cardType() != CARD_NONE); 

      // Sincroniza o cronômetro da Web com o tempo real do RTC
      json += ", \"status\": \"" + this->experimento->getTempoRestante(this->relogio->getUnixAgora()) + "\"";
      json += ", \"rodando\": " + String(this->experimento->isRunning() ? "true" : "false");
      json += ", \"pausado\": " + String(this->experimento->isPaused() ? "true" : "false");
      json += ", \"count\": " + String(this->experimento->getContador());
      json += ", \"sdStatus\": " + String(sdOk ? "true" : "false"); 
      json += "}";
      server.send(200, "application/json", json);
  });

  server.on("/start", [this](){
    long tempo = server.arg("tempo").toInt();
    long intervalo = server.arg("intervalo").toInt();
    if(intervalo > 0) this->experimento->setIntervalo(intervalo);

    if(server.hasArg("ano")) {
      this->relogio->ajustarHora(
        server.arg("ano").toInt(), server.arg("mes").toInt(), server.arg("dia").toInt(),
        server.arg("h").toInt(), server.arg("m").toInt(), server.arg("s").toInt()
      );
    }
    this->experimento->start(tempo, this->relogio->getUnixAgora());
    server.send(200, "text/plain", "OK");
  });

  server.on("/stop", [this](){ this->experimento->stop(); server.send(200, "text/plain", "OK"); });
  
  server.on("/pause", [this](){ 
    this->experimento->pause(this->relogio->getUnixAgora()); 
    server.send(200, "text/plain", "OK"); 
  });

  server.on("/resume", [this](){ 
    this->experimento->resume(this->relogio->getUnixAgora()); 
    server.send(200, "text/plain", "OK"); 
  });

  // NOVA ROTA: Reiniciar o ESP32 via Software (Segurança remota)
  server.on("/restart", [this](){
    server.send(200, "text/plain", "RESTARTING...");
    delay(1000);
    ESP.restart();
  });
}