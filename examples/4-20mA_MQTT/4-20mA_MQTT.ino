#include <Arduino.h>
#include <WiFi.h>
#include <TinkerC6.h>
#include <Wire.h>
#include <PubSubClient.h>
#include "esp_pm.h"
#include "esp_wifi.h"

// WiFi Configs
#include "secret.h"
const char * ssid = WIFI_SSID; // WiFi Name
const char * password = WIFI_PASSWORD; // WiFi Password

const char * mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

void mqtt_cb(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

WiFiClient espClient;
PubSubClient client(mqtt_server, mqtt_port, mqtt_cb, espClient);

void setup() {
  Serial.begin(115200);

  // Enable Power Management
  esp_pm_config_t pm_config = {
    .max_freq_mhz = 160,
    .min_freq_mhz = 10,
    .light_sleep_enable = true
  };
  esp_pm_configure(&pm_config);

  Serial.println("Sensor reading...");
  float mA = TinkerC6.Analog.getCurrent();

  Serial.print("Current: ");
  Serial.print(mA, 2);
  Serial.print(" mA");
  Serial.println();

  // Connact WiFi
  Serial.print("WiFi Connect");
  WiFi.begin(ssid, password);
  unsigned long start_connect = millis();
  while(!WiFi.isConnected()) {
    Serial.print(".");
    if ((millis() - start_connect) > (30 * 1000)) { // if stall can't connect wifi after 30 sec
      break;
    }
    // TinkerC6.Power.enterToLightSleep(1000); // wait wifi connect with light sleep
    delay(1000);
  }

  if (!WiFi.isConnected()) { // If wifi can't connect
    Serial.println("Fail, sleep !");
    TinkerC6.Power.enterToDeepSleep(5000); // Go to deep sleep : Not do thing after here
  }

  Serial.println(" Connected");

  // Enable WiFi Power Save
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

  // MQTT Server connect
  Serial.print("MQTT Connect... ");
  client.connect("Tinker-C6-I2C-MQTT");
  /*start_connect = millis();
  while(!client.connect("Tinker-C6-I2C-MQTT")) {
    Serial.print(".");
    if ((millis() - start_connect) > (30 * 1000)) { // if stall can't connect mqtt after 30 sec
      break;
    }
    TinkerC6.Power.enterToLightSleep(1000); // wait mqtt connect with light sleep
  }
  Serial.println();*/

  if (!client.connected()) {
    Serial.println("Fail, sleep !");
    TinkerC6.Power.enterToDeepSleep(5000); // Go to deep sleep : Not do thing after here
  }

  Serial.println("Connected");

  String payload = "{ \"mA\": " + String(mA, 2) + " }";

  // Send to MQTT
  int err_count = 0;
  while(!client.publish("/Tinker-C6/sensor1", payload.c_str())) {
    Serial.println("MQTT Publish fail !");
    TinkerC6.Power.enterToLightSleep(1000);
    err_count++;
    if (err_count > 5) { // try only 5 times
      break;
    }
  }

  Serial.println("MQTT Publish OK !");

  Serial.println("MQTT Disconnect");
  client.disconnect();

  // Done ! Go to Deep Sleep
  Serial.println("Go to Deep Sleep");
  TinkerC6.Power.enterToDeepSleep(5000); // Go to deep sleep : Not do thing after here
}

void loop() {
  
}
