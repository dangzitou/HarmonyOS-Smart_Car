# OpenHarmony智能小车

## 项目简介

本项目是基于OpenHarmony物联网轻量级系统开发的智能机器人小车演示程序，支持多种控制模式，包括循迹、超声波避障和手动控制等功能。项目采用Hi3861开发板作为主控芯片，集成了多种传感器和执行器。

## 功能特性

- **多模式切换**：通过按键切换停止、循迹、超声波避障三种工作模式
- **循迹功能**：基于红外传感器实现黑线循迹，支持实时速度调节
- **超声波避障**：利用HC-SR04超声波传感器检测障碍物，自动规避
- **舵机控制**：SG90舵机实现转向控制，增强避障能力
- **OLED显示**：SSD1306 OLED屏幕实时显示当前工作状态和参数
- **速度控制**：支持通过按键调节小车运行速度
- **PWM驱动**：采用PWM控制电机转速，运行更平稳

## 硬件组成

| 组件 | 型号 | 连接GPIO | 功能描述 |
|------|------|----------|----------|
| 主控芯片 | Hi3861 | - | 系统核心控制器 |
| 电机驱动 | L9110S | GPIO0/1/9/10 | 双轮差速驱动 |
| 超声波传感器 | HC-SR04 | GPIO2/3 | 距离检测 |
| 舵机 | SG90 | GPIO7 | 转向控制 |
| 红外传感器 | - | GPIO11/12 | 循迹检测 |
| OLED显示屏 | SSD1306 | GPIO13/14 | 状态显示 |
| 按键/电位器 | - | GPIO5 | 模式切换和速度调节 |

## 目录结构

```
robot_demo/
├── BUILD.gn                # 构建配置文件
├── robot_control.c         # 主控制逻辑
├── robot_control.h         # 控制接口头文件
├── robot_hcsr04.c          # 超声波传感器驱动
├── robot_l9110s.c          # 电机驱动控制
├── robot_sg90.c            # 舵机控制
├── trace_model.c           # 循迹模块
├── ssd1306_test.c          # OLED显示控制
├── ssd1306/                # OLED驱动库
│   ├── BUILD.gn
│   ├── ssd1306.c
│   ├── ssd1306.h
│   ├── ssd1306_conf.h
│   ├── ssd1306_fonts.c
│   └── ssd1306_fonts.h
└── README.md               # 项目说明文档
```

## 工作模式

### 1. 停止模式（CAR_STOP_STATUS）
- 小车停止运动，所有电机刹车
- OLED显示 "Robot Car Stop"
- 等待用户切换到其他模式

### 2. 循迹模式（CAR_TRACE_STATUS）
- 基于红外传感器检测黑线
- 自动调整左右轮速度保持循迹
- OLED显示 "trace" 和当前速度
- 支持实时速度调节（通过S1、S2）

### 3. 超声波避障模式（CAR_OBSTACLE_AVOIDANCE_STATUS）
- 前方检测到障碍物时自动避障
- 舵机左右转动探测最佳路径
- 距离小于20cm时后退并转向
- OLED显示 "ultrasonic"

## 控制接口

### 按键控制
- **短按USER**：循环切换工作模式（停止→循迹→避障→停止）
- **按键调节**：在循迹模式下调节运行速度, S1加速, S2减速

### 速度参数
- `SPEED_FORWARD`：前进速度（范围：0-8000）
- `SPEED_TURN`：转弯速度（固定：4000）
- 速度调节步长：500

## 编译和运行

### 环境要求
- OpenHarmony 3.0 LTS 或更高版本
- Hi3861开发板
- DevEco Device Tool

### 编译步骤
1. 确保项目位于OpenHarmony源码树中
2. 配置构建环境：
```bash
hb set
# 选择hispark_pegasus产品
```

3. 编译项目：
```bash
hb build
```

4. 烧录固件到Hi3861开发板

### 运行说明
1. 上电后系统自动初始化
2. OLED屏幕显示 "Hello OpenHarmony!"
3. 按下按键切换到所需工作模式
4. 观察OLED显示的状态信息

## 技术特点

- **实时性**：采用1ms定时器确保循迹响应速度
- **稳定性**：按键防抖设计，避免误触发
- **可扩展性**：模块化设计，便于功能扩展
- **节能设计**：空闲时关闭看门狗，降低功耗

## 贡献者信息
[HiHope开源社区](https://gitee.com/hihope_iot)
[开发者个人主页](https://dangzitou.github.io)

## 许可证

本项目遵循[Mulan PSL v2许可证](http://license.coscl.org.cn/MulanPSL2)。

## 相关仓库
[项目来源](https://gitee.com/hihope_iot/hispark-pegasus-smart-car)

[OpenHarmony文档](https://gitee.com/openharmony/docs)

[设备驱动子系统](https://gitee.com/openharmony/docs/blob/master/zh-cn/readme/驱动子系统.md)

## 注意事项

1. 使用前请确保硬件连接正确
2. 注意电源供电稳定性
3. 循迹时请使用黑色胶带制作轨道
4. 避障模式下请确保周围有足够的活动空间