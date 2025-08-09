#include <Arduino.h>
#include <WiFi.h>
#include <TinkerC6.h>
#include <Wire.h>
#include <ArtronShop_SHT45.h>
#include <PubSubClient.h>
#include "esp_pm.h"
#include "esp_wifi.h"

// WiFi Configs
#include "secret.h"
const char * ssid = WIFI_SSID; // WiFi Name
const char * password = WIFI_PASSWORD; // WiFi Password

const char * mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

ArtronShop_SHT45 sht45(&Wire, 0x44); // SHT45-AD1B => 0x44


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
  
  // Enable I2C Power Output (Sensor ON)
  Serial.println("Sensor ON");
  TinkerC6.I2C.enable();

  Wire.begin();

  bool sensor_fail = false;
  int err_count = 0;
  Serial.print("Sensor connect...");
  while (!sht45.begin()) {
    Serial.print(".");
    TinkerC6.Power.enterToLightSleep(1000);
    err_count++;
    if (err_count > 5) { // try only 5 times
      sensor_fail = true;
      break;
    }
  }

  if (sensor_fail) { // if sensor fail
    Serial.println(" Fail, sleep !");
    TinkerC6.Power.enterToDeepSleep(5000); // Go to deep sleep : Not do thing after here
  }

  Serial.println(" Connected");

  Serial.print("Sensor reading");
  err_count = 0;
  while(!sht45.measure()) {
    Serial.print(".");
    TinkerC6.Power.enterToLightSleep(1000);
    err_count++;
    if (err_count > 5) { // try only 5 times
      sensor_fail = true;
      break;
    }
  }
  Serial.println();

  float t = sht45.temperature();
  float h = sht45.humidity();

  Serial.print("Temperature: ");
  Serial.print(t, 1);
  Serial.print(" *C\tHumidity: ");
  Serial.print(h, 1);
  Serial.print(" %RH");
  Serial.println();

  // Disable I2C Power Output (Sensor OFF)
  Serial.println("Sensor OFF");
  TinkerC6.I2C.disable();

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

  String payload = "{ \"t\": " + String(t, 1) + ", \"h\": " + String(h, 1) + " }";

  // Send to MQTT
  err_count = 0;
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
