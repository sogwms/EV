# USER-GUIDE

EV 将 vehicle 视为由各个部件而组成的，初始的 vehicle 是个空壳子，需要安装部件来组成真正的 vehicle。部件需要一个个手动安装。部件可视为两种，一是模板部件，二是终端部件。最终安装到 vehicle 上的都应是终端部件，模板部件需要用户"打磨"（继承，实现等）来形成终端部件。!模板部件并不是必须的

ps：当前的（初期） EV 提供的多是模板部件，需要用户做的对接工作可能较多。如模板部件不能符合需求，可以打造自己的模板部件或直接做终端部件。EV 的前进应是伴随着越来越丰富和完善的部件，希望小伙伴在打造自己部件的同时能够多多反馈到 EV 上来，让 EV 更 EV！

## 使用流程

- 0 （选择）建立自己的终端部件

- 1 建立初始的 vehicle, 然后安装所需部件

```c
struct ev my_vehicle;
int main()
{
    ev_init(&my_vehicle);
    // xxx_install(...);
    // ...
}
```

- 2 启动 vehicle

```c
ev_startup(&my_vehicle);
```

## 自定义

EV 的 Pub-Sub 是基于 topic 的，且 topic 是预先定义好的，若用户想添加自己的 topic 可以这样配置：

1. 首先需要通过 'menuconfig' 使能自定义支持。

2. 然后建立 ‘my_ev_custom.h’ 文件到工程

3. 最后在 ‘my_ev_custom.h’ 文件添加 topic, 格式如下：
```c
#ifndef __MY_EV_CUSTOM_H__
#define __MY_EV_CUSTOM_H__

#define MY_EV_TOPICS \
    MY_TOPIC_1,      \
    MY_TOPIC_2,      \
    ...

#endif /* __MY_EV_CUSTOM_H__ */
```

TODO 启动顺序

## 注意事项

- 电机编号方式: 俯视 vehicle，取x轴的正半轴，逆时针旋转，扫过的电机依次编号为 m1,m2,...

- IMU标定请参考 [imu.md](./imu.md)

- *__牵涉到坐标的务必对应上EV的[坐标系](./figures/6.jpg)__*

## 注

欢迎反馈！ 您的反馈将是 EV 持续优化的强有力动力！
