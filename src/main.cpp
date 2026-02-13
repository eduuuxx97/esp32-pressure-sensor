#include <Arduino.h>
#include <SPI.h>           
#include <LittleFS.h>      
#include "TimeManager.h"
#include "SensorLib.h"
#include "ExperimentManager.h"
#include "DataLogger.h"
#include "WebManager.h"

TimeManager timeManager;
SensorLib sensores;
DataLogger logger;
ExperimentManager experimento;
WebManager web;

char logBuffer[450];    
String nomeArquivoSDAtual = ""; 
unsigned long ultimaAmostragem = 0;

void setup() {
  Serial.begin(115200);
  
  if(!LittleFS.begin(true)) Serial.println("Erro LittleFS");
  
  timeManager.begin();
  sensores.begin();
  logger.begin();
  
  // --- RECUPERAÇÃO DE ESTADO APÓS QUEDA DE ENERGIA ---
  if (LittleFS.exists("/status.txt")) {
    File f = LittleFS.open("/status.txt", "r");
    if (f && f.size() > 0) {
      String conteudo = f.readStringUntil('\n');
      f.close();
      
      // Formato: Arquivo;Contador;UnixFim;Pausado
      int p1 = conteudo.indexOf(';');
      int p2 = conteudo.indexOf(';', p1 + 1);
      int p3 = conteudo.lastIndexOf(';');

      if (p1 != -1 && p2 != -1 && p3 != -1) {
          nomeArquivoSDAtual = conteudo.substring(0, p1);
          unsigned long cont = conteudo.substring(p1 + 1, p2).toInt();
          uint32_t unixFim = (uint32_t)conteudo.substring(p2 + 1, p3).toInt();
          bool pausado = (conteudo.substring(p3 + 1).toInt() == 1);

          // Só recupera se o tempo de fim ainda for futuro
          if (unixFim > timeManager.getUnixAgora()) {
              experimento.setContador(cont);
              experimento.setUnixFim(unixFim);
              experimento.setPausado(pausado);
              experimento.setRunning(true);
              Serial.println(">>> EXPERIMENTO RECUPERADO: " + nomeArquivoSDAtual);
          }
      }
    }
  }

  web.begin(&sensores, &logger, &experimento, &timeManager);
}

void loop() {
  web.handle();      
  sensores.update(); // Aqui reside a Camada 2 (Watchdog I2C)

  if (experimento.isRunning()) {
    uint32_t agora = timeManager.getUnixAgora();

    // Lógica de gravação: baseada no intervalo e estado de pausa
    if (!experimento.isPaused() && (millis() - ultimaAmostragem >= experimento.getIntervalo())) {
      ultimaAmostragem = millis();

      // Se é a primeira gravação após o boot ou início
      if (nomeArquivoSDAtual == "") {
        nomeArquivoSDAtual = timeManager.getNomeArquivoFormatado();
        logger.log(nomeArquivoSDAtual, "Data;Hora;S1;S2;S3;S4;S5;S6;Tempo_Exp;Modo");
      }

      // Snprintf evita fragmentação de memória em longos períodos
      snprintf(logBuffer, sizeof(logBuffer), "%s;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%s;%s",
               timeManager.getDataHoraCSV().c_str(), 
               sensores.getPressure(0), sensores.getPressure(1),
               sensores.getPressure(2), sensores.getPressure(3),
               sensores.getPressure(4), sensores.getPressure(5),
               experimento.getTempoRestante(agora).c_str(),
               sensores.isHardwareOK() ? "AUTOMATICO" : "RECUPERACAO_I2C");

      logger.log(nomeArquivoSDAtual, String(logBuffer)); 
      experimento.incrementarContador();

      // Atualiza Checkpoint no LittleFS para proteção contra desligamento
      File f = LittleFS.open("/status.txt", "w");
      if(f) {
        f.printf("%s;%lu;%u;%d", 
                 nomeArquivoSDAtual.c_str(), 
                 experimento.getContador(), 
                 experimento.getUnixFim(),
                 experimento.isPaused() ? 1 : 0);
        f.close();
      }
    }
    
    // Auto-finalização se o tempo acabar
    if (experimento.getTempoRestante(agora) == "00:00:00") {
        experimento.stop();
        if (LittleFS.exists("/status.txt")) LittleFS.remove("/status.txt");
        nomeArquivoSDAtual = "";
    }
  } 
  else {
    // Se não está rodando, garante que o arquivo de status não existe
    if (LittleFS.exists("/status.txt")) LittleFS.remove("/status.txt");
  }
}