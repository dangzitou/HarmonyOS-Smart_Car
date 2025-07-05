#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "hi_time.h"
#include "iot_pwm.h"
#include "hi_pwm.h"

#define GPIO0 0
#define GPIO1 1
#define GPIO9 9
#define GPIO10 10
#define GPIOFUNC 0

#define PWM_DUTY_MAX 8000 // 最大占空比
#define PWM_FREQ 8000 // PWM频率

#define PWM_PORT_LEFT HI_PWM_PORT_PWM3   // GPIO0 -> PWM3
#define PWM_PORT_RIGHT HI_PWM_PORT_PWM0  // GPIO9 -> PWM0
#define IO_NAME_GPIO_0 0
#define IO_NAME_GPIO_1 1
#define IO_NAME_GPIO_9 9
#define IO_NAME_GPIO_10 10
#define IO_FUNC_GPIO_0_PWM3_OUT HI_IO_FUNC_GPIO_0_PWM3_OUT
#define IO_FUNC_GPIO_9_PWM0_OUT HI_IO_FUNC_GPIO_9_PWM0_OUT

void pwm_init(){
    //初始化PWM相关GPIO并设置为PWM复用
    IoTGpioInit(IO_NAME_GPIO_0);
    IoTGpioInit(IO_NAME_GPIO_1);
    IoTGpioInit(IO_NAME_GPIO_9);
    IoTGpioInit(IO_NAME_GPIO_10);
    hi_io_set_func(IO_NAME_GPIO_0, IO_FUNC_GPIO_0_PWM3_OUT); // 左轮PWM
    hi_io_set_func(IO_NAME_GPIO_9, IO_FUNC_GPIO_9_PWM0_OUT); // 右轮PWM
    //初始化PWM端口
    hi_pwm_init(PWM_PORT_LEFT);
    hi_pwm_init(PWM_PORT_RIGHT);
}

void gpio_control (unsigned int  gpio, IotGpioValue value) {
    hi_io_set_func(gpio, GPIOFUNC);
    IoTGpioSetDir(gpio, IOT_GPIO_DIR_OUT);//将GPIO引脚设置为输出方向，表示该引脚将用作输出信号的发送端。在输出模式下，可以通过控制GPIO引脚的输出电平值
    IoTGpioSetOutputVal(gpio, value);//设置GPIO引脚的输出电平值。
}

void set_wheel_pwm(unsigned short left_duty, unsigned short right_duty) {
    // 左轮控制
    if (left_duty == 0) {
        // 停止PWM，两个引脚都拉高，实现L9110S刹车
        hi_pwm_stop(PWM_PORT_LEFT);
        gpio_control(IO_NAME_GPIO_0, IOT_GPIO_VALUE0);
        gpio_control(IO_NAME_GPIO_1, IOT_GPIO_VALUE0);
    } else {
        // 设置PWM输出，方向引脚拉低
        hi_io_set_func(IO_NAME_GPIO_0, IO_FUNC_GPIO_0_PWM3_OUT);
        hi_pwm_start(PWM_PORT_LEFT, left_duty, PWM_FREQ);
        gpio_control(IO_NAME_GPIO_1, IOT_GPIO_VALUE0);
    }
    // 右轮控制
    if (right_duty == 0) {
        // 停止PWM，两个引脚都拉高，实现L9110S刹车
        hi_pwm_stop(PWM_PORT_RIGHT);
        gpio_control(IO_NAME_GPIO_9, IOT_GPIO_VALUE0);
        gpio_control(IO_NAME_GPIO_10, IOT_GPIO_VALUE0);
    } else {
        // 设置PWM输出，方向引脚拉低
        hi_io_set_func(IO_NAME_GPIO_9, IO_FUNC_GPIO_9_PWM0_OUT);
        hi_pwm_start(PWM_PORT_RIGHT, right_duty, PWM_FREQ);
        gpio_control(IO_NAME_GPIO_10, IOT_GPIO_VALUE0);
    }
}

// 小车后退
// GPIO0为低电平，GPIO1为高电平，左轮反转
// GPIO9为低电平，GPIO10为高电平，右轮反转
void car_backward(void) {
    gpio_control(GPIO0, IOT_GPIO_VALUE0);
    gpio_control(GPIO1, IOT_GPIO_VALUE1);
    gpio_control(GPIO9, IOT_GPIO_VALUE0);
    gpio_control(GPIO10, IOT_GPIO_VALUE1);
}

// 小车前进
// GPIO0为高电平，GPIO1为低电平，左轮正转
// GPIO9为高电平，GPIO10为低电平，右轮正转
void car_forward(void) {
    gpio_control(GPIO0, IOT_GPIO_VALUE1);
    gpio_control(GPIO1, IOT_GPIO_VALUE0);
    gpio_control(GPIO9, IOT_GPIO_VALUE1);
    gpio_control(GPIO10, IOT_GPIO_VALUE0);
}

// 小车左转
// GPIO0和1为低电平,左轮停止
// GPIO9为高电平，GPIO10为低电平，右轮正转
void car_left(void) {
    gpio_control(GPIO0, IOT_GPIO_VALUE0);
    gpio_control(GPIO1, IOT_GPIO_VALUE0);
    gpio_control(GPIO9, IOT_GPIO_VALUE1);
    gpio_control(GPIO10, IOT_GPIO_VALUE0);
}

// 小车右转
// GPIO0为高电平，GPIO1为低电平，左轮正转
// GPIO9和10为低电平,右轮停止
void car_right(void) {
    gpio_control(GPIO0, IOT_GPIO_VALUE1);
    gpio_control(GPIO1, IOT_GPIO_VALUE0);
    gpio_control(GPIO9, IOT_GPIO_VALUE0);
    gpio_control(GPIO10, IOT_GPIO_VALUE0);
}

// 小车停止
// GPIO0和1为高电平,左轮刹车
// GPIO9和10为高电平,右边轮刹车
void car_stop(void) {
    gpio_control(GPIO0, IOT_GPIO_VALUE1);
    gpio_control(GPIO1, IOT_GPIO_VALUE1);
    gpio_control(GPIO9, IOT_GPIO_VALUE1);
    gpio_control(GPIO10, IOT_GPIO_VALUE1);
}