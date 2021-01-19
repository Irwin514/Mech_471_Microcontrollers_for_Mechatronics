
// ADC example #1

#include <Arduino.h>
#include <math.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#define BIT(a) (1 << (a))

void setup()
{
	Serial.begin(115200);
 
 	cli();  // disable interrupts
 
	// select ADC channel #5
	ADMUX = 0;
	ADMUX |= BIT(MUX0) | BIT(MUX2);
	
  	// be careful about changing the channel when a 
	// conversion is in process.
 
	// set ADC reference (max input voltage) to 5V (Vcc) from micrcontroller
	// note: default (0) takes the reference from the Aref pin
	ADMUX |= BIT(REFS0);
 
	// set ADC control and status register A
	ADCSRA = 0;
	
	ADCSRA |= BIT(ADEN); // ADC enable
		
	ADCSRA |= BIT(ADIE); // ADC interrupt enable
  // note: uncomment out this line if you want the ADC complete
  // interrupt to function	
  // note: you don't need to use ADC interrupts 
	// you can just set BIT(ADSC) and wait for it to return to 0
  // as is done in the loop() function
	
	// ADPS0, ADPS1, ADPS2 - system clock to ADC input clock prescaler bits
	// smaller prescaler results in faster conversions, but too fast
	// reduces ADC resolution and accuracy
	ADCSRA |= BIT(ADPS0) | BIT(ADPS1) | BIT(ADPS2); // 128 prescaler (slowest)
  // this gives a conversion theoretical time of 116 microseconds for one channel
  
//ADCSRA |= BIT(ADPS1) | BIT(ADPS2); // 64 prescaler
  // this gives a conversion time of 60 microseconds for one channel

  // note: smaller prescaler values will result in faster conversion times.
  // however, at some point accuracy will be lost if not enough time is allowed
  // for the conversion to complete to the required number of bits (12).
  // if less than 12 bits (eg 8 bits) is needed the prescaler can be reduced further.
  // testing the conversion result for known voltages can be used to determine
  // if the prescaler value is too small.  the result will become inaccurate for
  // values that are too small.

  // note: it will take at least N times the times above to sample N channels
	
	// note: the default is single conversion mode
	// ie conversions are triggered manually using the ADSC bit as follows
	ADCSRA |= BIT(ADSC); // ADC start conversion
	// Note: this is needed for the interrupts
	
	// BIT(ADSC) will read as 1 when a conversion is in process
	// and turn to 0 when complete	
		
	// free running mode can be enabled by setting ADATE 
	// (auto trigger enable) and ADTS (auto trigger source) 
//	ADCSRA |= BIT(ADATE);
//	ADCSRB = 0; // free running mode

	// note you can use the 2-byte ADC register to read the ADC
	// after the conversion is complete
	// note only the first 10 bits are valid (bits 0 to 9)
	// note the access should be atomic to avoid splitting data	
	// note ADC = Vin*1024/Vref
		
	// initialize timer0 (ie micros())
	TCNT0 = 0;
  
	sei(); // enable interrupts
}


void loop()
{
	// make i volatile to avoid i++ being optimized from the while loop
	volatile unsigned int i=0;
	static float dt=0.0,t1,t2,Vin,Vref;
 	volatile int adc1;
	
/*	
	t1 = micros();
		
	ADCSRA |= BIT(ADSC); // ADC start conversion
	// BIT(ADSC) will read as 1 when a conversion is in process
	// and turn to 0 when complete	
 
	// wait for conversion to complete
	// wait until BIT(ADSC) of ADCSRA is off
  // note: if you want to see how fast the ADC conversion is you 
  // should not enable conversion complete interrupts in setup()
  // since the interrupt takes time
	while( ADCSRA & BIT(ADSC) ) i++; // blocking / waiting / polling
	
	// read the ADC (10-bits) // 0 - 1023
	adc1 = ADC;
  
 	t2 = micros();
	
	dt = t2 - t1;
	
 	// note ADC = Vin*1023/Vref
	Vref = 5.0;
	Vin  = adc1/1023.0*Vref;
	
	Serial.print("\nt(s) = ");
	Serial.print(t1*1.0e-6);
	Serial.print("\tdt(us) = ");
	Serial.print(dt);
	Serial.print("\tadc = ");
	Serial.print(adc1);
	Serial.print("\tVin = ");
	Serial.print(Vin);
*/	
	delay(500);
	
}


ISR(ADC_vect)
// ADC conversion complete interrupt
// note:
// arduino time functions such as micros() and serial 
// communication can work in this interrupt function
{
	static int i = 0;
	int adc1;
  	
	ADCSRA |= BIT(ADSC); // start new ADC conversion
	// better to put at beginning as long as we can complete
	// the rest of the function before it completes
	// that way we are "multi-tasking"
	// you can do things while you're waiting
	//
	// the other advantage is you get faster conversion
	// rate because you don't wait to finish the function to complete
	// -> convert as fast as possible -- immediately start a new 
	// conversion when the current one is finished
	i++;
	
	Serial.print("\nADC complete interrupt, i = ");
	Serial.println(i);
	
	// read the ADC (10-bits) // 0 - 1023
	adc1 = ADC;	
	
	Serial.print("adc = ");
	Serial.print(adc1);
}
