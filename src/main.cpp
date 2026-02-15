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

char logBuffer[500]; // Buffer aumentado para a nova coluna    
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
      
      int p1 = conteudo.indexOf(';');
      int p2 = conteudo.indexOf(';', p1 + 1);
      int p3 = conteudo.lastIndexOf(';');

      if (p1 != -1 && p2 != -1 && p3 != -1) {
          nomeArquivoSDAtual = conteudo.substring(0, p1);
          unsigned long cont = conteudo.substring(p1 + 1, p2).toInt();
          uint32_t unixFim = (uint32_t)conteudo.substring(p2 + 1, p3).toInt();
          bool pausado = (conteudo.substring(p3 + 1).toInt() == 1);

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
  sensores.update(); 

  if (experimento.isRunning()) {
    uint32_t agora = timeManager.getUnixAgora();

    if (!experimento.isPaused() && (millis() - ultimaAmostragem >= experimento.getIntervalo())) {
      ultimaAmostragem = millis();

      if (nomeArquivoSDAtual == "") {
        nomeArquivoSDAtual = timeManager.getNomeArquivoFormatado();
        // Adicionado "Luz" ao cabeçalho
        logger.log(nomeArquivoSDAtual, "Data;Hora;S1;S2;S3;S4;S5;S6;Tempo_Exp;Modo;Luz");
      }

      // Adicionado "sim" ao final da linha de dados
      snprintf(logBuffer, sizeof(logBuffer), "%s;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%s;%s;SIM",
               timeManager.getDataHoraCSV().c_str(), 
               sensores.getPressure(0), sensores.getPressure(1),
               sensores.getPressure(2), sensores.getPressure(3),
               sensores.getPressure(4), sensores.getPressure(5),
               experimento.getTempoRestante(agora).c_str(),
               sensores.isHardwareOK() ? "AUTOMATICO" : "RECUPERACAO_I2C");

      logger.log(nomeArquivoSDAtual, String(logBuffer)); 
      experimento.incrementarContador();

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
    
    if (experimento.getTempoRestante(agora) == "00:00:00") {
        experimento.stop();
        if (LittleFS.exists("/status.txt")) LittleFS.remove("/status.txt");
        nomeArquivoSDAtual = "";
    }
  } 
  else {
    if (LittleFS.exists("/status.txt")) LittleFS.remove("/status.txt");
  }
}