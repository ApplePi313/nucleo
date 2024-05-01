#pragma once

#include <inttypes.h>
#include <stdbool.h>

void clock_dac(bool);
void dac_set_output(uint16_t);
void dac_enable_channel(uint8_t, bool);