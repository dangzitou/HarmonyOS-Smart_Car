#ifndef __APP_DEMO_ROBOT_CAR_H__
#define __APP_DEMO_ROBOT_CAR_H__


#define     CAR_CONTROL_DEMO_TASK_STAK_SIZE   (1024*10)
#define     CAR_CONTROL_DEMO_TASK_PRIORITY    (25)
#define     DISTANCE_BETWEEN_CAR_AND_OBSTACLE (20)
#define     KEY_INTERRUPT_PROTECT_TIME        (30)
#define     CAR_TURN_LEFT                     (0)
#define     CAR_TURN_RIGHT                    (1)

typedef enum {
    CAR_STOP_STATUS = 0,
    CAR_OBSTACLE_AVOIDANCE_STATUS, 
    CAR_TRACE_STATUS,
    CAR_CONTROL_STATUS
} CarStatus;


void switch_init(void);
void interrupt_monitor(void);
void car_mode_control_func(void);
#endif