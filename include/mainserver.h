#ifndef YOLO_MAIN_SERVER_H
#define YOLO_MAIN_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "global.h"

#define LED1_PIN 45  // Onboard RGB NeoPixel
#define LED2_PIN 48  // Onboard regular LED
constexpr uint8_t SERVER_BOOT_PIN = 0; // Avoid Arduino's BOOT_PIN definition.

String mainPage();
String mobilePage();
String mobileSettingsPage();
String settingsPage();

void startAP();
void setupServer();
void connectToWiFi();
void main_server_task(void *pvParameters);

#endif
