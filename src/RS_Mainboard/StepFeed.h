/*
StepFeed.h - An object for representing the step feeder
Created by Shawn Westcott (www.8tinybits.com), Feb 2017.

Schematics, Gerbers, and related source code can be found on this project's Github
https://github.com/slwstctt/ResistorSortSystem

This code is currently unlicensed and is only available for educational purposes.
If you'd like to use this code for a non-educational purpose, please contact Shawn
Westcott (shawn.westcott@8tinybits.com).

*/

#ifndef _STEPFEED_h
#define _STEPFEED_h 1

#include "Arduino.h"
#include <Wire.h>

class StepFeed {
	public:
		StepFeed(int wireChannel);	// Constructor -- needs the Wire channel that the controller is on.
		
		// Sends the command to cycle the step feeder count times and updates internal representation.
		int cycleFeed(int count);

		// Returns the state of the load platform
		bool loadPlatformEmpty();

		// Returns the state of the measurement platform.
		bool measurePlatformEmpty();

		// Returns true when all feed holds empty.
		bool feedEmpty();

		// Loads a new resistor into the system at the load platform
		void load();
		
		// "Dispenses" a resistor from the system at the measurement platform (Actuation still needed)
		void dispense();

	private:
		int _wireChannel;
		uint8_t _queue;			// The resistor queue is represented internally by a collection of bits. The LSB = feed platform, bit 4 = test platform. Bits 5-7 are ignored.

};


#endif