
#include <Arduino.h>

// note io.h defines the physical memory location of the ports
// and other registers on the microcontroller

// note: with microcontrollers we typically deal with physical memory
// locations (i.e. pointers with specific integer values)
// eg memory location 1000 could be porta
// this is different than a normal pointer for a variable which
// is a virtual memory address -- not a physical address
#include <avr/io.h> 

// bit a
#define BIT(a) (1 << (a))

// the setup function runs once when you press reset or power the board
void setup() {
  
	// be careful of interfering with registers needed 
	// for used Arduino library functions
	Serial.begin(9600);
	
	// initialize digital pin 13 as an output.
	// note DDRx can be used instead
	//pinMode(13, OUTPUT);
  
	// note the ports are like global variables with all their associated
	// pitfalls -- beware !
  
	// configure pin 13 as output using the DDRx register
	// note: bit 5 on portb is pin 13 on the arduino
	// note: bit = 1 is output, bit = 0 is input (opposite of PIC chip)
	DDRB = DDRB | BIT(5); // set pin 13 to output
  
	// DDRB = *(mem_address_or_pointer_for_DDRB)
  
	// note: portd is pins 0-7 on arduino
	// set pins 0-7 to input
	DDRD = 0;
	
	// note: pin 7 should be connected to pin 13 for the example to work
  
}


// the loop function runs over and over again forever
void loop() {

	int pin7;
/*	
	// note: pointers can also be used to access the ports as follows
	// this example toggles pin 13 on and off
	volatile unsigned char *p;
    p = &PORTB;
	*p = *p | BIT(5);
	delay(500);
	*p = *p & ~BIT(5);
	delay(500);
*/

// useful delay function for very small delays
// delayMicroseconds(us)
// us: the number of microseconds to pause (unsigned int)

	// note: bit 5 on portb is pin 13 on the arduino
	
	PORTB |= BIT(5); // *** turn pin 13 on

	delay(3000);
	
	// just turn pin 13 off
	//
	// 11011111 - donut / mask = ~(BIT(5))
	// 10101010 - PORTB &
	// 10001010
//	PORTB &= ~(BIT(5)); // *** turn bit 5 off
	
//	delay(1); // wait for pin to go high (normally this would happen in 5us)
	// and maybe the input to go high

	// note: portd is pins 0-7 on arduino
	// note: pin 7 should be connected to pin 13 for the example to work
	
	pin7 = PIND & BIT(7); // read pin7, note: use PIND to read not PORTD	
	Serial.print("\npin17 = ");
	Serial.print(pin7);
	
	delay(1000);

	PORTB &= ~BIT(5); // turn pin 13 off	

//	delay(1);

  // set pin 7 of port D to input pullup -- assuming
  // the pin has been set to input with the DDRD register
	PORTD = PORTD | BIT(7);
  
	pin7 = PIND & BIT(7); // read pin7	
	Serial.print("\npin27 = ");
	Serial.print(pin7);	
	
	delay(1000);
	
}
