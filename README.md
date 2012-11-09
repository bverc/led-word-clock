LED Word Clock
==============

A functional clock which displays the time in words in increments of 5 minutes. It uses multiplexing to control a gridded array of LEDs and uses 74HC595 Shift Registers. It is written for the Attiny2313 but could be easily be adapted for any microcontroller. The RTC Clock is the DS1307.

Build Instructions
------------------

A detailed explaination of the design process of hardware selection is available at:
http://no8hacks.com/blog/2012/8/16/led-word-clock

Interactive schematics and BOM are also available at: http://upverter.com/bverc/5d5df4dc0c286f73/LED-Word-Clock/

TWI Driver (I2C)
----------------

The TWI drivers are implemented using drivers written by doctek. His detailed instrucable is available:
http://www.instructables.com/id/I2C_Bus_for_ATtiny_and_ATmega/