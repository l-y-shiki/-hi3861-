#ifndef __OLED_I2C_H
#define	__OLED_I2C_H

#include <stdint.h>

#define OLED_ADDRESS	0x78 //閫氳繃璋冩暣0R鐢甸樆,灞忓彲浠?x78鍜?x7A涓や釜鍦板潃 -- 榛樿0x78

uint8_t OLED_CheckDevice(uint8_t _Address);//妫€娴婭2C鎬荤嚎璁惧OLED
void I2C_WriteByte(uint8_t addr,uint8_t data);
void WriteCmd(unsigned char I2C_Command);
void WriteDat(unsigned char I2C_Data);
void OLED_Init(void);
void OLED_SetPos(unsigned char x, unsigned char y);
void OLED_Fill(unsigned char fill_Data);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N);
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
uint8_t OLED_Test(void) ;//OLED妫€娴嬫祴璇?


#endif