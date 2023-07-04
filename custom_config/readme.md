## CuprumTurbo V16 自定义配置开发文档  
CuprumTurbo V16支持用户对调度配置进行自定义，通过修改调度参数可以满足用户对性能调控的更多需求.  
此项目使用模块化设计, 每个子Json对象对应一个`Module`, 通过设置各个模块的`enable`字段可以启用或禁用模块.  
当`enable`字段设置为`false`时程序将不会继续加载其余参数, 可以删除该子Json对象中的其余字段.  
### Json信息  
在这里你可以定义配置文件的名称和作者信息；请注意不要修改配置文件版本，此字段将会用于验证调度与该配置是否兼容.  
|字段            |类型   |定义               |
|:---------------|:------|:-----------------|
|name            |string |配置文件的名称     |
|author          |string |配置文件的作者信息  |
|configVersion   |int    |配置文件版本       |
### CpuGovernor - CPU混合调频器  
> 此模块可以通过在部分场景下选择合适的CPU频率以改善能效表现.
#### 调频器全局设定  
|字段            |类型   |定义                      |
|:---------------|:------|:------------------------|
|slowSampleTime  |int    |慢速采样间隔时间(单位:ms)  |
|fastSampleTime  |int    |快速采样间隔时间(单位:ms)  |
  
采样间隔时间是计算CPU负载的重要参数，通常Linux内核每3.33ms会记录一次CPU在这段时间内的状态(300HZ)，活跃时间/总时间*100即为CPU负载百分比.  
如果采样间隔过短将无法获得有效的CPU负载百分比(例如每10ms采样一次将只能获得0% 33% 66% 100%四种负载)，过长将导致CPU调频反应速度过慢.  
当CPU整体处于较空闲状态时使用`slowSampleTime`时长采样, 当CPU整体处于较繁忙状态时使用`fastSampleTime`时长采样.  
#### policies - 策略组    
此项配置类型为`ArrayJson`, 即数组中的每个Json元素对应一个策略组.  
每个策略组中的CPU频率将会同步控制，应当与内核中每个cluster中包含的CPU对应.  
由于是按照数组的序号来为策略组编号的,所以策略组的排序应与cluster的排序一致.  
例如SDM845为4+4设计，即`policy0: CPU0-3; policy1: CPU4-7`.  
V16支持的CPU核心数量和CPU策略组数量无上限, 可以按需设置.  
|字段            |类型    |定义                                    |
|:--------------|:-------|:---------------------------------------|
|coreNum        |int     |策略组中包含的CPU核心数量                 |
|perfScale      |int     |CPU相对同频算力值                        |
|lowPowerFreq   |int     |CPU功耗最低频率(单位:MHz)                |
|optimalFreq    |int     |CPU最优频率(单位:MHz)                    |
|modelFreq      |int     |用于生成CPU功耗模型的CPU频率(单位:MHz)     |
|modelPower     |int     |处于modelFreq时CPU的满载功耗(单位:mW)     |
  
`CpuGovernor`模块设定中的所有频率都将会被取近似值, 例如CPU频率表中有`1200, 1450, 1700`三个频率, 设定频率为`1500`, 最终取值将为`1450`.  
#### modes - 模式参数  
|字段            |类型     |定义                         |
|:---------------|:-------|:----------------------------|
|powerLimit      |ArrayInt|CPU整体功耗限制(单位:mW)      |
|perfMargin      |ArrayInt|CPU性能冗余(范围:0-100)       |
|upRateLimitMs   |ArrayInt|CPU频率提升限制(单位:ms)      |
|smoothFreq      |ArrayInt|保持流畅度的最低频率(单位:MHz) |
  
各项参数均使用`ArrayInt`即整数数组方式存储, 数组的序号对应策略组编号.  
CPU整体功耗限制会影响CPU频率和性能上限，混合调频器计算的是满载功耗，不会随CPU负载变化而改变.  
CPU频率提升限制用于降低CPU频率被提升得过高的几率, 每次提升CPU频率后需要在超过`upRateLimitMs`设置的时长后才能再次提升.  
频率提升限制和性能冗余会影响CPU频率提升是否积极，频率提升限制越低冗余越高CPU频率提升越积极，性能越好，耗电越严重.  
#### freqBurst - CPU频率加速  
CPU频率加速可以在特定条件触发时调高CPU频率提升积极性，用于降低重负载下卡顿的几率.  
触发条件包含`tap` `swipe` `gesture` `heavyload`，分别在点击屏幕、滑动屏幕、进行手势操作和重负载(如点亮屏幕)时触发.  
触发的优先级为`tap` < `swipe` < `gesture` < `heavyload`, 当更高优先级的加速触发时将覆盖低优先级的加速, 反之则忽略.  
|字段            |类型   |定义                         |
|:---------------|:------|:---------------------------|
|unlimitPower    |bool   |是否不限制功耗               |
|boost           |int    |频率加速值(范围:0-100)       |
|durationTime    |int    |CPU频率加速持续时间(单位:ms)  |
  
