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
#include "ProgmemData.h"

#include <PWMServo.h>
#include <Wire.h>
#include <ShiftRegister74HC595.h>		// See: http://shiftregister.simsso.de/

#include "StepFeed.h"
#include "SortWheel.h"

#include <ADC.h>

// Setting up ADC object
ADC *adc = new ADC();

// Declaring Servos. ContactArm presses contacts onto resistors for measurement, SwingArm releases and retains resistors.
PWMServo ContactArm;
PWMServo SwingArm;

// Init the SortWheel and StepFeed objects
SortWheel Wheel(cupCount, SortController);
StepFeed Feed(FeedController);

// Number of shift registers in circuit
const int srCount = 2;

// Declaring the Shift Register stack.
ShiftRegister74HC595 ShiftReg(srCount, DAT0, SCLK0, LCLK0);

// Shift register states
uint8_t srState[10][srCount] = {
	{B00000000, B00000000},	// Empty
	{B10000000, B00000000},	// 10M Range
	{B01000000, B00000000},	// 1M Range
	{B00000100, B00000000},	// 100k Range
	{B00001000, B00000000},	// 10k Range
	{B00010000, B00000000},	// 1k Range
	{B00100000, B00000000},	// 100R Range
	{B00000010, B01000000},	// 100mA Source
	{B00000001, B01000000},	// 29.37mA Source
	{B00000000, B11000000} 	// 18.18mA Source
};

// These variables keep track of states of slave processors.
volatile bool feedInProcess = false;
volatile bool sortMotionInProcess = false;

// Maximum Analog Value, calculated at setup.
int maxAnalog = 0;

// State Machine Variable
volatile int cState = 0;

// Global rep of measurement data
double measurement = 0.0;

// Bool set when feeding out the remainder of the feed stack
bool feedToEnd = false;

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
	pinMode(AttTrig0, INPUT_PULLUP);	// Trigger from Sort Controller indicating last command completed successfully
	pinMode(AttTrig1, OUTPUT);
	pinMode(AttTrig2, OUTPUT);
	pinMode(AttTrig3, INPUT_PULLUP);	// Trigger from Feed Controller indicating last command completed successfully
	pinMode(AttTrig4, OUTPUT);
	pinMode(ledPin, OUTPUT);

	// AttTrig3 is an interrupt from the slave processor that lets us know the last feed command is complete.
	attachInterrupt(AttTrig3, isrFeedClear, RISING);

	// AttTrig0 is an interrupt from the slave processor that lets us know the last sort move command is complete.
	attachInterrupt(AttTrig0, isrWheelClear, RISING);

	// Reset and enable output on the shift register
	clearRegisters();
	digitalWrite(OE0, LOW);		// Output Enable is Active-low
	digitalWrite(ledPin, HIGH);

	// I2C Wire init
	Wire.begin();

	// Wait 100ms just to make sure the Feed Controller is finished with setup before sending it data
	delay(100);

	// Send setup value to StepFeeder controller
	Wire.beginTransmission(FeedController);
	
	// Number of steps per feed action = 115 degrees, 1.8 deg/step = ~64 steps per action.
	Wire.write(64);
	Wire.endTransmission();
	
	// Set up ADC
	adc->setReference(ADC_REF_EXT);
	adc->setResolution(bitPrecision);
	adc->setAveraging(32);
	maxAnalog = adc->getMaxValue();
	adc->setSamplingSpeed(ADC_VERY_LOW_SPEED);
	adc->setConversionSpeed(ADC_VERY_LOW_SPEED);

	// Begin Serial comms with RPi
	Serial.begin(9600);
	
	// Wait for the Ready from the RPi.
	do {
		;
	} while (!cmdReady());

	String incCmd = Serial.readString();
	Command handshake = parseCmd(incCmd);

	if (handshake.cmd == "RDY") {
		sendAck();
		digitalWrite(ledPin, LOW);
	} else {
		sendError("Inv Handshake on Startup. Halting.");
		while (true) { ; }	// Something went very wrong. Stop here.
	}
}

