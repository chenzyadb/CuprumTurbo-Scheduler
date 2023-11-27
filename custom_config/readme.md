## CuprumTurbo V17 自定义配置开发文档  
此项目使用模块化设计, 每个子Json对象对应一个`Module`, 通过设置各个模块的`enable`字段可以启用或禁用模块.  
当`enable`字段设置为`false`时程序将不会继续加载其余参数, 可以删除该子Json对象中的其余字段.  
### Json信息  
在这里你可以定义配置文件的名称和作者信息, 请注意不要修改配置文件版本, 此字段将会用于验证调度与该配置是否兼容.  
|字段            |类型   |定义               |
|:---------------|:------|:-----------------|
|name            |string |配置文件的名称     |
|author          |string |配置文件的作者信息  |
|configVersion   |int    |配置文件版本       |

```json
{
  "name": "Dimensity1100/1200/1300",
  "author": "chenzyadb",
  "configVersion": 8,
    .....
    
}
```

### CpuGovernor - CPU混合调频器  
> 此模块通过在各种不同场景下选择合适的CPU频率以改善使用体验.  
#### params - 调频器参数  
|字段             |类型   |定义                 |
|:---------------|:------|:-------------------|
|activeRateHz    |int    |活跃时工作频率        |
|idleRateHz      |int    |空闲时工作频率        |
|minFreqStep     |int    |最小频率差值          |

```json
"CpuGovernor": {
    "enable": true,
    "params": {
      "activeRateHz": 60,
      "idleRateHz": 30,
      "minFreqStep": 200
    },
    .....
}
         
```
工作频率是CPU混合调频器的重要参数, 通常Linux内核频率为300HZ,即3.33ms记录一次, 活跃时间/总时间*100即为CPU负载百分比.  
如果工作频率过高将会导致调频器的开销增加且无法获得有效的CPU负载(例如100HZ时只能获得0% 33% 66% 100%四种负载), 过低将导致调频器无法应对瞬时负载.  
最小频率差值为生成CPU频率表的关键参数, 设置得过小将会导致调频速度过慢,过大将会导致调频不够精细.  
#### policies - 策略组    
此项配置类型为`ArrayJson`, 即数组中的每个Json元素对应一个策略组.  
每个策略组中的CPU频率将会同步控制, 应当与内核中每个cluster中包含的CPU对应.  
由于是按照数组的序号来为策略组编号的, 所以策略组的排序应与cluster的排序一致.  
例如SDM845为4+4设计, 即`policy0: CPU0-3; policy1: CPU4-7`.  
|字段            |类型    |定义                                    |
|:--------------|:-------|:---------------------------------------|
|coreNum        |int     |策略组中包含的CPU核心数量                 |
|perfScale      |int     |CPU相对同频算力值                        |
|lowPowerFreq   |int     |CPU功耗最低频率(单位:MHz)                |
|optimalFreq    |int     |CPU最优频率(单位:MHz)                    |
|modelFreq      |int     |用于生成CPU功耗模型的CPU频率(单位:MHz)     |
|modelPower     |int     |处于modelFreq时CPU的满载功耗(单位:mW)     |
  
`CpuGovernor`模块设定中的所有频率都将会被取近似值, 例如CPU频率表中有`1200, 1450, 1700`三个频率, 设定频率为`1500`, 最终取值将为`1450`.  

```json
"CpuGovernor": {
    ....
    policies": [
      {
        "coreNum": 4,
        "perfScale": 100,
        "lowPowerFreq": 600,
        "optimalFreq": 1400,
        "modelFreq": 1800,
        "modelPower": 400
      },
      {
        "coreNum": 4,
        "perfScale": 180,
        "lowPowerFreq": 800,
        "optimalFreq": 2200,
        "modelFreq": 2800,
        "modelPower": 1760
      }
    ],
    ......
}
          
```
#### modes - 模式参数  
|字段            |类型     |定义                         |
|:---------------|:-------|:----------------------------|
|powerLimit      |int     |CPU整体功耗限制(单位:mW)       |
|perfMargin      |ArrayInt|CPU性能冗余(范围:0-100)        |
|upRateLatency   |int     |CPU频率提升延迟(单位:ms)       |


```json
"CpuGovernor": {
   ......
    "modes": {
      "powersave": {
        "powerLimit": 3000,
        "perfMargin": [10, 10, 10],
        "upRateLatency": 1000,
       
      },
      "balance": {
     ......
      },
      "performance": {
     ......
      },
      "fast": {
     ......
    }
  }
}
          
```
  
