// LIQUIDCRYSTAL_I2C_PCF8574 V1.0

// Foreword: To successfully use this library you will to ensure the following.
// 1. Your PCF8574 LCD adaptor board is mapped either the same as outlined below or you have correctly re-assigned the pins for your board. I used the following;
//    http://www.ebay.co.uk/itm/1Pcs-IIC-I2C-Serial-Interface-Board-Module-For-Arduino-LCD1602-LCD2004-Display-/281366957696?pt=LH_DefaultDomain_3&hash=item4182c70e80
// 2. You have to implement a simple delay routine capable of delaying in uSecs if you choose not to use the one provided. Which uses Timer 1 in 16bit mode.
// 3. If you change the dimensions of the LCD you adapt the initialisation to suit. This code was written for a 4 by 20.
// 4. You use the PIC18 Peripheral Library for I2C control. If not you will need to adapt your own routines for;
//    I2C_Write_Byte_Single_Reg();
//    I2C_Read_Byte_Single_Reg();
// Anywhere you may typically need to make changes have been tagged with 'TODO Adapt'
// Note : It was written and tested using the PIC18F2685. MPLABX v2.15, Compiler XC8 v1.35 and ICD 3.
//        Some of the inspiration for the library was taken from the Arduino LiquidCrystal library and adapted for the XC8 Compiler
//        See here : http://hmario.home.xs4all.nl/arduino/LiquidCrystal_I2C/
// Data sheets and useful information can be found here;
// LCD        : https://www.sparkfun.com/datasheets/LCD/HD44780.pdf
// PCF8574    : http://www.nxp.com/documents/data_sheet/PCF8574.pdf
// I2C        : http://ww1.microchip.com/downloads/en/AppNotes/00735a.pdf
//              http://ww1.microchip.com/downloads/en/DeviceDoc/i2c.pdf
//              http://www.nxp.com/documents/application_note/AN10216.pdf
// I2C Lib    : C:\Program Files (x86)\Microchip\xc8\v1.35\sources\pic18\plib\i2c
// PIC18F2685 : http://www.microchip.com/wwwproducts/Devices.aspx?product=PIC18F2685

#include "LCD_I2C_PCF8574.h"
#include "sapi.h"

#define LCD_CLEAR_DISPLAY           0x01    // Mode : Clears display

#define LCD_RETURN_HOME             0x02    // Mode : Returns cursor to home posn.

// Entry Mode Set
#define LCD_ENTRY_MODE_SET          0x04    // Mode : Entry Mode Set, Sets the cursor move dir and specs whether or not to shift the display
#define LCD_INCREMENT               0x02        // Sub Mode of ENTRY_MODE_SET : Increment DDRAM (I/D), Entry Left
#define LCD_DECREMENT               0x00        // Sub Mode of ENTRY_MODE_SET : Decrement DDRAM (I/D), Entry Right
#define LCD_SHIFT_ON                0x01        // Sub Mode of ENTRY_MODE_SET : Shift On  (S), Shift Display when byte written. Display Shift
#define LCD_SHIFT_OFF               0x00        // Sub Mode of ENTRY_MODE_SET : Shift Off (S), Don't shift display when byte written. Cursor Move

// Display Function
#define LCD_DISPLAY_ON_OFF          0x08    // Mode : Display On/Off, Sets on/off of all display, Cursor on/off, Cursor Blink on/off
#define LCD_DISPLAY_ON              0x04        // Sub Mode of DISPLAY_ON_OFF : Puts display on  (D)
#define LCD_DISPLAY_OFF             0x00        // Sub Mode of DISPLAY_ON_OFF : Puts display off (D)
#define LCD_CURSOR_ON               0x02        // Sub Mode of DISPLAY_ON_OFF : Puts cursor on   (C)
#define LCD_CURSOR_OFF              0x00        // Sub Mode of DISPLAY_ON_OFF : Puts cursor off  (C)
#define LCD_BLINKING_ON             0x01        // Sub Mode of DISPLAY_ON_OFF : Blinking cursor  (B)
#define LCD_BLINKING_OFF            0x00        // Sub Mode of DISPLAY_ON_OFF : Solid cursor     (B)

