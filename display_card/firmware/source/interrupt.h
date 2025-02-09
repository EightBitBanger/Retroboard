// Card select interrupt handler
#define _CONTROL_IN__     PINB
#define _SIGNAL_READ__    1
#define _SIGNAL_WRITE__   0

#define _ADDRESS_IN__     PINC
#define _BUS_DIR__        PORTB
#define _BUS_OUT__        PORTA
#define _BUS_IN__         PINA
#define _BUS_DDR__        DDRA

void shiftFrameUp(void);


ISR (INT2_vect) {
	
    // READ request
	if ((_CONTROL_IN__ & (1 << _SIGNAL_READ__)) != (1 << _SIGNAL_READ__)) {
		
		_BUS_DIR__ = 0b00001000; // Output on data bus
		
		_BUS_DDR__ = 0xff;
		
		if (_ADDRESS_IN__ < 0x0a) {
            
            _BUS_OUT__ = deviceROM[_ADDRESS_IN__];
            
		} else {
            
            _BUS_OUT__ = char_rom[_ADDRESS_IN__];
        }
        
		while ((_CONTROL_IN__ & (1 << _SIGNAL_READ__)) != (1 << _SIGNAL_READ__)) {
			__asm__("nop");
		}
		
		_BUS_DIR__ = 0b00011000; // Disable the bus
		
		return;
	}
	
	// WRITE request
	if ((_CONTROL_IN__ & (1 << _SIGNAL_WRITE__)) != (1 << _SIGNAL_WRITE__)) {
		
		_BUS_DIR__ = 0b00000000; // Input from the bus
		
		_BUS_DDR__ = 0x00;
		
		// Write the byte to device RAM
		if (_ADDRESS_IN__ >= 0x0a) {
            
            uint32_t address_offset = frameBuffer.registery[2] + (frameBuffer.registery[1] * 128);
            
            // VRAM or CHAR_RAM
            if (frameBuffer.registery[7] == 0) {
                
                frameBuffer.registery[_ADDRESS_IN__ + address_offset] = _BUS_IN__;
                
            } else {
                
                char_rom[ (_ADDRESS_IN__ - 0x0a) + (frameBuffer.registery[2] * 6) ] = _BUS_IN__;
            }
            
		} else {
            
            frameBuffer.registery[_ADDRESS_IN__] = _BUS_IN__;
            
            if (frameBuffer.registery[5] != 0) 
                shiftFrameUp();
            
        }
		
		_BUS_DDR__ = 0xff;
		
		_BUS_DIR__ = 0b00011000; // Disable the bus
		
		return;
	}
	
	return;
}

