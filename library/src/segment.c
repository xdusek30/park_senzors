/***********************************************************************
 * Title:    Seven-segment display library
 * Author:   Tomas Fryza, Brno University of Technology, Czechia
 * Software: avr-gcc, tested with avr-gcc 4.9.2
 * Hardware: Any AVR
 *
 * MIT License
 *
 * Copyright (c) 2019 Tomas Fryza
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **********************************************************************/

/* Includes ----------------------------------------------------------*/
#include <util/delay.h>
#include "gpio.h"
#include "segment.h"
#include <math.h>

/* Define ------------------------------------------------------------*/
/* Variables ---------------------------------------------------------*/
/* Active low digit 0 to 9 */
uint8_t segment_digit[] = {
   //DPgfedcba
    0b11000000,      // Digit 0
    0b11111001,      // Digit 1
    0b10100100,      // Digit 2
    0b10110000,      // Digit 3
    0b10011001,      // Digit 4
    0b10010010,      // Digit 5
    0b10000010,      // Digit 6
    0b11111000,      // Digit 7
    0b10000000,      // Digit 8
    0b10010000};     // Digit 9

// Active high position 0 to 3 
uint8_t segment_position[] = {
    0b00001000,   // Position 0
    0b00000100,   // Position 1
    0b00000010,   // Position 2
    0b00000001};  // Position 3
uint8_t DP = 0b01111111;
/* Functions ---------------------------------------------------------*/
void SEG_putc(uint8_t digit, uint8_t position, uint8_t dp)
{

    uint8_t i;
    /* Read values from look-up tables */
    if(dp==1){
    digit  = segment_digit[digit]&DP;
    }
    else
    {
       digit  = segment_digit[digit];
    }
    
    position = segment_position[position];
   /*To display digit put 1st byte to serial data */
    for (i = 0; i < 8; i++) {
        GPIO_write(&PORTB, SEGMENT_DATA, bit_is_set(digit, 7-i));
        SEG_toggle_clk();
    }
    /*To select one of four position put 2nd byte to serial data */
    for (i = 0; i < 8; i++) {
        GPIO_write(&PORTB, SEGMENT_DATA, bit_is_set(position, 7-i));
        SEG_toggle_clk();
    }
    
    GPIO_write(&PORTD, SEGMENT_LATCH, 1);
    _delay_us(1);
    GPIO_write(&PORTD, SEGMENT_LATCH, 0);
}
/*--------------------------------------------------------------------*/
void SEG_toggle_clk(void)
{
    _delay_us(1);
    GPIO_write(&PORTD, SEGMENT_CLK, 1);
    _delay_us(1);
    GPIO_write(&PORTD, SEGMENT_CLK, 0);

}
/*--------------------------------------------------------------------*/
void four_dig_print(double digits){

    uint16_t digit;
    double dec;
    double fractional = modf(digits, &dec);
    fractional = fractional/0.1;  
    // float t = 0.5;
    digit = (int)dec;
    uint16_t numb1= digit/100;
    if(numb1!=0)
    SEG_putc(numb1, 3, 0);
    uint16_t numb2= (digit%100)/10;
    if(numb2!=0)
    SEG_putc(numb2, 2, 0);
    uint16_t numb3= (digit%10);
    SEG_putc(numb3, 1, 1);
    uint8_t numb4= (digit%10);
    SEG_putc(fractional, 0, 0);
}