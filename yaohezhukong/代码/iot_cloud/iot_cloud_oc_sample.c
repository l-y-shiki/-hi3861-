#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "cmsis_os2.h"
#include "ohos_init.h"
#include "lwip/sockets.h"
#include "wifi_connect.h"

#include "E53_IA1.h"
#include "oc_mqtt.h"
#include "JX90614.h"
#include "MY_USART1.h"
#include "oled.h"
#include "heart.h"

#define MSGQUEUE_COUNT 16 // number of Message Queue Objects
#define MSGQUEUE_SIZE 10
#define CLOUD_TASK_STACK_SIZE (1024 * 10)
#define CLOUD_TASK_PRIO 24
#define SENSOR_TASK_STACK_SIZE (1024 * 2)
#define SENSOR_TASK_PRIO 25
#define TASK_DELAY_3S 3

#define UART_BUFF_SIZE 1024//接收血氧数据

typedef struct { // object data type
    char *Buf;
    uint8_t Idx;
} MSGQUEUE_OBJ_t;

MSGQUEUE_OBJ_t msg;
osMessageQueueId_t mid_MsgQueue; // message queue id

#define CLIENT_ID "bearpi"
#define USERNAME "fc70ac3fa5e8792e875f23c7a88b5f4b"
#define PASSWORD "123123"

// #define WIFI_ACCOUNT "OrayBox-2022"//wifi账号
// #define WIFI_PASSWORD "13648400889"//wifi密码
#define WIFI_ACCOUNT "kiss"//wifi账号
#define WIFI_PASSWORD "13215008017"//wifi密码

#define MEDICINE_NUM 4//药品数量

E53IA1Data rdata;//采集的数据

typedef enum {
    en_msg_cmd = 0,
    en_msg_report,
} en_msg_type_t;

typedef struct {
    char *request_id;
    char *payload;
} cmd_t;

typedef struct {
    int Temperature;
} report_t;

typedef struct {
    en_msg_type_t msg_type;
    union {
        cmd_t cmd;
        report_t report;
    } msg;
} app_msg_t;

typedef struct {
    int connected;
    int led;
} app_cb_t;
static app_cb_t g_app_cb;
//*****************************药盒
typedef struct{
    int clock_status;   //药盒时钟0表示关闭，1表示打开
    char command[20];
}medicine_t;
static medicine_t medicine[MEDICINE_NUM];

void medicine_init(void)
{
    for(int i = 0; i < MEDICINE_NUM; i++)
    {
        medicine[i].clock_status = 0;
    }
}

char w[10] = "75.4";//体重字符串

void MsgRcvCallback(uint8_t *recv_data, size_t recv_size, uint8_t **resp_data, size_t *resp_size)
{
    app_msg_t *app_msg;

    int ret = 0;
    app_msg = malloc(sizeof(app_msg_t));
    app_msg->msg_type = en_msg_cmd;
    app_msg->msg.cmd.payload = (char *)recv_data;

    printf("recv data is %s\n", recv_data);
    ret = osMessageQueuePut(mid_MsgQueue, &app_msg, 0U, 0U);//添加队列
    if (ret != 0) {
        free(recv_data);
    }
    *resp_data = NULL;
    *resp_size = 0;
}

