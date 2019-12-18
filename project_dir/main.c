#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "gpio.h"
#include "uart.h"
#include "timer.h"
#include "segment.h"
#include <util/delay.h>

#define S_velosity 340 / 10
#define TRIGER_PIN PB1
#define TRIGER 9600

float ECHO_time = 0;
float distance = 0;
uint16_t distance_int = 0;
uint16_t distance_decimal = 0;
uint16_t ticks_rise = 0;
uint16_t ticks_fall = 0;
uint8_t count = 0;
char string[5];
volatile uint16_t i=0;

void BZR(uint16_t dist);


typedef enum {
  UP,
  DOWN
}
state;

state current_state = DOWN;
state next_state = UP;

//-----------------Timers configuration with functional registers set up-------------------------//
int main() {

  GPIO_config_output(&DDRD, SEGMENT_CLK);				//Configure clock pin of 7-seg display;
  GPIO_config_output(&DDRD, SEGMENT_LATCH);				//Configure latch pulse pin of 7-seg display;
 
  DDRB |= _BV(SEGMENT_DATA);						//Configure data pin of 7-seg display;
  PORTB &= ~_BV(SEGMENT_DATA);


  GPIO_write(&PORTD, SEGMENT_CLK, 0);
  GPIO_write(&PORTD, SEGMENT_LATCH, 0);  
 
  TIM_config_prescaler(TIM0, TIM_PRESC_1); 				//Configure Timer_0 prescaler;
  TIM_config_interrupt(TIM0, TIM_OVERFLOW_ENABLE); 			//Enable interrupt on overflov;

  TIM_config_prescaler(TIM2, TIM_PRESC_8);				//Configure Timer_2 for buzer functionality;
  TIM_config_interrupt(TIM2, TIM_OVERFLOW_ENABLE);  

  TIM_config_prescaler(TIM1, TIM_PRESC_256); 				//Configure Timer_1 prescaler;
  TIMSK1 |= _BV(ICIE1); 						//Input Capture Interrupt Enable;
  TCCR1B |= _BV(ICES1) | _BV(ICNC1); 					//Set up Input Capture Edge Select and Enable Input Capture Noise Canceler;

  DDRB |= _BV(PB1);							//Confugure triger pin;
  PORTB &= ~_BV(PB1);

  DDRD |= _BV(PD3);
  PORTD &= ~_BV(PD3);


  sei();
  
//-----------------Infinitive For cycle to translate 16-bit data to float varible of distance----------//
  for (;;) {

    if (current_state == DOWN) {
      cli();
      if (ticks_fall > ticks_rise) {
        ECHO_time = (float)(ticks_fall - ticks_rise) / (float)(F_CPU / 256) * 1000000;
      } 
	  else
        ECHO_time = (float)((65535 - ticks_rise) + ticks_fall) / (float)(F_CPU / 256) * 1000000;

      distance = ((float) S_velosity * (float) ECHO_time)/(float) 2000;
      distance_int = (uint16_t) distance;
      distance_decimal = (uint16_t)((((float) distance - (float) distance_int) * 1000)) / 100;
      
	  
      sprintf(string, "%u.%u", distance_int, distance_decimal);
	  sei();
    }

    four_dig_print(distance);						// Print distance on 7-segment;
  }

  return 0;
}



ISR(TIMER1_CAPT_vect) { 						//on interrupt we get data, when pin changes its state;

  if (next_state == UP) {
    ticks_rise = ICR1L; 						//colect low 8bit of timer;
    ticks_rise |= (ICR1H << 8);						//colect high 8bit of timer;
    TCCR1B &= ~_BV(ICES1);						//Reverse edge detector;
    current_state = UP;							//Change state status;
    next_state = DOWN;

  } else {
    ticks_fall = ICR1L;
    ticks_fall |= (ICR1H << 8);
    TCCR1B |= _BV(ICES1); 						//Reverse edge detector;

    current_state = DOWN;						//Change state status;
    next_state = UP;
  }
}

ISR(TIMER0_OVF_vect) {  						// Timer configured to send pulse once in 700 overflovs; 

  cli();

  count++;
  if (count == 1)
    GPIO_write(&PORTB, TRIGER_PIN, 1);
  if (count == 2)
    GPIO_write(&PORTB, TRIGER_PIN, 0);
  if (count == 700)
    count = 0;

  sei();
}
ISR(TIMER2_OVF_vect)   							//Timer oferflov interrupt to run buzer function;
{
  BZR(distance_int);

}



void BZR(uint16_t dist)  						//Buzer function to express distance
{
uint16_t delay=0;							//Function configured for higher tone signalization on closer distance and higher delay pause on longer distance; 
uint16_t  freq=0;
  
 if (dist<=400 && dist>200) { freq=20; delay=6000; }
else if (dist<=200 && dist>70) { freq=19; delay=5650; }
else if (dist<=70 && dist>25) { freq=16; delay=4600; }
else if (dist<=25 && dist>18) { freq=15; delay=4250; }
else if (dist<=18 && dist>14) { freq=14; delay=3900; }
else if (dist<=14 && dist>11) { freq=13; delay=3550; }
else if (dist<=11 && dist>10) { freq=12; delay=3200; }
else if (dist<=10 && dist>9) { freq=11; delay=2850; }
else if (dist<=9 && dist>8) { freq=10; delay=2500; }
else if (dist<=8 && dist>7) { freq=9; delay=2150; }
else if (dist<=7 && dist>6) { freq=8; delay=1800; }
else if (dist<=6 && dist>5) { freq=7; delay=1450; }
else if (dist<=5 && dist>4) { freq=6; delay=1100; }
else if (dist<=4 && dist>3) { freq=5; delay=750; }
else if (dist<=3 && dist>2) { freq=4; delay=400; }
else { freq=3; delay=0; }

  i++;
  if (i%freq==0 || (i-1)%freq==0  && i<900){
  
   GPIO_write(&PORTD, PD3, 1);
   GPIO_write(&PORTD, PD3, 0);
  
   if (i+1%freq==0 )
   GPIO_write(&PORTD, PD3, 0);   
   else 
   GPIO_write(&PORTD, PD3, 1);   

   
  }
else if(i==1000+delay)
i=0;
}
