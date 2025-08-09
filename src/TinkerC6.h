#include <TinkerC6-Power.h>
#include <TinkerC6-I2C.h>
#include <TinkerC6-Analog.h>
#include <TinkerC6-RS485.h>

class TinkerC6Class {
    public:
        TinkerC6_Power Power;
        TinkerC6_I2C I2C;
        TinkerC6_Analog Analog;
        TinkerC6_RS485 RS485;

};

extern TinkerC6Class TinkerC6;