static void ReportMsg(report_t *report)//上传json数据的拼装
{
    oc_mqtt_profile_service_t service;
    oc_mqtt_profile_kv_t Temperature;
    oc_mqtt_profile_kv_t Weight;
    oc_mqtt_profile_kv_t HeartRate;
    oc_mqtt_profile_kv_t BloodOxygen;
    oc_mqtt_profile_kv_t BMI;

    //json数据的拼装
    service.event_time = NULL;
    service.service_id = "Health_System";//服务id，与华为云上要一致
    service.service_property = &BMI;
    service.nxt = NULL;

    char BMIString[10];
    char WeightString[10];
    E53_Float_To_String(rdata.BMI, BMIString);
    BMI.key = "BMI";
    BMI.value = BMIString;
    BMI.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    BMI.nxt = &BloodOxygen;

    BloodOxygen.key = "BloodOxygen";
    BloodOxygen.value = &rdata.BloodOxygen;
    BloodOxygen.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    BloodOxygen.nxt = &HeartRate;

    HeartRate.key = "HeartRate";
    HeartRate.value = &rdata.HeartRate;
    HeartRate.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    HeartRate.nxt = &Weight;

    E53_Float_To_String(rdata.Weight, WeightString);
    Weight.key = "Weight";
    Weight.value = WeightString;
    Weight.type = EN_OC_MQTT_PROFILE_VALUE_STRING;
    Weight.nxt = &Temperature;
    //上下对应连接
    Temperature.key = "Temperature";
    Temperature.value = &report->Temperature;//&report->Temperature;
    Temperature.type = EN_OC_MQTT_PROFILE_VALUE_INT;
    Temperature.nxt = NULL;

    int statue = oc_mqtt_profile_propertyreport(USERNAME, &service);//上报数据
    if(statue == en_oc_mqtt_err_ok)
    {
        printf("上报成功\n");
    }
    else{
        printf("上报失败\n");
        //oc_mqtt_entry();
    }
    return;
}

static void oc_cmdresp(cmd_t *cmd, int cmdret)
{
    oc_mqtt_profile_cmdresp_t cmdresp;
    ///< do the response
    cmdresp.paras = NULL;
    cmdresp.request_id = cmd->request_id;
    cmdresp.ret_code = cmdret;
    cmdresp.ret_name = NULL;
    (void)oc_mqtt_profile_cmdresp(NULL, &cmdresp);
}
///< COMMAND DEAL
#include <cJSON.h>
static void DealCmdMsg(cmd_t *cmd)//命令json解析
{
    cJSON *obj_root, *obj_cmdname, *obj_paras, *obj_para;

    int cmdret = 1;

    obj_root = cJSON_Parse(cmd->payload);
    if (obj_root == NULL) {
        oc_cmdresp(cmd, cmdret);
    }

    obj_cmdname = cJSON_GetObjectItem(obj_root, "command_name");
    if (obj_cmdname == NULL) {
        cJSON_Delete(obj_root);
    }
    //定闹钟
    if (strcmp(cJSON_GetStringValue(obj_cmdname), "Medicine_Clock") == 0) 
    {
        obj_paras = cJSON_GetObjectItem(obj_root, "Paras");
        if (obj_paras == NULL) {
            cJSON_Delete(obj_root);
        }
        obj_para = cJSON_GetObjectItem(obj_paras, "Clock");
        if (obj_para == NULL) {
            cJSON_Delete(obj_root);
        }
        char* command = cJSON_GetStringValue(obj_para);//指令赋值
        printf("s = %s\n", command);//收到命令下发信息
        MY_USART1_SendString(command);//发送命令给32
        
        medicine[command[0] - '0'].clock_status = 1;//药盒时钟开启
        strcpy(medicine[command[0] - '0'].command, command);//记录药盒指令
        printf("command = %s\n", medicine[command[0] - '0'].command);

        cmdret = 0;
    }
    else if(strcmp(cJSON_GetStringValue(obj_cmdname), "Height") == 0) 
    {
        obj_paras = cJSON_GetObjectItem(obj_root, "Paras");
        if (obj_paras == NULL) {
            cJSON_Delete(obj_root);
        }
        obj_para = cJSON_GetObjectItem(obj_paras, "height");
        if (obj_para == NULL) {
            cJSON_Delete(obj_root);
        }
        rdata.Height = cJSON_GetNumberValue(obj_para);
        printf("height = %d\n", rdata.Height);
        cmdret = 0;
    }

    cJSON_Delete(obj_root);
}

