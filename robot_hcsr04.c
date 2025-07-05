#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "hi_time.h"

//HC-SR04 超声波测距模块通过GPIO7和8连接到3861
#define GPIO_8 8
#define GPIO_7 7

#define GPIO_FUNC 0

//测距功能实现
float GetDistance  (void) {
    static unsigned long start_time = 0, time = 0;
    float distance = 0.0;
    IotGpioValue value = IOT_GPIO_VALUE0;
    unsigned int flag = 0;

    IoTWatchDogDisable();
    hi_io_set_func(GPIO_8, GPIO_FUNC);

    IoTGpioSetDir(GPIO_8, IOT_GPIO_DIR_IN);//GPIO_8设置为输入引脚
    IoTGpioSetDir(GPIO_7, IOT_GPIO_DIR_OUT);//GPIO_7设置为输出引脚

    //GPIO_7输出一个脉冲触发信号到超声波测距模块
    IoTGpioSetOutputVal(GPIO_7, IOT_GPIO_VALUE1);
    hi_udelay(20);
    IoTGpioSetOutputVal(GPIO_7, IOT_GPIO_VALUE0);

    //超声波测距模块接收到GPIO_7输出的脉冲触发信号后,模块输出回响信号(高电平)到GPIO_8
    while (1) {
        IoTGpioGetInputVal(GPIO_8, &value);

        ////测量回响信号(高电平)时间
        if ( value == IOT_GPIO_VALUE1 && flag == 0) {
            start_time = hi_get_us();
            flag = 1;
        }
        if (value == IOT_GPIO_VALUE0 && flag == 1) {
            time = hi_get_us() - start_time;
            start_time = 0;
            break;
        }

    }

    //距离=高电平时间*0.034 / 2
    distance = time * 0.034 / 2;
    printf("distance is %f\r\n",distance);
    return distance;
}

