#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "hi_time.h"
#include "ssd1306.h"
#include "iot_i2c.h"
#include "iot_watchdog.h"
#include "robot_control.h"
#include "iot_errno.h"
#include <unistd.h>

#define OLED_I2C_BAUDRATE 400*1000
#define GPIO13 13
#define GPIO14 14
#define FUNC_SDA 6
#define FUNC_SCL 6
extern unsigned char g_car_status;
extern unsigned short SPEED_FORWARD;
extern unsigned int MOVING_STATUS;

char moving_status[5][20] = {
    "Stopping...",
    "Turing right...",
    "Turing left...",
    "Moving forward...",
    "Obstacles ahead..."
};

void Ssd1306TestTask(void* arg)
{
    (void) arg;
    hi_io_set_func(GPIO13, FUNC_SDA);
    hi_io_set_func(GPIO14, FUNC_SCL);
    IoTI2cInit(0, OLED_I2C_BAUDRATE);

    IoTWatchDogDisable();

    usleep(20*1000);
    ssd1306_Init();
    ssd1306_Fill(Black);
    ssd1306_SetCursor(0, 0);
    ssd1306_DrawString("Hello OpenHarmony!", Font_7x10, White);

    uint32_t start = HAL_GetTick();
    ssd1306_UpdateScreen();
    uint32_t end = HAL_GetTick();
    printf("ssd1306_UpdateScreen time cost: %d ms.\r\n", end - start);
    osDelay(100);

    while (1)
    {
        //printf("g_car_status is %d\r\n",g_car_status);
        ssd1306_Fill(Black);
        ssd1306_SetCursor(10, 10);
        if(g_car_status == CAR_OBSTACLE_AVOIDANCE_STATUS) {
            
            ssd1306_DrawString("ultrasonic", Font_7x10, White);
            
        }
        else if (g_car_status == CAR_STOP_STATUS)
        {
            ssd1306_DrawString("Robot Car Stop", Font_7x10, White);
        }
        else if (g_car_status == CAR_TRACE_STATUS)
        {
            ssd1306_DrawString("trace", Font_7x10, White);
            ssd1306_SetCursor(10, 30);

            char speed_str[20];  // 创建字符串缓冲区
            sprintf(speed_str, "Speed: %u", SPEED_FORWARD);  // 格式化转换
            ssd1306_DrawString(speed_str, Font_7x10, White);

            ssd1306_SetCursor(10, 45);
            if(MOVING_STATUS == 0)
                ssd1306_DrawString(moving_status[0], Font_7x10, White);
            else if(MOVING_STATUS == 1)
                ssd1306_DrawString(moving_status[1], Font_7x10, White);
            else if(MOVING_STATUS == 2)
                ssd1306_DrawString(moving_status[2], Font_7x10, White);
            else if(MOVING_STATUS == 3)
                ssd1306_DrawString(moving_status[3], Font_7x10, White);
            else if(MOVING_STATUS == 4)
                ssd1306_DrawString(moving_status[4], Font_7x10, White);
        }
        ssd1306_UpdateScreen();
        osDelay(10);
    }
    
}

void Ssd1306TestDemo(void)
{
    osThreadAttr_t attr;

    attr.name = "Ssd1306Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = 25;

    if (osThreadNew(Ssd1306TestTask, NULL, &attr) == NULL) {
        printf("[Ssd1306TestDemo] Falied to create Ssd1306TestTask!\n");
    }
}
APP_FEATURE_INIT(Ssd1306TestDemo);