static int CloudMainTaskEntry(void)
{
    app_msg_t *app_msg;

    uint32_t ret = WifiConnect(WIFI_ACCOUNT, WIFI_PASSWORD);//连接wifi

    device_info_init(CLIENT_ID, USERNAME, PASSWORD);
    //oc_mqtt_init();
    oc_mqtt_entry();
    oc_set_cmd_rsp_cb(MsgRcvCallback);//设置命令回调函数（收到华为云的信息进入函数添加队列）
    extern uint8_t recv_flag;
    extern char recv_data[512];
    while (1) {
        app_msg = NULL;
        (void)osMessageQueueGet(mid_MsgQueue, (void **)&app_msg, NULL, 0U);//循环获取消息
        if (app_msg != NULL) {
            switch (app_msg->msg_type) {
                case en_msg_cmd:
                    printf("接收到指令\n");
                    DealCmdMsg(&app_msg->msg.cmd);//接收指令
                    break;
                case en_msg_report:
                    ReportMsg(&app_msg->msg.report);//上报数据
                    break;
                default:
                    break;
            }
            free(app_msg);
        }
        if(recv_flag)
        {
            //printf("recv_flag:%d\n", recv_flag);
            //printf("recv_data:%s\n", recv_data);
            app_msg_t *app_msg;
            app_msg = malloc(sizeof(app_msg_t));
            app_msg->msg_type = en_msg_cmd;
            app_msg->msg.cmd.payload = (char *)recv_data;
            printf("recv data is %s\n", recv_data);
            ret = osMessageQueuePut(mid_MsgQueue, &app_msg, 0U, 0U);//添加队列
            recv_flag = 0;
        }
    }
    return 0;
}

static int SensorTaskEntry(void)//采集数据
{
    int ret;
    app_msg_t *app_msg;
    medicine_init();//药盒初始化
    ret = E53IA1Init(&rdata);
    if (ret != 0) {
        printf("E53_IA1 Init failed!\r\n");
        return;
    }
    while (1) {
        ret = E53IA1ReadData(&rdata);//读取数据
        E53_DealData(rdata);//数据传给32
        //E53_ShowData(rdata);//显示数据
        if (ret != 0) {
            printf("E53_IA1 Read Data failed!\r\n");
            return;
        }
        app_msg = malloc(sizeof(app_msg_t));
        if (app_msg != NULL) {//给发送的信息赋值
            app_msg->msg_type = en_msg_report;
            app_msg->msg.report.Temperature = (int)rdata.Temperature;
            if (osMessageQueuePut(mid_MsgQueue, &app_msg, 0U, 0U) != 0) {//发送消息
                free(app_msg);
            }
        }
        sleep(TASK_DELAY_3S);
    }
    return 0;
}

#define TCP_TASK_STACK_SIZE (1024 * 10)
#define CONFIG_CLIENT_PORT 8888      // 要连接的服务器端口
#define TCP_BACKLOG 10

static void TCPServerTask(void)
{
    //sleep(60);//睡眠60s
    printf("wake!!!!!!\n");

    // 在sock_fd 进行监听，在 new_fd 接收新的链接
    int sock_fd, new_fd;

    // 服务端地址信息
    struct sockaddr_in server_sock;

    // 客户端地址信息
    struct sockaddr_in client_sock, *cli_addr;
    int sin_size;

    // 创建socket
    /*
    参数1：ipv4
    参数2：tcp协议
    参数3：第二个参数定义过了，通常就填0
    */
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket is error\r\n");
        return;
    }
    //填入bind的信息
    bzero(&server_sock, sizeof(server_sock));
    server_sock.sin_family = AF_INET;
    server_sock.sin_addr.s_addr = htonl(INADDR_ANY);//ip地址为0.0.0.0，等待分配
    server_sock.sin_port = htons(CONFIG_CLIENT_PORT);//设置端口号

    // 调用bind函数绑定socket和地址
    if (bind(sock_fd, (struct sockaddr *)&server_sock, sizeof(struct sockaddr)) == -1) {
        return;
    }

    // 调用listen函数监听(指定port监听)
    if (listen(sock_fd, TCP_BACKLOG) == -1) {
        return;
    }

    printf("start accept\n");

    // 调用accept函数从队列中
    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sock_fd, (struct sockaddr *)&client_sock, (socklen_t *)&sin_size)) == -1) {
            perror("accept");
            continue;
        }

        cli_addr = malloc(sizeof(struct sockaddr));

        printf("accept addr\r\n");

        if (cli_addr != NULL) {
            int ret;
            if  (ret = memcpy_s(cli_addr, sizeof(struct sockaddr), &client_sock, sizeof(struct sockaddr)) != 0) {
                perror("memcpy is error\r\n");
                return;
            }
        }
        // 处理目标
        ssize_t ret;
        char recvbuf[512];//接收的数据
        char buf[10];//上报的数据
        while (1) {
            
            memset_s(recvbuf, sizeof(recvbuf), 0, sizeof(recvbuf));//数据清零
            printf("ready receive\n");
            if ((ret = recv(new_fd, recvbuf, sizeof(recvbuf), 0)) == -1) {
                printf("recv error \r\n");
            }
            printf("recv :%s\r\n", recvbuf);//收到体重数据
            strcpy(w, recvbuf);
            rdata.Weight = E53_Weight(w);
            // printf("weight = %d\n", rdata.Weight);
            //  sleep(2);
            E53_Height_To_String(rdata.Height, buf);
            if ((ret = send(new_fd, buf, strlen(buf) + 1, 0)) == -1) {
                perror("send : ");
            }
            printf("ret = %d, send_buf = %s\n", ret, buf);
        }
        close(new_fd);
    }
}

