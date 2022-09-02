## CuprumTurbo 自定义配置文件 帮助文档  
CuprumTurbo V12 支持用户对调度配置进行自定义，通过修改调度参数可以满足用户对性能调控的多元化需求.  
在进行自定义前请务必仔细阅读此文档以了解调度的工作原理，避免操作不当导致无法达到预定目标.  
### json头部信息  
|字段   |类型 |定义             |
|:-----|:----|:----------------|
|name  |char |配置文件的名称    |
|author|char |配置文件的作者信息 |
### 调度组件设定  
CuprumTurbo 调度包含多个组件，各个组件均由单独的线程执行，使用全局变量传递参数，互不干扰.  
所有设定将在调度初始化时统一读取，组件将会按照预设的参数执行.
#### TasksetHelper 线程优化组件  
在同样的性能释放下，优化线程的亲和性设定可以极大地改善用户的使用体验，在做到低功耗的同时不降低设备的流畅度  
TasksetHelper支持扫描前台窗口的所有线程，并按照线程名称和类型进行硬亲和性设定和调度优先级设定  
前台窗口信息均由CuToolbox无障碍服务提供，无障碍使用AccessibilityEvent事件驱动，开销极低  
通过修改配置文件可以自定义各种类型线程的绑核及调度优先级设定，TasksetHelper的线程分类如下：
- `efficiency`性能需求极低的日志/垃圾回收线程，尽量降低分配给此类线程的CPU资源以减少性能浪费.  
- `common`性能需求中等的普通线程，如WebView/Download/Media线程，无需分配大量的CPU资源.  
- `multi_perf`性能需求较高且依赖多核性能的线程，如Unity引擎的WorkerThread，必须优先考虑此类线程的CPU资源分配且尽量绑定在较多的CPU核心上，如CPU大核丛集.  
- `single_perf`性能需求较高且依赖单核性能的线程，如负责界面渲染的RenderThread，必须优先考虑此类线程的CPU资源分配且尽量绑定在单核性能较强的CPU核心上，如CPU超大核丛集. 
- `other`其他未知线程，尽量不与`single_perf`重叠以避免抢占单核性能.
##### TasksetHelper Scene定义
TasksetHelper支持两种Scene，`normal`对应的是普通情况下的设定，`boost`对应的是瞬时性能需求较高情况下的设定，主要为了应对手机亮屏/app冷启动等场景.
##### 线程类型对应参数设定   
|字段           |类型  |定义                          |
|:--------------|:----|:-----------------------------|
|cpu_set_t      |char |线程绑核设定，按照%d-%d方式读取 |
|sched_priority |int  |线程调度优先级设定，范围100-139 |
#### PowerModel 能耗模型  
CuprumTurbo用户态调频器依赖计算出的虚拟能耗模型进行功耗限制，仅需提供一组数据就能完成自动计算.  
CPU功耗计算公式:`Power=C*Freq*volt^2`，其中的C为功耗常量，依据提供的真实功耗进行推算.  
其中的`sc`对应小核丛集，`bc`对应大核丛集，`xc`对应超大核丛集，不存在的丛集请将参数全部置零.  
|字段                |类型  |定义                                            |
|:------------------|:-----|:------------------------------------------------|
|sc_capacity        |int   |小核丛集算力值，用于分配cpu功耗                   |
|bc_capacity        |int   |大核丛集算力值，用于分配cpu功耗                   |
|xc_capacity        |int   |超大核丛集算力值，用于分配cpu功耗                 |
|sc_basic_freq_mhz  |int   |小核丛集基础频率，当CPU存在一定负载时保持在此频率 |
|bc_basic_freq_mhz  |int   |大核丛集基础频率，当CPU存在一定负载时保持在此频率 |
|xc_basic_freq_mhz  |int   |超大核丛集基础频率，当CPU存在一定负载时保持在此频率|
|sc_burst_freq_mhz  |int   |小核丛集加速频率，CPU电压在这个挡位开始升高       |
|bc_burst_freq_mhz  |int   |大核丛集加速频率，CPU电压在这个挡位开始升高       |
|xc_burst_freq_mhz  |int   |超大核丛集加速频率，CPU电压在这个挡位开始升高     |
|sc_expect_freq_mhz |int   |小核丛集期望频率，超过此频率的CPU能效比急剧降低   |
|bc_expect_freq_mhz |int   |小核丛集期望频率，超过此频率的CPU能效比急剧降低   |
|xc_expect_freq_mhz |int   |超大核丛集期望频率，超过此频率的CPU能效比急剧降低 |
|sc_current_freq_mhz|int   |小核丛集真实频率，用于建立能耗模型                |
|bc_current_freq_mhz|int   |大核丛集真实频率，用于建立能耗模型                |
|xc_current_freq_mhz|int   |超大核丛集真实频率，用于建立能耗模型              |
|sc_power_mw        |int   |小核丛集真实功耗，用于建立能耗模型                |
|bc_power_mw        |int   |大核丛集真实功耗，用于建立能耗模型                |
|xc_power_mw        |int   |超大核丛集真实功耗，用于建立能耗模型              |
#### Governor 用户态调频器 
一直以来，CuprumTurbo都依靠一套基于CPU负载的主动调频机制进行动态性能限制，她可以无视系统和内核的差异，实现全平台统一体验.  
CuprumTurbo Governor的CPU调频机制类似于Android早期的interactive调频器，依据CPU实时负载进行调频，具体流程如下：  
1.获取实时CPU负载(cpu_load):  
- cpu_load计算公式:`cpu_load=max_usage+(100-max_usage)*boost/100`，其中`max_usage`为丛集最高CPU负载，`boost`为用于加速的额外性能冗余.
  
