# ResistorSortSystem
Resistor Sortation System Codebase and Documentation

## Synopsis

This repository contains all the code for my Senior Project, which was a Resistor Sortation System. The finished project accepts resistors within a certain level of straightness at the input, sends them through a stepper-driven feeder that orients and centers them. Next, a servo presses contacts down onto the resistor leads and the machine steps through a ranging process to find the proper configuration to measure the resistor accurately and the contacts are released. After the resistor value is known, a stepper underneath a wheel with receptacles drives until the proper cup is under the dispensing chute. Then another servo is triggered that opens a swing arm releasing the resistor down the chute and into the receptacle.

## Utility to Others

The code in this project contains several snippets that may be useful to users in the future. This project was put on GitHub to make versioning easier for myself, but ultimately will be left online for this purpose. I expect that others may run into similar issues in the future and will attempt to make the code as well-documented as possible for an outsider perspective.

## Motivation

The initial project concept came from seeing containers in one of the University of Memphis’s Engineering Technology labs filled with a of jumbled pile of misshapen resistors of unknown values and wattage ratings with tangled leads. 

This project presented an interesting and challenging Senior Project opportunity, as well as a way to create something useful for both the University Labs and my own personal electronics lab. 

## Platform

The project consists of four parts.

 1. Scripts that run on a Raspberry Pi which speak with controllers on the mainboard over Serial.
 2. The primary program, which runs on a Teensy 3.2. This drives servos, issues commands for steppers, measures resistors, and contains logic that relates these commands to eachother.
 3. A Program that runs on one ATTiny84A which controls the feed stepper. This controller is slaved to the Teensy over I2C and accepts simple commands.
 4. A program that runs on the other ATTiny84A which controls the sort wheel. This controller is also slaved to the Teensy over I2C.

The Raspberry Pi portion is not yet coded, but will be implemented in either Python or C++.
All microcontroller portions are coded using the Arduino codebase and whatever extra drivers are needed for each microcontroller (for example, Teensyduino for the Teensy 3.2).

## Contributors

Until this project is completed at the end of this semester, no external contributors will be allowed. You can, of course, fork the project, but I cannot offer any reasonable support for the project until it has reached a certain state of completion.

## License

This project has no license at the current time. After completion, I will likely move parts of the project into an Open license of some kind, but that is not yet determined.