CPU整体功耗限制会影响CPU频率上限, 调频器计算的是满载功耗,不会随CPU负载变化而改变.  
`perfMargin`使用`ArrayInt`即整数数组方式存储参数, 数组的序号对应策略组编号.  
CPU频率提升延迟用于降低CPU频率被提升得过高的几率, 每次升频时调频器都会根据频率提升延迟和能耗比变化判定是否需要升频.  
频率提升延迟和性能冗余会影响CPU频率提升是否积极, 延迟越低冗余越高CPU频率提升越积极, 性能越好, 耗电越严重.  
##### freqBurst - CPU频率加速  
CPU频率加速可以在特定条件触发时调高CPU频率提升积极性, 用于降低部分场景下卡顿的几率.  
|字段            |类型   |定义                         |
|:---------------|:------|:---------------------------|
|durationTime    |int    |频率加速持续时间(单位:ms)     |
|unlimitPower    |bool   |是否不限制功耗               |
|lowLatency      |bool   |是否降低延迟                 |
|extraMargin     |int    |额外性能冗余(范围:0-100)      |
|boost           |int    |频率加速值(范围:0-100)       |
  
触发条件包含`tap` `swipe` `gesture` `heavyload` `jank` `bigJank`,分别在 点击屏幕 滑动屏幕 手势操作 重负载 掉帧 严重掉帧 时触发.  
触发的优先级为`none` < `tap` < `swipe` < `gesture` < `heavyload` < `jank` < `bigJank`, 当更高优先级的加速触发时将覆盖低优先级的加速.  
当要求调频器不限制功耗时模式的`powerLimit`将被忽略, 适用于应用冷启动等短时间性能需求极高的场景.  
当要求调频器降低延迟时调频器将会以最快的速度提升CPU频率, 适用于检测到掉帧等需要迅速提升CPU频率的场景.  
`extraMargin`值用于提供额外的性能冗余, 计算公式如下: `acturalMargin = perfMargin + extraMargin`.  
`boost`值用于夸大实际的CPU负载, 计算公式如下: `cpuLoad = cpuLoad + (100 - cpuLoad) * boost / 100`.  

```json
"CpuGovernor": {
   ......
    "modes": {
      "powersave": {
        "powerLimit": 3000,
        "perfMargin": [ 10, 10, 10 ],
        "upRateLatency": 1000,
        "freqBurst": {
          "none": {
            "durationTime": 0,
            "unlimitPower": false,
            "lowLatency": false,
            "extraMargin": 0,
            "boost": 0
          },
          "tap": {
            "durationTime": 1000,
            "unlimitPower": false,
            "lowLatency": false,
            "extraMargin": 0,
            "boost": 20
          },
          "swipe": {
            "durationTime": 500,
            "unlimitPower": false,
            "lowLatency": false,
            "extraMargin": 10,
            "boost": 0
          },
          "gesture": {
            "durationTime": 1000,
            "unlimitPower": false,
            "lowLatency": false,
            "extraMargin": 20,
            "boost": 20
          },
          "heavyload": {
            "durationTime": 2000,
            "unlimitPower": true,
            "lowLatency": false,
            "extraMargin": 20,
            "boost": 0
          },
          "jank": {
            "durationTime": 0,
            "unlimitPower": false,
            "lowLatency": false,
            "extraMargin": 0,
            "boost": 0
          },
          "bigJank": {
            "durationTime": 100,
            "unlimitPower": true,
            "lowLatency": false,
            "extraMargin": 0,
            "boost": 40
          }
        },
      },
      "balance": {
     ......
    },
      "performance": {
     ......
    },
      "fast": {
     ......
    }
  }
}   
  
          
```

##### heatControl - 发热控制  
发热控制可以按照CPU温度动态调整CPU整体功耗上限(优先级大于模式的`powerLimit`和`freqBurst`的`unlimitPower`).  
此项配置类型为`ArrayJson`, 即数组中的每个Json元素对应一个发热控制策略.  
|字段            |类型   |定义                          |
|:---------------|:------|:----------------------------|
|cpuTemp         |int    |CPU温度(单位:摄氏度, 范围0-99) |
|maxPower        |int    |CPU整体功耗上限(单位:mW)       |
  
当CPU温度大于设定的`cpuTemp`时, CPU整体最大功耗将被限制在`maxPower`内, 例如:  

```json
"CpuGovernor": {
    ......
    "modes": {
      "powersave": {
     ......
        "heatControl": [
         {"cpuTemp": 80, "maxPower": 5000},
         {"cpuTemp": 90, "maxPower": 4000}
        ]
      },
     ......
  }
}
          
```
当CPU温度小于80度时将不限制最大功耗, CPU温度大于等于80度小于90度时最大功耗限制在5000mW, CPU温度大于等于90度时最大功耗限制在4000mW.

### ThreadSchedOpt - 线程调度优化  
> 此模块通过智能分类线程来实现较为合理的线程调度策略  
  
