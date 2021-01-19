   
#include <Arduino.h>
#include <Servo.h>
#include <avr/wdt.h> // for sw reset function

#include <SPI.h>
#include "RF24.h"
#include "printf.h" // for print details function

// set up nRF24L01 radio on SPI bus plus pins 7 and 8
RF24 radio(7,8);

byte addresses[][6] = {"1Node","2Node"};

// hall encoder pins
const int ENC_A = 2;
const int ENC_B = 3;

// actuator servos pins
const int SER_1 = 5; // servo1 pin
const int SER_2 = 6; // servo2 pin

// servo library objects
Servo Servo1;
Servo Servo2;

void write_actuators(unsigned char pw1, unsigned char pw2);

// turn off robot and stop the program
void stop_robot();

// software reset function
void sw_reset();

void setup_RF24_wireless();

void setup()
{
	int i;
	char buffer[32]; // RF24 wireless module max buffer size = 32 bytes
	int len; // length of input data

	// encoder
	pinMode(ENC_A,INPUT_PULLUP);
	pinMode(ENC_B,INPUT_PULLUP);

	// servo objects
	Servo1.attach(SER_1);
	Servo2.attach(SER_2);	
	
	// for debugging with serial monitor if needed
	Serial.begin(9600); 
	
	setup_RF24_wireless();	
	
	// wait for start messge from PC -- this section is neccessary
	// as readBytes doesn't wait properly
	// -> robot waits until the PC contacts it
	// -> need to turn on arduino first then run PC program
	// because of this
	
	// must start listening to read / receive data
	radio.startListening();
	
	// wait for message from RF24 wireless module
	// no timeout here since it's just a start message
	// and the robot hasn't been started
	i = 0;
	while ( ! radio.available() ) i++;	

	len = 3; // number of bytes for message
	
	// read the start message
	// note the read function doesn't block so you need
	// to use available function as above to block
	radio.read(buffer,len);

	delay(100);	
}


void loop()
{ 
	static char buffer[32]; // RF24 wireless module max buffer size = 32 bytes
	int len; // length of input data
	int i,n;
	unsigned char b;
  
	unsigned char start_char = 255; // start character for message 
	// -- don't use this value for anything else
	
	// Arduino inputs sent from PC
  	unsigned char pw1, pw2;
	unsigned long start_time;
	
	// read data from RF24 wireless module
  
	// wait until new data is available
	// this code is good for synchronization with
	// completely reliable communication but it will
	// hang here (ie infinite loop) of there if
	// communication is stopped from the PC
	
	// TODO: read 3 byte message and check 1st byte for start byte
	// discard message and flush RX buffer if 1st byte is not start byte
	
	// note that RX does not flush automatically but write will
	// result in nothing in TX FIFO -- either flush on fail
	// or empty on write ack
	
	// must start listening to read / receive data
	radio.startListening();
	
	// wait for message from RF24 wireless module
	start_time = millis();
	while ( ! radio.available() ) {
		if( millis() - start_time > 1000 ) { // check for timeout
			Serial.print("\nwireless timeout error -- stop program");
			stop_robot();
			delay(100);  // give time for printing before stopping program
			exit(0); // stop program
		}
	}

	len = 3; // number of bytes for message
	
	// read the start message
	// note the read function doesn't block so you need
	// to use available function as above to block
	radio.read(buffer,len);
	
	// first character in message is the start character
	b = buffer[0]; 

	// check for message start byte
	// note: the purpose of the start byte
	// is to synchronize the message
	// in case communication is interrupted,
	// otherwise the inputs can get out of sync
	// because we assume the inputs are in a certain
	// order in the wireless message.
	
	// check for start character
	if(b != start_char) { 
		Serial.print("\nstart character missing from message");
		delay(2);
		return; // try again
	}

	// TODO: if first byte of message is incorrect 
	// discard mesage, flush RX ? and continue	

	// TODO: check for RF24 time out ? (ie communication fault)
	// and turn off robot if that happens
	// note: default serial time out on Arduino is 1s
	
	// get inputs from buffer after the start byte
	pw1 = buffer[1];
	pw2 = buffer[2];
		
	// for testing / debugging	
	Serial.print("\n");
	Serial.print(pw1);
	Serial.print(" ");
	Serial.print(pw2);
	
	// saturation of Arduino inputs -- protect actuators / robot

	// prevent throttle from getting too large
	// 90 should be zero throttle with the ESC
	// once it's been configured properly
	if(pw1 > 120) pw1 = 120;
	if(pw1 < 60)  pw1 = 60;		
	
	// prevent steering angle from getting too large
	if(pw2 > 120) pw2 = 120;
	if(pw2 < 60)  pw2 = 60;	
	
	// send back the data for error checking
	// this is good for debugging but it will stop the robot
	// when that happens which is not good for the final version
	// which you want "robust" to errors.
//	Serial.write(buffer,len);

	// repeatedly perform the following:
	// 1. Read sensors / outputs
	// 2. Calculate control input
	// 3. Write actuators / inputs
	
	// write actuators
	write_actuators(pw1,pw2);
	
}


