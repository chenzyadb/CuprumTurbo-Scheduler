## CuprumTurbo Adjustment V3  
#### 一个通用动态性能调节工具(Universal Dynamic Performance Adjust Tool)  
### 特性  
1.不依赖于第三方内核及其他工具进行性能调整  
2.极高的兼容性,无需人工填写配置文件,通过抓取内核和系统提供的数据进行分析调整  
3.不需要busybox，仅需安卓系统自带的binary即可运行  
4.简单易上手，提供magisk模块一键安装，自动完成配置，内置APP进行方便快捷的模式切换  
5.在magisk以外的环境也可运行，仅需在终端中执行打包后的CuLoader文件自动完成首次配置和运行  
6.通过CuAware分析系统负载，针对不同场景进行智能的性能调控  
7.CuAware仅需root权限即可运行，不需要关闭SELinux，维持系统的安全和稳定  
8.兼容Linux3.10+的HMP/EAS内核，支持interactive(HMP);sched,schedutil(EAS)调度器  
9.支持通过内核提供的perfmgr,ppm,kgsl,devfreq...接口，对CPU和GPU性能进行调整  
10.支持Snapdragon/Helio/Dimensity/Kirin/UniSoc/Tegra/Exynos等多种手机Soc  
11.支持安卓8-12的arm64位操作系统  
### 组件及功能  
#### CuAware (Cuprum Aware Engine):  
- 识别当前系统的场景状态并动态的对性能进行调整  
- 每隔一段时间自动采样系统负载并对CPU频率
- 接收来自调度其他组件提供的系统场景数据
- 监测PowerManager状态以识别手机屏幕的熄灭和点亮  
- 从Linux内核层面读取手机屏幕的滑动及点击数据  
- 实时获取调度当前模式并通知libcuprum进行基本sysfs节点调整  
#### CuLoader:
- 自动准备调度所有的临时文件及环境，调整基本sysfs节点以保证调度正常运行  
- 自动抓取并计算调度的CPU频率表，无需人工填写，降低开发者的工作量  
- 基于CPU频率和核心等数据计算CPUMask，使CuAware能基于当前频率和负载精确获取需要调整到的CPU频率  
- 唤醒调度所有组件，保证组件协调运行  
- Cuprum SafetyMode 安全机制，在监测调度导致系统崩溃后自动阻止调度二次运行  
- 提前绑定所有系统重要进程到大核，将不重要进程绑定到小核以减少大核唤醒
- 关闭所有内核态boost和hotplug，结束所有用户态boost，保证调度CPU频率调整不被干扰
#### libcuprum:
- 基于CuprumTurbo Adjustment V2的cuprumd，继承来自旧版本的全部调整功能  
- 被CuAware唤醒后自动调整CPU GPU CGROUP DDR UFS等sysfs节点  
- 包含Kirin，Dimensity，Helio，Snapdragon，Exynos，Unisoc，Tegra等多个芯片厂商的通用sysfs节点  
- sysfs节点写入全部在1s以内完成(Cortex-A55*4@1.0Ghz)  
#### TasksetHelper:
- 在CuprumTurbo V3.0.9 beta版本加入  
- 实时监测安卓cgroup分组更新情况(Linux 4.1+)  
- 基于oom_score_adj等数据判断进程类型  
- 将重要的前台进程绑定至大核，非重要进程维持调度默认设定  
- 每500ms扫描一次前台进程，实时对CPU进程绑定进行调整  
- 优化性能模式下游戏进程的绑定，改善《原神》《崩坏3》等游戏的游玩体验  
#### frame_analysis:
- 效果不佳，在CuprumTurbo V3.0.9 beta版本删除  
- 使用分析dumpsys SurfaceFlinger --latency数据的方式获取当前掉帧情况并通知CuAware提升频率  
#### taskclassify:
- 效果不佳，在CuprumTurbo V3.0.9 beta版本删除  
- 使用dumpsys windows 方式抓取前台APP包名
- 绑定前台APP至大核，后台APP即时写入background cpuset以降低大核使用频率
- 已使用基于c语言重写的TasksetHelper代替
### 安装  
1.通过提供的magisk模块：下载发行版的压缩包并在magisk20.3+刷入并重启  
2.通过终端中运行打包后的调度文件：  
①下载发行版的压缩包并解压  
②提取/common/bin/下的所有文件  
③通过绝对路径运行或者放置到/system/bin/目录下  
④chmod0777给予运行权限并且输入su命令给予超级用户权限  
⑤输入CuLoader或者绝对路径/CuLoader运行  
⑥查看/data/Cuprum_Log.txt检查调度是否正常运行  
### 调度模式切换  
1.通过自带的app进行性能切换  
2.echo [模式] > /data/Cuprum_Custom/mode  
3.sh /data/powercfg.sh [模式]  
*[模式]包括 *Powersave Balance Performance*，如果填写不存在的模式调度将崩溃
### 使用中可能遇到的问题  
Q：为何刷入后仍不省电  
A：调度仅仅只是降低多余的性能消耗，如果是使用习惯导致的耗电（如挂载大量后台或者运行大型游戏）那么调度也不能帮助你  
Q：为何系统出现异常卡顿耗电  
A：调度要求在官方内核，默认调度状态下运行，如果已经刷入第三方内核或者已经使用其他调度那么可能会出现兼容性问题或者调度冲突导致的异常  
Q：是否需要删除系统温控  
A：请自便，调度不删除温控仍可正常运行，性能模式在删除温控的情况下效果会更好但是发热也会更严重  
### 支持的处理器平台：（带*表示可能不兼容）  
Snapdragon 429,430,435,439,450,460,480,610,61x,625,626,630,632,636,650,652,653,66x(AIE),670,675,690  
710,712,720(G),730(G),750(G),76x(G),778(G)* ,780(G)* ,82x(EAS Only),835,845(include overclock),855(Plus)  
865(Plus),870* ,888(Plus)*  
Kirin 65x,710(A/F),810,820,950,955,960,970,980,985,990(E),9000(E)  
MediaTek mt6750,mt6752,mt6753  
Helio A22,P10,P20,P22,P25,P35,P60,P65,P70,P90,G25,G35,G70,G80,G85,G90(T),G95,X10  
Dimensity 700,720,800,800U,810,820,900,920,1000(Plus),1000L,1100,1200,1220  
Exynos 7420,7580,7870,9611,9609,8890*,8895*,9810*,9820*,9825*,990*,980*,1080*,2100*  
Unisoc T618,T740,SC9863A  
NVIDIA Tegra X1(16nm/20nm)