ThreadSchedOpt模块基于线程名称和CPU占用等数据分类前台线程, 组别如下:  
`MainThread`分组: 包含应用程序的主线程.   
`GameSingleThread`分组: 包含游戏程序中占用CPU单核性能较多的线程.  
`GameMultiThread`分组: 包含游戏程序中占用CPU多核性能较多的线程.  
`UIThread`分组: 包含应用程序中参与渲染用户界面的相关线程.  
`MediaThread`分组: 包含应用程序中负责媒体（例如音频/视频解码）的相关线程.  
`WebViewThread`分组: 包含应用程序中WebView组件的相关线程.  
`ProcessThread`分组: 包含应用程序中负责数据处理的相关线程.  
`NonRealTimeThread`分组: 包含应用程序中不需要数据实时处理的线程.  
`OtherThread`分组: 包含其他未被分类的线程.  



|字段           |类型    |定义                              |
|:--------------|:-------|:--------------------------------|
|cpus           |ArrayInt|此分组的cpu亲和性设定              |
|nice           |int     |此分组的调度优先级(范围:-20~19)    |

cpu亲和性设定即为限制线程仅能在指定的CPU核心上运行, 例如`"cpus": [0, 1]`将限制线程仅能运行在CPU0, CPU1上.  
调度优先级与系统调度nice值定义相同, 范围为-20~19,数字越小优先级越高.  


```json
{  
  ......
  
 "ThreadSchedOpt": {
    "enable": true,
    "MainThread": {
      "cpus": [ 4, 5, 6 ],
      "nice": -10
    },
    "GameSingleThread": {
      "cpus": [ 7 ],
      "nice": -16
    },
    "GameMultiThread": {
      "cpus": [ 4, 5, 6 ],
      "nice": -16
    },
    "UIThread": {
      "cpus": [ 4, 5, 6 ],
      "nice": -10
    },
    "MediaThread": {
      "cpus": [ 0, 1, 2, 3, 4, 5, 6 ],
      "nice": -16
    },
    "WebViewThread": {
      "cpus": [ 4, 5, 6, 7 ],
      "nice": -10
    },
    "ProcessThread": {
      "cpus": [ 4, 5, 6 ],
      "nice": -8
    },
    "NonRealTimeThread": {
      "cpus": [ 0, 1, 2, 3 ],
      "nice": 0
    },
    "OtherThread": {
      "cpus": [ 0, 1, 2, 3, 4, 5, 6 ],
      "nice": 0
    }
  },
  ......
}
```

### MtkGpuGovernor - 联发科GPU调频器  
> 这个简易的GPU调频器可以满足对联发科GPU频率的基础调控  
#### params - 调频器参数  
|字段             |类型    |定义                     |
|:---------------|:-------|:-----------------------|
|activeRateHz    |int     |活跃时工作频率            |
|idleRateHz      |int     |空闲时工作频率            |
|preferredFreq   |ArrayInt|偏好GPU频率(单位:MHz)     |
  
当GPU负载为0时调频器按照`idleRateHz`频率工作, 当负载非0时调频器按照`activeRateHz`频率工作.  
由于联发科内核提供的GPU频率数量过多, 此调频器将只会选取部分GPU频率,具体信息请查看调度日志.  
`preferredFreq`为偏好的GPU频率, 调度选取GPU频率时将优先考虑这些频率.  

```json
{
 ......
 "MtkGpuGovernor": {
    "enable": true,
    "params": {
      "activeRateHz": 60,
      "idleRateHz": 30,
      "preferredFreq": [ 540, 660, 770 ]
    },
   .....
  },
  ......
}
```

#### modes - 模式参数 
|字段            |类型    |定义                      |
|:---------------|:-------|:------------------------|
|maxFreq         |int     |GPU频率上限(单位:MHz)     |
|minFreq         |int     |GPU频率下限(单位:MHz)     |
|upRateThres     |int     |GPU升频阈值(范围:0-100)   |
|downRateDiff    |int     |GPU降频差值(范围:0-100)   |
  
当GPU负载大于`upRateThres`时提升频率, 当GPU负载减少的差值大于`downRateDiff`时降低频率.  
例如: 设置`upRateThres=90, downRateDiff=10`, 当GPU负载为`75`时降低GPU频率, 当GPU负载为`85`时GPU频率不变,当GPU负载为`95`时提升GPU频率.  
`upRateThres`的值越小升频越积极, `downRateDiff`的值越大降频越缓慢, `downRateDiff`的值不得大于`upRateThres`.  

```json
{
 ......
  "MtkGpuGovernor": {
    "enable": true,
    "params": {
      ......
    },
    "modes": {
      "powersave": {
        "maxFreq": 540,
        "minFreq": 0,
        "upRateThres": 90,
        "downRateDiff": 10
      },
      "balance": {
      ......
      },
      "performance": {
      ......
      },
      "fast": {
      ......
      }
    }
  },
  ......
}
```

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
  
