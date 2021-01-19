
// keyboard control of car robot

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <iostream>

#include <conio.h>
#include <windows.h>

// serial communication functions
#include "serial_com.h"

#include "timer.h"

// macro that check for a key press
#define KEY(c) ( GetAsyncKeyState((int)(c)) & (SHORT)0x8000 )

using namespace std;

int main()
{ 
	HANDLE h1;
	const int NMAX = 32; // 32 bytes is the max size of RF24 messages 
	char buffer_in[NMAX], buffer_out[NMAX];
	int n, i;
	double v, th, dv, dth;
	unsigned char pw1, pw2; // ints that go from 0 to 255
	
	unsigned char start_char = 255; // start character for message 
	// -- don't use this value for anything else

	// note: the serial port driver should be set to the same 
	// data (baud) rate in the device manager com port settings
	// and the Arduino program.
	open_serial("COM5",h1);
	
	// note: open_serial will reset the Arduino board
	// and start the program

	// synchronize serial port communication

	cout << "\npress c key to continue";
	while( !KEY('C') ) Sleep(1);

	// send start message to the Arduino connected to PC
	// note: setup() ends and loop() begins after this	
	n = 1;
	buffer_out[0] = 's';	
	serial_send(buffer_out,n,h1);
	Sleep(100);

	cout << "\n\npress arrow keys to control robot (press x to exit)\n";
	
	// initial inputs
	v  = 0.0;
	th = 0.0;
	pw1 = 0;
	pw2 = 0;

	// input increments
	dv = 0.05;
	dth = 3.0;

	while(1) {

		// read keys and set robot inputs
		if( KEY(VK_UP) ) {
			v += dv; // increase velocity input
		}

		if( KEY(VK_DOWN) ) {
			v -= dv; // decrease velocity input
		}

		if( KEY(VK_LEFT) ) {
			th += dth; // increase rotation input
		}

		if( KEY(VK_RIGHT) ) {
			th -= dth; // decrease rotation input
		}

		// stop robot
		if( KEY(VK_SPACE) ) {
			v = 0.0;
		}

		// approximate saturation of virtual inputs
		if(v > 1.0)  v = 1.0;
		if(v < -1.0) v = -1.0;
		if(th > 45.0) th = 45.0;
		if(th < -45.0) th = -45.0;

		// convert from v, th to actuator values
		pw1 = 90.1 + v*90; // 0 to 180
		pw2 = 90.1 + th;  // 45 to 180-45
		
		// print out variables for testing / debugging
//		cout << "\nv = " << v;
//		cout << " th = " << th;
		cout << "\npw1 = " << (int)pw1;
		cout << " pw2 = " << (int)pw2;
				
		// send input to Arduino using serial communication
		buffer_out[0] = start_char; // message start character
		buffer_out[1] = pw1;
		buffer_out[2] = pw2;		

		// send inputs to Arduino
		
		n = 3; // total number of bytes for message

		// terminate program if there is an error
		if ( serial_send(buffer_out,n,h1) ) {
			cout << "\nserial send error.\npress c key to continue\n";
			while( !KEY('C') ) Sleep(1);
			return 1;
		}

		// wait for data to be sent to Arduino
		// the delay also results in an wireless packet being sent
		Sleep(50); // can probably reduce this to 10 ms
		
		// note: we need to sleep a bit to slow the loop down
		// otherwise the Arduino serial input buffer will overflow

/*
		// receive data for checking or data logging

		// this part checks to make sure the message received is the same
		// as the message sent out.
		// this is good for debugging / performance measuremnt (optimization)
		// but not real practice since it stops the robot if even
		// just one message is lost.
		
		// check for communication errors / fault
		// terminate program if there is an error
		if ( serial_recv(buffer_in,n,h1) ) {
			cout << "\nserial receive error.\npress c key to continue\n";
			while( !KEY('C') ) Sleep(1);			
			return 1;
		}
		for(i=0;i<n;i++) {
			if(buffer_in[i] != buffer_out[i]) {
				cout << "\nserial communication error";
				return 1; // terminate program
				// fault recovery code could be put here
			}
		}
*/
		if( KEY('X') ) break;
	}

	// turn off robot before ending program
	buffer_out[0] = start_char; // message start character
	for(i=1;i<n;i++) buffer_out[i] = 90; // neutral input
	serial_send(buffer_out,n,h1);
	Sleep(500); // wait for transmission before closing serial

	close_serial(h1);

	cout << "\ndone.\npress c key to continue\n";
	while( !KEY('C') ) Sleep(1);

 	return 0; // no errors
}