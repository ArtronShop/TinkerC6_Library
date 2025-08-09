#include <Arduino.h>
#include "TinkerC6-RS485.h"
#include "TinkerC6-Pin.h"

TinkerC6_RS485::TinkerC6_RS485() : HardwareSerial(1) {
    this->setPins(RS485_RX_PIN, RS485_TX_PIN);
}

void TinkerC6_RS485::enable() {
    pinMode(RS485_DIR_PIN, OUTPUT);
    digitalWrite(RS485_DIR_PIN, LOW);

    gpio_pullup_en((gpio_num_t) RS485_RX_PIN);
    gpio_hold_en((gpio_num_t) RS485_RX_PIN);

    pinMode(POWER_RS485_EN, OUTPUT);
    digitalWrite(POWER_RS485_EN, LOW);
    gpio_hold_en((gpio_num_t) POWER_RS485_EN); // Enable pad hold for keep logic output when in light sleep mode
}

void TinkerC6_RS485::disable() {
    pinMode(RS485_DIR_PIN, ANALOG);
    
    gpio_hold_dis((gpio_num_t) POWER_RS485_EN); // Disable pad hold
    digitalWrite(POWER_RS485_EN, HIGH);
    pinMode(POWER_RS485_EN, ANALOG);
}

void TinkerC6_RS485_preTransmission() {
    gpio_hold_dis((gpio_num_t) RS485_DIR_PIN); // Disable pad hold
    digitalWrite(RS485_DIR_PIN, HIGH);
    gpio_hold_dis((gpio_num_t) RS485_DIR_PIN); // Disable pad hold
}

void TinkerC6_RS485_postTransmission() {
    gpio_hold_dis((gpio_num_t) RS485_DIR_PIN); // Disable pad hold
    digitalWrite(RS485_DIR_PIN, LOW);
    gpio_hold_dis((gpio_num_t) RS485_DIR_PIN); // Disable pad hold
}
