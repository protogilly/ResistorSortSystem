#define I2C_SLAVE_ADDRESS 0x1

#include <TinyWireS.h>
#include <AccelStepper.h>

#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE (16)
#endif

const int pinStep = 7;
const int pinDir = 8;
const int pinSleep = 9;
const int pinEnable = 10;
const int attTrig0 = 1;

volatile bool inMotion = false;

AccelStepper sortWheel(1, pinStep, pinDir);

void setup() {
	TinyWireS.begin(I2C_SLAVE_ADDRESS);
	TinyWireS.onReceive(receiveEvent);

	pinMode(pinSleep, OUTPUT);
	pinMode(attTrig0, OUTPUT);

	// TODO: Implement Sleep mode
	digitalWrite(pinSleep, HIGH);

	sortWheel.setEnablePin(pinEnable);
	sortWheel.setMaxSpeed(75.0);
	sortWheel.setAcceleration(100.0);

	sortWheel.disableOutputs();
}

void loop() {
	if (inMotion) {
		if (sortWheel.distanceToGo() == 0) {
			// If we've reached the end of the last motion command, leave motion and send the trigger to the mainboard.
			inMotion = false;
			digitalWrite(attTrig0, HIGH);
			tws_delay(5);
			digitalWrite(attTrig0, LOW);

			// Disable Outputs to save power.
			sortWheel.disableOutputs();
		} else {
			sortWheel.run();
		}
	}
}

void receiveEvent(uint8_t uCount) {
	int count = (int) uCount;	// Conversion to signed int

	// Number of steps per cup position = 36 degrees / 0.9 degrees per step = 40 steps
	sortWheel.move(count * 40);	// 40 steps per cup position
	sortWheel.enableOutputs();
	inMotion = true;
}