void loop() {

	Command thisCommand;

	// At the start of every loop, check if a command is waiting.
	if (cmdReady()) {
		String incCmd = Serial.readString();
		Command thisCommand = parseCmd(incCmd);
	}

	// The loop is a state machine, the action the system takes depends on what state it is in.
	//switch (cState) {
	//	case 0:
	//	// Waiting for Mode Set (RPi Command). Do nothing.
	//		break;
	//	
	//	case 1:
	//	// Ready for next Resistor Load Command
	//		// NXT is the command that indicates the user has pressed the button saying they loaded a resistor.
	//		if (thisCommand.cmd == "NXT") {
	//			if (feedInProcess) {
	//				sendError("Feed In Process");
	//			} else if (!Feed.loadPlatformEmpty()) {
	//				sendError("Load Platform Not Empty");
	//			} else {
	//				Feed.load();
	//				cState = 2;		// Feed Process
	//			}
	//		}

	//		if (thisCommand.cmd == "END") {
	//			feedToEnd = true;
	//			cState = 2;			// Feed Process
	//		}
	//		
	//		break;

	//	case 2:
	//	// Resistor Feeding
	//		if (Feed.loadPlatformEmpty()) {
	//			// If the feed platform is empty, but we're in a motion, wait.
	//			if (feedInProcess) {
	//				break;
	//			}

	//			// If we're not feeding to the end after waiting, we're clear for a new command.
	//			if (!feedToEnd) {
	//				sendReady();
	//				cState = 1;		// Ready for next command
	//				break;
	//			}
	//		}

	//		// Because of breaks, we only get to this point if the load platform is full.
	//		// Structuring in this way allows a states where we are feeding to the end to fall through to this point.

	//		if (!Feed.measurePlatformEmpty()) {
	//			// If the measurement platform is full, we have to handle that first.
	//			cState = 3;				// Measure Resistor
	//		} else {
	//			if (Feed.feedEmpty()) {
	//				// If the feed is empty, we must be ready.
	//				sendReady();
	//				cState = 1;			// Ready for next command
	//			} else {
	//				// Otherwise, cycle the feed and mark the motion in process.
	//				Feed.cycleFeed(1);
	//				feedInProcess = true;
	//			}
	//		}

	//		break;

	//	case 3:
	//	// Measure Resistor
	//		if (!Feed.measurePlatformEmpty()) {
	//			// If the measurement platform isn't empty, measure the resistor
	//			measurement = measureResistor();

	//			// Get the target cup and begin the sort motion
	//			int targetSortPos = getTargetCup(measurement);
	//			Wheel.moveTo(targetSortPos);
	//			sortMotionInProcess = true;
	//			cState = 4;				// Dispense Resistor
	//		} else {
	//			// If it's empty, we either need to go back to feeding or attempt dispense again.
	//			if (sortMotionInProcess) {
	//				cState = 4;			// Dispense Resistor
	//			} else {
	//				cState = 2;			// Feed Process
	//			}
	//		}

	//		break;

	//	case 4:
	//	// Dispense Resistor
	//		if (!sortMotionInProcess) {
	//			// A dispense state occurs after a sort motion has begun. Wait for the sort motion to complete and dispense. EZPZ.
	//			SwingArm.write(swingOpen);
	//			delay(swingTime);			// actual delay here, since we shouldn't move or process anything else until we're sure this is clear.
	//			Feed.dispense();
	//			SwingArm.write(swingHome);
	//			delay(swingTime);
	//			cState = 2;				// Feed Process
	//		}
	//
	//}
}

void isrFeedClear() {
	feedInProcess = false;
	digitalWrite(ledPin, LOW);
}

void isrWheelClear() {
	sortMotionInProcess = false;
	digitalWrite(ledPin, LOW);
}

