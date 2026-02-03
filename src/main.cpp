#include <Arduino.h>
#include "Config.h"
#include <SensorLib.h>
#include <DataLogger.h>
#include <WebManager.h>
#include <ExperimentManager.h>
#include <TimeManager.h>

SensorLib sensores;
DataLogger logger;
WebManager web;
ExperimentManager experimento;
TimeManager timeManager;

unsigned long ultimo_log = 0;
String nomeArquivoSDAtual = ""; 

void setup() {
  Serial.begin(115200);
  timeManager.begin();
  sensores.begin();
  
  // Forçamos o início do SD com o pino 5 que validamos no teste
  if(logger.begin()) { 
    Serial.println("SD Inicializado com sucesso!");
  } else {
    Serial.println("Falha ao iniciar SD no sistema principal.");
  }

  web.begin(&sensores, &logger, &experimento, &timeManager);
}

void loop() {
  web.handle();
  sensores.update();

  if (experimento.isRunning()) {
    if (nomeArquivoSDAtual == "") {
      nomeArquivoSDAtual = timeManager.getNomeArquivoFormatado(); // Ex: /L021830.csv
      logger.log(nomeArquivoSDAtual, "Data;Hora;S1;S2;S3;S4;S5;S6;Tempo_Exp;Modo");
    }

    if(millis() - ultimo_log >= experimento.getIntervalo()) {
      ultimo_log = millis();
      experimento.incrementarContador(); // ID vital para o filtro do Python
      
      char linha[160]; 
      String dh = timeManager.getDataHoraCSV(); 
      float p[6];
      for(int i = 0; i < 6; i++) p[i] = sensores.getPressure(i);

      // Formatação padrão CSV Brasil (Separador ";" e Decimal ",")
      sprintf(linha, "%s;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f;%s;AUTOMATICO", 
              dh.c_str(), p[0], p[1], p[2], p[3], p[4], p[5], 
              experimento.getTempoRestante().c_str());

      String strFinal = String(linha);
      strFinal.replace(".", ","); 
      
      logger.log(nomeArquivoSDAtual, strFinal);
    }
  } else {
    nomeArquivoSDAtual = ""; // Reset para gerar novo arquivo no próximo clique
  }
}