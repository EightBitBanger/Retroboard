#include "main.h"
#include "char_rom.h"

#include "bitwise.h"

volatile uint8_t deviceROM[] = {0x10, 'd','i','s','p','l','a','y',' ',' ',' ', 
                       '1','2','8','x','6','4' };

#define clearInterruptFlag   __asm__("cli")
#define setInterruptFlag     __asm__("sei")

struct FrameBuffer {
    
    // 0 Video mode
    // 1 Cursor line
    // 2 Cursor position
    // 3 Cursor blink rate
    // 4 Cursor character
    // 5 Shift screen up
    // 6 Invert display output
    // 7 Write to VRAM=0 or CHAR_RAM=1
    // 8 
    // 9 
    
    volatile uint8_t registery[0x0a];
    
    uint8_t vram[1024];
    
} frameBuffer;

#include "interrupt.h"

uint8_t cursor_line     = 0;
uint8_t cursor_position = 0;


void SetPosition(uint8_t position) {
    PORTA = 0b01000000 + position;
    PORTD = 0b00011001;
    PORTD = 0b00011000;
    _delay_us(10);
    PORTD = 0b00000001;
    return;
}

void SetLine(uint8_t line) {
    PORTA = 0b10111000 + line;
    PORTD = 0b00011001;
    PORTD = 0b00011000;
    _delay_us(10);
    PORTD = 0b00000001;
    return;
}





void ClearScreen(void) {
    
    for (uint8_t l=0; l < 8; l++) {
        
        SetLine(l);
        SetPosition(0);
        
        for (uint8_t a=0; a < 64; a++) {

            PORTA = 0x00;
            
            PORTD = 0b00011101;
            PORTD = 0b00011100;
            _delay_us(10);
            PORTD = 0b00000101;
            
        }
        
    }
    
    SetLine(0);
    SetPosition(0);
    
    return;
}



void drawGlyph(uint8_t* glyph, uint16_t width, uint16_t char_offset) {
    
    for (uint16_t i=0; i < width; i++) {
        
        uint8_t glyphSlice = glyph[i + (width * char_offset)];
        
        // Invert output bits
        if (frameBuffer.registery[6] == 1) 
            glyphSlice = InvertBits( glyphSlice );
        
        PORTA = glyphSlice;
        
        // Display half selects
        if (cursor_position > 63) {
            
            PORTD = 0b00001101;
            PORTD = 0b00001100;
            _delay_us(10);
            PORTD = 0b00000101;
            
        } else {
            
            PORTD = 0b00010101;
            PORTD = 0b00010100;
            _delay_us(10);
            PORTD = 0b00000101;
            
        }
        
        cursor_position++;
    }
    
    return;
}


void drawVRAM(uint8_t* glyph, uint16_t width, uint16_t char_offset) {
    
    for (uint16_t i=0; i < width; i++) {
        
        uint8_t glyphSlice = glyph[i + (width * char_offset)];
        
        //ReorientBits();
        
        PORTA = glyphSlice;
        
        // Display half selects
        if (cursor_position > 63) {
            
            PORTD = 0b00001101;
            PORTD = 0b00001100;
            _delay_us(10);
            PORTD = 0b00000101;
            
        } else {
            
            PORTD = 0b00010101;
            PORTD = 0b00010100;
            _delay_us(10);
            PORTD = 0b00000101;
            
        }
        
        cursor_position++;
    }
    
    return;
}



void shiftFrameUp(void) {
    
    // Check shift frame
    if (frameBuffer.registery[5] != 0) {
        frameBuffer.registery[5] = 0;
        
        for (uint32_t l=0; l < 7; l++) {
            
            for (uint32_t c=0; c < 21; c++) 
                frameBuffer.vram[c + (l * 128)] = frameBuffer.vram[c + ((l+1) * 128)];
            
        }
        
        // Clear last line
        for (uint32_t c=0; c < 21; c++) 
            frameBuffer.vram[c + (7 * 128)] = ' ';
        
    }
}


