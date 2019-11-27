/*
 * PCF8574 GPIO Port Expander Library
 * Copyright (c) 2020 Matt Bradford <arduino@mattbradford.com> All Rights Reserved.
 * Copyright (c) 2017 Renzo Mischianti www.mischianti.org All right reserved.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Renzo Mischianti www.mischianti.org All right reserved.
 * Portions Copyright (c) 2019 Matt Bradford arduino@mattbradford.com. All Rights Reserved.
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

#include "PCF8574.h"
#include "Wire.h"

/**
 * Constructor
 * @param address: i2c address
 */
PCF8574::PCF8574(uint8_t address){
	_wire = &Wire;

	_address = address;
};

/**
 * Construcor
 * @param address: i2c address
 * @param interruptPin: pin to set interrupt
 * @param interruptFunction: function to call when interrupt raised
 */
PCF8574::PCF8574(uint8_t address, uint8_t interruptPin,  void (*interruptFunction)() ){
	_wire = &Wire;

	_address = address;
	_interruptPin = interruptPin;
	_interruptFunction = interruptFunction;
	_usingInterrupt = true;
};

#if !defined(__AVR) && !defined(__STM32F1__) && !defined(TEENSYDUINO)
	/**
	 * Constructor
	 * @param address: i2c address
	 * @param sda: sda pin
	 * @param scl: scl pin
	 */
	PCF8574::PCF8574(uint8_t address, uint8_t sda, uint8_t scl){
		_wire = &Wire;

		_address = address;
		_sda = sda;
		_scl = scl;
	};

	/**
	 * Constructor
	 * @param address: i2c address
	 * @param sda: sda pin
	 * @param scl: scl pin
	 * @param interruptPin: pin to set interrupt
 	 * @param interruptFunction: function to call when interrupt raised
	 */
	PCF8574::PCF8574(uint8_t address, uint8_t sda, uint8_t scl, uint8_t interruptPin,  void (*interruptFunction)() ){
		_wire = &Wire;

		_address = address;
		_sda = sda;
		_scl = scl;

		_interruptPin = interruptPin;
		_interruptFunction = interruptFunction;

		_usingInterrupt = true;
	};
#endif

#ifdef ESP32
	/**
	 * Constructor
	 * @param address: i2c address
	 */
	PCF8574::PCF8574(TwoWire *pWire, uint8_t address){
		_wire = pWire;

		_address = address;
	};

	/**
	 * Construcor
	 * @param address: i2c address
	 * @param interruptPin: pin to set interrupt
	 * @param interruptFunction: function to call when interrupt raised
	 */
	PCF8574::PCF8574(TwoWire *pWire, uint8_t address, uint8_t interruptPin,  void (*interruptFunction)() ){
		_wire = pWire;

		_address = address;
		_interruptPin = interruptPin;
		_interruptFunction = interruptFunction;
		_usingInterrupt = true;
	};

	/**
	 * Constructor
	 * @param address: i2c address
	 * @param sda: sda pin
	 * @param scl: scl pin
	 */
	PCF8574::PCF8574(TwoWire *pWire, uint8_t address, uint8_t sda, uint8_t scl){
		_wire = pWire;

		_address = address;
		_sda = sda;
		_scl = scl;
	};

	/**
	 * Constructor
	 * @param address: i2c address
	 * @param sda: sda pin
	 * @param scl: scl pin
	 * @param interruptPin: pin to set interrupt
	 * @param interruptFunction: function to call when interrupt raised
	 */
	PCF8574::PCF8574(TwoWire *pWire, uint8_t address, uint8_t sda, uint8_t scl, uint8_t interruptPin,  void (*interruptFunction)() ){
		_wire = pWire;

		_address = address;
		_sda = sda;
		_scl = scl;

		_interruptPin = interruptPin;
		_interruptFunction = interruptFunction;

		_usingInterrupt = true;
	};
#endif

/**
 * Start the library and check that the chip is reachable. Also sets pins to 
 * default as per power on, as per documentation.
 */
