#include "timer.h"

void clock_timer(uint8_t number, bool enable) {
    // 0x4002 1000 + 0x58 is APB1ENR, bits 0-5 are timers 2 - 7

    *(volatile uint32_t *)(0x40021000 + 0x58) &= ~(1U << (number - 2));
    *(volatile uint32_t *)(0x40021000 + 0x58) |= enable << (number - 2);
}

void timer_enable_counter(struct basicTimer *tim, bool enable) {
    tim->CR1 |= enable;
}

void timer_set_prescaler(struct basicTimer *tim, uint16_t prescaler) {
    tim->PSC = prescaler;
}

void timer_set_auto_reload(struct basicTimer *tim, uint16_t auto_reload) {
    tim->ARR = auto_reload;
}

void timer_generate_event(struct basicTimer *tim) {
    tim->EGR |= 1U;
}

uint16_t timer_get_counter(struct basicTimer *tim) {
    return (uint16_t)tim->CNT;
}