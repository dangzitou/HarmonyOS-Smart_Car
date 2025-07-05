#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "hi_io.h"
#include "hi_time.h"
#include "iot_watchdog.h"
#include "robot_control.h"
#include "iot_errno.h"
#include "hi_pwm.h"
#include "hi_timer.h"
#include "iot_pwm.h"

//左右两轮电机各由一个L9110S驱动
//GPOI0和GPIO1控制左轮,GPIO9和GPIO10控制右轮。通过输入GPIO的电平高低控制车轮正转/反转/停止/刹车。
#define GPIO0 0
#define GPIO1 1
#define GPIO9 9
#define GPIO10 10

//查阅机器人板原理图可知
//左边的红外传感器通过GPIO11与3861芯片连接
//右边的红外传感器通过GPIO12与3861芯片连接
#define GPIO11 11
#define GPIO12 12

#define GPIO_FUNC 0
#define car_speed_left 0
#define car_speed_right 0
extern unsigned char g_car_status;   

unsigned int g_car_speed_left = car_speed_left;
unsigned int g_car_speed_right = car_speed_right;
IotGpioValue io_status_left;
IotGpioValue io_status_right;

// 全局变量，保存红外传感器状态（volatile保证多线程/中断下数据一致性）
volatile IotGpioValue g_trace_left = IOT_GPIO_VALUE1;
volatile IotGpioValue g_trace_right = IOT_GPIO_VALUE1;

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

extern void pwm_init();
extern void set_wheel_pwm(unsigned short left_duty, unsigned short right_duty);
extern float GetDistance(void);

#define DISTANCE_BETWEEN_CAR_AND_OBSTACLE 20.0f

unsigned short SPEED_TURN = 5000;
unsigned short SPEED_FORWARD = 6000;
unsigned int MOVING_STATUS = 0;
static unsigned char obstacle_detected = 0;

//获取红外传感器的值，调整电机的状态
void timer1_callback(unsigned int arg)
{
    // 只采集红外传感器状态，不做决策
    IoTGpioGetInputVal(GPIO11, (IotGpioValue*)&g_trace_left);
    IoTGpioGetInputVal(GPIO12, (IotGpioValue*)&g_trace_right);
}

void trace_module(void)
{
    unsigned int timer_id1;
    pwm_init();

    set_wheel_pwm(0, 0);

    hi_timer_create(&timer_id1);
    // 启动系统周期定时器用来按照预定的时间间隔1ms触发timer1_callback任务的执行
    hi_timer_start(timer_id1, HI_TIMER_TYPE_PERIOD, 1, timer1_callback, 0);

    while (1) {
        // float distance = GetDistance();
        
        // // 检查前方是否有障碍物
        // if (distance < DISTANCE_BETWEEN_CAR_AND_OBSTACLE) {
        //     // 检测到障碍物，停车并等待
        //     set_wheel_pwm(0, 0);
        //     MOVING_STATUS = 4;  // 障碍物状态码
        //     printf("[trace] Obstacle detected at %.2f cm, stopping...\n", distance);
            
        //     // 持续检测直到障碍物消失
        //     while (1) {
        //         float current_distance = GetDistance();
        //         if (current_distance >= DISTANCE_BETWEEN_CAR_AND_OBSTACLE) {
        //             printf("[trace] Obstacle cleared, resuming trace mode\n");
        //             break;
        //         }
        //         hi_udelay(100000); // 100ms延时，减少检测频率
                
        //         // 检查是否需要退出循迹模式
        //         if (g_car_status != CAR_TRACE_STATUS) {
        //             break;
        //         }
        //     }
        //     if (g_car_status != CAR_TRACE_STATUS) {
        //         break;
        //     }
        //     // 障碍物消失后，继续循迹逻辑
        //     continue;
        // }

        if (g_trace_left == IOT_GPIO_VALUE0 && g_trace_right == IOT_GPIO_VALUE0) {
            // 两边都检测到黑色，刹车
            set_wheel_pwm(0, 0);
            MOVING_STATUS = 0;  //停车状态码
            printf("[trace] Brake: both sensors on black\n");
        } else if (g_trace_right == IOT_GPIO_VALUE0 && g_trace_left != IOT_GPIO_VALUE0) {
            // 右边黑线，左边白，右转
            set_wheel_pwm(SPEED_TURN, 0);
            MOVING_STATUS = 1;  //右转状态码
            printf("[trace] Turn right\n");
        } else if (g_trace_left == IOT_GPIO_VALUE0 && g_trace_right != IOT_GPIO_VALUE0) {
            // 左边黑线，右边白，左转
            set_wheel_pwm(0, SPEED_TURN);
            MOVING_STATUS = 2;  //左转状态码
            printf("[trace] Turn left\n");
        } else if(g_trace_left == IOT_GPIO_VALUE1 && g_trace_right == IOT_GPIO_VALUE1){
            // 都是白色，直行
            set_wheel_pwm(SPEED_FORWARD, SPEED_FORWARD);
            MOVING_STATUS = 3;  //直行状态码
            printf("[trace] Forward\n");
        }
        
        hi_udelay(20);
        if (g_car_status != CAR_TRACE_STATUS) {
            break;
        }
    }
    // 退出trace模式，关闭PWM
    hi_pwm_stop(PWM_PORT_LEFT);
    hi_pwm_stop(PWM_PORT_RIGHT);
    hi_timer_delete(timer_id1);
}