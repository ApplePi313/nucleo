#include "dac.h"

void clock_dac(bool enable) {
    // 0x4002 1000 + 0x58 is APB1ENR, bit 29 is for dac
    *((volatile uint32_t*)(0x40021000 + 0x58)) |= (1 << 29);
}