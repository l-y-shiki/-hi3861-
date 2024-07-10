#ifndef __LCD_INIT_H
#define __LCD_INIT_H
#include <stdint.h>

#define USE_HORIZONTAL 1  //设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 128
#define LCD_H 160

#else
#define LCD_W 160
#define LCD_H 128
#endif

#define u8  unsigned char
#define u16 unsigned int

#define OLED_I2C_SCL_PIN		0
#define OLED_I2C_SDA_PIN		1
#define OLED_I2C_RES_PIN        2
#define OLED_I2C_DC_PIN         9
#define OLED_I2C_CS_PIN         10
#define OLED_I2C_BLK_PIN        5       ////////

//-----------------LCD端口定义----------------

#define LCD_SCLK_Clr() IoTGpioSetOutputVal(OLED_I2C_SCL_PIN, 0)
#define LCD_SCLK_Set() IoTGpioSetOutputVal(OLED_I2C_SCL_PIN, 1)

#define LCD_MOSI_Clr() IoTGpioSetOutputVal(OLED_I2C_SDA_PIN, 0)
#define LCD_MOSI_Set() IoTGpioSetOutputVal(OLED_I2C_SDA_PIN, 1)

#define LCD_RES_Clr() IoTGpioSetOutputVal(OLED_I2C_RES_PIN, 0)
#define LCD_RES_Set() IoTGpioSetOutputVal(OLED_I2C_RES_PIN, 1)

#define LCD_DC_Clr() IoTGpioSetOutputVal(OLED_I2C_DC_PIN, 0)
#define LCD_DC_Set() IoTGpioSetOutputVal(OLED_I2C_DC_PIN, 1)

#define LCD_CS_Clr()  IoTGpioSetOutputVal(OLED_I2C_CS_PIN, 0)
#define LCD_CS_Set()  IoTGpioSetOutputVal(OLED_I2C_CS_PIN, 1)

#define LCD_BLK_Clr()  IoTGpioSetOutputVal(OLED_I2C_BLK_PIN, 0)
#define LCD_BLK_Set()  IoTGpioSetOutputVal(OLED_I2C_BLK_PIN, 0)



void delay_ms(unsigned int ms);//不准确延时函数
void LCD_GPIO_Init(void);//初始化GPIO
void LCD_Writ_Bus(u8 dat);//模拟SPI时序
void LCD_WR_DATA8(u8 dat);//写入一个字节
void LCD_WR_DATA(u16 dat);//写入两个字节
void LCD_WR_REG(u8 dat);//写入一个指令
void LCD_Address_Set(u16 x1,u16 y1,u16 x2,u16 y2);//设置坐标函数
void Lcd_Init(void);//LCD初始化
#endif