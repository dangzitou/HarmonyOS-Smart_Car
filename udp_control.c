#include "hi_wifi_api.h"
#include "lwip/ip_addr.h"
#include "lwip/netifapi.h"
#include "lwip/sockets.h"
#include "lwip/netif.h"
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "cJSON.h"

#include "udp_control.h"
#include "robot_control.h"
#include "robot_l9110s.h"

extern unsigned int MOVING_STATUS;
extern unsigned char g_car_status;
char recvline[1024];

// 添加状态变量，避免重复执行相同指令
static int last_moving_status = -1;
static unsigned long last_command_time = 0;

void cotrl_handle(char *recvline, int ret)
{
    cJSON *recvjson;
    //进行json解析
    recvjson = cJSON_Parse(recvline);
    printf("Enter cotrl_handle\r\n");
    if(recvjson != NULL)
    {
        cJSON *modeItem = cJSON_GetObjectItem(recvjson, "mode");
        cJSON *cmdItem = cJSON_GetObjectItem(recvjson, "cmd");
        printf("Processing message...\r\n");
        // 如果有mode字段，处理模式切换
        if(modeItem != NULL && modeItem->valuestring != NULL)
        {
            if(strcmp("stop", modeItem->valuestring) == 0)
            {
                printf("stop mode\r\n");
                g_car_status = CAR_STOP_STATUS;
            }

            if(strcmp("obstacle_avoidance", modeItem->valuestring) == 0)
            {
                printf("obstacle avoidance mode\r\n");
                g_car_status = CAR_OBSTACLE_AVOIDANCE_STATUS;
            }

            if(strcmp("trace", modeItem->valuestring) == 0)
            {
                printf("trace mode\r\n");
                g_car_status = CAR_TRACE_STATUS;
            }

            if(strcmp("control", modeItem->valuestring) == 0)
            {
                printf("remote control mode\r\n");
                g_car_status = CAR_CONTROL_STATUS;
                // 如果同时有cmd，直接处理控制指令
                if(cmdItem != NULL && cmdItem->valuestring != NULL)
                {
                    udp_control(recvline);
                }
            }
        }
        // 如果只有cmd字段（控制指令），直接处理
        else if(cmdItem != NULL && cmdItem->valuestring != NULL)
        {
            printf("Processing control command\r\n");
            udp_control(recvline);
        }
        else
        {
            printf("No valid message detected\r\n");
        }

        cJSON_Delete(recvjson);
    }
}

void udp_control(char *recvline){
    cJSON *recvjson;
    //进行json解析
    recvjson = cJSON_Parse(recvline);

    printf("enter udp_control\r\n");

    if(recvjson != NULL)
    {
        cJSON *cmdItem = cJSON_GetObjectItem(recvjson, "cmd");
        if(cmdItem != NULL && cmdItem->valuestring != NULL)
        {
            printf("cmd : %s\r\n", cmdItem->valuestring);
            // 确保在远控模式下才响应控制指令
            if (g_car_status != CAR_CONTROL_STATUS) {
                printf("Not in remote control mode, ignore control commands\r\n");
                cJSON_Delete(recvjson);
                return;
            }
            
            if(strcmp("forward", cmdItem->valuestring) == 0)
            {
                car_forward();
                MOVING_STATUS = 3;
                printf("forward\r\n");
            }
            else if(strcmp("backward", cmdItem->valuestring) == 0)
            {
                car_backward();
                MOVING_STATUS = 5;
                printf("backward\r\n");
            }
            else if(strcmp("left", cmdItem->valuestring) == 0)
            {
                car_left();
                MOVING_STATUS = 2;
                printf("left\r\n");
            }
            else if(strcmp("right", cmdItem->valuestring) == 0)
            {
                car_right();
                MOVING_STATUS = 1;
                printf("right\r\n");
            }
            else if(strcmp("stop", cmdItem->valuestring) == 0)
            {
                car_stop();
                MOVING_STATUS = 0;
                printf("stop\r\n");
            }
            // 新增：处理速度指令
            else if(strcmp("speed", cmdItem->valuestring) == 0)
            {
                cJSON *valueItem = cJSON_GetObjectItem(recvjson, "value");
                if (valueItem && cJSON_IsNumber(valueItem)) {
                    extern unsigned short SPEED_FORWARD;
                    SPEED_FORWARD = (unsigned short)valueItem->valueint;
                    printf("Set SPEED_FORWARD to %d\r\n", SPEED_FORWARD);
                } else {
                    printf("speed command missing or invalid value\r\n");
                }
            }
            else
            {
                printf("Unknown command: %s\r\n", cmdItem->valuestring);
            }
        }
        else
        {
            printf("No cmd found in JSON\r\n");
        }
        cJSON_Delete(recvjson);
    }
    else
    {
        printf("Failed to parse JSON in udp_control\r\n");
    }
}

