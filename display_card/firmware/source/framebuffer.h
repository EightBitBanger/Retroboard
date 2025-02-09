//
// Frame buffer data structure

struct FrameBuffer {
	
	// Frame buffer
	volatile uint8_t Buffer[0x9f];    // 0x00 - 0x9f
	
	// Cursor control
	volatile uint8_t cursorLine;      // 0xa0
	volatile uint8_t cursorPos;       // 0xa1
	volatile uint8_t cursorCharacter; // 0xa2
	// Blink rate
	volatile uint8_t cursorBlinkTime; // 0xa3
	// Frame redraw rate
	volatile uint8_t frameRate;       // 0xa4
	// Register flags
	volatile uint8_t clearBufferFlag; // 0xa5
	volatile uint8_t clearMaskFlag;   // 0xa6
	volatile uint8_t clearLineFlag;   // 0xa7
	volatile uint8_t uploadCGramFlag; // 0xa8
	// Shift flags
	volatile uint8_t shiftFrameUp;    // 0xa9
	volatile uint8_t shiftFrameDown;  // 0xaa
	
	volatile uint8_t PADDING3; // 0xab
	volatile uint8_t PADDING4; // 0xac
	volatile uint8_t PADDING5; // 0xad
	volatile uint8_t PADDING6; // 0xae
	volatile uint8_t PADDING7; // 0xaf
	volatile uint8_t PADDING8; // 0xb0
	volatile uint8_t PADDING9; // 0xb1
	volatile uint8_t PADDINGA; // 0xb2
	volatile uint8_t PADDINGB; // 0xb3
	
	// Cursor blink internal
	volatile uint8_t cursorBlinkState;
	volatile uint8_t cursorBlinkCounter;
	// Frame counter internal
	volatile uint16_t frameCounter;
	volatile uint16_t baseCounter;
	
} frameBuffer;

