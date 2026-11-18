#include <Arduino.h>
#include "mainserver.h"

// Optional notification hook for future network services.
void internet_task(void *pvParameters)
{
    while (true) {
        if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY) == pdTRUE) {
            Serial.println("[NET] Wi-Fi connected to router.");
        }
    }
}

void setup()
{
    Serial.begin(115200);
    const uint32_t serialWaitStarted = millis();
    while (!Serial && millis() - serialWaitStarted < 5000) {
        delay(10);
    }
    delay(500);

    // Create the semaphore before either task can access it.
    xBinarySemaphoreInternet = xSemaphoreCreateBinary();
    if (xBinarySemaphoreInternet == nullptr) {
        Serial.println("Cannot create Wi-Fi notification semaphore.");
        return;
    }

    // The server owns the LEDs and BOOT button; no separate blink task runs.
    if (xTaskCreate(main_server_task, "Web Server", 8192, nullptr, 2, nullptr) != pdPASS) {
        Serial.println("Cannot start web server task.");
        vSemaphoreDelete(xBinarySemaphoreInternet);
        xBinarySemaphoreInternet = nullptr;
        return;
    }

    if (xTaskCreate(internet_task, "Wi-Fi Notification", 2048, nullptr, 1, nullptr) != pdPASS) {
        Serial.println("Cannot start Wi-Fi notification task; web server remains available.");
    }
}

void loop()
{
    delay(1000);
}