// Display Control
#define LCD_MV_CUR_SHIFT_DISPLAY    0x10    // Mode : Move the cursor and shifts the display
#define LCD_DISPLAY_SHIFT           0x08        // Sub Mode of CURSOR_SHFT_DIS : Display shifts after char print   (SC)
#define LCD_CURSOR_SHIFT            0x00        // Sub Mode of CURSOR_SHFT_DIS : Cursor shifts after char print    (SC)
#define LCD_SHIFT_RIGHT             0x04        // Sub Mode of CURSOR_SHFT_DIS : Cursor or Display shifts to right (RL)
#define LCD_SHIFT_LEFT              0x00        // Sub Mode of CURSOR_SHFT_DIS : Cursor or Display shifts to left  (RL)

// Function Set
#define LCD_FUNCTION_SET            0x20    // Mode : Set the type of interface that the display will use
#define LCD_INTF8BITS               0x10        // Sub Mode of FUNCTION_SET : Select 8 bit interface         (DL)
#define LCD_INTF4BITS               0x00        // Sub Mode of FUNCTION_SET : Select 4 bit interface         (DL)
#define LCD_TWO_LINES               0x08        // Sub Mode of FUNCTION_SET : Selects two char line display  (N)
#define LCD_ONE_LINE                0x00        // Sub Mode of FUNCTION_SET : Selects one char line display  (N)
#define LCD_FONT_5_10               0x04        // Sub Mode of FUNCTION_SET : Selects 5 x 10 Dot Matrix Font (F)
#define LCD_FONT_5_7                0x00        // Sub Mode of FUNCTION_SET : Selects 5 x 7 Dot Matrix Font  (F)

#define LCD_CG_RAM_ADDRESS          0x40        // Mode : Enables the setting of the Char Gen (CG) Ram Address, to be or'ed with require address
#define LCD_CG_RAM_ADDRESS_MASK     0b00111111  // Used to mask off the lower 6 bits of valid CG Ram Addresses

#define LCD_DD_RAM_ADDRESS          0x80        // Mode : Enables the setting of the Display Data (DD) Ram Address, to be or'ed with require address
#define LCD_DD_RAM_ADDRESS_MASK     0b01111111    // Used to mask off the lower 6 bits of valid DD Ram Addresses

//#define USE_BUSY_FLAG               // Define this if you wish to use busy flag polling on slow LCD activities

// Change here for your I2C to 16 pin parallel interface // TODO Adapt
#define Bl 0b00001000  // Backlight enable bit (On = 1, Off =0)
#define En 0b00000100  // Enable bit (Enable on low edge)
#define Rw 0b00000010  // Read/Write bit (Read = 1, Write = 0)
#define Rs 0b00000001  // Register select bit (Data = 1, Control = 0)

// Change here for your I2C to 16 pin parallel interface // TODO Adapt
#define LCD_INIT      ((0b00000000 | En) & ~Rs) & (~Rw) // Used to set all the O/Ps on the PCF8574 to initialise the LCD
#define LCD_8BIT_INIT 0b00110000 // Used to initialise the interface at the LCD
#define LCD_4BIT_INIT 0b00100000 // Used to initialise the interface at the LCD

#define LCD_PCF8574_ADDR        0x27// (0x27<<1)  // Modify this if the default address is altered
#define LCD_PCF8574_WEAK_PU      0b11110000 // Used to turn on PCF8574 Bits 7-4 on. To allow for read of LCD.

#define LCD_BUSY_FLAG_MASK       0b10000000 // Used to mask off the status of the busy flag
#define LCD_ADDRESS_COUNTER_MASK 0b01111111 // Used to mask off the value of the Address Counter
#define LCD_MAX_COLS             20
#define LCD_MAX_ROWS             4

