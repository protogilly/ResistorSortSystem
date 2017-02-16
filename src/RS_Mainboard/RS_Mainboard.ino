/*
	RS_Mainboard.ino - Code to control the Mainboard for the Resistor Sortation Project
	Created by Shawn Westcott (www.8tinybits.com), Feb 2017.

	Schematics, Gerbers, and related source code can be found on this project's Github
	https://github.com/slwstctt/ResistorSortSystem

	This code is currently unlicensed and is only available for educational purposes.
	If you'd like to use this code for a non-educational purpose, please contact Shawn
	Westcott (shawn.westcott@8tinybits.com).

*/

#include <Arduino.h>
#include <PWMServo.h>
#include <ShiftRegister74HC595.h>		// See: http://shiftregister.simsso.de/
#include <Wire.h>
#include "ProgmemData.h"

// Declaring Servos. ContactArm presses contacts onto resistors for measurement, SwingArm releases and retains resistors.
PWMServo ContactArm;
PWMServo SwingArm;

// Number of shift registers in circuit
const int srCount = 2;

// Declaring the Shift Register stack.
ShiftRegister74HC595 ShiftReg(srCount, DAT0, SCLK0, LCLK0);

// Shift register states
const uint8_t srState[10][srCount] = {
	{B00000000, B00000000},	// Empty
	{B10000000, B00000000},	// 10M Range
	{B01000000, B00000000},	// 1M Range
	{B00100000, B00000000},	// 100k Range
	{B00010000, B00000000},	// 10k Range
	{B00001000, B00000000},	// 1k Range
	{B00000100, B00000000},	// 100R Range
	{B00000010, B01000000},	// 100mA Source
	{B00000001, B01000000},	// 29.37mA Source
	{B00000000, B11000000} 	// 18.18mA Source
};

// The resistor queue is represented internally by a collection of bits. 0 indicates no resistor in this position, while 1 indicates a resistor.
// The LSB represents where the user feeds a resistor into the system, and bit 4 indicates the test platform. Bits 5-7 are ignored.
uint8_t resistorQueue = {B00000000};

// These variables keep track of states of slave processors.
volatile bool feedInProcess = false;
volatile bool sortMotionInProcess = false;

// State Machine Variable
// Possible States:
	//	00	Waiting for Mode Set (RPi Command)
	//	01	Ready for next resistor
	//	02	Measurement
	//	04	Dispense
	//	05	Cycling through to end
volatile int cState = 0;

// Sort Mode from RPi
int cMode = 0;

// Maximum Analog Value, calculated at setup.
int maxAnalog = 0;

void setup() {
	// Init Servos
	ContactArm.attach(SrvoA);
	SwingArm.attach(SrvoB);

	// Home the Servos
	SwingArm.write(swingHome);
	ContactArm.write(contactHome);

	// Setting up various pins
	pinMode(OE0, OUTPUT);
	pinMode(RST0, OUTPUT);
	pinMode(AttTrig0, INPUT);	// Trigger from Sort Controller indicating last command completed successfully
	pinMode(AttTrig1, OUTPUT);
	pinMode(AttTrig2, OUTPUT);
	pinMode(AttTrig3, INPUT);	// Trigger from Feed Controller indicating last command completed successfully
	pinMode(AttTrig4, OUTPUT);

	// AttTrig3 is an interrupt from the slave processor that lets us know the last feed command is complete.
	attachInterrupt(AttTrig3, isrFeedClear, RISING);

	// AttTrig0 is an interrupt from the slave processor that lets us know the last sort move command is complete.
	attachInterrupt(AttTrig0, isrWheelClear, RISING);

	// Reset and enable output on the shift register
	clearRegisters();
	digitalWrite(OE0, LOW);		// Output Enable is Active-low

	// I2C Wire init
	Wire.begin();
	
	// Enable External AREF
	analogReference(EXTERNAL);
	analogReadResolution(bitPrecision);
	analogReadAveraging(16);					// Average 16 reads at ADC for result.
	maxAnalog = pow(2, bitPrecision) - 1;		// Max value 4095 for 12 bits

	// Begin Serial comms with RPi
	Serial.begin(9600);
	Serial.println("RDY");
}

void loop() {

	// The loop is a state machine, the action the system takes depends on what state it is in.
	switch (cState) {
		
		case 0:
		// Waiting for Mode Set (RPi Command)
			if (Serial.available() > 0) {
				cMode = Serial.parseInt();
				if (cMode < 0 || cMode > 4) {
					Serial.println("X");
					cMode = 0;
				} else {
					Serial.println("ACK");
					setNewModeState(cMode);
				}
			}
			break;
		
		case 1:
		// Ready for next resistor
			//TODO: rest of case logic
			break;
	}
}

