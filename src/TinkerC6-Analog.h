#ifndef __TINKER_C6_ANALOG__
#define __TINKER_C6_ANALOG__
#include <Wire.h>

class TinkerC6_Analog {
    private:
        uint8_t _addr;
        TwoWire *_wire = NULL;
        int16_t _raw_shunt_voltage;
        int16_t _4mA_raw_value, _20mA_raw_value;

        bool writeReg(uint8_t addr, uint16_t *data) ;
        bool readReg(uint8_t addr, uint16_t *data) ;

    public:
        TinkerC6_Analog() ;

        void enable() ;
        void disable() ;
        float getCurrent() ; // return 0 - 20 mA

};

#endif
