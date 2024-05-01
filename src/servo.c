#include "servo.h"


const uint16_t counts_per_cycle = 400;
const uint16_t time_scale = counts_per_cycle / 200;
const uint8_t timer_prescaler = 200;

    /*
    | o |
    |   |  +
    |   |
      |
      |
      |
      0°


    */
    // 0: -90°
    // 5: -45°
    // 10: 0°
    // 15: 45°
    // 20: 90°
    // 25: 135° ignore

void servo_setup_pin(struct Servo* servo, uint16_t p) {
    clock_gpio_bank((char)(p >> 8) + 'A', 1);
    gpio_set_pin_mode(p, GPOutput);

    servo->pin = p;
    servo->start_time = -1; // initializing this here, otherwise setting rotation doesn't work
}

void servo_setup_timer(struct Servo* servo, struct basicTimer* tim) {
    timer_enable_counter(tim, 1);
    timer_set_prescaler(tim, timer_prescaler);
    timer_set_auto_reload(tim, 200);
    timer_generate_event(tim);

    servo->timer = tim;
}

void servo_set_rotation(struct Servo* servo, int16_t deg) {
    deg += 10;
    deg %= (int8_t) 360;
    deg = deg > 100 ? 100 : deg;
    deg = deg < -80 ? -80 : deg;
    servo->servo_time = (uint16_t)(deg * 25/225 + 10) * time_scale;
    // servo->servo_time = (uint16_t)deg * time_scale;

    if (servo->start_time != -1 && servo->timer->CNT % counts_per_cycle <= (uint16_t)(servo->start_time + servo->servo_time)) {
        gpio_atomic_set_pin_out(servo->pin, 1);
    } else if (servo->start_time == -1 && servo->timer->CNT % counts_per_cycle <= servo->servo_time) {
        servo->start_time = (int16_t) (servo->timer->CNT % counts_per_cycle);
        gpio_atomic_set_pin_out(servo->pin, 1);
    } else {
        servo->start_time = -1;
        gpio_atomic_set_pin_out(servo->pin, 0);
    }
}