Command parseCmd(String incCmd) {
	// parses a command string into the Command struct. Also handles certain vital commands, such as Halt.
	
	Command output;
	
	// Start by deleting the verification bit
	incCmd.remove(0, 1);

	// Fetch the first 3 characters and delete them, with the semicolon.
	output.cmd = incCmd.substring(0, 3);
	incCmd.remove(0, 4);

	int argIndex = 0;
	int commaIndex = incCmd.indexOf(',');

	// Fetch all args until there are no delimiters left
	while (commaIndex != -1) {
		output.args[argIndex] = incCmd.substring(0, commaIndex);
		incCmd.remove(0, commaIndex + 1);
		argIndex++;
	}

	// The rest of the string is the last arg.

	output.args[argIndex] = incCmd;
	output.numArgs = argIndex + 1;

	// Commands that follow are Debug commands, which are handled immediately.

	// Halt command recieved. Requires reset.
	if (output.cmd == "HCF") {
		sendAck();
		while (1) { ; }
	}

	 // Always ACK a RDY.
	 if (output.cmd == "RDY") {
	    sendAck();
	 }

	// Debugging command: Cycle Feed
	if (output.cmd == "CFD") {
		int numCycles = output.args[0].toInt();
		Feed.cycleFeed(numCycles);
		digitalWrite(ledPin, HIGH);
		sendAck();
	}

	// Debugging command: Move Sort Wheel
	if (output.cmd == "MSW") {
		int targetCup = output.args[0].toInt();
		Wheel.moveTo(targetCup);
		digitalWrite(ledPin, HIGH);
		sendAck();
	}

	// Debugging command: Cycle Dispense Arm
	if (output.cmd == "CDA") {
		digitalWrite(ledPin, HIGH);
		SwingArm.write(swingOpen);
		delay(swingTime);
		SwingArm.write(swingHome);
		delay(swingTime);
		digitalWrite(ledPin, LOW);
		sendAck();
	}

	// Debugging command: Test Measurement
	if (output.cmd == "TME") {
		sendAck();

		// Get the current cup and attempt a measurement
		double testMeasurement = measureResistor();
		int thisCup = Wheel.getCurrentPosition();

		// Construct a response
		Command mesCmd;
		mesCmd.cmd = "MES";
		mesCmd.numArgs = 2;
		mesCmd.args[0] = String(thisCup);
		mesCmd.args[1] = String(testMeasurement);

		// Send the measurement data
		sendCommand(mesCmd);

	}

	 if (output.cmd == "RST") {
	    sendAck();
	    _reboot_Teensyduino_();
	 }

	return(output);
}

String parseCmd(Command outCmd) {
	// parses a Command struct into a command string for sendout.

	String output;
	output = outCmd.cmd;
	output += ";";

	for (int i = 0; i < outCmd.numArgs; i++) {
		output += outCmd.args[i];
		output += ",";
	}

	// Delete the last character in the string
	if (outCmd.numArgs > 0) {
		output = output.substring(0, output.length() - 1);
	}

	int verifyValue = output.length() + 1;

	char lenVerify = (char)verifyValue;
	output = lenVerify + output;

	return(output);
}

bool cmdReady() {
	// The first byte of a command will be the number of bytes in the command.
	bool result = ((int) Serial.peek() == (int)Serial.available());
	return(result);
}

void sendCommand(Command sendCmd) {
	String output = parseCmd(sendCmd);

	Serial.println(output);
	Serial.flush();
}


void sendError(String err) {
	// Sends an error to the RPi

	Command errCommand;

	errCommand.cmd = "ERR";
	errCommand.numArgs = 1;
	errCommand.args[0] = err;

	sendCommand(errCommand);
}

void sendDat(String dat) {
	// Sends misc data to the RPi

	Command datCommand;

	datCommand.cmd = "DAT";
	datCommand.numArgs = 1;
	datCommand.args[0] = dat;

	sendCommand(datCommand);
}

void sendReady() {
	Command readyCommand;

	readyCommand.cmd = "RDY";
	readyCommand.numArgs = 0;

	sendCommand(readyCommand);
}

void sendAck() {
	Command ackCommand;

	ackCommand.cmd = "ACK";
	ackCommand.numArgs = 0;

	sendCommand(ackCommand);
}

/*
case 1:
// Mode 1 is Major Divisions, powers of 10 with precisions included.
	for (int i = 0; i < 8; i++) {
		// For each target cup, give it a min and max corresponding to a power of 10
		double min = pow(10.0, i);
		double max = min * 10;
		min = min - (min * pMultiplier);
		max = max + (max * pMultiplier);
		Cups[i].setCupRange(min, max);
	}

	// State 1, waiting for a resistor
	return(1);
	break;
*/

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

