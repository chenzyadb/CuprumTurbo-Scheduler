## CuprumTurbo V13 自定义配置开发文档  
CuprumTurbo V13支持用户对调度配置进行自定义，通过修改调度参数可以满足用户对性能调控的多元化需求.  
在进行自定义前请务必仔细阅读此文档以了解调度的工作原理，避免操作不当导致无法达到预定目标.  
### Json信息  
|字段            |类型    |定义             |
|:---------------|:------|:----------------|
|name            |string |配置文件的名称    |
|author          |string |配置文件的作者信息|
|configVersion   |int    |配置文件版本     |
### CPU策略组 cpuPolicy  
`cpuPolicy`需包含几个同步控制的CPU核心，它们的cpu频率将会同步升降.  
通常情况下，`cpuPolicy`需与`cpuCluster`对应，例如`SDM845`平台上的`0-3`和`4-7`核心.  
当给出的CPU核心不存在时，请设定对应的CPU策略组为`-1`  
|字段   |类型   |定义               |
|:------|:------|:-----------------|
|cpu%d  |int    |CPU核心对应的策略组|
### 策略组设定 policy%d_Config  
你需要提供每个策略组的相关参数以帮助程序生成对应的调度数据.  
策略设定名称需与上方的策略组对应，例如您定义了`policy0` `policy1`策略组，就需要提供`policy0_Config` `policy1_Config`.  
注意:需保证`lowPowerFreq` <= `basicFreq` <= `expectFreq`，否则调度程序将无法正常初始化.  
|字段           |类型    |定义                              |
|:--------------|:------|:---------------------------------|
|enable         |int    |是否启用此策略组                   |
|freqTablePath  |string |CPU频率表地址                      |
|cpu_capacity   |int    |CPU算力量化值                      |
|lowPowerFreq   |int    |CPU功耗最低频率(单位:MHz)           |
|basicFreq      |int    |能保持流畅度的最低CPU频率(单位:MHz) |
|expectFreq     |int    |CPU能效比最高频率(单位:MHz)         |
|modelFreq      |int    |用于生成能耗模型的CPU频率(单位:MHz) |
|modelPower     |int    |用于生成能耗模型的CPU功耗(单位:mW)  |
### CPU协调频器设定 CoCpuGovernor_Config  
CPU协调频器可以自动调整内核调频器的各项参数以改善系统流畅度和降低功耗.  
|字段           |类型    |定义                   |
|:--------------|:------|:-----------------------|
|fastSampleTime |int    |快速采样间隔时间(单位:ms)|
|slowSampleTime |int    |缓慢采样间隔时间(单位:ms)|
### 线程优化模块设定 TasksetHelper_Config  
线程优化模块可以自动调整前台进程的线程亲和性/线程优先级以改善流畅度和能效表现.  
`efficiency`分组: 不需要大量CPU资源的日志/垃圾回收线程，如`ReferenceQueue` `Jit thread pool` `xcrash`.  
`singlePerf`分组: 需要大量CPU单核资源的线程，如`UnityMain` `GameThread` `RenderThread`.  
`multiPerf`分组: 需要大量CPU多核资源的线程，如`Worker Thread` `Job.Worker` `TaskGraphNP`.  
`other`分组: 无法分类的未知类型线程.  
CPU核心由单个数字表示，如`0-3,6-7`核心可写为"012367"，`4-7`核心可写为"4567".  
调度优先级与Linux内核nice值相同，范围为-20~19，数字越小优先级越高.  
|字段           |类型     |定义                                              |
|:--------------|:-------|:--------------------------------------------------|
|enable         |int     |是否启用线程优化模块                                |
|setDelayTime   |int     |亲和性设定生效延迟(单位:ms)                         |
|efficiencyCpus |string  |包含在`efficiency`分组的CPU核心                     |
|multiPerfCpus  |string  |包含在`multiPerf`分组的CPU核心                      |
|singlePerfCpus |string  |包含在`singlePerf`分组的CPU核心                     |
|otherCpus      |string  |包含在`other`分组的CPU核心                          |
|efficiencyNice |int     |`efficiency`分组调度优先级(范围-20~19)              |
|performanceNice|int     |`multiPerf` `singlePerf`分组调度优先级(范围-20~19)  |
|otherNice      |int     |`other`分组调度优先级(范围-20~19)                   |
### 模式切换器 modeSwitcher  
CuprumTurbo支持`powersave` `balance` `performance` `fast`四种调度模式.  
当用户进行模式切换操作后，程序将按照模式切换器中预设的参数重新设定调度程序.  
#### 功耗限制 powerLimit  
您可以设定各个模式的功耗限制以控制CPU功耗和性能释放水平.  
|字段           |类型     |定义                      |
|:--------------|:-------|:--------------------------|
|powerLimit     |int     |CPU功耗最大值限制(单位:mW)  |
#### 策略组 policy%d  
`upRateLatency`为一个基准值，实际CPU升频延迟将按照实时状态动态调整.  
`downRateDelay`为CPU降频延迟时间，两次降频请求的间隔不得小于这个值.  
|字段           |类型     |定义                      |
|:--------------|:-------|:--------------------------|
|upRateLatency  |int     |CPU升频延迟(单位:ms)       |
|downRateDelay  |int     |CPU降频延迟(单位:ms)       |
#### 调度加速设定 boostConfig  
调度程序支持`touch` `swipe` `gesture` `heavyload`四种加速策略.  
`touch`为用户按压/触碰屏幕时触发的加速.  
`swipe`为用户滑动屏幕一段距离时触发的加速.  
`gesture`为用户进行手势操作时触发的加速.  
`heavyload`为处于点亮屏幕/切换应用等重负载场景时触发的加速.  
当触发的加速策略`boost`大于当前boost值时切换到新的`boost`，反之则维持当前`boost`.  
当`boost`触发的时间超过`durationTime`时结束`boost`并将boost值置零.  
CPU目标频率计算公式(参考):`cpuMaxFreq * (cpuUtil + (1024 - cpuUtil) * boost / 100) / 1024`
|字段           |类型     |定义                    |
|:--------------|:-------|:-----------------------|
|boost          |int     |CPU调频加速值(范围0~100) |
|durationTime   |int     |boost持续时间(单位:ms)   |
