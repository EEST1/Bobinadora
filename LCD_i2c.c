/* Copyright 2016, Eric Pernia.
 * All rights reserved.
 *
 * This file is part sAPI library for microcontrollers.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Date: 2016-04-26
 */

/*==================[inclusions]=============================================*/

//#include "blinky.h"   // <= own header (optional)
#include "sapi.h"       // <= sAPI header
#include "LCD_I2C_PCF8574.h"
#include "LCD_i2c.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/
/**
 * Conjunto de caracteres predefinidos y caracteres programados especiales
 */
unsigned char x = 0;
unsigned char hex[] = { "0123456789ABCDEF" };
unsigned char bell[8] = { 0x04, 0x0e, 0x0e, 0x0e, 0x1f, 0x00, 0x04 };
unsigned char note[8] = { 0x02, 0x03, 0x02, 0x0e, 0x1e, 0x0c, 0x00 };
unsigned char clock[8] = { 0x00, 0x0e, 0x15, 0x17, 0x11, 0x0e, 0x00 };
unsigned char heart[8] = { 0x00, 0x0a, 0x1f, 0x1f, 0x0e, 0x04, 0x00 };
unsigned char duck[8] = { 0x00, 0x0c, 0x1d, 0x0f, 0x0f, 0x06, 0x00 };
unsigned char check[8] = { 0x00, 0x01, 0x03, 0x16, 0x1c, 0x08, 0x00 };
unsigned char cross[8] = { 0x00, 0x1b, 0x0e, 0x04, 0x0e, 0x1b, 0x00 };
unsigned char retarrow[8] = { 0x01, 0x01, 0x05, 0x09, 0x1f, 0x08, 0x04 };
unsigned char * charmap[8] = { bell, note, clock, heart, duck, check, cross,
		retarrow };

/**
 * Funciones de prueba de los comandos del LCD
 */

void autoIncrement(void) {
	unsigned char Autoscroll[] = "Autoscroll";
	unsigned char NoAutoscroll[] = "No Autoscroll";
	unsigned char * p;
	LCDclear();
	LCDcursorOff();
	LCDblinkOff();
	LCDautoscroll();
	p = Autoscroll;
	LCDsetCursor(15, 1);
	while (*p) {
		LCD_Write_Char((char) *p++);
		delayInaccurate(400);

	}
	delayInaccurate(5000);

	LCDclear();
	LCDnoAutoscroll();
	p = NoAutoscroll;
	LCDsetCursor(3, 1);
	while (*p) {
		LCD_Write_Char((char) *p++);
		delayInaccurate(400);
	}
	delayInaccurate(5000);
}

void displayLeftRightScroll(void) {
	unsigned char x = 0;
	LCDclear();
	LCDcursorOff();
	LCDblinkOff();
	LCDsetCursor(0, 0);
	LCD_Write_Str("Scroll Right");
	LCDsetCursor(0, 1);
	LCD_Write_Str("---->");
	for (x = 0; x < 8; x++) {
		LCDscrollDisplayRight();
		delayInaccurate(500);
	}

	LCDclear();
	LCDsetCursor(8, 0);
	LCD_Write_Str("Scroll Left ");
	LCDsetCursor(8, 1);
	LCD_Write_Str("<----");
	for (x = 0; x < 8; x++) {
		LCDscrollDisplayLeft();
		delayInaccurate(500);
	}

	delayInaccurate(5000);
}

void displayOnOff(void) {
	unsigned char x = 0;
	LCDclear();
	LCDcursorOff();
	LCDblinkOff();
	for (x = 0; x < 6; x++) {
		if (x % 2) {
			LCDhome();
			LCD_Write_Str("           ");
			LCDhome();
			LCD_Write_Str("Display On ");
			LCDdisplayOn();
		} else {
			LCDsetCursor(0, 0);
			LCD_Write_Str("           ");
			LCDsetCursor(0, 0);
			LCD_Write_Str("Display Off");
			LCDdisplayOff();
		}
		delayInaccurate(750);
	}
}

void backLightControl(void) {
	unsigned char x = 0;
	LCDclear();
	LCDcursorOff();
	LCDbacklight();
	LCD_Write_Str("Backlight On");
	delayInaccurate(1000);
	LCDclear();
	//LCDsetCursor(0,0);
	LCDnoBacklight();
	LCD_Write_Str("Backlight Off");
	delayInaccurate(1000);
	LCDclear();
	//LCDsetCursor(0,0);
	LCDbacklight();
	LCD_Write_Str("Backlight On");
	delayInaccurate(1000);
	for (x = 0; x < 6; x++) {
		if (x % 2) {
			LCDsetCursor(0, 0);
			LCD_Write_Str("             ");
			LCDsetCursor(0, 0);
			LCD_Write_Str("Backlight On ");
			LCDbacklight();
		} else {
			LCDclear();
			LCD_Write_Str("Backlight Off");
			LCDnoBacklight();
		}
		delayInaccurate(750);
	}
}

