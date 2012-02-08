/*
 * mac7219.c
 * 
 * This code is Alpha quality. API might change any time!
 *
 * For the MAX7219, serial data at DIN, sent in 16-bit packets, is shifted 
 * into the internal 16-bit shift register with each rising edge of CLK 
 * regardless of the state of LOAD. For the MAX7221, CS must be low to clock 
 * data in or out. The data is then latched into either the digit or control 
 * registers on the rising edge of LOAD/CS. LOAD/CS must go high concurrently 
 * with or after the 16th rising clock edge, but before the next rising clock 
 * edge or data will be lost. Data at DIN is propagated through the shift 
 * register and appears at DOUT 16.5 clock cycles later. Data is clocked out 
 * on the falling edge of CLK. Data bits are labeled D0–D15 (Table 1). D8–D11 
 * contain the register address. D0–D7 contain the data, and D12–D15 are 
 * “don’t care” bits. The first received is D15, the most significant bit (MSB).
 *
 *  http://datasheets.maxim-ic.com/en/ds/MAX7219-MAX7221.pdf
 *
 * This file is part of Triple-A library:
 *   https://github.com/tuupola/triple-a
 *
 * Copyright 2012 Mika Tuupola
 *
 * Licensed under the MIT license:
 *   http://www.opensource.org/licenses/mit-license.php
 *
 */
 
#include <stdint.h>
#include <avr/io.h>

#include "shift/shift.h"

/* Registers */
#define NOOP            0x00
#define DIGIT_0         0x01
#define DIGIT_1         0x02
#define DIGIT_2         0x03
#define DIGIT_3         0x04
#define DIGIT_4         0x05
#define DIGIT_5         0x06
#define DIGIT_6         0x07
#define DIGIT_7         0x08
#define DECODE_MODE     0x09
#define INTENSITY       0x0a
#define SCAN_LIMIT      0x0b
#define SHUTDOWN        0x0c
#define DISPLAY_TEST    0x0f

#define MODE_MATRIX     0x00
#define MODE_DECODE     0xff

#define NUM_DEVICES     1

uint8_t frame_buffer[8];

void max7219_register(uint8_t register_number, uint8_t value) {
    shift_out(register_number);
    shift_out(value);
    shift_out_latch();    
}

void max7219_init(void) {
    shift_out_init(); 
    max7219_register(SCAN_LIMIT, 0x07);         /* Show all 8 digits. */
    max7219_register(DISPLAY_TEST, 0x00);       /* Disable test mode. */
    max7219_register(DECODE_MODE, MODE_MATRIX); /* Enter matrix mode. */
    max7219_clear();                            /* Clear frame buffer. */
    max7219_register(INTENSITY, 0x0f);          /* Maximum brigthness. */
    max7219_register(SHUTDOWN, 0x01);           /* Normal operation. */
}

void max7219_write(uint8_t x, uint8_t y, uint8_t value) {
    if (1 == value) {
        frame_buffer[y] |= _BV(7 - x);      /* Set bit in frame buffer. */
    } else {
        frame_buffer[y] &= ~(_BV(7 - x));   /* Unset bit in frame buffer. */
    }
    max7219_register(y+1, frame_buffer[y]); /* Sync row in chip. */
}

uint8_t max7219_read(uint8_t x, uint8_t y) {
    return (frame_buffer[y] & _BV(7 - x)) != 0 ? 1 : 0;
}

void max7219_toggle(uint8_t x, uint8_t y, uint8_t value) {
    frame_buffer[y] ^= (_BV(7 - x));
}

void max7219_clear() {
    for(uint8_t y = 0; y <= 7; y++) {
        frame_buffer[y] = 0b00000000;
        max7219_register(y+1, frame_buffer[y]);
    }
}

void max7219_sprite(uint8_t sprite[]) {
    for(uint8_t y = 0; y <= 7; y++) {
        frame_buffer[y] = sprite[y];
        max7219_register(y+1, sprite[y]);
    }
}