void setup_RF24_wireless()
{
//	printf_begin(); // for print details function
   
	radio.begin();

	radio.setPALevel(RF24_PA_MAX);	
	// Set Power Amplifier (PA) level to one of four levels:
	// RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX
	// The power levels correspond to the following output levels respectively:
	// NRF24L01: -18dBm, -12dBm,-6dBM, and 0dBm
    // Reset value is MAX  
	
//	radio.setDataRate(RF24_250KBPS);  
	// reduce data rate to 250 kbps for longer range better reliability (default = 1 Mbps)
	
	// enable auto acknowledgements on transmit -- re-transmit if errors are encountered
	// as configured below with setRetries
	// false will disable checking -- faster but less chance to catch errors
	radio.setAutoAck(true); 
	
	radio.setRetries(5,3); // total max delay = (5 + 1) * 3 * 250 us = 4.5 ms

	// more than 3 retries doesn't seem to help much
    
	// Set the number and delay of retries upon failed submit
    // first argument -- How long to wait between each retry, in multiples of 250us,
    // max is 15.  0 means 250us, 15 means 4000us.  5 means 1500 us. 5 is min for 250 kbps.
	// second argument -- How many retries before giving up, max 15	
	
	// note 15 was default for 2nd argument instead of 6 but that's a bit too slow IMHO	

	radio.setChannel(107); // valid range 0 to 125
	// can use scanner example to find free channels
	
	// set reading and writing connections / pipes
	radio.openWritingPipe(addresses[1]); // use [0] for other module
	radio.openReadingPipe(1,addresses[0]); // use [1] for other module
  
	// note pipes 0 and 1 will store a full 5-byte address. Pipes 2-5 will technically 
	// only store a single byte, borrowing up to 4 additional bytes from pipe #1 per the
	// assigned address width.  
	// warning pipe 0 is also used by the writing pipe.  So if you open
	// pipe 0 for reading, and then startListening(), it will overwrite the
	// writing pipe, so call openWritingPipe() again before write(). 
  
	// print a large block of debug information
	// -- useful for checking parameters and their defaults
//	radio.printDetails();	

}


void write_actuators(unsigned char pw1, unsigned char pw2)
{
	int th1, th2;
	
	th1 = pw1;
	th2 = pw2;
	
	// set servos
	Servo1.write(th1);
	Servo2.write(th2);
}


// turn off robot and stop the program
void stop_robot()
{	
  	unsigned char pw1, pw2;

	// set inputs to zero / neutral values
	pw1 = 90;
	pw2 = 90;
	
	// write actuators
	write_actuators(pw1,pw2);

	// different options available below
	
//	cli(); // disable all interrupts to suspend program
	// and allow uninterrupted turning off of the robot
	
	// stop the program, note exit(0) does the same thing
//	cli(); // turn off interrupts / background functions
//	while(1); // infinite do nothing loop
//	or
//	exit(0); // stop program

	// alternatively we can reset the Arduino
	// this allows restarting the program wirelessly
	// without turning off the Arduino power
	// note: you still have to connect to the Arduino
	// to the USB if you want to change the program
//	sw_reset();
}


void sw_reset()
// software reset function using watchdog timer
{
  wdt_enable(WDTO_15MS); // sets a "watchdog" timer to 15 ms
  // so the arduino will reset in 15 ms if the timer is not stopped
  while(1); // infinite do nothing loop -- wait for the countdown
}

