
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>

#include "cmsis_os2.h"
#include "iot_errno.h"
#include "iot_gpio.h"
#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "OLED_I2C.h"
#include "i2c_gpio.h"

#include "lcd.h"
#include "lcdfont.h"
#include "pic.h"
#include "lcd_init.h"



#include "wifi_device.h"
#include "lwip/netifapi.h"
#include "lwip/api_shell.h"

#include "lwip/sockets.h"
#include "wifi_connect.h"



#define ADC_TASK_STACK_SIZE (1024 * 8)
#define ADC_TASK_PRIO 24
#define HX711_GPIO_DT 11
#define HX711_GPIO_SCK 12
#define ADC_CHANNEL 1
#define ADC_TASK_DELAY_1S 100000

#define ADC_VREF_VOL 1.8
#define ADC_COEFFICIENT 4
#define ADC_RATIO 4096


static const char send_data[20];

static double tall=1.78;
static float bmi=0;
static int32_t num;
char str1[10];
int a,b;
double weight;

#define TASK_STACK_SIZE (1024 * 10)
#define TASK_DELAY_10S 2
#define CONFIG_WIFI_SSID            "kiss"                            // 要连接的WiFi 热点账号
#define CONFIG_WIFI_PWD             "13215008017"                          // 要连接的WiFi 热点密码
#define CONFIG_SERVER_IP            "192.168.254.227"                     // 要连接的服务器IP
#define CONFIG_SERVER_PORT          8888                                // 要连接的服务器端口




/**
 * @brief get ADC sampling value and convert it to voltage
 *
 */



//GPIO初始化
void LCD_GPIO_INIT()
{
    //SCL初始化
	IoTGpioInit(OLED_I2C_SCL_PIN);
    IoTGpioSetFunc(OLED_I2C_SCL_PIN,0);
    IoTGpioSetDir(OLED_I2C_SCL_PIN, IOT_GPIO_DIR_OUT);
    //SDA初始化
	IoTGpioInit(OLED_I2C_SDA_PIN);
    IoTGpioSetFunc(OLED_I2C_SDA_PIN,0);
    IoTGpioSetDir(OLED_I2C_SDA_PIN, IOT_GPIO_DIR_OUT);
    //REF初始化
    IoTGpioInit(OLED_I2C_RES_PIN);
    IoTGpioSetFunc(OLED_I2C_RES_PIN,0);
    IoTGpioSetDir(OLED_I2C_RES_PIN, IOT_GPIO_DIR_OUT);
    //DC初始化
    IoTGpioInit(OLED_I2C_DC_PIN);
    IoTGpioSetFunc(OLED_I2C_DC_PIN,0);
    IoTGpioSetDir(OLED_I2C_DC_PIN, IOT_GPIO_DIR_OUT);
    //CS初始化
    IoTGpioInit(OLED_I2C_CS_PIN);
    IoTGpioSetFunc(OLED_I2C_CS_PIN,0);
    IoTGpioSetDir(OLED_I2C_CS_PIN, IOT_GPIO_DIR_OUT);
    //BLK初始化
    IoTGpioInit(OLED_I2C_BLK_PIN);
    IoTGpioSetFunc(OLED_I2C_BLK_PIN,0);
    IoTGpioSetDir(OLED_I2C_BLK_PIN, IOT_GPIO_DIR_OUT);

}


static void HX711_Init(void)
{
    //SCK端口初始化
    IoTGpioInit(HX711_GPIO_SCK);
    IoTGpioSetFunc(HX711_GPIO_SCK,0);
    IoTGpioSetDir(HX711_GPIO_SCK, IOT_GPIO_DIR_OUT);
     IoTGpioSetOutputVal(HX711_GPIO_SCK,1);
    //DataOUT端口初始化
     IoTGpioInit(HX711_GPIO_DT);
    IoTGpioSetFunc(HX711_GPIO_DT,0);
     IoTGpioSetDir(HX711_GPIO_DT, IOT_GPIO_DIR_IN);
   // IoTGpioSetPull(HX711_GPIO_DT,IOT_GPIO_PULL_NONE);
}

