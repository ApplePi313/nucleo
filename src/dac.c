#include "dac.h"

void clock_dac(bool enable) {
    // 0x4002 1000 + 0x58 is APB1ENR, bit 29 is for dac
    *((volatile uint32_t*)(0x40021000 + 0x58)) |= (enable << 29);
}

void dac_set_output(uint16_t value) {
    *((volatile uint32_t *)(0x40007400 + 0x14)) = value;
}

void dac_enable_channel(uint8_t channel, bool enable) {
    if (channel != 1 && channel != 2) return;
    *((volatile uint32_t *)(0x40007400 + 0x00)) |= (enable << 16 * (channel - 1));
}