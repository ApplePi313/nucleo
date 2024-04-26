#include <inttypes.h>
#include <stdbool.h>

#include "gpio.h"
#include "dac.h"

#define PIN(bank, pin) ((uint16_t)((bank - 'A') << 8) | ((uint16_t)pin))

#define GPIO(bank) = ((0x48000000) + (bank - 'A') * 0x400)

struct gpio
{
    volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFRL, AFRH, BRR;
};

struct basicTimer
{ // for TIM 6 and 7
    volatile uint32_t CR1, CR2, RES1, DIER, SR, EGR, RES2, RES3, RES4, CNT, PSC, ARR;
};

// Timer functions
void basic_timer_enable_counter(struct basicTimer *, bool);
void basic_timer_generate_event(struct basicTimer *);
void basic_timer_set_prescaler(struct basicTimer *, uint16_t);
void basic_timer_set_auto_reload(struct basicTimer *, uint16_t);
uint16_t basic_timer_get_counter(struct basicTimer *);

// Register clocking functions
void clock_timer(uint8_t, bool);
void clock_i2c(uint8_t, bool);

/*
 * Things to keep in mind:
 * GPIO banks begin at 0x48000000
 * For GY-521 gyro, i2c address is 0x68 when AD0 pin is low, 0x69 when it's high
 * Joystick buttons use a pull-up resistor, and are HIGH when not pressed
 */

int main(void) {
    uint16_t pin_A5 = PIN('A', 5);
    uint16_t pin_A0 = PIN('A', 0);

    struct basicTimer *tim7 = (struct basicTimer *)(0x40000000 + 0x400 * (7 - 2));

    clock_gpio_bank('A', 1);
    clock_timer(6, 1);
    clock_timer(7, 1);

    gpio_set_pin_mode(pin_A5, Analog);
    gpio_set_pin_mode(pin_A0, Input);

    gpio_set_pin_pupdr(pin_A0, 2); // set pin A0 to have a pull-down resistor

    (tim7->CR1) |= 0x1; // enable counter
    (tim7->PSC) = 1024; // set prescaler (how many cycles it takes to increment 1)
    (tim7->ARR) = 4096; // set auto reload (when the timer loops back to 0)
    (tim7->EGR) |= 1;   // reinitialize timer

    // timer_enable_counter(6, 1);(tim7 -> PSC) = 256; // set prescaler
    // timer_set_prescaler(6, 256);
    // timer_set_auto_reload(6, 4096);
    // timer_generate_event(6);

    clock_dac(1);
    *((volatile uint32_t *)(0x40007400 + 0x00)) |= (1 << 16);

    while (true)
    {
        // if ((tim7 -> CNT) != 0) {
        //     gpio_set_pin_out(pin_A5, gpio_read_pin(pin_A0));
        // } else {
        //     gpio_set_pin_out(pin_A5, 0);
        // }
        *((volatile uint32_t *)(0x40007400 + 0x14)) = (tim7->CNT);
    }
}

/*
 *
 * Timer functions:
 *
 */
void basic_timer_enable_counter(struct basicTimer *tim, bool enable) {
    tim->CR1 |= enable;
}

void basic_timer_set_prescaler(struct basicTimer *tim, uint16_t prescaler) {
    tim->PSC = prescaler;
}

void basic_timer_set_auto_reload(struct basicTimer *tim, uint16_t auto_reload) {
    tim->ARR = auto_reload;
}

void basic_timer_generate_event(struct basicTimer *tim) {
    tim->EGR |= 1U;
}

uint16_t basic_timer_get_counter(struct basicTimer *tim) {
    return (uint16_t)tim->CNT;
}

/*
 * I²C Functions:
 */

/*
 * Register Clocking Functions:
 */
void clock_timer(uint8_t number, bool enable) {
    // 0x4002 1000 + 0x58 is APB1ENR, bits 0-5 are timers 2 - 7

    *(volatile uint32_t *)(0x40021000 + 0x58) &= ~(1U << (number - 2));
    *(volatile uint32_t *)(0x40021000 + 0x58) |= enable << (number - 2);
}

void clock_i2c(uint8_t clockNum, bool enable) { 
    // i2c4 is in APB1ENR2, and cannot be enable with this function
    // 0x4002 1000 + 0x58 is APB1ENR1, bits 21-23 are i2c 1-3
    // Because i2c is 1-3, 0 or values greater than 3 are not acceptable inputs for clockNum

    *(volatile uint32_t *)(0x40021000 + 0x58) &= ~(1U << (clockNum + 19));
    *(volatile uint32_t *)(0x40021000 + 0x58) |= enable << (clockNum + 19);
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
