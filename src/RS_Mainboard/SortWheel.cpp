/*
SortWheel.cpp - An object for representing the sortation wheel and its cups
Created by Shawn Westcott (www.8tinybits.com), Feb 2017.

Schematics, Gerbers, and related source code can be found on this project's Github
https://github.com/slwstctt/ResistorSortSystem

This code is currently unlicensed and is only available for educational purposes.
If you'd like to use this code for a non-educational purpose, please contact Shawn
Westcott (shawn.westcott@8tinybits.com).

*/

#include "Arduino.h"
#include <Wire.h>
#include "SortWheel.h"

SortWheel::SortWheel(int numCups, int wireChannel) {
	_wireChannel = wireChannel;
	_numCups = numCups;
	_curPos = 1;
}

int SortWheel::getCurrentPosition() {
	return(_curPos);
}

void SortWheel::moveTo(int target) {
	int rightPath = 0;
	int leftPath = 0;
	int bestRoute = 0;

	if (_curPos < target) {
		rightPath = target - _curPos;
		leftPath = _numCups - target + _curPos;
	} else {
		rightPath = _numCups - _curPos + target;
		leftPath = _curPos - target;
	}

	bestRoute = rightPath < leftPath ? rightPath : -leftPath;

	Wire.beginTransmission(_wireChannel);
	Wire.write(bestRoute);
	Wire.endTransmission();

	_curPos = target;
}

SortCup::SortCup() {
	
	_minVal = 0.0;
	_maxVal = 0.0;
	_rejectCup = true;
}

SortCup::SortCup(double minValue, double maxValue) {
	
	this->setCupRange(minValue, maxValue);
	_rejectCup = false;
}

SortCup::SortCup(double nominal, int precision) {

	this->setCupRange(nominal, precision);
	_rejectCup = false;
}

double SortCup::getMin() {
	return(_minVal);
}

double SortCup::getMax() {
	return(_maxVal);
}

bool SortCup::canAccept(double value) {
	int iVal = (int) value;

	if (this->isReject() && iVal == 0) {
		return(true);
	}

	if (value <= _maxVal && value >= _minVal) {
		return(true);
	} else {
		return(false);
	}
}

bool SortCup::isReject() {
	return(_rejectCup);
}

void SortCup::setRejectState(bool state) {
	_rejectCup = state;
}

void SortCup::setCupRange(double minValue, double maxValue) {

	_minVal = minValue;
	_maxVal = maxValue;
}

void SortCup::setCupRange(double nominalValue, int precision) {
	double range = nominalValue * ((double) precision / 100.0);

	_minVal = nominalValue - range;
	_maxVal = nominalValue + range;
}