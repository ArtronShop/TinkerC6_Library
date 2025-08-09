#include <Arduino.h>
#include <Wire.h>
#include "TinkerC6-Pin.h"
#include "TinkerC6-Analog.h"
#include "TinkerC6.h"

static const char * TAG = "TinkerC6-Analog";

// Register
#define CONFIGURATION_REGISTER (0x01)
#define SHUNT_VOLTAGE_REGISTER (0x02)

// Struct
typedef struct {
  uint32_t MODE : 3; // Operating Mode
  uint32_t VSHCT : 3; // Shunt Voltage Conversion Time
  uint32_t VBUSCT : 3; // Bus Voltage Conversion Time
  uint32_t AVG : 3; // Averaging Mode
  uint32_t _unused1 : 3;
  uint32_t RST : 1; // Reset Bit
} ConfigurationRegister_t;

TinkerC6_Analog::TinkerC6_Analog() {
    this->_wire = &Wire;
    this->_addr = 0x45;

    // ---
    this->_4mA_raw_value = 6410;
    this->_20mA_raw_value = 31952;
}

void TinkerC6_Analog::enable() {
    pinMode(POWER_4_20mA_EN_PIN, OUTPUT);
    digitalWrite(POWER_4_20mA_EN_PIN, LOW);
    gpio_hold_en((gpio_num_t) POWER_4_20mA_EN_PIN); // Enable pad hold for keep logic output when in light sleep mode
}

void TinkerC6_Analog::disable() {
    gpio_hold_dis((gpio_num_t) POWER_4_20mA_EN_PIN); // Disable pad hold
    digitalWrite(POWER_4_20mA_EN_PIN, HIGH);
    pinMode(POWER_4_20mA_EN_PIN, ANALOG);
}

float TinkerC6_Analog::getCurrent() {
    // Enable ADC power
    this->enable();

    this->_wire->begin();

    // Configs ADC
    ConfigurationRegister_t configs;
    memset(&configs, 0, sizeof(configs));
    configs.MODE = 0b001; // Shunt voltage, triggered
    configs.VSHCT = 0b100; // Shunt Voltage Conversion Time : 1.1 ms
    configs.VBUSCT = 0b100; // Bus Voltage Conversion Time : 1.1 ms
    configs.AVG = 0b010; // Averaging : 16 times
    writeReg(0x00, (uint16_t*) &configs);

    // Wait Conversion
    /* esp_sleep_enable_timer_wakeup(20000); // wait 20mS
    esp_light_sleep_start(); */
    TinkerC6.Power.enterToLightSleep(20); // wait 20mS

    // Read Shunt Voltage
    this->readReg(1, (uint16_t *) &this->_raw_shunt_voltage);

    // Disable ADC power
    this->disable();

    return map(this->_raw_shunt_voltage, this->_4mA_raw_value, this->_20mA_raw_value, 400, 2000) / 100.0f;
}

bool TinkerC6_Analog::writeReg(uint8_t addr, uint16_t *data) {
  this->_wire->beginTransmission(this->_addr);
  this->_wire->write(addr);
  this->_wire->write((*data >> 16) & 0xFF);
  this->_wire->write(*data & 0xFF);
  int error = this->_wire->endTransmission();
  return error == 0;
}

bool TinkerC6_Analog::readReg(uint8_t addr, uint16_t *data) {
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