void udp_thread(void *pdata)
{
    int ret;
    struct sockaddr_in servaddr;
    static int thread_initialized = 0; // 防止重复初始化

    pdata = pdata;

    // 防止重复启动线程
    if (thread_initialized) {
        printf("UDP thread already initialized, exiting...\r\n");
        return;
    }
    thread_initialized = 1;

    // 等待网络配置完成
    printf("udp_thread started, waiting for network...\r\n");
    osDelay(2000);

    int sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("Socket creation failed!\r\n");
        return;
    }

    // 设置socket选项，允许地址重用
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        printf("Set socket option failed!\r\n");
    }
 
    //服务器 ip port
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(50001);

    // 多次尝试获取有效IP
    int ip_found = 0;
    for(int i = 0; i < 10; i++) {
        struct netif *netif = netif_default;
        if(netif != NULL) {
            u32_t ip = ip4_addr_get_u32(ip_2_ip4(&netif->ip_addr));
            if(ip != 0 && ip != 0xFFFFFFFF) {
                // 正确显示IP地址信息
                printf("Car IP: %s\r\n", ip4addr_ntoa(ip_2_ip4(&netif->ip_addr)));
                printf("Netmask: %s\r\n", ip4addr_ntoa(ip_2_ip4(&netif->netmask)));
                printf("Gateway: %s\r\n", ip4addr_ntoa(ip_2_ip4(&netif->gw)));
                ip_found = 1;
                break;
            }
        }
        printf("Waiting for valid IP... attempt %d\r\n", i+1);
        osDelay(1000);
    }

    if (!ip_found) {
        printf("Failed to get valid IP address after 10 attempts\r\n");
        close(sockfd);
        return;
    }

    if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        printf("Bind failed! Port 50001 may already be in use\r\n");
        close(sockfd);
        return;
    }
    
    printf("UDP server successfully bound and listening on port 50001\r\n");
    
    while(1)
    {
        struct sockaddr_in addrClient;
        int sizeClientAddr = sizeof(struct sockaddr_in);

        memset(recvline, 0, sizeof(recvline));
        // 减少日志输出频率，避免影响性能
        // printf("Waiting for UDP data...\r\n");

        ret = recvfrom(sockfd, recvline, 1024, 0, (struct sockaddr*)&addrClient,(socklen_t*)&sizeClientAddr);
        // printf("recvfrom returned: %d\r\n", ret);

        if(ret > 0)
        {
            char *pClientIP = inet_ntoa(addrClient.sin_addr);
 
            printf("Received from %s-%d: %s (length: %d)\r\n", 
                   pClientIP, ntohs(addrClient.sin_port), recvline, ret);

            cotrl_handle(recvline, ret); 
        }
        else if(ret == 0)
        {
            // printf("Received empty packet\r\n");
        }
        else
        {
            printf("recvfrom error: %d\r\n", ret);
        }
        
        // 添加适当的延时，避免过于频繁的轮询
        osDelay(10);
    }
}

void start_udp_thread(void)
{
    osThreadAttr_t attr;

    attr.name = "udp_control_thread";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096*2;
    attr.priority = 36;

    if (osThreadNew((osThreadFunc_t)udp_thread, NULL, &attr) == NULL) {
        printf("[LedExample] Falied to create LedTask!\n");
    }
}