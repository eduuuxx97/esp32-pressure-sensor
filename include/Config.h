#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- Wi-Fi ---
// 'static' diz: Essa variável é privada para cada arquivo que a inclui.
// Isso evita o erro de "multiple definition".
static const char* WIFI_SSID = "ESP32_Pressao";
static const char* WIFI_PASS = "12345678";

// --- Hardware Sensores ---
#define NUM_SENSORES 6

// Arrays TAMBÉM precisam ser static
static const int PINOS_SENSORES[NUM_SENSORES] = {34, 35, 39, 36, 32, 33};

// Variáveis numéricas simples
static const float R_SHUNT = 150.0;
static const float V_REF = 3.3;
static const int ADC_RES = 4095;
static const float PRESSAO_MIN = 0.5;
static const float PRESSAO_MAX = 3.0;
static const float ALPHA = 0.1; // Fator do filtro

// --- Hardware SD Card ---
#define PIN_CS_SD 5
// Os outros pinos SPI são padrão do ESP32 (MOSI 23, MISO 19, SCK 18)

#endif