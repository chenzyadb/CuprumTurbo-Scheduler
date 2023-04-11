## CuprumTurbo V15 自定义配置开发文档  
CuprumTurbo V15支持用户对调度配置进行自定义，通过修改调度参数可以满足用户对性能调控的多元化需求.  
在进行自定义前请务必仔细阅读此文档以了解调度的工作原理，避免调参有误导致无法达到预定目标.  
### Json信息  
在这里你可以定义配置文件的名称和作者信息；请注意不要修改配置文件版本，此字段将会用于验证调度与该配置是否兼容.  
configVersion 对应的APP版本: `1: V7.0.x; 2: V7.1.x; 3: V7.2.x; 4: V7.3.x; 5: V7.4.x; 6: V7.5.x`  
|字段            |类型   |定义               |
|:---------------|:------|:------------------|
|name            |string |配置文件的名称     |
|author          |string |配置文件的作者信息 |
|configVersion   |int    |配置文件版本       |
|debug           |bool   |是否启用调试模式   |
### CpuGovernor - CPU混合调频器  
CPU混合调频器可以和内核调频器协同工作，帮助内核调频器在部分场景下选择更合适的CPU频率以改善能效表现.  
采样间隔时间是计算CPU负载的重要参数，Linux内核每3.33ms会记录一次CPU在这段时间内的状态，活跃时间/总时间*100即为CPU负载百分比.  
如果采样间隔过短将无法获得有效的CPU负载百分比(例如每10ms采样一次将只能获得0% 33% 66% 100%四种负载)，过长将导致CPU调频反应速度过慢.  
当CPU负载处在一个较低的值时，混合调频器将使用slowSampleTime间隔采样，当CPU负载处于一个较高的值时，将使用fastSampleTime进行采样.  
|字段            |类型   |定义                      |
|:---------------|:------|:-------------------------|
|enable          |bool   |是否启用此模块            |
|slowSampleTime  |int    |慢速采样间隔时间(单位:ms) |
|fastSampleTime  |int    |快速采样间隔时间(单位:ms) |
#### 策略组  
每个策略组中的CPU频率将会同步控制，应当与内核中每个cluster中包含的CPU对应.  
例如SDM845为4+4设计，即`policy0: CPU0-3; policy1: CPU4-7`.  
CPU数量上限为10个，策略组最大数量为10个，每个策略组最多包含10个CPU.   
CPU的动态功耗与CPU频率和负载均有关联，并不是CPU频率越低实际功耗就越低.  
`basicFreq`关系到CPU混合调频器判定CPU负载是否处于较高值，不应设置得过低，否则会导致混合调频器无法正常工作.  
当`lowPowerFreq <= expectFreq <= modelFreq`条件不成立时，调度将无法正常初始化.  
|字段            |类型    |定义                                    |
|:--------------|:-------|:---------------------------------------|
|cpuCore        |ArrayInt|策略组中包含的CPU                        |
|perfScale      |int     |CPU相对同频算力值                        |
|lowPowerFreq   |int     |CPU功耗最低频率(单位:MHz)                |
|baseFreq       |int     |能保持一定流畅度的最低CPU频率(单位:MHz)     |
|optimalFreq    |int     |CPU能效比最高频率(单位:MHz)               |
|modelFreq      |int     |用于生成CPU功耗模型的CPU频率(单位:MHz)     |
|modelPower     |int     |处于modelFreq时CPU的满载功耗(单位:mW)     |
#### 模式参数  
CPU混合调频器支持为不同调度模式设置不同参数以实现按模式进行性能调控.  
CPU整体功耗限制会影响CPU频率和性能上限，混合调频器计算的是满载功耗，不会随CPU负载变化而改变.  
升频延迟和性能冗余会影响CPU频率提升是否积极，延迟越低冗余越高CPU频率提升越积极，性能越好，耗电越严重.  
|字段            |类型   |定义                      |
|:---------------|:------|:-------------------------|
|powerLimit      |int    |CPU整体功耗限制(单位:mW)  |
|upRateLatency   |int    |CPU频率提升延迟(单位:ms)  |
|perfMargin      |int    |CPU性能冗余(范围:0-100)   |
#### CPU频率加速  
CPU频率加速可以在特定条件触发时短暂地提升CPU频率提升积极性，用于预知CPU负载升高以降低卡顿的几率.  
触发条件包含`touch` `swipe` `gesture` `heavyload`，分别在触碰到屏幕、滑动屏幕、进行手势操作和重负载(如点亮屏幕)时触发.  
频率加速时CPU负载计算公式如下: `cpuLoad = cpuLoad + (100 - cpuLoad) * boost`.  
当CPU加速触发的时间超过`durationTime`时，将会终止CPU加速.  
|字段            |类型   |定义                         |
|:---------------|:------|:----------------------------|
|boost           |int    |频率加速值(范围:0-100)       |
|durationTime    |int    |CPU频率加速持续时间(单位:ms) |
### ThreadSchedOpt - 线程调度优化  
ThreadSchedOpt模块可以基于线程名称和CPU占用等数据智能分类前台线程，组别如下:  
`MainThread`分组: 包含应用程序的主线程.   
`GameSingleThread`分组: 包含游戏程序中占用CPU单核性能较多的线程.  
`GameMultiThread`分组: 包含游戏程序中占用CPU多核性能较多的线程.  
`RenderThread`分组: 包含应用程序中负责界面渲染的相关线程.  
`UIThread`分组: 包含应用程序中负责生成用户界面的相关线程.  
`MediaThread`分组: 包含应用程序中负责媒体（包括音频/视频解码或渲染）的相关线程.  
`WebViewThread`分组: 包含应用程序中WebView组件的相关线程.  
`ProcessThread`分组: 包含应用程序中负责数据处理的相关线程.  
`NonRealTimeThread`分组: 包含应用程序中不需要数据实时处理的线程.  
`OtherThread`分组: 包含ThreadSchedOpt无法分类的线程.  
默认的ThreadSchedOpt规则已经可以覆盖绝大多数场景，如果出现分类不合理导致的卡顿问题请联系开发者.  
CPU核心由单个数字表示，如`0-3,6-7`核心可写为"012367"，`4-7`核心可写为"4567".  
调度优先级与Linux内核nice值相同，范围为-20~19，数字越小优先级越高.  
|字段           |类型    |定义                              |
|:--------------|:-------|:---------------------------------|
|enable         |bool    |是否启用此模块                    |
|cpus           |ArrayInt|此分组的cpu亲和性设定             |
|nice           |int     |此分组的调度优先级(范围:-20~19)   |
### MtkGpuGovernor - 联发科GPU调频器  
联发科默认的GPU调频器并没有提供接口可以调控性能冗余和GPU频率上限，这个简易的GPU调频器可以满足对联发科GPU频率的简单调控.  
由于联发科默认的GPU频率表频点数量过多，此调频器将只会选取部分GPU频率，具体信息请查看调度日志.  
当GPU负载为0时将按照`slowSampleTime`间隔读取GPU负载，当负载非0时按照`fastSampleTime`间隔读取GPU负载.  
与CpuGovernor相同，升频延迟和性能冗余将会影响GPU频率提升是否积极，延迟越低冗余越高GPU频率提升越积极.  
`preferredFreq`为偏好的GPU频率,调度生成GPU频率表时将优先考虑这些GPU频率.  
触摸升频可以帮助GPU调频器在部分场景下迅速提升频率以改善卡顿问题.  
|字段            |类型    |定义                      |
|:---------------|:-------|:-------------------------|
|enable          |bool    |是否启用此模块            |
|slowSampleTime  |int     |慢速采样间隔时间(单位:ms) |
|fastSampleTime  |int     |快速采样间隔时间(单位:ms) |
|preferredFreq   |ArrayInt|偏好GPU频率(单位:MHz)    |
|maxFreq         |int     |GPU频率上限(单位:MHz)     |
|upRateLatency   |int     |GPU频率提升延迟(单位:ms)  |
|perfMargin      |int     |GPU性能冗余(范围:0-100)   |
|touchBoost      |bool    |是否启用触摸升频          |
