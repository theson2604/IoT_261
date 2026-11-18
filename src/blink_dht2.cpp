#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_AHTX0.h>
#include <Wire.h>
#include <cmath>

#define NEO_PIN 45
#define LED_COUNT 1

#define I2C_SCL GPIO_NUM_12
#define I2C_SDA GPIO_NUM_11

Adafruit_AHTX0 dht20;

void neo_blinky(void *pvParameters){

    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    // Set all pixels to off to start
    strip.clear();
    strip.show();

    while(1) {                          
        strip.setPixelColor(0, strip.Color(255, 0, 0)); // Set pixel 0 to red
        strip.show(); // Update the strip

        // Wait for 1000 milliseconds
        vTaskDelay(500);

        strip.setPixelColor(0, strip.Color(0, 255, 0)); // Set pixel 0 to red
        strip.show(); // Update the strip

        // Wait for 1000 milliseconds
        vTaskDelay(500);

        strip.setPixelColor(0, strip.Color(0, 0, 255)); // Set pixel 0 to red
        strip.show(); // Update the strip

        // Wait for 1000 milliseconds
        vTaskDelay(500);

        // Set the pixel to off
        strip.setPixelColor(0, strip.Color(0, 0, 0)); // Turn pixel 0 off
        strip.show(); // Update the strip

        // Wait for another 1000 milliseconds
        vTaskDelay(500);
    }
}

void led_blinky(void *pvParameters) {
    pinMode(GPIO_NUM_48, OUTPUT); // Initialize LED pin

    while(1) {
        digitalWrite(GPIO_NUM_48, HIGH); // Turn ON LED
        vTaskDelay(500);
        digitalWrite(GPIO_NUM_48, LOW); // Turn OFF LED
        vTaskDelay(500);
    }
}

void counterTask(void *parameter)
{
    int counter = 0;

    while (1)
    {
        Serial.printf("Counter: %d\n", counter);

        counter++;

        vTaskDelay(1000);
    }
}

void dht20Task(void *parameter)
{
    sensors_event_t humidity, temp;

    while (1) {
        if (dht20.getEvent(&humidity, &temp) &&
            std::isfinite(temp.temperature) && std::isfinite(humidity.relative_humidity)) {
            // One JSON object per line for the computer's serial-to-MQTT bridge.
            Serial.printf("{\"temperature\":%.2f,\"humidity\":%.2f}\n",
                          temp.temperature, humidity.relative_humidity);
        } else {
            Serial.println("DHT20 reading failed; skipping invalid data.");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void setup() {
  // put your setup code here, to run once for task creation
  Serial.begin(115200);

  // Give Serial Monitor time to open, then continue even without a USB host.
  const uint32_t serialWaitStarted = millis();
  while (!Serial && millis() - serialWaitStarted < 5000) {
    delay(10);
  }
  delay(500);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!dht20.begin()) {
      Serial.println("DHT20 not detected!");
      while (1) {
          delay(1000);
      }
  }

  Serial.printf("DHT20 detected at address: 0x%02X\n",
                AHTX0_I2CADDR_DEFAULT);

  xTaskCreate(led_blinky, "LED Control", 2048, NULL, 2, NULL);
  xTaskCreate(neo_blinky, "NEO Control", 2048, NULL, 2, NULL);
  xTaskCreate(counterTask, "Serial Counter", 2048, NULL, 2, NULL);
  xTaskCreate(dht20Task, "Sensor Reading", 2048, NULL, 2, NULL);
}

void loop() {
    delay(1000); // Sensor and LED work runs in FreeRTOS tasks.
}
