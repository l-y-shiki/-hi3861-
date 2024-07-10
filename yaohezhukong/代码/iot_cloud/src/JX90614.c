//#include "MYI2C.H"

#include <stdio.h>
#include <unistd.h>

#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "ohos_init.h"

#define JX90614_ADDRESS 0Xfe

#define SCL     0
#define SDA     1

void Delay_us(int x)
{
    int i;
    while(x--)
    {
        i = 10;
        while(i--);
    }
}

void MYI2C_Init(void)//使用的时候也要改一下初始化时钟
{
    //初始化0口
    IoTGpioInit(SDA);
    IoTGpioSetFunc(SDA, IOT_GPIO_FUNC_GPIO_1_GPIO);
    IoTGpioSetDir(SDA, IOT_GPIO_DIR_OUT);
    IoTGpioInit(SCL);
    IoTGpioSetFunc(SCL, IOT_GPIO_FUNC_GPIO_0_GPIO);
    IoTGpioSetDir(SCL, IOT_GPIO_DIR_OUT);

    IoTGpioSetOutputVal(SDA, 1);
    IoTGpioSetOutputVal(SCL, 1);
}

void SDA_Out(void)
{
    IoTGpioInit(SDA);
    IoTGpioSetFunc(SDA, IOT_GPIO_FUNC_GPIO_1_GPIO);
    IoTGpioSetDir(SDA, IOT_GPIO_DIR_OUT);
}
void SDA_In(void)
{
    IoTGpioInit(SDA);
    IoTGpioSetFunc(SDA, IOT_GPIO_FUNC_GPIO_1_GPIO);
    IoTGpioSetDir(SDA, IOT_GPIO_DIR_IN);
    IoTGpioSetPull(SDA, IOT_GPIO_PULL_UP);
}
void MYI2C_W_SCL(int BitValue)
{
    if(BitValue > 0)
    {
        BitValue = 1;
    }
    IoTGpioSetOutputVal(SCL, BitValue);
    Delay_us(10);
}

void MYI2C_W_SDA(int BitValue)
{
    SDA_Out();
    if(BitValue > 0)
    {
        BitValue = 1;
    }
    IoTGpioSetOutputVal(SDA, BitValue);
    Delay_us(10);
}

int MYI2C_R_SDA(void)
{
    int BitValue;
    IoTGpioGetInputVal(SDA, &BitValue);
    Delay_us(10);
    return BitValue;
}

void MYI2C_Start(void)//SCL在高电平时SDA产生一个下降沿
{
    MYI2C_W_SDA(1);
    MYI2C_W_SCL(1);
    MYI2C_W_SDA(0);
    MYI2C_W_SCL(0);
}

void MYI2C_Stop(void)//SCL在高电平时SDA产生一个上升沿
{
    MYI2C_W_SDA(0);
    MYI2C_W_SCL(1);
    MYI2C_W_SDA(1);
}

void MYI2C_SendByte(int Byte)
{
    int i;
    for(i = 0; i < 8; i++)
    {
        MYI2C_W_SDA(Byte & (0x80 >> i));//将数据依次放在SDA上
        MYI2C_W_SCL(1);
        MYI2C_W_SCL(0);
    }
}

int MYI2C_ReceiveByte(void)
{
    int i;
    int Byte = 0x00;
    MYI2C_W_SDA(1);//主机释放SDA
    SDA_In();
    for(i = 0; i < 8; i++)
    {
        MYI2C_W_SCL(1);//高电平时读数据
        if(MYI2C_R_SDA() == 1)
        {
            Byte |= 0x80 >> i;
        }
        MYI2C_W_SCL(0);
    }
    return Byte;
}

void MYI2C_SendAck(int AckBit)
{
    MYI2C_W_SDA(AckBit);//将数据放在SDA上
    MYI2C_W_SCL(1);
    MYI2C_W_SCL(0);
}

int MYI2C_ReceiveAck(void)
{
    int AckBit;
    MYI2C_W_SDA(1);//主机释放SDA
    SDA_In();
    MYI2C_W_SCL(1);//高电平时读数据
    AckBit = MYI2C_R_SDA();
    MYI2C_W_SCL(0);
    return AckBit;
}

void JX90614_WriteRegister(uint8_t Address, uint8_t Data)//指定寄存器写入数据
{
	MYI2C_Start();
	MYI2C_SendByte(JX90614_ADDRESS);//呼叫从机写地址
	MYI2C_ReceiveAck();
	
	MYI2C_SendByte(Address);//指针移到相应地址
	MYI2C_ReceiveAck();
	
	MYI2C_SendByte(Data);//在对应地址写入数据
	MYI2C_ReceiveAck();
	
	MYI2C_Stop();
}

uint8_t JX90614_ReadRegister(uint8_t Address)//读指定寄存器数据
{
	uint8_t Data;
	
	MYI2C_Start();
	MYI2C_SendByte(JX90614_ADDRESS);//呼叫从机写地址
	MYI2C_ReceiveAck();
	
	MYI2C_SendByte(Address);//指针移到相应地址
	MYI2C_ReceiveAck();

	MYI2C_Start();
	MYI2C_SendByte(JX90614_ADDRESS | 0x01);//呼叫从机读地址
	MYI2C_ReceiveAck();
	
	Data = MYI2C_ReceiveByte();
	MYI2C_SendAck(1);//不给从机应答，停止接收
	
	MYI2C_Stop();
	
	return Data;
}

void JX90614_Init(void)
{
	MYI2C_Init();
	JX90614_WriteRegister(0x30, 0x00);
	JX90614_WriteRegister(0x30, 0x08);
}

void JX90614_GetTemp(int32_t *Temp)
{
	uint32_t DataH, DataM, DataL;
	
	DataH = JX90614_ReadRegister(0x10);
	DataM = JX90614_ReadRegister(0x11);
	DataL = JX90614_ReadRegister(0x12);
	*Temp = (DataH << 16) | (DataM << 8) | DataL;
}

