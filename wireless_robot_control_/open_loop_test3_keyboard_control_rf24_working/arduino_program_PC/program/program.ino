
#include <Arduino.h>
#include <Servo.h>
#include <avr/wdt.h> // for sw reset function

#include <SPI.h>
#include "RF24.h"
#include "printf.h" // for print details function

// set up nRF24L01 radio on SPI bus plus pins 7 and 8
RF24 radio(7,8);

byte addresses[][6] = {"1Node","2Node"};

// note you should pick a special address for each node
// on your network -- avoids interference with other groups
// if they happen to choose the same channel by accident
// byte addresses[][6] = {"1GR01","2GR01"}; // for group #1, etc.

// suggest each group use:
// channel = 95 + 2*k, where k is your group number

void setup_RF24_wireless();

void setup()
{
	int i;
	char buffer[32]; // RF24 wirless module max buffer size = 32 bytes
	int len; // length of input data
	
	// initialize serial port communication (usb, etc.) 
	// note: you have to change the communication rate in the arduino program here, 
	// the VC++ program, and the device driver (with device manager), and
	// the serial port monitor if you use it.
	// it's also a good idea to unplug and plug back in the Arduino usb cable
	// after change the device driver setting to restart the driver.
	Serial.begin(9600);
//  Serial.begin(115200);
//  Serial.begin(500000);

	// set serial timeout to 1000 ms
	// Serial.readBytes(...) will return after this time
	// (with the wrong number of bytes)
	Serial.setTimeout(1000);

	setup_RF24_wireless();
	
	// wait for start messge from PC -- this section is neccessary
	// as readBytes doesn't wait properly
	// -> robot waits until the PC contacts it
	// -> need to turn on arduino first then run PC program
	// because of this
	i = 0;
	while( Serial.available() == 0 ) i++;

	// read start message
	len = 1; // number of bytes for message from PC
	Serial.readBytes(buffer,len);

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
	
	// read data from PC serial connection
  
	// wait until new data is available
	// this code is good for synchronization with
	// completely reliable communication but it will
	// hang here (ie infinite loop) of there if
	// communication is stopped from the PC
//	i = 0;
//	while( Serial.available() == 0 ) i++;
	
	// reset buffers to make sure data is new
	for(i=0;i<3;i++) buffer[i] = 0;

	// wait for message start byte
	while(1) {
		len = 1; // number of bytes for message from PC
		n = Serial.readBytes(buffer,len);
		
		// check for message start byte
		// note: the purpose of the start byte
		// is to synchronize the message
		// in case communication is interrupted,
		// otherwise the inputs can get out of sync
		// because we assume the inputs are in a certain
		// order in the wireless message.
		b = buffer[0];
		if(b == start_char) break;
	}

	// read the rest of the incoming message
	len = 2; // number of bytes for message from PC
	// use buffer+1 so the data read starts at element [1]
	n = Serial.readBytes(buffer+1,len);
	
	// the previous function will return an error if
	// no message in 1000 ms (1s) as specified by the serial time out
	// parameter in setup.
	
	if(n != len) { 
		Serial.print("\nreadBytes error");
		return; // try again
	}
	
	// send 3 byte message using RF24 wireless module
	len = 3;
	
	// blocking write / send to car arduino wireless module
	// the write function will check if the autoacknowledgment
	// is succesful or not -- if not it will retry a certain
	// number of times -- if that fails then the function returns
	// a zero value
	if ( !radio.write(buffer,len) ) {
		Serial.print("\nwrite failed -- wasn't received / acknowledged");
	}	
	
	// we won't be a dog with a bone here -- let's finish loop
	
	// check for serial time out (ie communication fault)
	// and turn off robot if that happens
	// note: default serial time out on Arduino is 1s
	
	// TODO: optionally receive message from wireless module
	// and send it back to PC via serial communication
	
	// TODO: can power down / power up reset the module ?
    // powerDown();
	// powerUp();	
	
	// send back the data for error checking
	// this is good for debugging but it will stop the robot
	// when that happens which is not good for the final version
	// which you want "robust" to errors.
//	Serial.write(buffer,len);
	
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
	radio.openWritingPipe(addresses[0]); 
	radio.openReadingPipe(1,addresses[1]); 
	
	// the first argument is the reading pipe number
	// a wireless module can read / listen to up to 6 pipes
	// to make a star like network topology
  
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
