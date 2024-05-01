#pragma once

#include <inttypes.h>
#include <stdbool.h>

struct basicTimer { // for TIM 6 and 7
    volatile uint32_t CR1, CR2, RES1, DIER, SR, EGR, RES2, RES3, RES4, CNT, PSC, ARR;
};

void clock_timer(uint8_t, bool);
void timer_enable_counter(struct basicTimer *, bool);
void timer_generate_event(struct basicTimer *);
void timer_set_prescaler(struct basicTimer *, uint16_t);
void timer_set_auto_reload(struct basicTimer *, uint16_t);
uint16_t timer_get_counter(struct basicTimer *);