## CuprumTurbo Scheduler
一个简单可靠的性能调度.  
`A Simple and Reliable Performance Scheduler.`  
#### 特性
- 通过识别使用场景动态进行性能调控以改善流畅度和能效
- 软件本体使用C++语言编写, 自身性能开销较小
- 支持多种调度模式, 配合CuToolbox APP可以实现动态模式切换
- 支持安卓8-13的arm64-v8a设备
- 不需要改动任何系统文件和注入运行中的进程
- 支持JSON格式的自定义配置
- 支持通过APP/magisk模块方式自动配置运行, 也可以手动使用shell命令启动
- 自带大多数热门平台的配置文件
#### 自定义配置
- 详见: [自定义配置帮助文档](https://github.com/chenzyyzd/CuprumTurbo-Scheduler/blob/main/custom_config/readme.md)
#### 开放源代码
- 基于新版本框架的项目: [CuPerfMonitor](https://github.com/chenzyyzd/CuPerfMonitor)
- 旧版本源码: `./source`
#### 使用的开源项目
- [Simple-JsonObject - chenzyyzd](https://github.com/chenzyyzd/Simple-JsonObject)
- [CuLogger - chenzyyzd](https://github.com/chenzyyzd/CuLogger)
- [CuSimpleMatch - chenzyyzd](https://github.com/chenzyyzd/CuSimpleMatch)
