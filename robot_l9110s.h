#ifndef ROBOT_L9110S_H
#define ROBOT_L9110S_H

#include "iot_gpio.h"

// GPIO引脚定义
#define GPIO0 0
#define GPIO1 1
#define GPIO9 9
#define GPIO10 10
#define GPIOFUNC 0

// PWM参数定义
#define PWM_DUTY_MAX 8000 // 最大占空比
#define PWM_FREQ 8000     // PWM频率

// GPIO到IO名称映射
#define IO_NAME_GPIO_0 0
#define IO_NAME_GPIO_1 1
#define IO_NAME_GPIO_9 9
#define IO_NAME_GPIO_10 10

// 速度参数
extern unsigned short SPEED_TURN;
extern unsigned short SPEED_FORWARD;
extern unsigned short SPEED_BACKWARD;

// 函数声明
/**
 * @brief 初始化PWM相关GPIO和PWM端口
 */
void pwm_init(void);

/**
 * @brief 停止所有PWM输出
 */
void pwm_stop(void);

/**
 * @brief GPIO控制函数
 * @param gpio GPIO引脚号
 * @param value GPIO输出值
 */
void gpio_control(unsigned int gpio, IotGpioValue value);

/**
 * @brief 小车后退
 */
void car_backward(void);

/**
 * @brief 小车前进
 */
void car_forward(void);

/**
 * @brief 小车左转
 */
void car_left(void);

/**
 * @brief 小车右转
 */
void car_right(void);

/**
 * @brief 小车停止
 */
void car_stop(void);

#endif // ROBOT_L9110S_H