void printAt(void) {
	LCDclear();
	LCDcursorOff();
	//LCDsetCursor(0,0);
	LCD_Write_Str("@:0,0");
	LCDsetCursor(1, 1);
	LCD_Write_Str("@:1,1");
	LCDsetCursor(2, 2);
	LCD_Write_Str("@:2,2");
	LCDsetCursor(3, 3);
	LCD_Write_Str("@:3,3");
	LCDsetCursor(12, 0);
	LCD_Write_Str("Print at");
	delayInaccurate(3000);
}

void cursorControl(void) {
	LCDclear();
	LCDcursorOff();
	LCD_Write_Str("Cursor Off");
	delayInaccurate(1500);
	LCDclear();
	LCDcursorOn();
	LCD_Write_Str("Cursor On");
	delayInaccurate(1500);
	LCDclear();
	LCDblinkOn();
	LCD_Write_Str("Blink On");
	delayInaccurate(1500);
	LCDclear();
	LCDblinkOff();
	LCD_Write_Str("Blink Off");
	delayInaccurate(1500);
	LCDclear();
	LCD_Write_Str("Cursor Home");
	LCDhome();
	delayInaccurate(1500);
	LCDclear();
	LCD_Write_Str("Cursor Home & Blink");
	LCDhome();
	LCDblinkOn();
	delayInaccurate(3000);
}

void printLeftAndRight(void) {
	LCDclear();
	LCDcursorOff();
	LCDblinkOff();
	//LCDsetCursor(0,0);
	LCDleftToRight();
	LCD_Write_Str("Print left to right");
	LCDsetCursor(0, 1);
	LCD_Write_Str("0123456789ABCDEFGHIJ");
	LCDsetCursor(0, 2);
	LCD_Write_Str("Print right to left");
	LCDrightToLeft();
	LCDsetCursor(19, 4);
	LCD_Write_Str("0123456789ABCDEFGHIJ");
	LCDleftToRight();
	delayInaccurate(5000);
}

void controlMemoryReading(void) {
	unsigned char x = 0;

	LCDclear();
	LCDcursorOn();
	LCDblinkOn();
	LCDsetCursor(1, 1); // At position 1,1 the Address Counter will read 0x41
	delayInaccurate(2000);
	x = LCDaddressCounter();
	LCDcursorOff();
	LCDblinkOff();
	LCDsetCursor(0, 2);
	LCD_Write_Str("Addr Counter : ");
	LCDsetCursor(15, 2);
	LCD_Write_Str("0x");
	LCDsetCursor(17, 2);
	LCD_Write_Char((char) hex[(x >> 4) & 0x0f]);
	LCDsetCursor(18, 2);
	LCD_Write_Char((char) hex[x & 0x0f]);
	//DelayMilliseconds(5000);

	x = LCDbusy();
	LCDsetCursor(0, 3);
	LCD_Write_Str("Busy Flag : ");
	LCDsetCursor(12, 3);
	LCD_Write_Str("0x");
	LCDsetCursor(14, 3);
	LCD_Write_Char((char) hex[(x >> 4) & 0x0f]);
	LCDsetCursor(15, 3);
	LCD_Write_Char((char) hex[x & 0x0f]);
	delayInaccurate(5000);
}

void a(void) {
	unsigned char x = 0;
	unsigned char p = 0;
	unsigned char r = 0;
	unsigned char i = 0;
	unsigned char c = 0;
	LCDclear();
	LCDcursorOff();
	LCDblinkOff();
	//for (unsigned char p = 0; p < (sizeof(charmap)/sizeof(unsigned char *)) ; p++)
	for (p = 0; p < 8; p++)
		LCDcreateChar(p, charmap[p]);
	LCDhome();

	LCDsetCursor(0, 0);
	LCD_Write_Str("Q = ");

	x = LCDreadDDRam(0x00);
	LCDsetCursor(4, 0);
	LCD_Write_Str("0x");
	LCDsetCursor(6, 0);
	LCD_Write_Char((char) hex[(x >> 4) & 0x0f]);
	LCDsetCursor(7, 0);
	LCD_Write_Char((char) hex[x & 0x0f]);
	LCDsetCursor(8, 0);
	LCD_Write_Char(',');

	for (i = 0; i < 8; i++) {
		LCDsetCursor(i + 10, 0);
		LCD_Write_Char(i);
	}

	LCDsetCursor(0, 1);
	LCD_Write_Str("Bell char Hex rep;");

	x = 0;
	for (r = 0; r < 2; r++)
		for (c = 0; c < 4; c++)
			putHexString(c, r, LCDreadCGRam(x++));
	delayInaccurate(5000);
}

void putHexString(unsigned char c, unsigned char r, unsigned char hexval) {
	LCDsetCursor(0 + (5 * c), 2 + (2 * r));
	LCD_Write_Str("0x");
	LCDsetCursor(2 + (5 * c), 2 + (2 * r));
	LCD_Write_Char((char) hex[(hexval >> 4) & 0x0f]);
	LCDsetCursor(3 + (5 * c), 2 + (2 * r));
	LCD_Write_Char((char) hex[hexval & 0x0f]);
}

/*==================[end of file]============================================*/