void isrFeedClear() {
	feedInProcess = false;
}

void isrWheelClear() {
	sortMotionInProcess = false;
}

void setNewModeState(int newMode) {
	// TODO: Set State from new Mode command
}

void clearRegisters() {
	// This function triggers the reset on the shift registers, then latches the empty register.

	digitalWrite(RST0, LOW);	// RESET is Active-low
	delay(1);
	digitalWrite(RST0, HIGH);

	// Going LO-HI-LO ensures a rising edge is seen, then the latch clock is set back to the expected rest state (LOW).
	digitalWrite(LCLK0, LOW);
	delay(1);
	digitalWrite(LCLK0, HIGH);
	delay(1);
	digitalWrite(LCLK0, LOW);
}

int cycleFeed(int count) {
	// This function triggers a feed cycle and updates the internal representation accordingly.
	// A result of 0 indicates success. Any other result indicates failure.

	// Only feed if the count requested is reasonable (no feeding more than 4 positions, no feeding 0 or less positions)
	if (count > 4 || count < 0) {
		return(-1);
	}

	// Only feed if the feed is not currently running
	if (feedInProcess) {
		return(-2);
	}

	bool isClear = true;

	// Only cycle feed if the test platform is clear. Bits 0-4 of the queue are the feed states.
	for (int i = (5 - count); i > 0; i--) {
		if (bitRead(resistorQueue, i) == true) {
			isClear = false;
		}
	}

	if (!isClear) {
		return(-3);
	}

	// Send the number of cycles to the Feed Controller.
	Wire.beginTransmission(FeedController);
	Wire.write(count);
	Wire.endTransmission();

	// Note that a feed is in process, we will not send additional feed commands until finished.
	feedInProcess = true;

	// Update the internal representation of the feed queue.
	resistorQueue = resistorQueue << count;

	return(0);

}

double measureResistor() {
	// This function completes a full measurement cycle and returns a resistance in Ohms.
	// Negative values represent failures, 0.0 represents a rejected resistor.
	// Checking errors should involve typecasting this to int to avoid issues with floating point precision tests.
	
	if (bitRead(resistorQueue, 4) == false) {
		return(-1.0);
	}
	
	ContactArm.write(contactTouch);
	delay(contactTime);
	
	// TODO: Determine algorithm to check if contact is positively made.
	bool contactMade = true
	
	// If contact is not made, bring the test arm to push on the contact
	if (!contactMade) {
		ContactArm.write(contactPress);
	}
	
	// TODO: Determine algorithm to check if contact is positively made.
	bool contactMade = true
	
	// Still no contact? Reject this resistor.
	if (!contactMade) {
		return(0.0);
	}
	
	int medianReading = maxAnalog / 2;
	int cDifference, bestDifference = 99999;	// Arbitrarily large value here to ensure any reading is superior.
	int bestRange = 0;							// Range 0 is with outputs turned off, a safe fallback in case of failure.
	int reading, bestReading = 0;
	
	// For each range...
	for (int i = 1, i == 9, i++) {
		// Enable the outputs for testing this range and take a measurement.
		ShiftReg.setAll(srState[i]);
		delay(10);								// 5ms maximum operating time for relays, doubled for safety.
		reading = analogRead(RMeas);
		
		// calculate the difference to center.
		cDifference = reading - medianReading;
		cDifference = abs(cDifference);
		
		// If this measurement is superior to the current best, take note.
		if (cDifference < bestDifference) {
			bestRange = i;
			bestDifference = cDifference;
			bestReading = reading;
		}
	}
	
	// TODO: Convert to Ohms based on bestRange and bestReading.
	double result = 0.0;
	
	
	// Return the Ohms value.
	return(result);
}

int dispenseResistor() {
	// This function is triggered when the system kicks a resistor out into a cup.
	// A result of 0 indicates success. Any other result indicates failure.

	// Only dispense if the test platform contains a resistor.
	if (bitRead(resistorQueue, 4) == false) {
		return(-1);
	}

	SwingArm.write(swingOpen);
	delay(swingTime);			// actual delay here, since we shouldn't move or process anything else until we're sure this is clear.
	SwingArm.write(swingHome);

	resistorQueue = resistorQueue & B11101111;	// Set bit 4 (Test Platform) low.

	return(0);

}

int feedResistor() {
	// This function is triggered when the user loads a resistor into the system.
	// A result of 0 indicates success. Any other result indicates failure.

	// resistorQueue will be even if the load platform is cleared. Only add a new resistor if the platform is clear.
	if (resistorQueue % 2 == 0) {
		resistorQueue = resistorQueue | B00000001;		// Set the LSB (load platform) high
		return(0);
	} else {
		return(-1);
	}

}