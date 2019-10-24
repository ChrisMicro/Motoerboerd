/*************************************************************************
 
 Solar Bird (Attiny85) 
 by Christoph Haberer, christoph@roboterclub-freiburg.de (main software)
    Urban Bieri, urbanbieri@gmx.ch (main software)
    Volker Böhm, vboehm@gmx.ch (sounddesign)
    Michael Egger, me@anyma.ch (make file)
    Atelier Hauert Reichmuth, info@hauert-reichmuth.ch (modifications 18.3.2013)
 
/*************************************************************************
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
/*************************************************************************
 
 Hardware
 
 prozessor:	ATtiny85
 clock:		128Khz internal oscillator
 
 avr-gcc compiler optimization setting: Os
 
 PIN1	RESET			connect 10K pullup
 PIN2	PORTB3/ADC3		
 PIN3	PORTB4/ADC2		led kathode ( light sensor )
 PIN4	GND
 
 PIN5	PORTB0/OC10		piezo speaker		
 PIN6	PORTB1			
 PIN7	PORTB2/ADC1		digital output
 PIN8	VCC
 
 *************************************************************************/
#include <avr/io.h>

#define F_CPU 128000UL

#define PIEZOSPEAKER (1<<PINB0)			// pin definition for pieoz speaker
#define PIEZOSPEAKER_NEGATIV (1<<PINB1)
#define OUTPIN  (1<<PB2)				// test pin out for checking led sensor with an oscilloscope
#define SENSPIN (1<<PB4)				// led charge/discharge for light sensor

#define SPEAKEROFF  TCCR0A=(0x02)
#define SPEAKERON TCCR0A=(1<<COM0A0) | (1<<COM0B0) |0x02

#define FREQ(F)  F_CPU/2/(F)-1 // frequency to counter calculation macro


// variables for random generator
uint16_t	r;


void init_timer()
{
	//TCCR0A=(1<<COM0A0) | 0x02; //CTC mode and toogle OC0A port on compare match
	TCCR0A=(1<<COM0A0) | (1<<COM0B0) |0x02; //CTC mode and toogle OC0A,OC0B port on compare match
	
	TCCR0B=(1<<CS00); 		// no prescaling,
	//OCR0A=255; 				// in CTC Mode the counter counts up to OCR0A
	//OCR0B=255; 				// in CTC Mode the counter counts up to OCR0A
	TCCR0B|=(1<<FOC0B); 	// force output compare on OC0B ( invert ouput signal )
	// CTC-Modefrequency calculation: f=IO_CLC/(2*N*(1+OCR0A)) N:Prescaler
	// N=1,IO_CLK=128kHz: OCR0A=128000UL/(2*f)-1
	// ==> OCR0A=F_CPU/2/f-1
	
	// initialize ports
	DDRB=OUTPIN|PIEZOSPEAKER|PIEZOSPEAKER_NEGATIV; // Pins as output
}


/****************************************************************************
 
 void delay_ms(uint16_t duration_ms)
 
 delay in ms
 
 This delay woks for the compiler option Os
 and avr-gcc 4.3.0
 
 ****************************************************************************/

void delay_ms(uint16_t duration_ms)
{
	duration_ms *= 14;
	while(duration_ms--)
	 {
		asm volatile(		
					 "nop	\n\t"
					 "nop	\n\t"
					 );
	 }
}


uint8_t rand() {
	r = (r*69069+1);
	return r>>8;
}


/****************************************************************************
 
 uint16_t getLightValue()
 
 return: intensity of darkness ( 0..0xFFFF )
 
 much light: return value = small
 low light: return value = big
 
 
 This routine uses a LED as light sensor. The LED is connected between 
 GND and one microcontroller pin. The measuring principle used in this 
 function works as follows:
 
 1. charge the LED capacity by connecting the internal pull up to VCC
 2. wait a little bit until the capacity is loaded
 3. switch the pin to high impedance
 4. wait until the surrounding light discharges the LED capacity
 
 This routine returns the number of ticks til the LED capacity
 is discharged.
 Therefore: the more light the faster it discharges ==> the number of 
 ticks returned by this function is small.
 
 if you want to know more about the principle refer to:
 www.mikrocontroller.net/articles/Lichtsensor_/_Helligkeitssensor
 
 
 Here are some experimental measurement result with different LED types
 
 voltage supply 5V
 
 20W Halogen lamp, LED ultra bright red
 =======================================================
 20W Halogen lamp: 170uS
 room light: 20ms 
 
 LED ultra bright blue, charging time with pull up: 10us
 =======================================================
 full sun lignt: 140us
 shadow: 7ms
 
 LED old 20mA red , charging time with pull up: 10us
 =======================================================
 full sun light: 14ms
 shadow: 250ms
 
 LED ultra bright green , charging time with pull up: 10us
 ==========================================================
 full sun light: 40us
 shadow: 14ms
 
 with 2.6V power supply:
 full sun light: 20us
 shadow: 9ms
 
 conclusion:
 - the ultra bright LEDs are much more light sensitive than the old 20mA ones
 - 5V power supply works imilar to 2.6V power supply
 
 range for the ultra bright LED goes from 20us to sevel 100ms in darkness
 
 ****************************************************************************/
