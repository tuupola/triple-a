/*
 * max7219.c
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

#include "max7219/max7219.h"
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

#define NUM_DEVICES     2
#define MATRIX_WIDTH    16
#define MATRIX_HEIGHT   8

uint8_t frame_buffer[8 * NUM_DEVICES];

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

void max7219_put_pixel(uint8_t x, uint8_t y, uint8_t value) {
    uint8_t chip = x >> 3;   /* Divide by 8 to find chip. */
    uint8_t bit = 7 - x % 8; /* 0 is left most pixel in matrix. */
    
    #ifdef DEBUG
    printf("maxx7219_put_pixel(%d, %d, %d) -> ", x, y, value);
    printf("chip %d, bit %d \n", chip, bit);
    #endif

    /* Check for boundaries. */
    if ((x > MATRIX_WIDTH - 1) || (y > MATRIX_HEIGHT - 1)) {
        return;
    }

    if (1 == value) {
        /* Set bit in frame buffer. */
        frame_buffer[y * NUM_DEVICES + chip] |= _BV(bit);
    } else {
        /* Unset bit in frame buffer. */
        frame_buffer[y * NUM_DEVICES + chip] &= ~(_BV(bit));
    }

    /* Sync current row. */
    max7219_sync_row(y);

}

uint8_t max7219_get_pixel(uint8_t x, uint8_t y) {
    uint8_t chip = x >> 3;
    uint8_t bit = 7 - x % 8;
    
    return (frame_buffer[y * NUM_DEVICES + chip] & _BV(bit)) != 0 ? 1 : 0;
}

void max7219_toggle(uint8_t x, uint8_t y, uint8_t value) {
    uint8_t chip = x >> 3;
    uint8_t bit = 7 - x % 8;
    
    frame_buffer[y * NUM_DEVICES + chip] ^= (_BV(bit));
    max7219_sync_row(y);
}

void max7219_clear(void) {
    uint8_t *pointer;

    pointer = &frame_buffer[0];
    for (uint8_t index = 0; index < 8 * NUM_DEVICES; ++index) {
        *pointer = 0b00000000;
        ++pointer;
    }
    max7219_sync_frame_buffer();
}

void max7219_sprite(int8_t offset_x, int8_t offset_y, uint8_t sprite[]) {
    for(uint8_t y = 0; y <= 7; y++) {
        for(uint8_t x = 0; x <= 7; x++) {
            max7219_put_pixel(x + offset_x, y + offset_y, (sprite[y] & _BV(x)) != 0);
        }
    }
}

/* Syncs one row from frame buffer to matrix. */
void max7219_sync_row(uint8_t y) {
    /* Sync current row. */
    for (uint8_t chip = 0; chip < NUM_DEVICES; chip++) {
        shift_out(y + 1);
        shift_out(frame_buffer[y * NUM_DEVICES + chip]);
    }
    /* Latch whole row at the time. */
    shift_out_latch();
}

/* Syncs contents of frame buffer to matrix. */
void max7219_sync_frame_buffer(void) {
    for (uint8_t y = 0; y < 8;  y++) {
        max7219_sync_row(y);
    }
}

/* ASCII dump framebuffer to stdout. You probably want to */
/* redirect stdout to uart.                               */
void max7219_dump_frame_buffer(void) {    
    for(uint8_t y = 0; y < MATRIX_HEIGHT; y++) {
        printf("%d", y);

        for(uint8_t x = 0; x < MATRIX_WIDTH; x++) {

            uint8_t chip = x >> 3;
            uint8_t bit = x % 8;
            
            if (0 == bit) {
                printf(" (%d) ", chip);
            }

            printf("%d", max7219_get_pixel(x, y));

        }
        printf(" %d\n", y);
    }
}
