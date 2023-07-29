#include <inttypes.h>
#include <stdbool.h>

#define PIN(bank, pin) (((bank - 'A') << 8) | (pin))

#define GPIO(bank) = ((*gpio) + bank * 0x400)

struct gpio {
  uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFRL, AFRH, BRR;
};

void clock_gpio_bank(char, bool);
void set_pin_mode(char, uint8_t, uint8_t);
void atomic_set_pin_out(char, uint8_t, bool);
void set_pin_out(char, uint8_t, bool);
void set_pin_type(char, uint8_t, bool);
void set_pin_pupdr(char, uint8_t, uint8_t);
bool read_pin(uint16_t);
void set_pin_speed(char, uint8_t, uint8_t);

uint8_t counter = 0;

uint16_t pin_A5 = PIN('A', 5);
uint16_t pin_A0 = PIN('A', 0);

// GPIO banks begin at 0x48000000

// For GY-521 gyro, i2c address is 0x68 when AD0 pin is low, 0x69 when it's high

int main(void) {

  clock_gpio_bank('A', 1);

  set_pin_mode('A', 5, 1);
  set_pin_mode('A', 0, 0);

  set_pin_pupdr('A', 0, 2); // set it to have a pull-down resistor

  // set_pin_speed('A', 5, 3); // Sets the A5 pin speed to very fast

  // set_pin_type('A', 5, 1); // sets the A5 pin type to open-drain
  // set_pin_pupdr('A', 1, 0); // sets the A1 pin to have a pull-up resistor


  for (;;) {
    atomic_set_pin_out('A', 5, read_pin(pin_A0));

    // atomic_set_pin_out('A', 5, alt_read_pin('A', 0));

    // if (counter%15 == 0) {
    //   atomic_set_pin_out('A', 5, 1);
    // } else {
    //   atomic_set_pin_out('A', 5, 0);
    // }

    counter++;
  }

}

bool read_pin(uint16_t pin) {
  // The IDR has an offset of 0x10

  char bank = (char) (pin >> 8);
  uint8_t pin_no = (uint8_t) (pin &= 0b11111111U);

  return (* (volatile uint32_t *) (0x48000000 + 0x400 * bank + 0x10) &= (0b1U << pin_no) >> pin_no);
}

void set_pin_mode(char bank, uint8_t pin, uint8_t mode) { // mode must be between 0 and 3
  // The MODER register has no offset, 0 is input, 1 is general purpose output, 2 is alternate function, 3(default) is analog

  * (volatile uint32_t *) (0x48000000 + 0x400 * (bank - 'A')) &= ~(3U << (pin * 2));
  * (volatile uint32_t *) (0x48000000 + 0x400 * (bank - 'A')) |= mode << (pin * 2);
}

void atomic_set_pin_out(char bank, uint8_t pin, bool output) {
  // The BSRR has an offset of 0x18, bits 16-31 reset, bits 0-15 set and have priority

  * (volatile uint32_t *) (0x48000000 + 0x18 + 0x400 * (bank - 'A')) &= ~(1U << pin);
  * (volatile uint32_t *) (0x48000000 + 0x18 + 0x400 * (bank - 'A')) |= output << pin;

  output = !output;
  * (volatile uint32_t *) (0x48000000 + 0x18 + 0x400 * (bank - 'A')) &= ~(1U << (pin + 16));
  * (volatile uint32_t *) (0x48000000 + 0x18 + 0x400 * (bank - 'A')) |= output << (pin + 16);
}

void set_pin_out(char bank, uint8_t pin, bool output) {
  // The ODR has an offset of 0x14

  * (volatile uint32_t *) (0x48000000 + 0x14 + 0x400 * (bank - 'A')) &= ~(1U << pin);
  * (volatile uint32_t *) (0x48000000 + 0x14 + 0x400 * (bank - 'A')) |= output << pin;
}

void set_pin_type(char bank, uint8_t pin, bool type) {
  // The OTYPER register has an offset of 0x04, and it sets push-pull(0, default) and open-drain(1)

  * (volatile uint32_t *) (0x48000000 + 0x04 + 0x400 * (bank - 'A')) &= ~(1U << pin);
  * (volatile uint32_t *) (0x48000000 + 0x04 + 0x400 * (bank - 'A')) |= type << pin;
}

void set_pin_pupdr(char bank, uint8_t pin, uint8_t mode) { // mode must be between 0 and 2
  // The PUPDR register has an offset of 0x0C, 0 is none, 1 is pull-up, 2 is pull-down, 3 is reserved

  * (volatile uint32_t *) (0x48000000 + 0x0C + 0x400 * (bank - 'A')) &= ~(3U << (pin * 2));
  * (volatile uint32_t *) (0x48000000 + 0x0C + 0x400 * (bank - 'A')) |= mode << (pin * 2);
}

void set_pin_speed(char bank, uint8_t pin, uint8_t speed) { // speed must be between 0 and 3
  // The OSPEEDR has an offset of 0x08, the speed goes low(0) to very fast(3)

  * (volatile uint32_t *) (0x48000000 + 0x08 + 0x400 * (bank - 'A')) &= ~(3U << (pin * 2));
  * (volatile uint32_t *) (0x48000000 + 0x08 + 0x400 * (bank - 'A')) |= speed << (pin * 2);
}


void clock_gpio_bank(char bank, bool enable) {
  // 0x4002 1000 + 0x4C is AHB2ENR, bits 0-8 are gpio banks

  * (volatile uint32_t *) (0x40021000 + 0x4C) &= ~(1U << (bank - 'A'));
  * (volatile uint32_t *) (0x40021000 + 0x4C) |= enable << (bank - 'A');
}
/*
void clock_i2c(uint8_t clockNum, bool enable) { // i2c4 is in APB1ENR2, and cannot be enable with this function
  // 0x4002 1000 + 0x58 is APB1ENR1, bits 21-23 are i2c 1-3
  // Because i2c is 1-3, 0 or values greater than 3 are not acceptable inputs for clockNum

  * (volatile uint32_t *) (0x40021000 + 0x58) &= ~(1U << (clockNum + 19));
  * (volatile uint32_t *) (0x40021000 + 0x58) |= enable << (clockNum + 19);
}*/


// Startup code
__attribute__((naked, noreturn)) void _reset(void) {
  // memset .bss to zero, and copy .data section to RAM region
  extern long _sbss, _ebss, _sdata, _edata, _sidata;
  for (long *src = &_sbss; src < &_ebss; src++) *src = 0;
  for (long *src = &_sdata, *dst = &_sidata; src < &_edata;) *src++ = *dst++;

  main();               // Call main()
  for (;;) (void) 0;  // Infinite loop in the case if main() returns
}

extern void _estack(void);  // Defined in link.ld

// 16 standard and 91 STM32-specific handlers
__attribute__((section(".vectors"))) void (*tab[16 + 91])(void) = {_estack, _reset};
