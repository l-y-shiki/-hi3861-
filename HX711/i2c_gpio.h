#ifndef _BSP_I2C_GPIO_H
#define _BSP_I2C_GPIO_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_errno.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include <inttypes.h>

#define OLED_I2C_WR	0		/* 鍐欐帶鍒禸it */
#define OLED_I2C_RD	1		/* 璇绘帶鍒禸it */

/* 瀹氫箟I2C鎬荤嚎杩炴帴鐨凣PIO绔彛, 鐢ㄦ埛鍙渶瑕佷慨鏀逛笅闈?琛屼唬鐮佸嵆鍙换鎰忔敼鍙楽CL鍜孲DA鐨勫紩鑴?*/
// #define OLED_GPIO_PORT_I2C	GPIOA			/* GPIO绔彛 */
// #define OLED_RCC_I2C_PORT 	RCC_APB2Periph_GPIOA		/* GPIO绔彛鏃堕挓 */
#define OLED_I2C_SCL_PIN		0
#define WIFI_IOT_IO_NAME_GPIO_0  0

#define OLED_I2C_SDA_PIN		1
#define WIFI_IOT_IO_NAME_GPIO_1	1	/* 杩炴帴鍒癝CL鏃堕挓绾跨殑GPIO */	/* 杩炴帴鍒癝DA鏁版嵁绾跨殑GPIO */


/* 瀹氫箟璇诲啓SCL鍜孲DA鐨勫畯锛屽凡澧炲姞浠ｇ爜鐨勫彲绉绘鎬у拰鍙槄璇绘€?*/

	#define OLED_I2C_SCL_1()  IoTGpioSetOutputVal(OLED_I2C_SCL_PIN, 1)			/* SCL = 1 */
	#define OLED_I2C_SCL_0()  IoTGpioSetOutputVal(OLED_I2C_SCL_PIN, 0)					/* SCL = 0 */
	
	#define OLED_I2C_SDA_1()  IoTGpioSetOutputVal(OLED_I2C_SDA_PIN, 1)				/* SDA = 1 */
	#define OLED_I2C_SDA_0()  IoTGpioSetOutputVal(OLED_I2C_SDA_PIN, 0)				/* SDA = 0 */


void i2c_Start(void);
void i2c_Stop(void);
void i2c_SendByte(uint8_t _ucByte);
uint8_t i2c_ReadByte(void);
uint8_t i2c_WaitAck(void);
void i2c_Ack(void);
void i2c_NAck(void);

void i2c_CfgGpio(void);
uint8_t OLED_I2C_SDA_READ(void);



#endif