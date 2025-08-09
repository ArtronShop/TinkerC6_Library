#include <Arduino.h>
#include "TinkerC6-I2C.h"
#include "TinkerC6-Pin.h"

TinkerC6_I2C::TinkerC6_I2C() {
    // ----
}

void TinkerC6_I2C::enable() {
    pinMode(POWER_I2C_EN_PIN, OUTPUT);
    digitalWrite(POWER_I2C_EN_PIN, LOW);
    gpio_hold_en((gpio_num_t) POWER_I2C_EN_PIN); // Enable pad hold for keep logic output when in light sleep mode
}

void TinkerC6_I2C::disable() {
    gpio_hold_dis((gpio_num_t) POWER_I2C_EN_PIN); // Disable pad hold
    digitalWrite(POWER_I2C_EN_PIN, HIGH);
    pinMode(POWER_I2C_EN_PIN, ANALOG);
}