void PCF8574::begin(){
	#if !defined(__AVR) && !defined(__STM32F1__) && !defined(TEENSYDUINO)
		_wire->begin(_sda, _scl);
	#else
		_wire->begin();
	#endif

	// Check we can contact the device.
	if (!isPresent()) {
		DEBUG_PRINTF("Couldn't contact PCF8574 chip at 0x%X! Aborting.",_address);
		return;
	}
	
	// We shouldn't assume the chip is in a clean state. Send through default pins.
	_setInputMask();
	
	// Setup interrupt handler if we're using it.
	if (_usingInterrupt){
		DEBUG_PRINTF("Attaching interrupt to pin %i; Pin is being set to INPUT_PULLUP.\n",_interruptPin);
		::pinMode(_interruptPin, INPUT_PULLUP);
		attachInterrupt(digitalPinToInterrupt(_interruptPin), (*_interruptFunction), FALLING);
	}

}

/**
 * Tests the i2c bus at _addr to see if the chip responds. 
 * Updates the internal _isPresent flag, and returns the same result.
 * @return true if an i2c device responded on the bus, false otherwise.
 */
bool PCF8574::isPresent() {
	// Check to see we can reach the chip. 
	_beginReadTransmission();
	_isPresent=(_wire->endTransmission()==0);
	return _isPresent;
}

/**
 * Sets the mode of a single pin to INPUT or OUTPUT. According to the documentation
 * for this chip (http://www.ti.com/lit/ds/symlink/pcf8574.pdf), a pin in INPUT mode
 * is held high by a 100 µA current source, so INPUT is always in effect INPUT_PULLUP.
 * 
 * @param pin: The pin to set 
 * @param mode: The mode to set the pin to (INPUT or OUTPUT)
 */
void PCF8574::pinMode(uint8_t pin, uint8_t mode){
	if (!_isValidPin(pin)) return;
	
	byte currmask = _inputmask; //Use this to check if there's any change.
	switch (mode) {
		case OUTPUT: 
			bitClear(_inputmask,pin); break;
		case INPUT:
		case INPUT_PULLUP: 
			bitSet(_inputmask,pin); break;
		default: 
			DEBUG_PRINTF("Invalid Mode: %i. Mode must be OUTPUT, INPUT, or INPUT_PULLUP.",mode);
			return;
	}
	if (_inputmask == currmask) {
		DEBUG_PRINTF("No change to _inputmask; not pushing the mask to the chip.");
		return;
	}
	
	DEBUG_PRINTF("Setting pin %i as mode %i.\n",pin,mode);
	_setInputMask();

};


#ifndef PCF8574_LOW_MEMORY
/**
 * Read value of all eight pins
 * @return a struct representing the on/off state for all eight pins.
 */
PCF8574::pinstate_t PCF8574::digitalReadAll(void) {
	pinstate_t states;
	if (!_isPresent) {
		DEBUG_PRINTLN("Chip is not present, or begin() has not been called. ");
		return states;
	}
	_requestBytes(1); // Request 1 byte - gives us input state of all pins.
	if(_wire->available()) {
		byte pstates = _wire->read();
		DEBUG_PRINTF("Received input states from chip: B");
		DEBUG_PRINTLN(pstates,BIN);
		// Yes, HIGH & LOW are 1 and 0, but just to be sure.
		states.p0 = (bitRead(pstates,0)==1)?HIGH:LOW;
		states.p1 = (bitRead(pstates,1)==1)?HIGH:LOW;
		states.p2 = (bitRead(pstates,2)==1)?HIGH:LOW;
		states.p3 = (bitRead(pstates,3)==1)?HIGH:LOW;
		states.p4 = (bitRead(pstates,4)==1)?HIGH:LOW;
		states.p5 = (bitRead(pstates,5)==1)?HIGH:LOW;
		states.p6 = (bitRead(pstates,6)==1)?HIGH:LOW;
		states.p7 = (bitRead(pstates,7)==1)?HIGH:LOW;	
	} else {
		DEBUG_PRINTLN("Request to read chip states failed. Returning empty states.");
	}
	return states;
};
#else
/**
 * Read value of all pins in byte format for low memory usage.
 * Just a wrapper for the internal function _readAllAsByte();
 * @return a single byte representing the on/off state for all eight pins.
 */