//
// Code was written with the following assumptions as to PCF8574 -> Parallel 4bit convertor interconnections
// controlling a 20 by 4 LCD display. Assumes A0...A2 on PCF8574 are all pulled high. Giving address of 0x4E or 0b01001110 (0x27)
// (the last bit [b0] is I2C R/nW bit)
//
// Pin out for LCD display (16 pins)
// ---------------------------------
// 1  - Gnd
// 2  - Vcc
// 3  - VContrast
// 4  - RS - P0 - Pin 4 PCF8574
// 5  - RW - P1 - Pin 5 PCF8574
// 6  - En - P2 - Pin 6 PCF8574
// 7  - D0 - Don't Care
// 8  - D1 - Don't Care
// 9  - D2 - Don't Care
// 10 - D3 - Don't Care
// 11 - D4 - P4 - Pin 9  PCF8574
// 12 - D6 - P5 - Pin 10 PCF8574
// 13 - D6 - P6 - Pin 11 PCF8574
// 14 - D7 - P7 - Pin 12 PCF8574
// 15 - Anode   LED
// 16 - Cathode LED
//
// PCF8574 register and pin mapping
// Bit 0 - RS  - P0 - Pin 4  PCF8574
// Bit 1 - RW  - P1 - Pin 5  PCF8574
// Bit 2 - En  - P2 - Pin 6  PCF8574
// Bit 3 - Led - P3 - Pin 7  PCF8574 (Active High, Led turned on)
// Bit 4 - D4  - P4 - Pin 9  PCF8574
// Bit 5 - D5  - P5 - Pin 10 PCF8574
// Bit 6 - D6  - P6 - Pin 11 PCF8574
// Bit 7 - D7  - P7 - Pin 12 PCF8574
//



// The display is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 4-bit interface data
//    N = 0; 2-line display
//    F = 0; 5x7 dot character font
// 3. Display on/off control:
//    D = 1; Display on
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//

#define LCD_LINE1	0x00		// Constant used to point to start of LCD Line 1
#define LCD_LINE2	0x40		// Constant used to point to start of LCD Line 2
#define LCD_LINE3	0x14		// Constant used to point to start of LCD Line 3
#define LCD_LINE4	0x54		// Constant used to point to start of LCD Line 4

// DDRAM address:
// Display position
// 0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19
// 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13
// 40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53
// 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27
// 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F 60 61 62 63 64 65 66 67


// Local variable declarations

static unsigned char _functionset = 0;
static unsigned char _entrymodeset = 0;
static unsigned char _displayfunction = 0;
static unsigned char _displaycontrol = 0;
static unsigned char _numlines = 0;
static unsigned char _backlightval = 0;


// Local function declarations

static void LCDsend(unsigned char value, unsigned char mode);
static unsigned char LCDreceive(unsigned char RsMode);
static void LCDwrite4bits(unsigned char value);
static unsigned char LCDread4bits(unsigned char RsEnMode);
static void LCDpulseEnableNeg(unsigned char value);
static void LCDpulseEnablePos(unsigned char value);
static void LCDwritePCF8574(unsigned char value);
static unsigned char LCDreadPCF8574(void);


