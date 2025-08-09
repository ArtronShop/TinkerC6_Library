#ifndef __TINKER_C6_RS485__
#define __TINKER_C6_RS485__

#include <Arduino.h>

void TinkerC6_RS485_preTransmission() ;
void TinkerC6_RS485_postTransmission() ;

class TinkerC6_RS485 : public HardwareSerial {
    public:
        TinkerC6_RS485() ;

        void enable() ;
        void disable() ;
        
};

#endif