int main(void) {
    
    // Data bus
    DDRA = 0xff;
    PORTA = 0x00;
    
    // Address bus
    DDRC = 0x00;
    PORTC = 0x00;
    
    // 0 - Write
    // 1 - Read
    // 2 - Interrupt select
    // 3 - Data bus direction
    // 4 - Data bus enable
    
    DDRB  = 0b00011000;
    PORTB = 0b00011000;
    
    // 0 - Display Enable (Active low)
    // 1 - 1=Read/0=write
    // 2 - Register select 1=data 0=instruction
    // 3 - CS2
    // 4 - CS1
    
    DDRD  = 0b00011111;
    PORTD = 0b00000001;
    
    // Allow display some time to initiate
    _delay_ms(100);
    
    
    // Initiate video ram
    for (uint16_t i=0; i < 1024; i++) 
        frameBuffer.vram[i] = ' ';
    
    frameBuffer.registery[0] = 0x00;
    frameBuffer.registery[1] = 0x00;
    frameBuffer.registery[2] = 0x00;
    frameBuffer.registery[3] = 50;
    frameBuffer.registery[4] = '_';
    frameBuffer.registery[5] = 0;
    frameBuffer.registery[6] = 0;
    frameBuffer.registery[7] = 0;
    
    
	// Initiate the display
    PORTA = 0b00111111;
    
    PORTD = 0b00011001;
    PORTD = 0b00011000;
    _delay_us(100);
    PORTD = 0b00000001;
    
    ClearScreen();
    
    // Set hardware interrupt
	EICRA = 0b00100000; // Enable INT2 falling edge
	EIMSK = 0b00000100; // Enable INT2 mask
	
	setInterruptFlag;
    
	uint16_t cursorCounter = 0;
	uint8_t  cursorState = 0;
	
	while(1) {
        
        //
        // Text mode
        if (frameBuffer.registery[0] == 0) {
            
            for (uint32_t l=0; l < 8; l++) {
                
                SetLine(l);
                SetPosition(0);
                
                cursor_line = l;
                cursor_position = 0;
                
                for (uint32_t c=0; c <= 21; c++) {
                    
                    // Fill blank space
                    if (c == 21) {
                        uint8_t blankSpace[] = {0x00,0x00};
                        
                        uint8_t numberOfBlankSpaces = 2;
                        
                        drawGlyph(blankSpace, numberOfBlankSpaces, 0);
                        
                        cursor_position -= numberOfBlankSpaces;
                        
                        break;
                    }
                    
                    uint8_t glyph = frameBuffer.vram[c + (l * 128)];
                    
                    if (glyph > 0xd0) 
                        glyph = 0;
                    
                    drawGlyph(char_rom, 6, glyph);
                    
                    
                }
                
                
                //
                // Cursor
                
                if (frameBuffer.registery[3] > 0) {
                    uint16_t cursorMax = (uint16_t)frameBuffer.registery[3];
                    
                    // Blink timer
                    cursorCounter++;
                    if (cursorCounter > cursorMax) {
                        
                        cursorCounter=0;
                        
                        if (cursorState == 1) {
                            cursorState = 0;
                            
                        } else {
                            cursorState = 1;
                        }
                        
                    }
                    
                    // Draw cursor
                    if (cursorState == 1) {
                        
                        SetLine( frameBuffer.registery[1] );
                        
                        cursor_position = (frameBuffer.registery[2] * 6);
                        if (cursor_position > 63) {
                            
                            SetPosition( (frameBuffer.registery[2] * 6) - 64 );
                            
                        } else {
                            
                            SetPosition( (frameBuffer.registery[2] * 6) );
                            
                        }
                        
                        if (frameBuffer.registery[2] == 10) {
                            
                            drawGlyph(char_rom_special, 6, 0);
                            
                        } else {
                            
                            drawGlyph(char_rom, 6, frameBuffer.registery[4]);
                            
                        }
                        
                    }
                    
                }
                
                continue;
            }
            
        }
        
        
        //
        // Graphics mode
        
        if (frameBuffer.registery[0] == 1) {
            
            
            for (uint32_t l=0; l < 8; l++) {
                
                SetLine(l);
                SetPosition(0);
                
                cursor_position = 0;
                
                drawVRAM(frameBuffer.vram, 128, l);
                
                continue;
            }
            
        }
        
        _delay_ms(30);
        
    }
    
    
    
    
    while(1) 
        _delay_ms(1);
    
    return 0;
}

