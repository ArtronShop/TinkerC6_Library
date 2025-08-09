#include <stdint.h>

// #define HAS_MAX17048

#ifndef HAS_MAX17048
#define R1_VALUE (33000) // 33 k ohm
#define R2_VALUE (100000) // 100 k ohm
#endif

#ifdef HAS_MAX17048
#include <Wire.h>
#endif

class TinkerC6_Power {
    private:
#ifdef HAS_MAX17048
        uint8_t _addr;
        TwoWire *_wire = NULL;

        bool writeReg(uint8_t addr, uint16_t *data) ;
        bool readReg(uint8_t addr, uint16_t *data) ;
#endif

    public:
        TinkerC6_Power();

        // Power 12V for 4-20mA and RS485
        void enable12V() ;
        void disable12V() ;

        // Low Power Mode
        void enterToLightSleep(uint64_t mS) ;
        void enterToDeepSleep(uint64_t mS) ;
        
        int getSOC() ; // percent of battery
        float getBatteryVoltage() ; // Get battery voltage

};
