/*
 * PCF8574 GPIO Port Expand
 * https://www.mischianti.org/2019/01/02/pcf8574-i2c-digital-i-o-expander-fast-easy-usage/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Renzo Mischianti www.mischianti.org All right reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef PCF8574_h
#define PCF8574_h

#include "Wire.h"

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// Uncomment to enable printing out nice debug messages.
// #define PCF8574_DEBUG

// Uncomment for low memory usage this prevent use of complex DigitalInput structure and free 7byte of memory
// #define PCF8574_LOW_MEMORY

// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

// Setup debug printing macros.
#ifdef PCF8574_DEBUG
	#define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
	#define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
	#define DEBUG_PRINTF(...) { DEBUG_PRINTER.printf(__VA_ARGS__); }
#else
	#define DEBUG_PRINT(...) {}
	#define DEBUG_PRINTLN(...) {}
	#define DEBUG_PRINTF(...) {}
#endif

//#define P0  	B00000001
//#define P1  	B00000010
//#define P2  	B00000100
//#define P3  	B00001000
//#define P4  	B00010000
//#define P5  	B00100000
//#define P6  	B01000000
//#define P7  	B10000000
//
#define P0  	0
#define P1  	1
#define P2  	2
#define P3  	3
#define P4  	4
#define P5  	5
#define P6  	6
#define P7  	7

#include <math.h>


class PCF8574 {
public:

	PCF8574(uint8_t address);
	PCF8574(uint8_t address, uint8_t interruptPin,  void (*interruptFunction)() );

#if !defined(__AVR) && !defined(__STM32F1__) && !defined(TEENSYDUINO)
	PCF8574(uint8_t address, uint8_t sda, uint8_t scl);
	PCF8574(uint8_t address, uint8_t sda, uint8_t scl, uint8_t interruptPin,  void (*interruptFunction)());
#endif

#ifdef ESP32
	///// changes for second i2c bus
	PCF8574(TwoWire *pWire, uint8_t address);
	PCF8574(TwoWire *pWire, uint8_t address, uint8_t sda, uint8_t scl);

	PCF8574(TwoWire *pWire, uint8_t address, uint8_t interruptPin,  void (*interruptFunction)() );
	PCF8574(TwoWire *pWire, uint8_t address, uint8_t sda, uint8_t scl, uint8_t interruptPin,  void (*interruptFunction)());
#endif

	void begin();
	void pinMode(uint8_t pin, uint8_t mode);

	uint8_t digitalRead(uint8_t pin);
	#ifndef PCF8574_LOW_MEMORY
	
	/* Set default state of struct to be 255 so that if there 
	   are errors getting data from the chip, we can detect that 
	   values aren't valid */
	typedef struct pinstates {
		uint8_t p0 = 255;
		uint8_t p1 = 255;
		uint8_t p2 = 255;
		uint8_t p3 = 255;
		uint8_t p4 = 255;
		uint8_t p5 = 255;
		uint8_t p6 = 255;
		uint8_t p7 = 255;
	} pinstate_t;


		pinstate_t digitalReadAll(void);
	#else
		byte digitalReadAll(void);
	#endif
	void digitalWrite(uint8_t pin, uint8_t value);

	/* Returns true if the chip is contactable. */
	bool isPresent();
	
private:
	/* NXP's documentation of this chip (https://www.nxp.com/docs/en/data-sheet/PCF8574_PCF8574A.pdf p8)
	 * states that "To enter the Read mode the master (microcontroller) addresses the slave
	 * device and sets the last bit of the address byte to logic 1 (address byte read)."  
	 * 
	 * We should then ensure that whenever we wish to read from the chip, we must use the 
	 * address "_addr | 0x80" to ensure we set MSB. If we don't, the chip will assume we are 
     * setting all the inputs to outputs. */
	byte _address;

	/* The chip always starts with all pins in input - i.e., the register is all 1's. NXP's documentation
     * (https://www.nxp.com/docs/en/data-sheet/PCF8574_PCF8574A.pdf) states that "Ensure a logic 1 is 
	 * written for any port that is being used as an input to ensure the strong external pull-down is 
	 * turned off." Therefore, start with _inputmask as being all 1's, and as we set pins to output, we 
	 * set the specified bit in _inputmask to 0, and push _inputmask to the chip. */
	byte _inputmask = 0xFF;
	
	#if defined(__AVR) || defined(__STM32F1__)
		uint8_t _sda;
		uint8_t _scl;
	#else
		uint8_t _sda = SDA;
		uint8_t _scl = SCL;
	#endif

	TwoWire *_wire;

	bool _usingInterrupt = false;
	uint8_t _interruptPin = 2;
	void (*_interruptFunction)(){};

	/* Indicates whether or not we've discovered the chip on the i2c bus */
	bool _isPresent = false;
	
	/* Internal function to push _inputmask to the chip */
	bool _setInputMask(); 
	
	/* Internal functions to help ensure we don't send the write address byte 
	 * when we wish to read. */
	void _beginReadTransmission();
	void _beginWriteTransmission();
	uint8_t _requestBytes(uint8_t bytes);
	
	/* Reads the 1-byte state register of the chip with one i2c call. */
	byte _readAllAsByte();
	

	/* Abstraction to test if the pin number provided is valid for the chip */
	bool _isValidPin(uint8_t pin);

};

#endif
