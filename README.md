# Overview

This is an example program to get the time from a DS3231 'Real-Time Clock' module using an STM32 chip. It is simply set to blink an LED every 10 "seconds", whatever those are.

It also sets a number of variables and demonstrates read/writing to the EEPROM chips included on the cheap and popular 'ZS-042' DS3231 boards, but those values are not surfaced outside of a debugger.

The included Makefile has options for building the program for an STM32L031K6 or STM32F031K6 chip - there are "Nucleo" boards available with both of those cores. Note that pins B6 and B7 are labeled D5 and D4 respectively, on those boards.

# Building and Uploading

You can build the code with `make`, and upload it with the 'st-flash' utility:

`st-flash write main.bin 0x08000000`
