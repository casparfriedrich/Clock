#include <stdint.h>
#include <stdio.h>

#include "i2cmaster.h"

#include "ht16k33_display.h"

#define I2C I2cMaster::getInstance()

static const uint8_t colonMask[] = {0b00000000, 0b00000010};

static const uint8_t numberMasks[] = {0b00111111, 0b00000110, 0b01011011,
        0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111, 0b01111111,
        0b01101111};

HT16K33_Display::HT16K33_Display(uint8_t address) {
    this->address = address;
    uint8_t initSequence[] = {0x21, 0xA0, 0xE0, 0x81};
    for (uint8_t i = 0; i < sizeof(initSequence); i++) {
        I2C->startTransmission();
        I2C->writeBytes(address, &initSequence[i], 1);
    }
    I2C->stopTransmission();
}

HT16K33_Display::~HT16K33_Display() {
}

void HT16K33_Display::setBrightness(uint8_t brightness) {
    if (brightness > 0xF) {
        brightness = 0xF;
    }
    brightness += 0xE0;  // Brightness register offset
    I2C->startTransmission();
    I2C->writeBytes(address, &brightness, 1);
    I2C->stopTransmission();
}

void HT16K33_Display::toggleColon() {
    uint8_t tmp[] = {digitColon, 0};
    I2C->startTransmission();
    I2C->writeBytes(address, &tmp[0], sizeof(tmp[0]));
    I2C->readBytes(address, &tmp[1], sizeof(tmp[1]));
    tmp[1] = ~tmp[1];
    I2C->startTransmission();
    I2C->writeBytes(address, tmp, sizeof(tmp));
    I2C->stopTransmission();
}

void HT16K33_Display::updateDigit(digit_t digit, uint8_t number) {
    uint8_t tmp[] = {digit, numberMasks[number]};
    I2C->startTransmission();
    I2C->writeBytes(address, tmp, sizeof(tmp));
    I2C->stopTransmission();
}
