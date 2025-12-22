#include "WebManager.h"

// Construtor: Inicia o server na porta 80
WebManager::WebManager() : server(80) {}

void WebManager::begin(SensorLib* s, DataLogger* l) {
  // Guarda as referências para usar depois
  this->sensores = s;
  this->logger = l;

  // Inicia o Wi-Fi (AP)
  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  Serial.print("Web Server iniciado no IP: ");
  Serial.println(WiFi.softAPIP());

  // Configura as rotas
  setupRoutes();
  
  // Inicia o servidor
  server.begin();
}

void WebManager::handle() {
  server.handleClient();
}

void WebManager::setupRoutes() {
  // Rota Principal
  server.on("/", [this](){
    server.send(200, "text/html", index_html);
  });

  // Rota de Dados (JSON)
  server.on("/dados", [this](){
    // Usa a seta (->) porque 'sensores' é um ponteiro
    String json = this->sensores->getJson(); 
    server.send(200, "application/json", json);
  });

  // Rota de Download
  server.on("/download", [this](){
    File f = this->logger->openForRead();
    if(f) {
      server.sendHeader("Content-Type", "text/csv");
      server.sendHeader("Content-Disposition", "attachment; filename=log.csv");
      server.streamFile(f, "text/csv");
      f.close();
    } else {
      server.send(404, "text/plain", "Erro ao abrir arquivo");
    }
  });

  // Rota de Apagar
  server.on("/delete", [this](){
    this->logger->deleteLog();
    server.send(200, "text/html", "Log Apagado! <a href='/'>Voltar</a>");
  });
}