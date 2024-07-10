#ifndef _HEART_H
#define _HEART_H


void Heart_Init(void);
void uart1_send(uint8_t *buf,uint16_t len);
uint16_t uart1_recv(uint8_t *buf,uint16_t len);



#endif