static uint32_t HX711_read(void)
{
    unsigned long Count;
    unsigned char i;
    IoTGpioSetOutputVal(HX711_GPIO_SCK,0);
    Count=0;
    IotGpioValue value={0},value2={0};
        IoTGpioGetOutputVal(HX711_GPIO_SCK, &value2);
        printf("a||||\n");
    printf("bbbb:%d\n",value2);
    IoTGpioGetInputVal(HX711_GPIO_DT, &value2);
    printf("%d\n",value2);
    while(value2==1){
        IoTGpioGetInputVal(HX711_GPIO_DT, &value2);
       // printf("%d\n",value2);
    }
    for (i=0;i<24;i++){
   IoTGpioSetOutputVal(HX711_GPIO_SCK,1);
    Count<<=1;
   // printf(" Count %d:",Count);
    IoTGpioSetOutputVal(HX711_GPIO_SCK,0);
    IoTGpioGetInputVal(HX711_GPIO_DT, &value2);
   // printf("%d\n",value2);
    if(value2==1)
     Count++;
 }
    //printf("%d",Count);
    IoTGpioSetOutputVal(HX711_GPIO_SCK,1);
    IoTGpioSetOutputVal(HX711_GPIO_SCK,0);
    IoTGpioSetOutputVal(HX711_GPIO_SCK,1);
    IoTGpioSetOutputVal(HX711_GPIO_SCK,0);

    //Count=Count^0x800000; 
    return Count;
}


//一位小数转字符串
void E53_Float_To_String(double FLOAT, char* DATA_S)
{
	DATA_S[0] = (int)FLOAT /10%10 + '0';
	DATA_S[1] = (int)FLOAT % 10 + '0';
	DATA_S[2] = '.';
	DATA_S[3] = (int)(FLOAT * 10) % 10 + '0';
	DATA_S[4] = '\0';
}


/**
 * @brief Adc task get adc sampling voltage
 *
 */
static void HX711Task(void)
{
     

     //0.96寸显示
     
/*
     i2c_CfgGpio();
    usleep(1000000);
    OLED_Init();
    if(OLED_Test()==0)
		{          
			// 没有检测到EEPROM 
			printf("\r\nno！\r\n");
			
		}
		else
		{
			printf("\r\n yes！\r\n");
			
		}
         OLED_Fill(0x00);
         for(int i=0;i<2;i++)
		{
			OLED_ShowCN(8+i*16,1,i);		//心率
		}
     */
      
        
        HX711_Init();
        
        //1.8寸显示
        LCD_GPIO_INIT();
        Lcd_Init();
        LCD_Fill(0,0,LCD_W,LCD_H,BLACK);
       

    // 总计发送 count 次数据
   
    while(1)
    { 
        // bzero(recvBuf, sizeof(recvBuf));

        // // 发送数据到服务远端
        // //sendto(sock_fd, send_data, strlen(send_data), 0, (struct sockaddr *)&send_addr, addr_length);

        // send(sock_fd, send_data, strlen(send_data), 0);
        // // 线程休眠一段时间
        // //sleep(TASK_DELAY_10S);

        // // 接收服务端返回的字符串
        // //recvfrom(sock_fd, recvBuf, sizeof(recvBuf), 0, (struct sockaddr *)&send_addr, &addr_length);
        // recv(sock_fd, recvBuf, sizeof(recvBuf), 0);
        //  printf("%s:%d=>%s\n", inet_ntoa(send_addr.sin_addr), ntohs(send_addr.sin_port), recvBuf);

        num=HX711_read();
        printf("num:%d\n",num);
        if(num<9000000)
        {
            num+=273838;
            num+=21882;
        }
        
        if(num>8000000)
        {
             num-=16511220;
        }
        
        weight=num*1.0;
        weight/=124;
        weight*=17;
         printf("weight:%lfg\n",weight);
        weight/=1000;
        weight*=10;
        int a=(int)weight%10;
        weight=(int)weight/10+a*0.1;
        weight-=0.4;
        printf("weight:%lfkg\n",weight);

        bmi=weight;
            bmi/=tall;
            bmi/=tall;
            printf("bmi:%f",bmi);
            if(weight>20.0)
            {
             E53_Float_To_String(weight,send_data);
             printf("ok");
            printf("%s\n",send_data);
            }

       // usleep(ADC_TASK_DELAY_1S); 
       
       LCD_ShowString(3,34,"weight:",RED,BLACK,32,0);
		//LCD_ShowIntNum(3,34,num,3,RED,BLACK,32);
        //LCD_ShowFloatNum1(1,34,weight,4,RED,BLACK,32);
        LCD_ShowFloatNum2(1,68,weight,3,RED,BLACK,32);
        LCD_ShowString(80,68,"kg",RED,BLACK,32,0);
        LCD_ShowString(1,104,"bmi:",RED,BLACK,32,0);
       // LCD_ShowIntNum(70,68,(int)bmi,3,RED,BLACK,32);
        LCD_ShowFloatNum2(61,104,bmi,3,RED,BLACK,32);
    

    }

}

