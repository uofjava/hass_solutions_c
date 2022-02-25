@page 海川片料机500网关程序

# 1. 案例简介
片料机500（三菱：3GA）通过485ADP扩展模块从站与网关485通信：
- 系统板级初始化
- 内核基础组件初始化
- application_start用户入口
- 等待wifi连接成功！通过命令：1：netmgr_example，2：netmgr -t wifi -c HUAWEI_AP abcdefg1234
- 创建消息队列：用于将modbus任务采集到数据传输到阿里云MQTT任务
- 开启连接阿里云物联网
- 开启modbus主站采集片料机数据（M7000-M7095, M7100-M7195, M7200-M7295, D7000-D7011)

该示例的运行依赖下述基本功能完成对接：
- linksdk,mbmaster,queue,task
- 内核的任务和中断运行正常
- 系统tick定时器正常运行

# 2. 基础知识
## 2.1 基础目录结构

```tree
├── main.c   # 等待wifi,创建消息队列，创建阿里iot任务，创建modbus-master任务，入口**application_start**
├── modbus.c   # 配置modbus,采集片料机数据，并将数据放入消息队列
├── al_iot.c   # 配置阿里mqtt，从消息队列中获取数据，并发布到云平台
├── k_app_config.h # 内核组件的配置开关，优先级低于**k_config.h**
├── maintask.c     # 系统主任务入口处理，入口**aos_maintask**
├── Makefile       # aos make编译时入口
├── package.yaml   # 编译系统配置文件
└── SConstruct     # Makefile => Scon => aostools
```

## 2.2 基本规范


