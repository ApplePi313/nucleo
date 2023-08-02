#include <inttypes.h>
#include <stdbool.h>

#define PIN(bank, pin) ((uint16_t) ((bank - 'A') << 8) | ((uint16_t) pin))

#define GPIO(bank) = ((0x48000000) + (bank - 'A') * 0x400)

struct gpio {
  volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFRL, AFRH, BRR;
};

struct basicTimer { // for TIM 6 and 7
  volatile uint32_t CR1, CR2, RES1, DIER, SR, EGR, RES2, RES3, RES4, CNT, PSC, ARR;
};

// GPIO functions
void gpio_set_pin_type(uint16_t, bool);
void gpio_set_pin_pupdr(uint16_t, uint8_t);
void gpio_set_pin_speed(uint16_t, uint8_t);
void gpio_set_pin_mode(uint16_t, uint8_t);
void gpio_atomic_set_pin_out(uint16_t, bool);
void gpio_set_pin_out(uint16_t, bool);
bool gpio_read_pin(uint16_t);


// Timer functions
void basic_timer_enable_counter(struct basicTimer*, bool);
void basic_timer_generate_event(struct basicTimer*);
void basic_timer_set_prescaler(struct basicTimer*, uint16_t);
void basic_timer_set_auto_reload(struct basicTimer*, uint16_t);
uint16_t basic_timer_get_counter(struct basicTimer*);

// Register clocking functions
void clock_timer(uint8_t, bool);
void clock_gpio_bank(char, bool);
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

  struct basicTimer* tim7 = (struct basicTimer*) (0x40000000 + 0x400 * (7 - 2));

  clock_gpio_bank('A', 1);
  clock_timer(6, 1);
  clock_timer(7, 1);

  gpio_set_pin_mode(pin_A5, 1);
  gpio_set_pin_mode(pin_A0, 0);

  gpio_set_pin_pupdr(pin_A0, 2); // set pin A0 to have a pull-down resistor

  (tim7 -> CR1) |= 0x1; // enable counter
  (tim7 -> PSC) = 128; // set prescaler
  (tim7 -> ARR) = 4096; // set auto reload
  (tim7 -> EGR) |= 1;

  // timer_enable_counter(6, 1);(tim7 -> PSC) = 256; // set prescaler
  // timer_set_prescaler(6, 256);
  // timer_set_auto_reload(6, 4096);
  // timer_generate_event(6);


  for (;;) {
    if ((tim7 -> CNT) == 0) {
        gpio_atomic_set_pin_out(pin_A5, gpio_read_pin(pin_A0));
    } else {
        gpio_atomic_set_pin_out(pin_A5, 0);
    }
  }

}

/*
 * GPIO functions
 */
void gpio_set_pin_type(uint16_t pin, bool type) {
  // The OTYPER register has an offset of 0x04, and it sets push-pull(0, default) and open-drain(1)

  uint8_t bank = (uint8_t) (pin >> 8);
  uint8_t pin_no = (uint8_t) pin;

  *(volatile uint32_t*) (0x48000000 + 0x04 + 0x400 * bank) &= ~(1U << pin_no);
  *(volatile uint32_t*) (0x48000000 + 0x04 + 0x400 * bank) |= type << pin_no;
}

void gpio_set_pin_pupdr(uint16_t pin, uint8_t mode) { // mode must be between 0 and 2
  // The PUPDR register has an offset of 0x0C, 0 is none, 1 is pull-up, 2 is pull-down, 3 is reserved

  uint8_t bank = (uint8_t) (pin >> 8);
  uint8_t pin_no = (uint8_t) pin;

  *(volatile uint32_t*) (0x48000000 + 0x0C + 0x400 * bank) &= ~(3U << (pin_no * 2));
  *(volatile uint32_t*) (0x48000000 + 0x0C + 0x400 * bank) |= mode << (pin_no * 2);
}

void gpio_set_pin_speed(uint16_t pin, uint8_t speed) { // speed must be between 0 and 3
  // The OSPEEDR has an offset of 0x08, the speed goes low(0) to very fast(3)

  uint8_t bank = (uint8_t) (pin >> 8);
  uint8_t pin_no = (uint8_t) pin;

  *(volatile uint32_t*) (0x48000000 + 0x08 + 0x400 * bank) &= ~(3U << (pin_no * 2));
  *(volatile uint32_t*) (0x48000000 + 0x08 + 0x400 * bank) |= speed << (pin_no * 2);
}