double measureResistor() {
	// This function completes a full measurement cycle and returns a resistance in Ohms.
	// 0.0 represents a rejected resistor.
	
	//ContactArm.write(contactTouch);
	//delay(contactTime);
	
	// TODO: Determine algorithm to check if contact is positively made.
	//bool contactMade = true;
	
	// If contact is not made, bring the test arm to push on the contact
	//if (!contactMade) {
	//	ContactArm.write(contactPress);
	//}
	
	// TODO: Determine algorithm to check if contact is positively made.
	//contactMade = true;
	
	// Still no contact? Reject this resistor.
	//if (!contactMade) {
	//	return(0.0);
	//}
	
	//int medianReading = maxAnalog / 2;
	//int cDifference, bestDifference = 99999;		// Arbitrarily large value here to ensure any reading is superior.
	//int bestRange = 0;							// Range 0 is with outputs turned off, a safe fallback in case of failure.
	//double reading, bestReading = 0.0;
	
	// For each range...
	//for (int i = 1; i <= 9; i++) {
		// Enable the outputs for testing this range and take a measurement.
	//	ShiftReg.setAll(srState[i]);
	//	delay(100);							// 5ms maximum operating time for relays, doubled for safety.
	//	reading = analogRead(RMeas);
	//	
		// calculate the difference to center.
	//	cDifference = reading - medianReading;
	//	cDifference = (cDifference < 0) ? -cDifference : cDifference;	// Absolute value
		
		// If this measurement is superior to the current best, take note.
	//	if (cDifference < bestDifference) {
	//		bestRange = i;
	//		bestDifference = cDifference;
	//		bestReading = reading;
	//	}
	//}

	int bestRange = 2;

	ShiftReg.setAll(srState[bestRange]);
	delay(100);

	double bestReading = adc->analogRead(RMeas);
	long readingSums = 0;

	int count = 0;
	for (int i = 0; i < 1000; i++) {
		readingSums = readingSums + adc->analogRead(RMeas);
		count++;
	}

	bestReading = readingSums / count;


	// Changing the range to a 0 index instead of a 1 index
	bestRange--;

	String data = String(bestReading);
	sendDat(data);
	
	double result = 0.0;

	// 7, 8, and 9 are the current source ranges. 1-6 are Volt Divider ranges
	if (bestRange < 6) {
		// First, convert the reading to volts. (High voltage for dividers)
		double vReading = bestReading * (avHigh / maxAnalog);
		
		data = String(vReading);
		sendDat(data);

		data = String(internalTestResistances[bestRange], 4);
		sendDat(data);
		
		// Voltage divider formula solved for R2...
		result = (internalTestResistances[bestRange] * vReading) / (avHigh - vReading);

	} else {
		// Bring the range down to 0 index from 6-8 index
		bestRange = bestRange - 6;

		//Convert the reading to volts. (Low voltage for current sources)
		double vReading = bestReading * (avLow / maxAnalog);

		// Current source measurement is simple, V=IR, solving for R gives R=V/I
		result = vReading / internalCurrentSources[bestRange];
	}

	// Return to home position after measurement made.
	//ContactArm.write(contactHome);
	//delay(contactTime);

	ShiftReg.setAll(srState[0]);
	
	// Return the Ohms value.
	return(result);
}

int getTargetCup(double measurement) {
	// This function checks the measurement against every non-reject cup. If a home is found, that cup number (not index) is returned.
	
	for (int i = 0; i < cupCount; i++) {
		// For each cup, look for a valid home that is not a reject.
		if (Wheel.cups[i].canAccept(measurement) && !Wheel.cups[i].isReject()) {
			int result = i + 1;
			return(result);	// Return that cup.
		}
	}

	for (int i = 0; i < cupCount; i++) {
		// Find the first available reject cup.
		if (Wheel.cups[i].isReject()) {
			int result = i + 1;
			return(result);	// Return that cup.
		}
	}

	sendError("No Reject Cup Found");
	return(-1);
}