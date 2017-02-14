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

// Pin Definitions. These follow the naming conventions on the Schematics and PCB.

// Servo Control Pins
#define SrvoA		4
#define SrvoB		3

// ATTiny Pins for various uses
#define AttTrig0	8
#define AttTrig1	10
#define AttTrig2	11
#define AttTrig3	12
#define AttTrig4	7

// Analog Inputs
#define RMeas		0		// Analog Pin 0, Digital 14
#define TempSense	1		// Analog Pin 1, Digital 15

// Shift Register Pins
#define DAT0		17
#define SCLK0		20
#define LCLK0		21
#define OE0		22
#define RST0		23

// I2C Pins
#define SDA			18
#define SCL			19

// I2C Slave Channels
#define FeedController 1
#define SortController 2

// Declaring Servos. ContactArm presses contacts onto resistors for measurement, SwingArm releases and retains resistors.
PWMServo ContactArm;
PWMServo SwingArm;

// Number of shift registers in circuit
const int srCount = 2;

// Declaring the Shift Register stack.
ShiftRegister74HC595 ShiftReg(srCount, DAT0, SCLK0, LCLK0);

// Constants for Moving Servos
const int contactHome = 45;
const int contactTouch = 5;
const int contactPress = 0;
const int swingHome = 180;
const int swingOpen = 130;

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
	pinMode(AttTrig0, INPUT);
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

	// Begin Serial comms with RPi
	Serial.begin(9600);
	Serial.println("RDY");



}

void loop() {

	/* add main program code here */

}

void isrFeedClear() {
	feedInProcess = false;
}

void isrWheelClear() {
	sortMotionInProcess = false;
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

	// Only cycle feed if the test platform is clear. Bit 4 of the queue is the test platform.
	for (int i = 5 - count; i = 0; i--) {
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
	resistorQueue << count;

	return(0);

}

int FeedResistor() {
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