# EV

## 1、介绍

EV 是一个致力于 vehicles（包括无人机） 开发的框架。内部采用了 Pub/Sub 机制，高度模块化。易于开发和使用。

### 许可证

EV 软件包遵循 Apache-2.0 许可，详见 LICENSE 文件。


## 2、获取软件包

使用 `EV` 软件包需要在 BSP 目录下使用 menuconfig 命令打开 Env 配置界面，在 `RT-Thread online packages → system packages` 中选择 EV 软件包，具体路径如下：

```shell
RT-Thread online packages
    system packages  --->
         [*] EV: Framework for efficient development of vehicles (including drones)  --->
            Version (latest) --->
```

配置完成后让 RT-Thread 的包管理器自动更新，或者使用 pkgs --update 命令更新包到 BSP 中。

## 3、使用

- 软件包详细介绍，请参考 [软件包介绍](docs/introduction.md)

- 如何从零开始使用，请参考 [用户指南](docs/user-guide.md)
- 框架设计说明，请参考 [设计手册](docs/design.md)
- 完整的 API 文档，请参考 [API 手册](docs/api.md)
- 详细的示例介绍，请参考 [示例文档](docs/samples.md)

## 4、注意事项

- None

## 5、联系方式 & 感谢

| 维护     | 主页       | 邮箱  |
| -------- | ---------- | --- |
| sogwms | https://github.com/sogwms | sogwyms@gmail.com |