void gpio_set_pin_mode(uint16_t pin, uint8_t mode) { // mode must be between 0 and 3
  // The MODER register has no offset, 0 is input, 1 is general purpose output, 2 is alternate function, 3(default) is analog

  uint8_t bank = (uint8_t) (pin >> 8);
  uint8_t pin_no = (uint8_t) (pin & 0xffff);

  *(volatile uint32_t*) (0x48000000 + 0x400 * bank) &= ~(3U << (pin_no * 2));
  *(volatile uint32_t*) (0x48000000 + 0x400 * bank) |= (mode << (pin_no * 2));
}

void gpio_atomic_set_pin_out(uint16_t pin, bool output) {
  // The BSRR has an offset of 0x18, bits 16-31 reset, bits 0-15 set and have priority

  uint8_t bank = (uint8_t) (pin >> 8);
  uint8_t pin_no = (uint8_t) pin;

  *(volatile uint32_t*) (0x48000000 + 0x18 + 0x400 * bank) &= ~(1U << pin_no);
  *(volatile uint32_t*) (0x48000000 + 0x18 + 0x400 * bank) |= output << pin_no;

  output = !output;
  *(volatile uint32_t*) (0x48000000 + 0x18 + 0x400 * bank) &= ~(1U << (pin_no + 16));
  *(volatile uint32_t*) (0x48000000 + 0x18 + 0x400 * bank) |= output << (pin_no + 16);
}

void gpio_set_pin_out(uint16_t pin, bool output) {
  // The ODR has an offset of 0x14

  uint8_t bank = (uint8_t) (pin >> 8);
  uint8_t pin_no = (uint8_t) pin;

  *(volatile uint32_t*) (0x48000000 + 0x14 + 0x400 * bank) &= ~(1U << pin_no);
  *(volatile uint32_t*) (0x48000000 + 0x14 + 0x400 * bank) |= output << pin_no;
}

bool gpio_read_pin(uint16_t pin) {
  // The IDR has an offset of 0x10

  uint8_t bank = (uint8_t) (pin >> 8);
  uint8_t pin_no = (uint8_t) pin;

  return (*(volatile uint32_t*) (0x48000000 + 0x400 * bank + 0x10) &= (0b1U << pin_no) >> pin_no);
}



/*
 *
 * Timer functions:
 *
 */
void basic_timer_enable_counter(struct basicTimer* tim, bool enable) {
  tim -> CR1 |= enable;
}

void basic_timer_set_prescaler(struct basicTimer* tim, uint16_t prescaler) {
  tim -> PSC = prescaler;
}

void basic_timer_set_auto_reload(struct basicTimer* tim, uint16_t auto_reload) {
  tim -> ARR = auto_reload;
}

void basic_timer_generate_event(struct basicTimer* tim) {
  tim -> EGR |= 1U;
}

uint16_t basic_timer_get_counter(struct basicTimer* tim) {
  return (uint16_t) tim -> CNT;
}


/*
 * Register Clocking Functions:
 */
void clock_timer(uint8_t number, bool enable) {
  // 0x4002 1000 + 0x58 is APB1ENR, bits 0-5 are timers 2 - 7

  *(volatile uint32_t*) (0x40021000 + 0x58) &= ~(1U << (number - 2));
  *(volatile uint32_t*) (0x40021000 + 0x58) |= enable << (number - 2);
}

void clock_gpio_bank(char bank, bool enable) {
  // 0x4002 1000 + 0x4C is AHB2ENR, bits 0-8 are gpio banks

  *(volatile uint32_t*) (0x40021000 + 0x4C) &= ~(1U << (bank - 'A'));
  *(volatile uint32_t*) (0x40021000 + 0x4C) |= enable << (bank - 'A');
}

void clock_i2c(uint8_t clockNum, bool enable) { // i2c4 is in APB1ENR2, and cannot be enable with this function
  // 0x4002 1000 + 0x58 is APB1ENR1, bits 21-23 are i2c 1-3
  // Because i2c is 1-3, 0 or values greater than 3 are not acceptable inputs for clockNum

  *(volatile uint32_t*) (0x40021000 + 0x58) &= ~(1U << (clockNum + 19));
  *(volatile uint32_t*) (0x40021000 + 0x58) |= enable << (clockNum + 19);
}




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
