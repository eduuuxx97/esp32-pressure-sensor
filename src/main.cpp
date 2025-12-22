#include <Arduino.h>
#include "Config.h"
#include <SensorLib.h>
#include <DataLogger.h>
#include <WebManager.h> 

// Instancia os objetos (As peças do Lego)
SensorLib sensores;
DataLogger logger;
WebManager web; // O gerenciador do site

unsigned long ultimo_log = 0;

// Função temporária de hora (enquanto não tem RTC)
String obterHora() {
  unsigned long t = millis() / 1000;
  char buffer[20];
  sprintf(buffer, "%02lu:%02lu:%02lu", (t/3600), (t%3600)/60, (t%60));
  return String(buffer);
}

void setup() {
  Serial.begin(115200);

  // 1. Inicia Sensores
  sensores.begin();

  // 2. Inicia SD
  Serial.print("SD Card... ");
  if(logger.begin()) Serial.println("OK");
  else Serial.println("FALHA");

  // 3. Inicia Web (Passando os sensores e o logger para ela gerenciar)
  // O símbolo '&' significa "endereço de"
  web.begin(&sensores, &logger); 
}

void loop() {
  // Mantém o site vivo
  web.handle();
  
  // Atualiza leitura
  sensores.update();

  // Lógica de Gravação (A cada 2s)
  if(millis() - ultimo_log > 2000) {
    ultimo_log = millis();
    
    String linha = obterHora() + ",";
    for(int i=0; i<NUM_SENSORES; i++) {
      linha += String(sensores.getPressure(i), 2);
      if(i < NUM_SENSORES-1) linha += ",";
    }
    logger.log(linha);
  }
}