void LCD_Init(void){
	uint8_t culo;
    //_backlightval &= ~Bl; // Off at start up
    _backlightval |= Bl; // On at start up
    _numlines = LCD_MAX_ROWS;

    // Ensure supply rails are up before config sequence
    delayInaccurateUs(50000);

    // Set all control and data lines low. D4 - D7, En (High=1), Rw (Low = 0 or Write), Rs (Control/Instruction) (Low = 0 or Control)
 //   I2C_Write_Byte_Single_Reg(LCD_PCF8574_ADDR, LCD_INIT); // Backlight off (Bit 3 = 0)
    culo = LCD_INIT;

    i2cWrite(I2C0,0x27,&culo,1,TRUE);
    delayInaccurateUs(100);

    // Sequence to put the LCD into 4 bit mode this is according to the hitachi HD44780 datasheet page 109

    // we start in 8bit mode
    LCDwrite4bits(LCD_8BIT_INIT);
    delayInaccurateUs(4500);  // wait more than 4.1ms

    // second write
    LCDwrite4bits(LCD_8BIT_INIT);
    delayInaccurateUs(150); // wait > 100us

    // third write
    LCDwrite4bits(LCD_8BIT_INIT);
    delayInaccurateUs(150);

    // now set to 4-bit interface
    LCDwrite4bits(LCD_4BIT_INIT);
    delayInaccurateUs(150);

    // set # lines, font size, etc.
    _functionset = LCD_INTF4BITS | LCD_TWO_LINES | LCD_FONT_5_7;
    LCDcommandWrite(LCD_FUNCTION_SET | _functionset);
    //DelayMicroseconds(150);

    _displayfunction = LCD_DISPLAY_OFF | LCD_CURSOR_OFF | LCD_BLINKING_OFF;
    LCDdisplayOff();

    // turn the display on with no cursor or blinking default
    LCDdisplayOn();

    // set the entry mode
    _entrymodeset = LCD_INCREMENT | LCD_SHIFT_OFF; // Initialize to default text direction (for roman languages)
    LCDcommandWrite(LCD_ENTRY_MODE_SET | _entrymodeset);

    // Display Function set
    // _displayfunction = LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINKING_OFF;
    LCDcommandWrite(LCD_DISPLAY_ON_OFF | _displayfunction);

    // Display Control set
    _displaycontrol = LCD_DISPLAY_SHIFT | LCD_SHIFT_LEFT;
    LCDcommandWrite(LCD_MV_CUR_SHIFT_DISPLAY | _displaycontrol);

    // clear display and return cursor to home position. (Address 0)
    LCDclear();
}





/********** high level commands, for the user! */


void LCD_Write_Char(char message)
{
    LCDdataWrite((unsigned char) message);
}

void LCD_Write_Str(const char *message)
{
    unsigned char *message_ptr = (unsigned char *) message;

    while (*message_ptr)
        LCDdataWrite((unsigned char) (*message_ptr++));
}


void LCDclear(void){
    LCDcommandWrite(LCD_CLEAR_DISPLAY);// clear display, set cursor position to zero
#ifdef USE_BUSY_FLAG
    while (LCDbusy()){};
#else
    delayInaccurateUs(30000);  // this command takes a long time!
#endif
}

void LCDhome(void){
    LCDcommandWrite(LCD_RETURN_HOME);  // set cursor position to zero
#ifdef USE_BUSY_FLAG
    while (LCDbusy()){};
#else
    delayInaccurateUs(30000);  // this command takes a long time!
#endif
}

