# This library is a rewrite of xreef's and is under slow development. Documentation and examples may be inconsistent.
This library was rewritten and not committed as a fork of xreef's due to quite significant code changes, the most significant is due to how the chip handles inputs and outputs and how code should reflect that. xreef's library works perfectly when trying to write to pins of the PCF8574 chip, and also to monitor interrupts on any pins, but xreef's library is unable to read the state of individual pins reliably due to the fundamental way in which it was written. This is explained best by a snippet of comments from the code in this libary's header:

> NXP's documentation of this chip (https://www.nxp.com/docs/en/data-sheet/PCF8574_PCF8574A.pdf p8)
> states that "To enter the Read mode the master (microcontroller) addresses the slave
> device and sets the last bit of the address byte to logic 1 (address byte read)."  
>
> We should then ensure that whenever we wish to read from the chip, we must use the 
> address "_addr | 0x80" to ensure we set MSB. If we don't, the chip will assume we are 
> setting all the inputs to outputs. 
> 
> The chip always starts with all pins in input - i.e., the register is all 1's. NXP's documentation
> (https://www.nxp.com/docs/en/data-sheet/PCF8574_PCF8574A.pdf) states that "Ensure a logic 1 is 
> written for any port that is being used as an input to ensure the strong external pull-down is 
> turned off." Therefore, start with _inputmask as being all 1's, and as we set pins to output, we 
> set the specified bit in _inputmask to 0, and push _inputmask to the chip. 

Reef's original library is here and deserves full credit for setting the foundation for this library. https://github.com/xreef/PCF8574_library. This library should function fine as a drop-in replacement for xreef's but is still under slow development; please refer to xreef's library page for documentation.
