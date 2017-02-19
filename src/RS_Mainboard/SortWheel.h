/*
SortWheel.h - An object for representing the sortation wheel
Created by Shawn Westcott (www.8tinybits.com), Feb 2017.

Schematics, Gerbers, and related source code can be found on this project's Github
https://github.com/slwstctt/ResistorSortSystem

This code is currently unlicensed and is only available for educational purposes.
If you'd like to use this code for a non-educational purpose, please contact Shawn
Westcott (shawn.westcott@8tinybits.com).

*/

#ifndef _SORTWHEEL_h
#define _SORTWHEEL_h 1

#include "Arduino.h"
#include <Wire.h>

class SortWheel {
	public:

	// Constructor -- needs the number of cups on the wheel and the Wire channel that the controller is on.
	SortWheel(int numCups, int wireChannel);

	// Returns the current position on the wheel as an integer (First cup is cup 1)
	int getCurrentPosition();

	// Calculates shortest path to the target and issues a command to the controller.
	void moveTo(int target);

	private:
	int _wireChannel;
	int _curPos;
	int _numCups;

};


#endif