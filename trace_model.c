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

#include "robot_l9110s.h"

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

#define DISTANCE_BETWEEN_CAR_AND_OBSTACLE 20.0f

extern unsigned char g_car_status;   
unsigned int g_car_speed_left = car_speed_left;
unsigned int g_car_speed_right = car_speed_right;
IotGpioValue io_status_left;
IotGpioValue io_status_right;

// 全局变量，保存红外传感器状态（volatile保证多线程/中断下数据一致性）
volatile IotGpioValue g_trace_left = IOT_GPIO_VALUE1;
volatile IotGpioValue g_trace_right = IOT_GPIO_VALUE1;

extern float GetDistance(void);

unsigned int MOVING_STATUS = 0;

static int g_obstacle_detected = 0;  // 障碍物检测标志
static int g_obstacle_check_counter = 0;  // 避障检测计数器

// 黑线持续检测时间阈值（单位：ms），可根据需要调整
unsigned int black_line_detect_time_ms = 15; 
unsigned int black_line_counter = 0; // 黑线检测计数器
volatile int g_black_line_stop = 0; // 黑线停车标志

//获取红外传感器的值，调整电机的状态
void timer1_callback(unsigned int arg)
{
    // 使用计数器,使得每50ms检测一次障碍物
    g_obstacle_check_counter++;
    if (g_obstacle_check_counter >= 50) {
        g_obstacle_check_counter = 0;
        
        // 获取前方距离
        float distance = GetDistance();
        
        // 检查是否有障碍物
        if (distance < DISTANCE_BETWEEN_CAR_AND_OBSTACLE) {
            if (!g_obstacle_detected) {
                printf("Obstacle detected! Distance: %.2f cm\n", distance);
                g_obstacle_detected = 1;
                // 立即停止小车
                car_stop();
                MOVING_STATUS = 4;  // 障碍物状态码
            }
        } else {
            if (g_obstacle_detected) {
                printf("Obstacle cleared! Distance: %.2f cm, resuming trace\n", distance);
                g_obstacle_detected = 0;
            }
        }
    }
    
    // 如果检测到障碍物，直接返回，不执行循迹逻辑
    if (g_obstacle_detected) {
        return;
    }

    // 正常的循迹逻辑：采集红外传感器状态
    IoTGpioGetInputVal(GPIO11, (IotGpioValue*)&g_trace_left);
    IoTGpioGetInputVal(GPIO12, (IotGpioValue*)&g_trace_right);

    // 黑线持续检测逻辑
    if (g_trace_left == IOT_GPIO_VALUE0 && g_trace_right == IOT_GPIO_VALUE0) {
        black_line_counter++;
        if (black_line_counter >= black_line_detect_time_ms) {
            g_black_line_stop = 1;
        }
    } else {
        black_line_counter = 0;
        g_black_line_stop = 0;
    }
}

void trace_module(void)
{
    unsigned int timer_id1;
    pwm_init();

    car_stop();

    // 初始化避障检测变量
    g_obstacle_detected = 0;
    g_obstacle_check_counter = 0;
    g_black_line_stop = 0;
    black_line_counter = 0;

    hi_timer_create(&timer_id1);
    // 启动系统周期定时器用来按照预定的时间间隔1ms触发timer1_callback任务的执行
    hi_timer_start(timer_id1, HI_TIMER_TYPE_PERIOD, 1, timer1_callback, 0);

    while (1) {
        // 只有在没有检测到障碍物时才执行循迹逻辑
        if (!g_obstacle_detected) {
            if (g_black_line_stop) {
                car_stop();
                MOVING_STATUS = 0;
                printf("[trace] Brake: black line detected for %d ms\n", black_line_detect_time_ms);
            } else {
                // 只保留左转/右转/直行逻辑，不再遇到黑线就立即停车
                if (g_trace_right == IOT_GPIO_VALUE0 && g_trace_left != IOT_GPIO_VALUE0) {
                    car_right();
                    MOVING_STATUS = 1;
                    printf("[trace] Turn right\n");
                } else if (g_trace_left == IOT_GPIO_VALUE0 && g_trace_right != IOT_GPIO_VALUE0) {
                    car_left();
                    MOVING_STATUS = 2;
                    printf("[trace] Turn left\n");
                } else if(g_trace_left == IOT_GPIO_VALUE1 && g_trace_right == IOT_GPIO_VALUE1){
                    car_forward();
                    MOVING_STATUS = 3;
                    printf("[trace] Forward\n");
                }else if(g_trace_left == IOT_GPIO_VALUE0 && g_trace_right == IOT_GPIO_VALUE0){
                    car_forward();
                    MOVING_STATUS = 3;
                    printf("[trace] Forward\n");
                }
            }
        }
        hi_udelay(20);
        if (g_car_status != CAR_TRACE_STATUS) {
            break;
        }
    }
    // 退出trace模式，关闭PWM
    pwm_stop();
    hi_timer_delete(timer_id1);
}