#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "gpio.h"
#include "dac.h"
#include "timer.h"
#include "servo.h"

#define PIN(bank, pin) ((uint16_t)((bank - 'A') << 8) | ((uint16_t)pin))

/*
 * Things to keep in mind:
 * GPIO banks begin at 0x48000000
 * For GY-521 gyro, i2c address is 0x68 when AD0 pin is low, 0x69 when it's high
 * Joystick buttons use a pull-up resistor, and are HIGH when not pressed
 */

int main(void) {
    struct Servo servo;
    uint16_t pin_A5 = PIN('A', 5);
    uint16_t pin_A0 = PIN('A', 0);
    struct basicTimer *tim7 = (struct basicTimer *)(0x40000000 + 0x400 * (7 - 2));
    struct basicTimer *tim6 = (struct basicTimer *)(0x40000000 + 0x400 * (6 - 2));

    clock_timer(6, 1);
    clock_timer(7, 1);

    gpio_set_pin_mode(pin_A0, Input);
    gpio_set_pin_pupdr(pin_A0, 2); // set pin A0 to have a pull-down resistor

    servo_setup_pin(&servo, pin_A5);
    servo_setup_timer(&servo, tim7);

    timer_enable_counter(tim6, 1);
    timer_set_prescaler(tim6, 60000);
    timer_set_auto_reload(tim6, 360);
    timer_generate_event(tim6);

    // clock_dac(1);
    // dac_enable_channel(2, 1);

    while (true)
    {
        // if ((tim7 -> CNT) != 0) {
        //     gpio_set_pin_out(pin_A5, gpio_read_pin(pin_A0));
        // } else {
        //     gpio_set_pin_out(pin_A5, 0);
        // }
        // *((volatile uint32_t *)(0x40007400 + 0x14)) = (tim7->CNT % 4096);
        // if (tim7->CNT > 4096) {
        //     dac_set_output((uint16_t)(4096 - ((uint16_t)tim7->CNT - 4096)/3));
        // } else {
        //     dac_set_output((uint16_t)tim7->CNT);
        // }
        // if (start_time != -1 && tim7->CNT % counts_per_cycle <= (uint16_t) (start_time + servo_time)) {
        //     // dac_set_output((uint16_t) 4095);
        //     gpio_atomic_set_pin_out(pin_A5, 1);
        // } else if (start_time == -1 && tim7->CNT % counts_per_cycle <= servo_time) {
        //     start_time = (int16_t) (tim7->CNT % counts_per_cycle);
        //     // dac_set_output((uint16_t) 4095);
        //     gpio_atomic_set_pin_out(pin_A5, 1);
        // } else {
        //     start_time = -1;
        //     // dac_set_output((uint16_t) 0);
        //     gpio_atomic_set_pin_out(pin_A5, 0);
        // }

        servo_set_rotation(&servo, (int16_t)(180 - abs(timer_get_counter(tim6) - 180) - 90));
        // servo_set_rotation(&servo, 0);
    }
}

// Startup code
__attribute__((naked, noreturn)) void _reset(void) {
    // memset .bss to zero, and copy .data section to RAM region
    extern long _sbss, _ebss, _sdata, _edata, _sidata;
    for (long *src = &_sbss; src < &_ebss; src++)
        *src = 0;
    for (long *src = &_sdata, *dst = &_sidata; src < &_edata;)
        *src++ = *dst++;

    main(); // Call main()
    for (;;)
        (void)0; // Infinite loop in the case if main() returns
}

extern void _estack(void); // Defined in link.ld

// 16 standard and 91 STM32-specific handlers
__attribute__((section(".vectors"))) void (*tab[16 + 91])(void) = {_estack, _reset};
