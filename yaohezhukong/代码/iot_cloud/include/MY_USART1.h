#ifndef _MY_USART1_H
#define _MY_USART1_H

void MY_USART1_Init(void);//初始化串口

int MY_USART1_SendString(char *string);

void MY_USART1_SendNum(int num, char flag);
#endif