当要求调频器不限制功耗时模式的`powerLimit`将被忽略, 适用于应用冷启动等瞬时高负载场景.  
频率加速时CPU负载计算公式如下: `cpuLoad = cpuLoad + (100 - cpuLoad) * boost`.  
当CPU加速触发的时间超过`durationTime`时，将会终止CPU加速.  
#### heatControl - 发热控制  
发热控制可以按照CPU温度动态调整CPU整体功耗上限(优先级大于模式的`powerLimit`和`freqBurst`的`unlimitPower`).  
此项配置类型为`ArrayJson`, 即数组中的每个Json元素对应一个发热控制策略.  
|字段            |类型   |定义                          |
|:---------------|:------|:----------------------------|
|cpuTemp         |int    |CPU温度(单位:摄氏度, 范围0-99) |
|maxPower        |int    |CPU整体功耗上限(单位:mW)       |
  
当CPU温度大于设定的`cpuTemp`时, CPU整体最大功耗将被限制在`maxPower`内, 例如:  
``` Json
[
  {"cpuTemp": 80, "maxPower": 5000},
  {"cpuTemp": 85, "maxPower": 4000},
  {"cpuTemp": 90, "maxPower": 3000}
]
```
当CPU温度低于80度时将不限制最大功耗, CPU温度大于80度低于85度时最大功耗限制在5000mW, CPU温度大于85度低于90度时最大功耗限制在4000mW.
### ThreadSchedOpt - 线程调度优化  
> 此模块可以通过智能分类线程来实现比安卓系统默认更优秀的调度策略
  
ThreadSchedOpt模块基于线程名称和CPU占用等数据智能分类前台线程，组别如下:  
`MainThread`分组: 包含应用程序的主线程.   
`GameSingleThread`分组: 包含游戏程序中占用CPU单核性能较多的线程.  
`GameMultiThread`分组: 包含游戏程序中占用CPU多核性能较多的线程.  
`UIThread`分组: 包含应用程序中负责生成用户界面的相关线程.  
`MediaThread`分组: 包含应用程序中负责媒体（包括音频/视频解码或渲染）的相关线程.  
`WebViewThread`分组: 包含应用程序中WebView组件的相关线程.  
`ProcessThread`分组: 包含应用程序中负责数据处理的相关线程.  
`NonRealTimeThread`分组: 包含应用程序中不需要数据实时处理的线程.  
`OtherThread`分组: 包含其他未被分类的线程.     
|字段           |类型    |定义                              |
|:--------------|:-------|:--------------------------------|
|cpus           |ArrayInt|此分组的cpu亲和性设定              |
|nice           |int     |此分组的调度优先级(范围:-20~19)    |

cpu亲和性设定即为限制线程仅能在指定的CPU核心上运行, 例如`"cpus": [0, 1]`将限制线程仅能运行在CPU0, CPU1上.  
调度优先级与系统调度nice值定义相同，范围为-20~19，数字越小优先级越高.  
### MtkGpuGovernor - 联发科GPU调频器  
> 这个简易的GPU调频器可以满足对联发科GPU频率的简单调控  
#### 调频器全局设定  
|字段            |类型     |定义                     |
|:---------------|:-------|:------------------------|
|slowSampleTime  |int     |慢速采样间隔时间(单位:ms)  |
|fastSampleTime  |int     |快速采样间隔时间(单位:ms)  |
|preferredFreq   |ArrayInt|偏好GPU频率(单位:MHz)     |
  
当GPU负载为0时将按照`slowSampleTime`间隔读取GPU负载，当负载非0时按照`fastSampleTime`间隔读取GPU负载.   
由于联发科默认的GPU频率表频点数量过多，此调频器将只会选取部分GPU频率，具体信息请查看调度日志.  
`preferredFreq`为偏好的GPU频率,调度生成GPU频率表时将优先考虑这些GPU频率.  
#### modes - 模式参数 
|字段            |类型    |定义                      |
|:---------------|:-------|:------------------------|
|maxFreq         |int     |GPU频率上限(单位:MHz)     |
|upRateLatency   |int     |GPU频率提升延迟(单位:ms)  |
|perfMargin      |int     |GPU性能冗余(范围:0-100)   |
  
与CpuGovernor相同，性能冗余将会影响GPU频率提升是否积极, 冗余越高GPU频率提升越积极. 
### FileWriter - 文件写入器
> 此模块用于在触发某些场景时自动写入文件
#### scenes - 场景触发器
当触发指定场景时将会自动向文件中写入预设的文本, 写入方式与`echo [text] > [path]`相同且效率更高, 写入单个文件的耗时通常不超过1ms.  
支持的场景如下:  
`init`: 调度初始化时触发, 仅执行一次.  
`screenOn`: 屏幕点亮时触发.  
`screenOff`: 屏幕熄灭时触发.  
`powersaveMode`: 切换到powersave模式时触发.  
`balanceMode`: 切换到balance模式时触发.  
`performanceMode`: 切换到performance模式时触发.  
`fastMode`: 切换到fast模式时触发.  
此项配置类型为`ArrayJson`, 即数组中的每个Json元素对应一个文件写入任务.  
|字段            |类型    |定义                      |
|:---------------|:-------|:------------------------|
|path            |string  |写入的目标地址            |
|text            |string  |需要写入的文本            | 