2.计算目标CPU频率(target_cpu_freq):  
- target_cpu_freq计算公式:`target_cpu_freq=cur_freq*cpu_load/freq_to_load(cur_freq)` `freq_to_load=cur_freq*(100-margin)/next_freq`.  

3.CPU功耗限制:  
- 在不同的调度模式下，CuprumTurbo调度主要通过CPU功耗限制来对应不同的性能释放需求，CPU功耗限制拥有两种状态，一种为常规状态，一种为加速状态.   
- CuprumTurbo调度依靠两种状态的切换来应对复杂的使用场景，在保证长时间使用下CPU功耗受控的同时，尽量提供充足的性能释放.     
- 当调度boost被触发时切换至加速状态功耗限制，提供充足的冗余以应对瞬时高爆发负载；当boost结束时使用常规状态功耗限制以降低功耗.   
- 当CPU功耗超过当前状态的功耗限制时，将会按照不同丛集的算力来分配各个丛集的最大功耗，扫描CPU频率表中对应的功耗并选择一个低于功耗限制的频率作为新的目标频率.   

4.CPU升降频限制:  
- 当目标频率低于当前频率且距离上次降低CPU频率的间隔小于`freq_down_delay`时，将会忽略当前的降频请求.  
- 当目标频率高于当前频率且距离上次提升CPU频率的间隔小于`freq_up_delay`时，将会忽略当前的升频请求.   

5.写入CPU频率:  
- CuprumTurbo的CPU频率写入器经过高度优化，性能开销极低，写入器支持多线程并行，每次写入CPU频率的时间低于0.1ms.  
##### 用户态调频器可以自定义的参数如下，按照`powersave` `balance` `performance` `fast`四个调度模式进行分类:  
|字段               |类型    |定义                                   |
|:-----------------|:-------|:--------------------------------------|
|sc_perf_margin    |int     |小核丛集性能冗余                        |
|bc_perf_margin    |int     |大核丛集性能冗余                        |
|xc_perf_margin    |int     |超大核丛集性能冗余                      |
|sc_freq_up_delay  |int     |小核丛集升频延迟(单位:ms)               |
|bc_freq_up_delay  |int     |大核丛集升频延迟(单位:ms)               |
|xc_freq_up_delay  |int     |超大核丛集升频延迟(单位:ms)             |
|sc_freq_down_delay|int     |小核丛集降频延迟(单位:ms)               |
|bc_freq_down_delay|int     |大核丛集降频延迟(单位:ms)               |
|xc_freq_down_delay|int     |超大核丛集降频延迟(单位:ms)             |
|comm_limit_pwr    |int     |常规状态CPU整体功耗最大值(单位:mW)      |
|boost_limit_pwr   |int     |加速状态CPU整体功耗最大值(单位:mW)      |
#### Hint 用户态场景触发信号  
CuprumTurbo能识别屏幕滑动，点击，全面屏手势，顶层activity切换场景，可用于触发调度进行实时调整.
Hint 的类型如下:
- `Touch`触摸到屏幕和手指离开屏幕时触发的Hint.
- `Swipe`手指在屏幕上滑动时触发的Hint.
- `Gesture`进行全面屏手势操作时触发的Hint.
- `topActivityChanged`顶层activity切换或活动的窗口变化时触发的Hint.

在触发Hint后，CuprumTurbo将会按照设定的参数切换对应的`boost`并持续`boost_duration`.   
##### Hint的自定义参数如下:
|字段           |类型    |定义                   |
|:--------------|:------|:----------------------|
|boost_type     |char   |对应的boost类型         |
|boost_duration |int    |boost持续的时间(单位:ms)|
#### Boost 用户态调频器加速  
依据Hint设定触发的调度状态调整策略，按照以下优先级切换:  
`heavyload` > `gesture` > `swipe` > `touch` > `none`  
当现在的boost持续时间未超过`boost_duration`且目标boost优先级低于当前boost时，忽略boost的切换请求.  
当距离上一次触发boost的时间超过`boost_duration`或目标boost优先级高于当前boost时，进行boost切换.  
当距离上一次触发boost的时间超过`boost_duration`且没有新的boost切换请求时，自动切换为`none`.  
##### Boost的自定义参数如下:
|字段                    |类型    |定义                                |
|:----------------------|:-------|:-----------------------------------|
|governor.boost         |int     |Boost被触发时用户态调频器调整的boost值|
