void LCDsetCursor(unsigned char col, unsigned char row)
{
  int row_offsets[] = { LCD_LINE1, LCD_LINE2, LCD_LINE3, LCD_LINE4 };
  if ( row >= _numlines ) {
    row = _numlines-1;    // we count rows starting w/0
    //if(row == 255)row = 0;
  }

  LCDcommandWrite(LCD_DD_RAM_ADDRESS | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LCDdisplayOff(void) {
    _displayfunction &= ~LCD_DISPLAY_ON;
    LCDcommandWrite(LCD_DISPLAY_ON_OFF | _displayfunction);
}

void LCDdisplayOn(void) {
    _displayfunction |= LCD_DISPLAY_ON;
    LCDcommandWrite(LCD_DISPLAY_ON_OFF | _displayfunction);
}

// Turns the underline cursor on/off
void LCDcursorOff(void) {
    _displayfunction &= ~LCD_CURSOR_ON;
    LCDcommandWrite(LCD_DISPLAY_ON_OFF | _displayfunction);
}

void LCDcursorOn(void) {
    _displayfunction |= LCD_CURSOR_ON;
    LCDcommandWrite(LCD_DISPLAY_ON_OFF | _displayfunction);
}

// Turn on and off the blinking cursor
void LCDblinkOff(void) {
    _displayfunction &= ~LCD_BLINKING_ON;
    LCDcommandWrite(LCD_DISPLAY_ON_OFF | _displayfunction);
}

void LCDblinkOn(void) {
    _displayfunction |= LCD_BLINKING_ON;
    LCDcommandWrite(LCD_DISPLAY_ON_OFF | _displayfunction);
}
// These commands scroll the display without changing the RAM
void LCDscrollDisplayLeft(void) {
    _displaycontrol &=  ~LCD_SHIFT_RIGHT;
    _displaycontrol |=   LCD_DISPLAY_SHIFT;
    LCDcommandWrite(LCD_MV_CUR_SHIFT_DISPLAY | _displaycontrol);
}

void LCDscrollDisplayRight(void) {
    _displaycontrol |=  LCD_SHIFT_RIGHT;
    _displaycontrol |=  LCD_DISPLAY_SHIFT;
    LCDcommandWrite(LCD_MV_CUR_SHIFT_DISPLAY | _displaycontrol);
}


// This is for text that flows Left to Right
void LCDleftToRight(void) {
    _entrymodeset |= LCD_INCREMENT;
    //_entrymodeset |= LCD_SHIFT_ON;
    LCDcommandWrite(LCD_ENTRY_MODE_SET | _entrymodeset);
}

// This is for text that flows Right to Left
void LCDrightToLeft(void) {
    _entrymodeset &= ~LCD_INCREMENT;
    //_entrymodeset &= ~LCD_SHIFT_ON;
    LCDcommandWrite(LCD_ENTRY_MODE_SET | _entrymodeset);
}

// This will 'right justify' text from the cursor. Display shift
void LCDautoscroll(void) {
    _entrymodeset |= LCD_SHIFT_ON;
    //_entrymodeset |= LCD_INCREMENT;
    LCDcommandWrite(LCD_ENTRY_MODE_SET | _entrymodeset);
}

// This will 'left justify' text from the cursor. Cursor Move
void LCDnoAutoscroll(void) {
    _entrymodeset &= ~LCD_SHIFT_ON;
    //_entrymodeset &= ~LCD_INCREMENT;
    LCDcommandWrite(LCD_ENTRY_MODE_SET | _entrymodeset);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LCDcreateChar(unsigned char location, unsigned char charmap[]) {
	int i=0;
    location &= 0x7; // we only have 8 locations 0-7
    LCDcommandWrite(LCD_CG_RAM_ADDRESS | (location << 3));
    for (i=0; i<8; i++)
        LCDdataWrite(charmap[i]);
}

// Turn the (optional) backlight off/on
void LCDnoBacklight(void) { 
    _backlightval &= ~Bl;
    LCDwritePCF8574(LCDreadPCF8574());  // Dummy write to LCD, only led control bit is of interest
}

void LCDbacklight(void) { 
    _backlightval |= Bl;
    LCDwritePCF8574(LCDreadPCF8574());  // Dummy write to LCD, only led control bit is of interest
}



/*********** mid level commands, for sending data/cmds */

inline void LCDcommandWrite(unsigned char value) {
    LCDsend(value, Rs & ~Rs);
}

inline unsigned char LCDcommandRead(void) {
    return LCDreceive(Rs & ~Rs);
}

inline void LCDdataWrite(unsigned char value) {
    LCDsend(value, Rs);
}

inline unsigned char LCDdataRead(void) {
    return LCDreceive(Rs);
}

unsigned char LCDbusy(void) {
    return LCDcommandRead() & LCD_BUSY_FLAG_MASK;
}

unsigned char LCDaddressCounter(void) {
    return LCDcommandRead() & LCD_ADDRESS_COUNTER_MASK;
}

unsigned char LCDreadDDRam(unsigned char address)
{
    LCDcommandWrite(LCD_DD_RAM_ADDRESS | (address & LCD_DD_RAM_ADDRESS_MASK));
    return LCDdataRead();
}

unsigned char LCDreadCGRam(unsigned char address)
{
    LCDcommandWrite(LCD_CG_RAM_ADDRESS | (address & LCD_CG_RAM_ADDRESS_MASK));
    return LCDdataRead();
}

/************ low level data write commands **********/

// Change this routine for your I2C to 16 pin parallel interface, if your pin interconnects are different to that outlined above // TODO Adapt

// write either command or data
static void LCDsend(unsigned char value, unsigned char RsMode) {
    unsigned char highnib = value & 0xF0;

    unsigned char lownib  = value << 4;
    lownib &= 0xF0;

    LCDwrite4bits((highnib) | En | RsMode);
    LCDwrite4bits((lownib ) | En | RsMode);
}

// Change this routine for your I2C to 16 pin parallel interface, if your pin interconnects are different to that outlined above // TODO Adapt

// read either command or data
static unsigned char LCDreceive(unsigned char RsMode) {
    unsigned char highnib;
    unsigned char lownib;

    LCDwritePCF8574(LCD_PCF8574_WEAK_PU | (En & ~En) | RsMode); // Set P7..P4 = 1, En = 0, RnW = 0, Rs = XX
    highnib = LCDread4bits(LCD_PCF8574_WEAK_PU | En | RsMode);
    lownib = LCDread4bits(LCD_PCF8574_WEAK_PU | En | RsMode);
    LCDwritePCF8574((LCD_PCF8574_WEAK_PU & ~LCD_PCF8574_WEAK_PU) | En | RsMode); // Set P7..P4 = 1, En = 1, RnW = 0, Rs = XX
    return (unsigned char) ((highnib & 0xF0) | ((lownib & 0xF0) >> 4));
}



static void LCDwrite4bits(unsigned char nibEnRsMode) {
    LCDwritePCF8574(nibEnRsMode & ~Rw);
    LCDpulseEnableNeg(nibEnRsMode & ~Rw);
}


static unsigned char LCDread4bits(unsigned char RsEnMode) {
    unsigned char b;
    LCDpulseEnablePos(RsEnMode | Rw);
    b = LCDreadPCF8574(); // Read the data from the LCD just after the rising edge. NOT WELL DOCUMENTED!
    LCDpulseEnableNeg(RsEnMode | Rw);
    return b;
}

static void LCDpulseEnableNeg(unsigned char _data){
    LCDwritePCF8574(_data | En);	// En high
    delayInaccurateUs(1);		// enable pulse must be >450ns

    LCDwritePCF8574(_data & ~En);	// En low
    delayInaccurateUs(50);		// commands need > 37us to settle
}

static void LCDpulseEnablePos(unsigned char _data){
    LCDwritePCF8574(_data & ~En);	// En low
    delayInaccurateUs(1);		// enable pulse must be >450ns

    LCDwritePCF8574(_data | En);	// En high
    delayInaccurateUs(50);		// commands need > 37us to settle
}


static void LCDwritePCF8574(unsigned char value) {
	uint8_t valor_i2c;
	valor_i2c = (value | _backlightval);
    i2cWrite(I2C0,LCD_PCF8574_ADDR,&valor_i2c,1,TRUE);
}


static unsigned char LCDreadPCF8574(void) {
	uint8_t dataToReadBuffer;
	uint8_t MSB;
    i2cRead(I2C0,LCD_PCF8574_ADDR,&dataToReadBuffer, 1, TRUE,
            &MSB, 1, TRUE);
    return MSB;
}