byte PCF8574::digitalReadAll(void){
	return _readAllAsByte();
};
#endif


/**
 * Read value of the specified pin. 
 * @param pin: the pin to query.
 * @return the state (HIGH or LOW) of the queried pin, or 255 if invalid pin. 
 */
uint8_t PCF8574::digitalRead(uint8_t pin) {
	if (!_isValidPin(pin)) return 255;
	byte pstates = _readAllAsByte();
	return (bitRead(pstates,pin)==1)?HIGH:LOW;
};

/**
 * Set output of pin. With this chip, it is essentially a call to pinMode().
 * @param pin: The pin to set the output for.
 * @param value: the value to set on the pin, HIGH or LOW.
 */
void PCF8574::digitalWrite(uint8_t pin, uint8_t value){
	if (!_isValidPin(pin)) return;
	if (!_isPresent) {
		DEBUG_PRINTLN("Chip is not present, or begin() has not been called. ");
		return;
	}
	
	switch (value) {
		case HIGH: pinMode(pin, OUTPUT); break;
		case LOW: pinMode(pin, INPUT); break;
		default: 
			DEBUG_PRINTF("Value %i passed to digitalWrite is invalid. Function expects HIGH or LOW.\n",value);
			return;
	}

	_wire->endTransmission();
};

/****************************** PRIVATE *********************************************/

/** Wrapper for _wire->beginTransmission(..), ensuring we set the MSB to 1
 *  to ensure we send a read request. 
 */
void PCF8574::_beginReadTransmission() {
	_wire->beginTransmission(_address | 0x80);
}

/** Wrapper for _wire->beginTransmission(..), ensuring we set the MSB to 0
 *  to ensure we send a write request. 
 */
void PCF8574::_beginWriteTransmission() {
	_wire->beginTransmission(_address & 0x7F);
}

/** Wrapper for _wire->requestFrom(..), ensuring we set the MSB to 1
 *  to ensure we send a read request. 
 * @param bytes: The number of bytes to read.
 * @return The number of bytes read from the device.
 */
uint8_t PCF8574::_requestBytes(uint8_t bytes) {
	return _wire->requestFrom((uint8_t)(_address | 0x80), (uint8_t)bytes);
}

/** 
 * writes the current _inputmask variable to the chip. 
 * @return TRUE if the device responded with ACK, FALSE otherwise.
 */
bool PCF8574::_setInputMask() {
	DEBUG_PRINT("Sending current _inputmask to chip: ");
	DEBUG_PRINTLN(_inputmask,BIN);

	_beginWriteTransmission();
	_wire->write(_inputmask); 
	return (_wire->endTransmission()==0);
}

/**
 * Read value of all pins in byte format - for use with higher level functions.
 * @return a single byte representing the on/off state for all eight pins.
 */
byte PCF8574::_readAllAsByte() {
	if (!_isPresent) {
		DEBUG_PRINTLN("Chip is not present, or begin() has not been called. ");
		return 0;
	}
	_requestBytes(1);// Request 1 byte - gives us input state of all pins.
	if(_wire->available()) {
		byte pstates = _wire->read();
		DEBUG_PRINTF("Received input states from chip: B");
		DEBUG_PRINTLN(pstates,BIN);
		
		return pstates;
	}
	DEBUG_PRINTLN("Request to read chip states failed. Returning zero.");
	return 0;
};

/**
 * Abstracts away debug code to check if a pin number is in range.
 * @return true if the pin number passed is a valid pin for the chip, false otherwise.
 */
bool PCF8574::_isValidPin(uint8_t pin) {
	if (pin < 0 || pin > 7) {
		DEBUG_PRINTF("Invalid Pin: %i. Pin must be between 0 and 7 inclusive.\n",pin);
		return false;
	}	
	return true;
}
