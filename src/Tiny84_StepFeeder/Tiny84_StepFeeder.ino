/*
	Tiny84_StepFeeder.ino - StepFeeder controller for the Resistor Sortation Project 
	Created by Shawn Westcott (www.8tinybits.com), Feb 2017.

	Schematics, Gerbers, and related source code can be found on this project's Github
	https://github.com/slwstctt/ResistorSortSystem

	This code is currently unlicensed and is only available for educational purposes.
	If you'd like to use this code for a non-educational purpose, please contact Shawn
	Westcott (shawn.westcott@8tinybits.com).
*/

#define I2C_SLAVE_ADDRESS 0x2

#include <TinyWireS.h>
#include <AccelStepper.h>

#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE (1)
#endif

// Pin Definitions
const int pinStep = 7;
const int pinDir = 8;
const int pinSleep = 9;
const int pinEnable = 10;
const int attTrig3 = 2;

// volatiles for I2C control
volatile bool hasSetup = false;	// Whether or not the controller has recieved a setup command
volatile bool inMotion = false;	// Whether or not a motion is in progress.
volatile uint8_t fwdSteps = 0;	// Number of steps for a step feed motion.
volatile uint8_t cycleCount = 0;	// Number of full step cycles to execute.

AccelStepper stepFeeder(1, pinStep, pinDir);

void setup() {
	// Start I2C comms.
	TinyWireS.begin(I2C_SLAVE_ADDRESS);
	TinyWireS.onReceive(receiveEvent);

	// Setup Output pins
	pinMode(pinSleep, OUTPUT);
	pinMode(attTrig3, OUTPUT);

	// TODO: Implement Sleep mode
	digitalWrite(pinSleep, HIGH);

	// Setup stepper settings.
	stepFeeder.setEnablePin(pinEnable);
	stepFeeder.setMaxSpeed(75.0);
	stepFeeder.setAcceleration(100.0);

	// Turn off the stepper motor on startup to save energy.
	stepFeeder.disableOutputs();
}

void loop() {
	if (inMotion) {
		if (cycleCount > 0) {
			// If there are cycles left to execute, do so.
			stepCycle();
			cycleCount--;
		} else {
			// Otherwise, turn off the flag and send a trigger to the master
			inMotion = false;
			digitalWrite(attTrig3, HIGH);
			tws_delay(5);
			digitalWrite(attTrig3, LOW);

			// Turn off the stepper to save energy between actions.
			stepFeeder.disableOutputs();
		}
	}

	// Continual check for new I2C commands.
	TinyWireS_stop_check();
}

void receiveEvent(uint8_t howMany) {
	if (howMany < 1 || howMany > TWI_RX_BUFFER_SIZE) {
		return;
	}

	uint8_t uCount = TinyWireS.receive();

	if (!hasSetup) {
		// If we haven't recieved a setup yet, this must be it. Set the number of steps per feed action.
		fwdSteps = uCount;
		hasSetup = true;
	} else {
		// Otherwise, this must be a command to move a certain number of cycles. Turn on the stepper motor and start the motion.
		cycleCount = uCount;
		stepFeeder.enableOutputs();
		inMotion = true;
	}
}

void stepCycle() {
	// Executes one step cycle.

	stepFeeder.move(fwdSteps);

	// Note: runToPosition is a blocking command, and is only appropriate here because the master waits on a trigger to send each command.
	stepFeeder.runToPosition();

	tws_delay(5);		// Delays exist here to allow a braking effect between actions.

	// A step cycle contains a forward and a reverse motion. The reverse motion is just the opposite of a forward motion.
	stepFeeder.move(-fwdSteps);
	stepFeeder.runToPosition();

	tws_delay(5);
}