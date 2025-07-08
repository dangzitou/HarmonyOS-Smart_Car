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

#define IO_NAME_GPIO_0 0
#define IO_NAME_GPIO_1 1
#define IO_NAME_GPIO_9 9
#define IO_NAME_GPIO_10 10
#define IO_FUNC_GPIO_0_PWM3_OUT HI_IO_FUNC_GPIO_0_PWM3_OUT
#define IO_FUNC_GPIO_1_PWM4_OUT HI_IO_FUNC_GPIO_1_PWM4_OUT
#define IO_FUNC_GPIO_9_PWM0_OUT HI_IO_FUNC_GPIO_9_PWM0_OUT
#define IO_FUNC_GPIO_10_PWM1_OUT HI_IO_FUNC_GPIO_10_PWM1_OUT

unsigned short SPEED_TURN = 6000;
unsigned short SPEED_FORWARD = 6000;
unsigned short SPEED_BACKWARD = 5000;

void pwm_init(){
    //初始化PWM相关GPIO并设置为PWM复用
    IoTGpioInit(IO_NAME_GPIO_0);
    IoTGpioInit(IO_NAME_GPIO_1);
    IoTGpioInit(IO_NAME_GPIO_9);
    IoTGpioInit(IO_NAME_GPIO_10);

    hi_io_set_func(IO_NAME_GPIO_0, IO_FUNC_GPIO_0_PWM3_OUT); // 左轮前PWM
    hi_io_set_func(IO_NAME_GPIO_1, IO_FUNC_GPIO_1_PWM4_OUT); // 左轮后PWM
    hi_io_set_func(IO_NAME_GPIO_9, IO_FUNC_GPIO_9_PWM0_OUT); // 右轮前PWM
    hi_io_set_func(IO_NAME_GPIO_10, IO_FUNC_GPIO_10_PWM1_OUT); //右轮后PWM

    //初始化PWM端口
    hi_pwm_init(HI_PWM_PORT_PWM3);
    hi_pwm_init(HI_PWM_PORT_PWM4);
    hi_pwm_init(HI_PWM_PORT_PWM0);
    hi_pwm_init(HI_PWM_PORT_PWM1);
}

void pwm_stop(){
    hi_pwm_stop(HI_PWM_PORT_PWM3);
    hi_pwm_stop(HI_PWM_PORT_PWM4);
	hi_pwm_stop(HI_PWM_PORT_PWM0);
    hi_pwm_stop(HI_PWM_PORT_PWM1);
}

void gpio_control (unsigned int  gpio, IotGpioValue value) {
    hi_io_set_func(gpio, GPIOFUNC);
    IoTGpioSetDir(gpio, IOT_GPIO_DIR_OUT);  //将GPIO引脚设置为输出方向，表示该引脚将用作输出信号的发送端。在输出模式下，可以通过控制GPIO引脚的输出电平值
    IoTGpioSetOutputVal(gpio, value);       //设置GPIO引脚的输出电平值。
}

// 小车后退
void car_backward(void) {
    pwm_stop();
    hi_pwm_start(HI_PWM_PORT_PWM4, SPEED_BACKWARD, PWM_DUTY_MAX);
	hi_pwm_start(HI_PWM_PORT_PWM1, SPEED_BACKWARD, PWM_DUTY_MAX);
}

// 小车前进
void car_forward(void) {
    pwm_stop();
    hi_pwm_start(HI_PWM_PORT_PWM3, SPEED_FORWARD, PWM_DUTY_MAX);
	hi_pwm_start(HI_PWM_PORT_PWM0, SPEED_FORWARD, PWM_DUTY_MAX);
}

// 小车左转
void car_left(void) {
    pwm_stop();
	hi_pwm_start(HI_PWM_PORT_PWM0, SPEED_FORWARD, PWM_DUTY_MAX);
    hi_pwm_start(HI_PWM_PORT_PWM4, SPEED_TURN, PWM_DUTY_MAX);
}

// 小车右转
void car_right(void) {
    pwm_stop();
	hi_pwm_start(HI_PWM_PORT_PWM3, SPEED_FORWARD, PWM_DUTY_MAX);
    hi_pwm_start(HI_PWM_PORT_PWM1, SPEED_TURN, PWM_DUTY_MAX);
}

// 小车停止
void car_stop(void) {
    pwm_stop();
}


// void set_wheel_pwm(unsigned short left_duty, unsigned short right_duty) {
//     // 左轮控制
//     if (left_duty == 0) {
//         // 停止PWM，两个引脚都拉高，实现L9110S刹车
//         hi_pwm_stop(PWM_PORT_LEFT);
//         gpio_control(IO_NAME_GPIO_0, IOT_GPIO_VALUE0);
//         gpio_control(IO_NAME_GPIO_1, IOT_GPIO_VALUE0);
//     } else {
//         // 设置PWM输出，方向引脚拉低
//         hi_io_set_func(IO_NAME_GPIO_0, IO_FUNC_GPIO_0_PWM3_OUT);
//         hi_pwm_start(PWM_PORT_LEFT, left_duty, PWM_FREQ);
//         gpio_control(IO_NAME_GPIO_1, IOT_GPIO_VALUE0);
//     }
//     // 右轮控制
//     if (right_duty == 0) {
//         // 停止PWM，两个引脚都拉高，实现L9110S刹车
//         hi_pwm_stop(PWM_PORT_RIGHT);
//         gpio_control(IO_NAME_GPIO_9, IOT_GPIO_VALUE0);
//         gpio_control(IO_NAME_GPIO_10, IOT_GPIO_VALUE0);
//     } else {
//         // 设置PWM输出，方向引脚拉低
//         hi_io_set_func(IO_NAME_GPIO_9, IO_FUNC_GPIO_9_PWM0_OUT);
//         hi_pwm_start(PWM_PORT_RIGHT, right_duty, PWM_FREQ);
//         gpio_control(IO_NAME_GPIO_10, IOT_GPIO_VALUE0);
//     }
// }