
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_errno.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "iot_uart.h"

#include "hi_uart.h"
#include "hi_timer.h"
#include "hi_io.h"


#define UART_BUFF_SIZE 1024     //接收缓冲大小
#define UART_TIMEOUT_MS 40      //接收超时时间
#define WIFI_IOT_UART_IDX_2 2

void Heart_Init(void)
{
    uint8_t ret;
    hi_uart_attribute uart_attr = {
        //.baud_rate = 38400,//传感器波特率
        .baud_rate = 38400,//传感器波特率
        .data_bits = 8,
        .stop_bits = 1,
        .parity = 0,
    };

    IoTGpioInit(11);
	IoTGpioInit(12);
    IoTGpioSetFunc(12,IOT_GPIO_FUNC_GPIO_12_UART2_RXD);
    IoTGpioSetDir(12, IOT_GPIO_DIR_IN);
    IoTGpioSetPull(12,IOT_GPIO_PULL_UP);
    IoTGpioSetFunc(11,IOT_GPIO_FUNC_GPIO_11_UART2_TXD);
    IoTGpioSetDir(11, IOT_GPIO_DIR_OUT);
    IoTGpioSetPull(11,IOT_GPIO_PULL_UP);
     //Initialize uart driver
    ret = hi_uart_init(HI_UART_IDX_2, &uart_attr, NULL);
    if (ret != HI_ERR_SUCCESS)
    {
        printf("Failed to init uart! Err code = %d\n", ret);
        return;
    }

    uint8_t ucSwitch = 0x8A;
    for(int i=0;i<3;i++)
    {
        ret = hi_uart_write(HI_UART_IDX_2, &ucSwitch,1);//打开传感器
        printf("write 0x8A is = %d\n", ret);
        usleep(50);
    }

}

//串口接收，无数据时阻塞，有数据时收完一帧或者收到最大值返回
uint16_t uart1_recv(uint8_t *buf,uint16_t len)
{
    uint16_t rxlen=0;
    uint16_t ret=0;
    // 原始代码
    while(len-rxlen>0){
     

    
        ret=hi_uart_read_timeout(HI_UART_IDX_2, buf+rxlen, len-rxlen,UART_TIMEOUT_MS);
        if(ret==0&&rxlen!=0){
            return rxlen;
        }
        else{
            rxlen+=ret;
    } 
    }
    return rxlen;
}



