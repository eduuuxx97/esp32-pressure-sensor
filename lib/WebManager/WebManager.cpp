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
      String json = this->sensores->getJson();
      json.remove(json.length() - 1);
      
      // Verificação direta do hardware para o LED da interface
      bool sdOk = (SD.cardType() != CARD_NONE); 

      json += ", \"status\": \"" + this->experimento->getTempoRestante() + "\"";
      json += ", \"rodando\": " + String(this->experimento->isRunning() ? "true" : "false");
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
    this->experimento->start(tempo);
    server.send(200, "text/plain", "OK");
  });

  server.on("/stop", [this](){ this->experimento->stop(); server.send(200, "text/plain", "OK"); });
  server.on("/pause", [this](){ this->experimento->pause(); server.send(200, "text/plain", "OK"); });
  server.on("/resume", [this](){ this->experimento->resume(); server.send(200, "text/plain", "OK"); });
}