static void HeartTask(void)//测量心率血氧任务
{
    uint8_t uart_buff[UART_BUFF_SIZE] = {0};
    uint8_t *uart_buff_ptr = uart_buff;
    uint8_t ret;
    uint16_t rxlen;
    rdata.HeartRate = 0;
    rdata.BloodOxygen = 0;
    Heart_Init();
    while (1)
    {
        //阻塞等待串口1数据
        rxlen=uart1_recv(uart_buff_ptr, UART_BUFF_SIZE);   
        if(rxlen==88)
        {
            //根据传感器说明书，第一个字节为0XFF,最后一个字节的bit2为0时候，数据才能使用
            if(uart_buff_ptr[0]==0xFF && (uart_buff_ptr[87] & 0x04) == 0)
            {
                if((uart_buff_ptr[65] != 0) && (uart_buff_ptr[66] != 0))
                {
                    rdata.HeartRate = (int)uart_buff_ptr[65];
                    rdata.BloodOxygen = (int)uart_buff_ptr[66];
                    printf("##########################################\n");
                    printf("HeartRate = %d\r\n",rdata.HeartRate);
                    printf("BloodOxygen = %d\r\n",rdata.BloodOxygen);
                }
            }
            else
            {
                //printf("value error\r\n");
            }
        }
        else
        {
            //printf("uart recv error\r\n");
        }
        usleep(2);
    }
}
static void IotMainTaskEntry(void)
{
    mid_MsgQueue = osMessageQueueNew(MSGQUEUE_COUNT, MSGQUEUE_SIZE, NULL);//创建消息队列
    if (mid_MsgQueue == NULL) {
        printf("Failed to create Message Queue!\n");
    }

    osThreadAttr_t attr;

    attr.name = "CloudMainTaskEntry";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = CLOUD_TASK_STACK_SIZE;
    attr.priority = CLOUD_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)CloudMainTaskEntry, NULL, &attr) == NULL) {
        printf("Failed to create CloudMainTaskEntry!\n");
    }
    attr.stack_size = SENSOR_TASK_STACK_SIZE;
    attr.priority = SENSOR_TASK_PRIO;                   //高优先级
    attr.name = "SensorTaskEntry";
    if (osThreadNew((osThreadFunc_t)SensorTaskEntry, NULL, &attr) == NULL) {
        printf("Failed to create SensorTaskEntry!\n");
    }
    //TCP线程
    attr.name = "TCPServerTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size =  TCP_TASK_STACK_SIZE;
    attr.priority = CLOUD_TASK_PRIO;
    if (osThreadNew((osThreadFunc_t)TCPServerTask, NULL, &attr) == NULL) {
        printf("[TCPServerDemo] Failed to create TCPServerTask!\n");
    }
    //心率血氧采集
    attr.name = "HeartTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 2048;
    attr.priority = CLOUD_TASK_PRIO;
    if (osThreadNew((osThreadFunc_t)HeartTask, NULL, &attr) == NULL) {
        printf(" Failed to create Heart Task!\n");
    }

}

APP_FEATURE_INIT(IotMainTaskEntry);