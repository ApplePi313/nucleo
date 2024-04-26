#pragma once

#include <inttypes.h>
#include <stdbool.h>

enum GpioMode {
    Input = 0,
    GPOutput = 1,
    AF = 2,
    Analog = 3
};

void clock_gpio_bank(char, bool);

void gpio_set_pin_type(uint16_t, bool);
void gpio_set_pin_pupdr(uint16_t, uint8_t);
void gpio_set_pin_speed(uint16_t, uint8_t);
void gpio_set_pin_mode(uint16_t, uint8_t);
void gpio_atomic_set_pin_out(uint16_t, bool);
void gpio_set_pin_out(uint16_t, bool);
bool gpio_read_pin(uint16_t);
