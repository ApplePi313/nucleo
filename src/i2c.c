#include "i2c.h"

void clock_i2c(uint8_t clockNum, bool enable) { 
    // i2c4 is in APB1ENR2, and cannot be enable with this function
    // 0x4002 1000 + 0x58 is APB1ENR1, bits 21-23 are i2c 1-3
    // Because i2c is 1-3, 0 or values greater than 3 are not acceptable inputs for clockNum

    if (clockNum < 1 || clockNum > 3) return;

    *(volatile uint32_t *)(0x40021000 + 0x58) &= ~(1U << (clockNum + 19));
    *(volatile uint32_t *)(0x40021000 + 0x58) |= enable << (clockNum + 19);
}