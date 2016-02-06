#ifndef DS1307_H_
#define DS1307_H_

#include <stdint.h>
#include <time.h>

#include <i2cmaster.h>

class DS1307 {
    public:
        DS1307(I2cMaster *i2cMaster, i2cAddress_t address);
        ~DS1307();
        time_t read();
        void write(time_t ntpTime);
    private:
    	I2cMaster *i2cMaster;
        i2cAddress_t address;
};

#endif
