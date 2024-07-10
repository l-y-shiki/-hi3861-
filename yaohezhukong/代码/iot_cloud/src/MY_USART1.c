#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include "cmsis_os2.h"
#include "iot_errno.h"
#include "iot_uart.h"
#include "ohos_init.h"
#include "iot_gpio_ex.h"
#include "iot_gpio.h"

#define UART_BUFF_SIZE 1000
#define WIFI_IOT_UART_IDX_1 1
#define TASK_DELAY_1S 1000000
#define UART_TX 6
#define UART_RX 5

void MY_USART1_Init(void)//初始化串口
{
    //配置io5
    IoTGpioInit(UART_RX);
    IoTGpioSetFunc(UART_RX, IOT_GPIO_FUNC_GPIO_5_UART1_RXD);
    IoTGpioSetDir(UART_RX, IOT_GPIO_DIR_IN);
    IoTGpioSetPull(UART_RX, IOT_GPIO_PULL_UP);
    //配置io6
    IoTGpioInit(UART_TX);
    IoTGpioSetFunc(UART_TX, IOT_GPIO_FUNC_GPIO_6_UART1_TXD);
    IoTGpioSetDir(UART_TX, IOT_GPIO_DIR_OUT);
    IoTGpioSetPull(UART_TX, IOT_GPIO_PULL_UP);

    // uint8_t uart_buff[UART_BUFF_SIZE] = { 0 };
    // uint8_t *uart_buff_ptr = uart_buff;
    uint8_t ret;

    IotUartAttribute uart_attr = {

        // baud_rate: 9600
        .baudRate = 9600,

        // data_bits: 8bits
        .dataBits = 8,
        .stopBits = 1,
        .parity = 0,
    };

    //Initialize uart driver
    ret = IoTUartInit(WIFI_IOT_UART_IDX_1, &uart_attr);
    if (ret != IOT_SUCCESS) {
        printf("Failed to init uart1! Err code = %d\n", ret);
    }
}

int MY_USART1_SendString(char *string)
{
    //改变string为与32通信的指令
    int i;
    unsigned char data[50];
    data[0] = '@';
    for(i = 0; string[i] != '\0'; i++)//退出循环时i指向string的'\0',
    {
        data[i + 1] = string[i];
    }
    data[i + 1] = '#';//最后三位赋值
    data[i + 2] = '#';
    data[i + 3] = '\0';


    uint8_t ret;//发送几个字符
    ret = IoTUartWrite(WIFI_IOT_UART_IDX_1, data, strlen(data));
    //printf("Number of bytes to be seNumnt = %d\n", ret);
    return ret;
}

void MY_USART1_SendNum(int num, char flag)//把int类型数字改为与32通信的字符串并发送
{
    char data[10];
    data[0] = flag;//发送什么类型数据
    if(num >= 100)//三位数
    {
        data[1] = num / 100 % 10 + '0';
        data[2] = num / 10 % 10 + '0';
        data[3] = num / 1 % 10 + '0';
        data[4] = '\0';
    }
    else if(num >= 10 && num <= 99)//两位数
    {
        data[1] = num / 10 % 10 + '0';
        data[2] = num / 1 % 10 + '0';
        data[3] = '\0';
    }
    else if(num >= 0 && num <= 9)//一位数 
    {
        data[1] = num / 1 % 10 + '0';
        data[2] = '\0';
    }
    MY_USART1_SendString(data);
}