/*
 uint16_t getLightValue()
 {
 uint16_t n;
 
 PORTB|=SENSPIN; // turn on pull up and load led capacity
 for(n=0;n<10;n++); // delay
 PORTB|=OUTPIN; // toggle out pin
 
 PORTB&=~SENSPIN; // switch to high impedance
 n=0;
 while((n<0xFFF0)&&(PINB&SENSPIN))n++; // prevent overflow, wait for capacity discharge
 
 PORTB&=~OUTPIN; // toggle out pin
 return n;
 }
 */
/****************************************************************************
 
 void chirp(uint8_t start, uint8_t stop,uint16_t step)
 
 linear chirp
 
 start: start counter value
 stop: stop counter value
 step: how fast the chirp is
 
 both chirps are possible:
 1. stop>start ==> frequency goes from high to low
 2. start>stop ==> frequency goes from low to high
 
 ****************************************************************************/

void chirp(uint8_t start, uint8_t stop, uint8_t step)
{
	uint16_t count;
	uint8_t diff;
	
	if(stop>start) diff = stop-start;
	else diff = start-stop;
	
	step = diff / step;
	if(step<1) step=1;
	
	count=start<<8;			
	SPEAKERON;
	if(stop>start) {
		while(count<(stop<<8)) {
			count+=step;			// timer up, frequency goes down
			OCR0A=count>>8;
		}
	}
	else {
		while(count>(stop<<8)) {
			count-=step;			// timer sown, frequency goes up
			OCR0A=count>>8;
		}
	}
	SPEAKEROFF;
}




/********* Alpensegler *********/

void alpensegler() 
{	
	uint8_t	offset = (rand()>>5)+5;
	uint8_t	start = (rand()>>4);
	uint8_t	end = (rand()>>4)+1;
	uint8_t	times = (rand()>>5)+10;
	uint8_t	dur = (rand()>>6)+1;
	uint8_t	i = times;
	
	while(i--) {
		chirp(start+offset, end+offset, dur); 
		if(i < (times>>1)) {
			offset++;
			dur++;
		}
	}
}



/********* Blaumeise *********/

void blaumeise() 
{	
	uint8_t	start = (rand()>>6)+15;
	uint8_t	end = start-(rand()>>5)-5;
	uint8_t	times = (rand()>>5)+5;
	uint8_t	dur = (rand()>>6);
	
	chirp((rand()>>5)+2, 13, 15);
	chirp(22, 17, 25);
	delay_ms(50);
	while(times--) {
		dur++;
		chirp(start, end, dur); 
	}
	delay_ms(rand());
}


/********* Piepmatz *********/

void piepmatz() {
	uint8_t i = (rand()>>6)+2;
	//uint8_t dur = (rand()>>5) + 1;
	uint8_t a, b;
	a = (rand()>>4)+11;
	b = a+(rand()>>4)+5;
	
	 while(i--) {
		 
		 if(rand()>>7) 
			 chirp(a, b, 5); 
		 else 
			 chirp(b, a, 6);
		 /*
		 if(rand()>>7) 
			 chirp(a, b, dur); 
		 else 
			 chirp(b, a, dur);
		 */
		 //delay_ms((rand()<<2)+200);
		 delay_ms(rand()<<2);
	 }
}



void zwitscher() {
	uint8_t		offset = (rand()>>5)+3;
	uint8_t		start, end;
	uint8_t		d, i;
	
	start = (rand()>>4);
	end = (rand()>>4)+1;
	i = (rand()>>5)+1;
	d = rand()>>4;
	while(i--) {
		chirp(start+offset, end, (rand()>>5)+2); 
		delay_ms(d);
		offset++;
	}
}



void preller() {
	uint8_t		offset = (rand()>>3)+10;
	uint8_t		i = (rand()>>4)+8;
	
	chirp(30, offset, 12);
	while(i--) {
		chirp((rand()>>4)+offset, (rand()>>4)+offset, 1);
		offset--;
		if(offset<2) offset=20;
	}
	chirp(offset, 30, 15);
	//delay_ms(rand()>>10);
	i = (rand()>>6)+2;
	while(i--) {
		chirp(30, 3, 3);
		delay_ms(rand()>>2);
	}
	delay_ms(rand());
	i = (rand()>>6)+2;
	while(i--) {
		chirp(30, 3, 3);
		delay_ms(rand());
	}
	
}


/****************************************************************************
 
 main program
 
 ****************************************************************************/
int main()
{
	
	uint8_t test;
	uint8_t pause = 2000; //definiert die Pausen zwischen den Sequenzen
	
	init_timer();
	
	
	SPEAKEROFF;		// start with silence!
	delay_ms(pause);
	
	
	while(1) {
		test = rand()>>4;		// 16 moeglichkeiten
		
		if(test<2) {
			alpensegler();
		}
		
		else if(test==2) {
			piepmatz();
		}
		
		else if(test==4)
			blaumeise();
		
		else if(test==5 || test==6) {
			zwitscher();
			preller();
		}
		
		else if(test==15 || test==14) {		// pause lang
			//SPEAKEROFF;
			pause++;
			delay_ms(rand()<<4);
			delay_ms(pause<<4);
		} 
		else {
			//SPEAKEROFF;
			delay_ms(rand());
		}
	}
	
	
	return 0; 
}
