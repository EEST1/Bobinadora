// LIQUIDCRYSTAL_I2C_PCF8574 V1.0

#ifndef LCD_I2C_PCF8574_H
#define LCD_I2C_PCF8574_H

#ifdef	__cplusplus
extern "C" {
#endif

  void LCD_Init(void);
  void LCD_Write_Char(char message);
  void LCD_Write_Str(const char *message);
  void LCDclear(void);
  void LCDhome(void);
  void LCDdisplayOff(void);
  void LCDdisplayOn(void);
  void LCDblinkOff(void);
  void LCDblinkOn(void);
  void LCDcursorOff(void);
  void LCDcursorOn(void);
  void LCDscrollDisplayLeft(void);
  void LCDscrollDisplayRight(void);
  void LCDleftToRight(void);
  void LCDrightToLeft(void);
  void LCDnoBacklight(void);
  void LCDbacklight(void);
  void LCDautoscroll(void);
  void LCDnoAutoscroll(void);
  void LCDcreateChar(unsigned char location, unsigned char charmap[]);
  void LCDsetCursor(unsigned char col, unsigned char row);
  inline void LCDcommandWrite(unsigned char value);
  inline unsigned char LCDcommandRead(void);
  inline void LCDdataWrite(unsigned char value);
  inline unsigned char LCDdataRead(void);
  unsigned char LCDbusy(void);
  unsigned char LCDaddressCounter(void);
  unsigned char LCDreadDDRam(unsigned char address);
  unsigned char LCDreadCGRam(unsigned char address);


#ifdef	__cplusplus
}
#endif

#endif // LIQUIDCRYSTAL_I2C_PCF8574_H
