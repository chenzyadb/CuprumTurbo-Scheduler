## CuprumTurbo Scheduler  
一个易于上手的调度  
### 自定义配置  
详见: [自定义配置帮助文档](https://github.com/chenzyyzd/CuprumTurbo-Scheduler/blob/main/custom_config/readme.md).  
### 使用的第三方库
[cJSON 1.7.15 - Dave Gamble and cJSON contributors](https://github.com/DaveGamble/cJSON)  
MIT Licence  
### 开源协议  
本项目使用BSD-3协议开源，二次开发请注明原作者chenzyadb并附上原有开源协议.  
源码支持用于商业用途和闭源.  
### 编译源码
**默认编译环境为Termux + CMake 3.25.1 + clang 15.0.6，设备指令集为arm64-v8a.**  
1.克隆此仓库.  
`git clone https://github.com/chenzyyzd/CuprumTurbo-Scheduler.git`  
2.安装CMake(已安装的可跳过).  
`apt-get install cmake`  
3.使用CMake编译源码.  
`cd ./CuprumTurbo-Scheduler/source`  
`cmake .`  
`make`  
4.编译后的二进制文件位于./CuDaemon  
