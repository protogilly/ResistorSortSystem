/*
	ProgmemData.cpp - Variable definitions for the Resistor Sortation Project
	Created by Shawn Westcott (www.8tinybits.com), Feb 2017.

	Schematics, Gerbers, and related source code can be found on this project's Github
	https://github.com/slwstctt/ResistorSortSystem

	This code is currently unlicensed and is only available for educational purposes.
	If you'd like to use this code for a non-educational purpose, please contact Shawn
	Westcott (shawn.westcott@8tinybits.com).

*/

#include "Arduino.h"
#include "ProgmemData.h"

// Constants for Moving Servos
	const int contactHome = 45;
	const int contactTouch = 5;
	const int contactPress = 0;
	const int contactTime = 15;		// How long (in millis) does it take to move from Home to Touch?
	const int swingHome = 180;
	const int swingOpen = 130;
	const int swingTime = 15;		// How long (in millis) does it take to move from Home to Open?

// Constants for Calibrated Measurements (Voltages, Resistances, Currents)
	const double avHigh = 3.3;
	const double avLow = 1.5;
	const int internalTestResistances[6] = {10000000, 1000000, 100000, 10000, 1000, 100};
	const double internalCurrentSources[3] = {0.1, 0.02937, 0.01818};

// Standard Resistor values

	// 1% E96 Series
	const int stdResistors1[96] = {
		100, 102, 105, 107, 110, 113, 115, 118, 121, 124, 127, 130, 133, 137, 140, 143,
		147, 150, 154, 158, 162, 165, 169, 174, 178, 182, 187, 191, 196, 200, 205, 210,
		215, 221, 226, 232, 237, 243, 249, 255, 261, 267, 274, 280, 287, 294, 301, 309,
		316, 324, 332, 340, 348, 357, 365, 374, 383, 392, 402, 412, 422, 432, 442, 453,
		464, 475, 487, 499, 511, 523, 536, 549, 562, 576, 590, 604, 619, 634, 649, 665,
		681, 698, 715, 732, 750, 768, 787, 806, 825, 845, 866, 887, 909, 931, 953, 976
	};

	// 2% and 5% E24 Series
	const int stdResistors2_5[24] = {
		100, 110, 120, 130, 150, 160, 180, 200, 220, 240, 270, 300, 330, 360, 390, 430,
		470, 510, 560, 620, 680, 750, 820, 910
	};

	// 10% E12 Series
	const int stdResistors10[12] = {
		100, 120, 150, 180, 220, 270, 330, 390, 470, 560, 680, 820
	};