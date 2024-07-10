#ifndef _JX90614_H
#define _JX90614_H

void JX90614_Init(void);

void JX90614_WriteRegister(uint8_t Address, uint8_t Data);//指定寄存器写入数据

uint8_t JX90614_ReadRegister(uint8_t Address);//读指定寄存器数据

void JX90614_GetTemp(int32_t *Temp);//读温度


void MYI2C_Init(void);//使用的时候也要改一下初始化时钟
void SDA_Out(void);
void SDA_In(void);
void MYI2C_W_SCL(int BitValue);
void MYI2C_W_SDA(int BitValue);
int MYI2C_R_SDA(void);
void MYI2C_Start(void);//SCL在高电平时SDA产生一个下降沿
void MYI2C_Stop(void);//SCL在高电平时SDA产生一个上升沿
void MYI2C_SendByte(int Byte);
int MYI2C_ReceiveByte(void);
void MYI2C_SendAck(int AckBit);
int MYI2C_ReceiveAck(void);



#endif
