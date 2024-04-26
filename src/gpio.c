#include "gpio.h"

void clock_gpio_bank(char bank, bool enable) {
    // 0x4002 1000 + 0x4C is AHB2ENR, bits 0-8 are gpio banks

    *(volatile uint32_t *)(0x40021000 + 0x4C) &= ~(1U << (bank - 'A'));
    *(volatile uint32_t *)(0x40021000 + 0x4C) |= enable << (bank - 'A');
}

void gpio_set_pin_type(uint16_t pin, bool type) {
    // The OTYPER register has an offset of 0x04, and it sets push-pull(0, default) and open-drain(1)

    uint8_t bank = (uint8_t)(pin >> 8);
    uint8_t pin_no = (uint8_t)pin;

    *(volatile uint32_t *)(0x48000000 + 0x04 + 0x400 * bank) &= ~(1U << pin_no);
    *(volatile uint32_t *)(0x48000000 + 0x04 + 0x400 * bank) |= type << pin_no;
}

void gpio_set_pin_pupdr(uint16_t pin, uint8_t mode) { 
    // mode must be between 0 and 2
    // The PUPDR register has an offset of 0x0C, 0 is none, 1 is pull-up, 2 is pull-down, 3 is reserved

    uint8_t bank = (uint8_t)(pin >> 8);
    uint8_t pin_no = (uint8_t)pin;

    *(volatile uint32_t *)(0x48000000 + 0x0C + 0x400 * bank) &= ~(3U << (pin_no * 2));
    *(volatile uint32_t *)(0x48000000 + 0x0C + 0x400 * bank) |= mode << (pin_no * 2);
}

void gpio_set_pin_speed(uint16_t pin, uint8_t speed) {
    // speed must be between 0 and 3
    // The OSPEEDR has an offset of 0x08, the speed goes low(0) to very fast(3)

    uint8_t bank = (uint8_t)(pin >> 8);
    uint8_t pin_no = (uint8_t)pin;

    *(volatile uint32_t *)(0x48000000 + 0x08 + 0x400 * bank) &= ~(3U << (pin_no * 2));
    *(volatile uint32_t *)(0x48000000 + 0x08 + 0x400 * bank) |= speed << (pin_no * 2);
}

void gpio_set_pin_mode(uint16_t pin, uint8_t mode) {
    // mode must be between 0 and 3
    // The MODER register has no offset, 0 is input, 1 is general purpose output, 2 is alternate function, 3(default) is analog

    uint8_t bank = (uint8_t)(pin >> 8);
    uint8_t pin_no = (uint8_t)(pin & 0xffff);

    *(volatile uint32_t *)(0x48000000 + 0x400 * bank) &= ~(3U << (pin_no * 2));
    *(volatile uint32_t *)(0x48000000 + 0x400 * bank) |= (mode << (pin_no * 2));
}

void gpio_atomic_set_pin_out(uint16_t pin, bool output) {
    // The BSRR has an offset of 0x18, bits 16-31 reset, bits 0-15 set and have priority

    uint8_t bank = (uint8_t)(pin >> 8);
    uint8_t pin_no = (uint8_t)pin;

    *(volatile uint32_t *)(0x48000000 + 0x18 + 0x400 * bank) &= ~(1U << pin_no);
    *(volatile uint32_t *)(0x48000000 + 0x18 + 0x400 * bank) |= output << pin_no;

    output = !output;
    *(volatile uint32_t *)(0x48000000 + 0x18 + 0x400 * bank) &= ~(1U << (pin_no + 16));
    *(volatile uint32_t *)(0x48000000 + 0x18 + 0x400 * bank) |= output << (pin_no + 16);
}

void gpio_set_pin_out(uint16_t pin, bool output) {
    // The ODR has an offset of 0x14

    uint8_t bank = (uint8_t)(pin >> 8);
    uint8_t pin_no = (uint8_t)pin;

    *(volatile uint32_t *)(0x48000000 + 0x14 + 0x400 * bank) &= ~(1U << pin_no);
    *(volatile uint32_t *)(0x48000000 + 0x14 + 0x400 * bank) |= output << pin_no;
}

bool gpio_read_pin(uint16_t pin) {
    // The IDR has an offset of 0x10

    uint8_t bank = (uint8_t)(pin >> 8);
    uint8_t pin_no = (uint8_t)pin;

    return (*(volatile uint32_t *)(0x48000000 + 0x400 * bank + 0x10) &= (0b1U << pin_no) >> pin_no);
}