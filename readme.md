## CuprumTurbo Scheduler
一个简单可靠的性能调度.  
`A Simple and Reliable Performance Scheduler.`  
  
[![Releases](https://img.shields.io/github/v/release/chenzyadb/CuprumTurbo-Scheduler?label=Release&logo=github)](https://github.com/chenzyadb/CuprumTurbo-Scheduler/releases/latest) [![License](https://img.shields.io/github/license/chenzyadb/CuprumTurbo-Scheduler?logo=bsd)](/LICENSE) ![Downloads](https://img.shields.io/github/downloads/chenzyadb/CuprumTurbo-Scheduler/total)
### 项目介绍  
- 通过识别使用场景动态进行性能调控以改善流畅度和能效表现.  
- 支持安卓9-16的arm64设备.  
- 软件本体使用C++语言编写, 自身性能开销较小.  
- 支持通过提供的APP/magisk模块方式自动配置运行, 也可自由修改为其他启动方式.  
- 支持多种调度模式, 配合CuToolbox APP可以实现动态模式切换.  
- 在只使用调度本体时不需要改动任何系统文件和注入运行中的进程.  
- 支持导入自定义配置, 详见 [自定义配置模板](https://github.com/chenzyadb/CuprumTurbo-Scheduler/tree/main/template).  
### 安装  
通过CuToolbox APP实现自动配置安装:  
1. 获取最新的CuToolbox APP: [CuToolbox Releases](https://github.com/chenzyadb/CuprumTurbo-Scheduler/releases).  
2. 下载后安装, 授予ROOT权限, 点击APP中的调度状态栏自动配置安装.  
3. APP的更多操作方法详见帮助文档.  

通过提供的magisk模块实现自动配置安装:
1. 获取最新的模块文件: [CuprumTurbo Scheduler Module](https://github.com/chenzyadb/CuprumTurbo-Scheduler/tree/main/magisk).  
2. 打包后在magisk或其他支持安装模块的APP中刷入模块即可自动配置安装.  
### 使用的开源项目  
- [CU-Utils](https://github.com/chenzyadb/CU-Utils) by chenzyadb.  
