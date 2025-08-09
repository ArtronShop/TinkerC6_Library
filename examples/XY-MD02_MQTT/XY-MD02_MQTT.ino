#ifdef MODBUS_MQTT_TEST

#include <Arduino.h>
#include <WiFi.h>
#include <TinkerC6.h>
#include <ModbusMaster.h>
#include <PubSubClient.h>
#include "esp_pm.h"
#include "esp_wifi.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"

// WiFi Configs
#include "secret.h"
const char * ssid = WIFI_SSID; // WiFi Name
const char * password = WIFI_PASSWORD; // WiFi Password

const char * mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;

ModbusMaster sensor;

void mqtt_cb(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

WiFiClient espClient;
PubSubClient client(mqtt_server, mqtt_port, mqtt_cb, espClient);

const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                       "abcdefghijklmnopqrstuvwxyz"
                       "0123456789";
const int charsetSize = sizeof(charset) - 1;

String generateRandomString(int length) {
  String result = "";
  for (int i = 0; i < length; i++) {
    // esp_random() returns a 32-bit unsigned int
    uint32_t rnd = esp_random();
    int index = rnd % charsetSize;
    result += charset[index];
  }
  return result;
}

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
  TinkerC6.Power.enable12V();

  // Wait sensor boot up
  TinkerC6.Power.enterToLightSleep(2000);

  // XY-MD02 init with Modbus slave ID 1
  TinkerC6.RS485.begin(9600, SERIAL_8N1);
  sensor.begin(1, TinkerC6.RS485);
  sensor.preTransmission(TinkerC6_RS485_preTransmission);
  sensor.postTransmission(TinkerC6_RS485_postTransmission);
  sensor.idle([]() { // when idle (no data coming)
    // esp_sleep_enable_uart_wakeup(UART_NUM_1);
    // uart_set_wakeup_threshold(UART_NUM_1, 1);
    // UART1.sleep_conf2.wk_mode_sel = 2; // UART_WK_MODE_SEL = 2: When the UART receiver detects a start bit, the chip will be woken up.
    // TinkerC6.Power.enterToLightSleep(5); // Light sleep : 100 mS for wait data come
    // sensor.begin(1, TinkerC6.RS485);
  });
  sensor.ku16MBResponseTimeout = 100;

  // Enable RS485
  TinkerC6.RS485.enable();

  bool sensor_fail = false;
  int err_count = 0;
  Serial.print("Sensor reading");
  uint8_t res;
  while ((res = sensor.readInputRegisters(1, 2)) != sensor.ku8MBSuccess) {
    Serial.print(".");
    Serial.print("Error Code: " + String(res) + "; ");
    TinkerC6.Power.enterToLightSleep(1000);
    err_count++;
    if (err_count > 5) { // try only 5 times
      sensor_fail = true;
      break;
    }
  }

  if (sensor_fail) { // if sensor fail
    Serial.println(" Fail !");
  } else {
    Serial.println(" OK !");
  }
  
  // Disable I2C Power Output (Sensor OFF)
  Serial.println("Sensor OFF");
  TinkerC6.RS485.end();
  TinkerC6.RS485.disable();
  TinkerC6.Power.disable12V();

  if (sensor_fail) {
    Serial.println("Sleep");
    TinkerC6.Power.enterToDeepSleep(5000); // Go to deep sleep : Not do thing after here
  }

  float t = sensor.getResponseBuffer(0) / 10.0;
  float h = sensor.getResponseBuffer(1) / 10.0;

  Serial.print("Temperature: ");
  Serial.print(t, 1);
  Serial.print(" *C\tHumidity: ");
  Serial.print(h, 1);
  Serial.print(" %RH");
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
  // esp_wifi_set_ps(WIFI_PS_MIN_MODEM); // <-- Don't use it are save power more -.-'

  // MQTT Server connect
  Serial.print("MQTT Connect... ");
  client.connect(String("Tinker-C6-I2C-MQTT-" + generateRandomString(20)).c_str());
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
  delay(100); // wait buffer go out
  Serial.println("MQTT Publish OK !");

  Serial.println("MQTT Disconnect");
  client.disconnect();

  // Done ! Go to Deep Sleep
  unsigned long sleep_time = constrain(30000 - millis(), 5000lu, 30000lu);
  Serial.printf("Go to Deep Sleep %d mS", sleep_time);
  TinkerC6.Power.enterToDeepSleep(sleep_time); // Go to deep sleep : Not do thing after here
}

void loop() {
  
}

#endif
