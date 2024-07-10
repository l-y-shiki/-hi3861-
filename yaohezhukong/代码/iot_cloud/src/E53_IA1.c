/*
 * Copyright (c) 2020 Nanjing Xiaoxiongpai Intelligent Technology Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "cmsis_os2.h"
#include "iot_errno.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "iot_i2c.h"
#include "iot_i2c_ex.h"
#include "E53_IA1.h"
#include "JX90614.h"
#include "MY_USART1.H"
#include "oled.h"

/***************************************************************
 * 函数名称: E53IA1IoInit
 * 说    明: E53_IA1 GPIO初始化
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
#define FIX 6
static void E53IA1IoInit(void)
{
    // IoTGpioInit(E53_IA1_GPIO_11);
    // IoTGpioSetFunc(E53_IA1_GPIO_11, IOT_GPIO_FUNC_GPIO_11_GPIO);
    // IoTGpioSetDir(E53_IA1_GPIO_11, IOT_GPIO_DIR_IN); // 设置GPIO_11为输入模式
    // IoTGpioSetPull(E53_IA1_GPIO_11, IOT_GPIO_PULL_UP);//配置为上拉模式

    IoTGpioInit(E53_IA1_LIGHT_GPIO);
    IoTGpioSetFunc(E53_IA1_LIGHT_GPIO, IOT_GPIO_FUNC_GPIO_14_GPIO);
    IoTGpioSetDir(E53_IA1_LIGHT_GPIO, IOT_GPIO_DIR_OUT); // 设置GPIO_14为输出模式，控制led灯
}

/***************************************************************
 * 函数名称: E53IA1Init
 * 说    明: 初始化E53_IA1
 * 参    数: 无
 * 返 回 值: 0 成功; -1 失败
 ***************************************************************/
int E53IA1Init(E53IA1Data *Data)
{
    Data->HeartRate = 0;
    Data->BloodOxygen = 0;
    Data->Height = 178;
    Data->Weight = 74.9;
    JX90614_Init();
    MY_USART1_Init();
    // OLED_Init();

    // OLED_ShowString(1, 1, "Temperature:");
    // OLED_ShowString(2, 1, "HeartRate:");
    // OLED_ShowString(3, 1, "BloodOxygen:");
    //E53IA1IoInit();
    return 0;
}

/***************************************************************
读取gpio_11的电平状态
 ***************************************************************/
int E53IA1ReadData(E53IA1Data *ReadData)
{
    int Temp;//读取数据
    float T;//真实温度
    //int Tp;//小数
    JX90614_Init();
    JX90614_GetTemp(&Temp);
	T = (float)(Temp/ 16384.0)+FIX;				//2^14
    ReadData->Temperature = T;
    ReadData->BMI = (float)(ReadData->Weight)/((float)(ReadData->Height*ReadData->Height)/10000);
    return 0;
}
void E53_ShowData(E53IA1Data data)
{
    OLED_ShowSignedNum(1, 14, data.Temperature, 2);
    OLED_ShowNum(2, 15, data.HeartRate, 2);
    OLED_ShowNum(3, 15, data.BloodOxygen, 2);
}
float E53_Weight(char* w_str)
{
	float weight = 0;
    weight = (w_str[0]-'0')*10+(w_str[1]-'0')*1 + (w_str[3]-'0')*0.1;
    //printf("weight = %d\n", weight);
	return weight;
}
void E53_Float_To_String(float FLOAT, char* DATA_S)
{
	DATA_S[0] = (int)FLOAT /10%10 + '0';
	DATA_S[1] = (int)FLOAT % 10 + '0';
	DATA_S[2] = '.';
	DATA_S[3] = (int)(FLOAT * 10) % 10 + '0';
	DATA_S[4] = '\0';
}

void E53_Height_To_String(int height, char* str)
{
	str[0] = height / 100 % 10 + '0';
    str[1] = '.';
	str[2] = height / 10 % 10 + '0';
    str[3] = height / 1 % 10 + '0';
	str[4] = '\0';
}

void E53_DealData(E53IA1Data data)
{
    unsigned char SendStr[50];
    int i = 0;
    SendStr[i++] = 'H';
    SendStr[i++] = data.HeartRate/10%10 + '0';
    SendStr[i++] = data.HeartRate/1%10 + '0';
    SendStr[i++] = 'O';
    SendStr[i++] = data.BloodOxygen/10%10 + '0';
    SendStr[i++] = data.BloodOxygen/1%10 + '0';
    SendStr[i++] = 'G';
    SendStr[i++] = data.Height/100%10 + '0';
    SendStr[i++] = data.Height/10%10 + '0';
    SendStr[i++] = data.Height/1%10 + '0';
    SendStr[i++] = 'T';
    SendStr[i++] = (int)data.Temperature/10%10 + '0';
    SendStr[i++] = (int)data.Temperature/1%10 + '0';
    SendStr[i++] = '.';
    SendStr[i++] = (int)(data.Temperature*10)/1%10 + '0';
    SendStr[i++] = 'W';
    SendStr[i++] = (int)data.Weight/10%10 + '0';
    SendStr[i++] = (int)data.Weight/1%10 + '0';
    SendStr[i++] = '.';
    SendStr[i++] = (int)(data.Weight*10)/1%10 + '0';
    SendStr[i++] = 'B';
    SendStr[i++] = (int)data.BMI/10%10 + '0';
    SendStr[i++] = (int)data.BMI/1%10 + '0';
    SendStr[i++] = '.';
    SendStr[i++] = (int)(data.BMI*10)/1%10 + '0';

    SendStr[i++] = '\0';
    MY_USART1_SendString(SendStr);
    printf("*********************************************************************************\r\n");
    printf("Temperature:        %.1f\r\n", data.Temperature);
    printf("HeartRate:          %d\r\n", data.HeartRate);
    printf("BloodOxygen:        %d\r\n", data.BloodOxygen);
    printf("Height:             %d\r\n", data.Height);
    printf("Weight:             %.1f\r\n", data.Weight);
    printf("BMI:                %.1f\r\n", data.BMI);

}