//任务二：传收数据
static void Thread2()
{

         int m = 0;
	int n = 0;
    double number;
	double integer = 0; //整数部分(为了相加)
	char strInt[6] = {0}; //整数字符串
	double decimal = 0; //小数部分
	char strDecimal[6] = {}; //小数字符串
	int IntdecimalPoint = 0; //是否已过整数小数点
	int DoudecimalPoint = 0; //是否已到小数小数点int


    //
    //strcpy(send_data,"63.2");
       // 在sock_fd 进行监听，在 new_fd 接收新的链接
    int sock_fd;

    // 服务器的地址信息
    struct sockaddr_in send_addr;
    socklen_t addr_length = sizeof(send_addr);
    char recvBuf[512];

    // 连接Wifi
    WifiConnect(CONFIG_WIFI_SSID, CONFIG_WIFI_PWD);

    // 创建socket
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("create socket failed!\r\n");
        return;
    }
printf("99999999999999999999\n");
    // 初始化预连接的服务端地址
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(CONFIG_SERVER_PORT);
    send_addr.sin_addr.s_addr = inet_addr(CONFIG_SERVER_IP);
    addr_length = sizeof(send_addr);

    connect(sock_fd,(struct sockaddr *)&send_addr, addr_length);
    printf("a1");
    if(connect(sock_fd,(struct sockaddr *)&send_addr, addr_length)==-1)
    printf("a2");
     
    while(1)
    {
 

    // 总计发送 count 次数据

     //向主机传数据
             
              bzero(recvBuf, sizeof(recvBuf));
              printf("99999999999999999999\n");
            // 发送数据到服务远端
              //send(sock_fd, /*send_data*/  str1, strlen(str1), 0);

              send(sock_fd, send_data, strlen(send_data), 0);

              printf("OKKKKKK****************************\n");
            printf("%s\n",send_data);
              // 线程休眠一段时间
             sleep(TASK_DELAY_10S);

             // 接收服务端返回的字符串
       recv(sock_fd, recvBuf, sizeof(recvBuf), 0);
       printf("%s:%d=>%s\n", inet_ntoa(send_addr.sin_addr), ntohs(send_addr.sin_port), recvBuf);

       usleep(ADC_TASK_DELAY_1S); 

        //将字符串转化成两位小数
        number=recvBuf[0]-'0'+0.1*(recvBuf[2]-'0')+0.01*(recvBuf[3]-'0');

    //将字符串转化成小数
	// for (int i = 0; i < strlen(recvBuf); i++) { //求整数字符串和小数字符串

	// 	if (recvBuf[i] == '.') { //已到整数小数点
	// 		IntdecimalPoint = 1; //设为true
	// 	}
	// 	if (recvBuf[i - 1] == '.') { //已到小数小数点
	// 		DoudecimalPoint = 1; //设为true
	// 	}
	// 	if (!IntdecimalPoint) { //检测是否已过整数小数点

	// 		strInt[m] = recvBuf[i]; //整数字符串 += 数字型字符
	// 		m++;
	// 	}
	// 	if (DoudecimalPoint) { //检测是否已到小数小数点
	// 		strDecimal[n] = recvBuf[i]; //小数字符串 += 数字型字符
	// 		n++;
	// 	}
	// }
	// for (int i = 0; i < m; i++) { //把整数字符串转换为整数
	// 	integer += (strInt[i] - '0') * pow(10, m - i - 1);
	// }
	// for (int i = 0; i < n; i++) { //把小数字符串转换为小数
	// 	//decimal += (strDecimal[i] - '0') * pow(10, 0 - i - 1);
    //     decimal += (strDecimal[i] - '0') * pow(0.1, 1+i);
	// }
	// double number = integer + decimal; //加起来

     tall=number;
	printf("NUMER:%lf", number);
   //bmi计算
            bmi=weight;
            bmi/=tall;
            bmi/=tall;
            printf("bmi:%f",bmi);
        
        a=num/10;
        b=num%10;
        str1[0] = a+'0';
        str1[1] = b+'0'; 
        str1[2] = '\0';
         printf("%d\n",a);
        printf("%c",str1);

        usleep(ADC_TASK_DELAY_1S);
    }
    
}

/**
 * @brief Main Entry of the Adc Example
 *
 */
static void HX711ExampleEntry(void)
{
    osThreadAttr_t attr;

    attr.name = "HX711Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = ADC_TASK_STACK_SIZE;
    attr.priority = ADC_TASK_PRIO;


     if (osThreadNew((osThreadFunc_t)HX711Task, NULL, &attr) == NULL) {
        printf("Failed to create HX711Task!\n");
     }

     attr.name = "Thread2";
    if (osThreadNew((osThreadFunc_t)Thread2, NULL, &attr) == NULL) {
        printf("Failed to create Thread2!\n");
    }

}

APP_FEATURE_INIT(HX711ExampleEntry);