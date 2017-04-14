/*
SortWheel.cpp - An object for representing the sortation wheel
Created by Shawn Westcott (www.8tinybits.com), Feb 2017.

Schematics, Gerbers, and related source code can be found on this project's Github
https://github.com/slwstctt/ResistorSortSystem

This code is currently unlicensed and is only available for educational purposes.
If you'd like to use this code for a non-educational purpose, please contact Shawn
Westcott (shawn.westcott@8tinybits.com).

*/

#include "Arduino.h"
#include <Wire.h>
#include "StepFeed.h"

StepFeed::StepFeed(int wireChannel) {
	_wireChannel = wireChannel;
	_queue = {B00000000};
}

int StepFeed::cycleFeed(int count) {
	// Only feed if the count requested is reasonable (no feeding more than 4 positions, no feeding 0 or less positions)
	if (count > 4 || count < 0) {
		return(-1);
	}

	Wire.beginTransmission(_wireChannel);
	Wire.write(count);
	Wire.endTransmission();

	// Left shift the queue (This mimics the physical action where resistors are shifted forward)
	_queue = _queue << count;

	return(0);
}

bool StepFeed::loadPlatformEmpty() {
	// queue will be even if the load platform is clear.
	if ((_queue % 2) == 0) {
		return(true);
	} else {
		return(false);
	}
}

bool StepFeed::measurePlatformEmpty() {
	// Queue entry 4 is the measurement platform.
	if (bitRead(_queue, 4)) {
		return(false);
	} else {
		return(true);
	}
}

bool StepFeed::feedEmpty() {
	if (_queue == 0) {
		return(true);
	} else {
		return (false);
	}
}

void StepFeed::load() {
	// Force a 1 into the Load Platform bit.
	_queue = _queue | B00000001;
}

void StepFeed::dispense() {
	// Force a 0 into the Test Platform bit
	_queue = _queue & B11101111;

}
