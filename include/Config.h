#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- Wi-Fi ---
static const char* WIFI_SSID = "ESP32_Pressao";
static const char* WIFI_PASS = "12345678";

// --- Configuração ADS1115 (Endereços I2C) ---
// IMPORTANTE:
// Módulo 1 (S0-S3): Ligue o pino ADDR no GND
#define ADS1_ADDR 0x48 
// Módulo 2 (S4-S5): Ligue o pino ADDR no VCC (3.3V)
#define ADS2_ADDR 0x49 

// --- Hardware Sensores ---
#define NUM_SENSORES 6

// --- Calibração ---
static const float R_SHUNT = 150.0;
static const float PRESSAO_MIN = 0.5;
static const float PRESSAO_MAX = 3.0;
static const float ALPHA = 0.1; // Filtro suave

// --- Hardware SD Card ---
#define PIN_CS_SD 5

// --- Hardware I2C (Relógio e ADS compartilham) ---
#define PIN_I2C_SDA 21
#define PIN_I2C_SCL 22

#endif