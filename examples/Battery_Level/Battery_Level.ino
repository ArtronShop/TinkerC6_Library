#include <Arduino.h>
#include <Wire.h>
#include <TinkerC6.h>

void setup() {
  Serial.begin(115200);

  Wire.begin();
}

void loop() {
  int soc = TinkerC6.Power.getSOC();
  float v_cell = TinkerC6.Power.getBatteryVoltage();
  Serial.printf("Batt SOC: %d%%\tVolt: %.02fV\n", soc, v_cell);
  delay(2000);
}
