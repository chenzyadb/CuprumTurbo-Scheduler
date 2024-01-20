## CuprumTurbo Scheduler
一个简单可靠的性能调度.  
`A Simple and Reliable Performance Scheduler.`  
#### 项目介绍  
- 通过识别使用场景动态进行性能调控以改善流畅度和能效.  
- 支持安卓9-14的arm64设备.  
- 软件本体使用C++语言编写, 自身性能开销较小.  
- 支持通过提供的APP/magisk模块方式自动配置运行, 可以自由修改为其他启动方式.  
- 支持多种调度模式, 配合CuToolbox APP可以实现动态模式切换.  
- 在只使用调度本体时不需要改动任何系统文件和注入运行中的进程.  
- 支持JSON格式的自定义配置, 详见[自定义配置开发文档](https://github.com/chenzyadb/CuprumTurbo-Scheduler/blob/main/docs/config_dev_helper.md).  
#### 使用的开源项目  
- [CuJSONObject](https://github.com/chenzyadb/CuJSONObject) by chenzyadb  
- [CuLogger](https://github.com/chenzyadb/CuLogger) by chenzyadb  
- [CuSimpleMatch](https://github.com/chenzyadb/CuSimpleMatch) by chenzyadb  
