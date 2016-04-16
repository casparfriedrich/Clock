#ifndef I2CMASTER_H_
#define I2CMASTER_H_

#include <stdint.h>

class I2cMaster {
   public:
    I2cMaster();
    virtual ~I2cMaster();

    int8_t startTransmission();
    int8_t stopTransmission();
    int8_t readBytes(uint8_t address, uint8_t bytes[], uint32_t numBytes);
    int8_t writeBytes(uint8_t address, uint8_t bytes[], uint32_t numBytes);
};

#endif