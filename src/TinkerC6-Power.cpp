#include <Arduino.h>
#include "TinkerC6-Power.h"
#include "TinkerC6-Pin.h"
#ifdef HAS_MAX17048
#include <Wire.h>
#endif

TinkerC6_Power::TinkerC6_Power() {
#ifdef HAS_MAX17048
    this->_addr = 0x36;
    this->_wire = &Wire;
#endif
}

void TinkerC6_Power::enable12V() {
    pinMode(POWER_12V_EN_PIN, OUTPUT);
    digitalWrite(POWER_12V_EN_PIN, HIGH);
    gpio_hold_en((gpio_num_t) POWER_12V_EN_PIN); // Enable pad hold for keep logic output when in light sleep mode
}

void TinkerC6_Power::disable12V() {
    gpio_hold_dis((gpio_num_t) POWER_12V_EN_PIN); // Disable pad hold
    digitalWrite(POWER_12V_EN_PIN, LOW);
    pinMode(POWER_12V_EN_PIN, ANALOG);
}

void TinkerC6_Power::enterToLightSleep(uint64_t mS) {
    esp_sleep_enable_timer_wakeup(mS * 1000);

    Serial.flush(); // wait serial out before sleep
    esp_light_sleep_start();
}


void TinkerC6_Power::enterToDeepSleep(uint64_t mS) {
    Serial.flush(); // wait serial out before sleep
    esp_deep_sleep(mS * 1000);
}

#ifdef HAS_MAX17048
bool TinkerC6_Power::writeReg(uint8_t addr, uint16_t *data) {
  this->_wire->beginTransmission(this->_addr);
  this->_wire->write(addr);
  this->_wire->write((*data >> 16) & 0xFF);
  this->_wire->write(*data & 0xFF);
  int error = this->_wire->endTransmission();
  return error == 0;
}

bool TinkerC6_Power::readReg(uint8_t addr, uint16_t *data) {
  this->_wire->beginTransmission(this->_addr);
  this->_wire->write(addr);
  if (this->_wire->endTransmission() != 0) {
    return false;
  }

  int n = this->_wire->requestFrom(this->_addr, (uint8_t) 2);
  if (n != 2) {
    return false;
  }

  *data = this->_wire->read() << 8;
  *data |= this->_wire->read();

  return true;
}
#endif

int TinkerC6_Power::getSOC() {
#ifdef HAS_MAX17048
    uint16_t soc = 0;
    this->readReg(0x04, &soc);

    return round(soc / 256.0f);
#else
    uint32_t mV = this->getBatteryVoltage() * 1000.0f;

    return map(mV, 2250, 4200, 0, 100); // 2.25V => 0%, 4.2V => 100%
#endif
}

float TinkerC6_Power::getBatteryVoltage() {
#ifdef HAS_MAX17048
    uint16_t vcell = 0;
    this->readReg(0x02, &vcell);

    return vcell * 78.125f / 1000000.0f;
#else
    uint32_t raw = analogReadMilliVolts(ADC_BAT_PIN);

    return ((raw * (R1_VALUE + R2_VALUE)) / R2_VALUE) / 1000.0f;
#endif
}

