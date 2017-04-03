/*
ProgmemData.h - Variable declarations for the Resistor Sortation Project
Created by Shawn Westcott (www.8tinybits.com), Feb 2017.

Schematics, Gerbers, and related source code can be found on this project's Github
https://github.com/slwstctt/ResistorSortSystem

This code is currently unlicensed and is only available for educational purposes.
If you'd like to use this code for a non-educational purpose, please contact Shawn
Westcott (shawn.westcott@8tinybits.com).

*/

// Pin Definitions. These follow the naming conventions on the Schematics and PCB.

#ifndef ProgmemData
#define ProgmemData 1

#include "Arduino.h"

// The struct for incoming and outgoing commands.
typedef struct {
	String cmd;
	int numArgs;
	String args[10];
} Command;

// Servo Control Pins
	#define SrvoA		4
	#define SrvoB		3

// ATTiny Pins for various uses
	#define AttTrig0	8
	#define AttTrig1	10
	#define AttTrig2	11
	#define AttTrig3	12
	#define AttTrig4	7

// LED Pin for Debugging
  #define ledPin  13

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
	#define SDA		18
	#define SCL		19

// I2C Slave Channels
	#define SortController 0x1	
	#define FeedController 0x2
	
// Analog Resolution
	#define bitPrecision 12

// Constants for Moving Servos
	extern const int contactHome;
	extern const int contactTouch;
	extern const int contactPress;
	extern const int contactTime;		// How long (in millis) does it take to move from Home to Touch?
	extern const int swingHome;
	extern const int swingOpen;
	extern const int swingTime;		// How long (in millis) does it take to move from Home to Open?

// Constants for Calibrated Measurements (Voltages, Resistances, Currents)
	extern const double avHigh;
	extern const double avLow;
	extern const double internalTestResistances[6];
	extern const double internalCurrentSources[3];

// Standard Resistor values

// 1% E96 Series
	extern const int E96Count;
	extern const int stdResistors1[96];

// 2% and 5% E24 Series
	extern const int E24Count;
	extern const int stdResistors2_5[24];

// 10% E12 Series
	extern const int E12Count;
	extern const int stdResistors10[12];

#endif
