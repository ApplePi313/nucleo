#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include "gpio.h"
#include "timer.h"

extern const uint16_t counts_per_cycle; // # of counts every 20ms
extern const uint16_t time_scale;
extern const uint8_t timer_prescaler;

struct Servo {
    uint16_t pin;
    struct basicTimer* timer;
    uint16_t servo_time; // in tens of milliseconds
    int16_t start_time;
};

void servo_setup_pin(struct Servo*, uint16_t);
void servo_setup_timer(struct Servo*, struct basicTimer*);
void servo_set_rotation(struct Servo*, int16_t deg);