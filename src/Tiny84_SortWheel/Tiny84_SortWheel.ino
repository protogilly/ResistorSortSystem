#define I2C_SLAVE_ADDRESS 0x1

#include <TinyWireS.h>
#include <AccelStepper.h>

#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE (1)
#endif

const int pinStep = 7;
const int pinDir = 8;
const int pinSleep = 9;
const int pinEnable = 10;
const int attTrig0 = 1;
const int backlashSteps = 10;

volatile bool inMotion = false;
volatile bool lastDir = false;

AccelStepper sortWheel(1, pinStep, pinDir);

void setup() {
	// Stepper Enable pin is inverted.
	sortWheel.setPinsInverted(false, false, true);

	// Begin Comms
	TinyWireS.begin(I2C_SLAVE_ADDRESS);
	TinyWireS.onReceive(receiveEvent);

	pinMode(pinSleep, OUTPUT);
	pinMode(attTrig0, OUTPUT);

	// TODO: Implement Sleep mode
	digitalWrite(pinSleep, HIGH);

	sortWheel.setEnablePin(pinEnable);
	sortWheel.setMaxSpeed(90.0);
	sortWheel.setAcceleration(95.0);

	sortWheel.disableOutputs();
}

void loop() {
	if (inMotion) {
		if (sortWheel.distanceToGo() == 0) {
			// If we've reached the end of the last motion command, leave motion and send the trigger to the mainboard.
			inMotion = false;
			digitalWrite(attTrig0, HIGH);
			tws_delay(10);		// 5ms seems like a long time to trigger an interrupt, but this allows a braking force on the sort wheel to complete.
			digitalWrite(attTrig0, LOW);

			// Disable Outputs to save power.
			sortWheel.disableOutputs();
		} else {
			sortWheel.run();
		}
	}
	TinyWireS_stop_check();
}

void receiveEvent(uint8_t howMany) {
	if (howMany < 1 || howMany > TWI_RX_BUFFER_SIZE) {
		return;
	}

	bool thisDir = false;

	uint8_t count = TinyWireS.receive();
	int dirCount = 0;

	// Values over 100 indicate an opposite direction.
	if (count > 100) {
		count = count - 100;
		dirCount = count;
		dirCount = dirCount * -1;
		thisDir = false;
	} else {
		dirCount = count;
		thisDir = true;
	}

	// Number of steps per cup position = 36 degrees / 0.9 degrees per step = 40 steps
	int steps = dirCount * 40;	// 40 steps per cup position

	// If the direction has changed, include some extra steps to take up the backlash
	if (thisDir != lastDir) {
		if (!thisDir) {
			steps = steps - backlashSteps;
		} else {
			steps = steps + backlashSteps;
		}
	}

	// Keep track of the last direction
	lastDir = thisDir;

	// Make the motion
	sortWheel.move(steps);	
	sortWheel.enableOutputs();
	inMotion = true;
}
