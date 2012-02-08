/*
 * mac7219.h
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
 
void max7219_register(uint8_t register_number, uint8_t value);
void max7219_init(void);
void max7219_write(uint8_t x, uint8_t y, uint8_t value);
uint8_t max7219_read(uint8_t x, uint8_t y);
void max7219_toggle(uint8_t x, uint8_t y, uint8_t value);
void max7219_clear();
void max7219_sprite(uint8_t sprite[]);

