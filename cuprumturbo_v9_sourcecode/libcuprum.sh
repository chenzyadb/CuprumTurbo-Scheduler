#!/system/bin/sh
CUPRUM_DIR=${0%/*}
COMMAND=$1
function write_value() {
 if [ -e $2 ] ; then
  if [ -w $2 ] ; then
   echo "$1" > "$2" 2>/dev/null &
  else
   chmod 0644 $2 2>/dev/null;
   echo "$1" > "$2" 2>/dev/null;
  fi
 fi
}
function lock_value() {
 if [ -e $2 ] ; then
  chmod 0666 $2;
  echo $1 > $2;
  chmod 0444 $2;
 fi
}
if [ -d /sys/devices/system/cpu/cpu7/ ] ; then
 core_num="7"
elif [ -d /sys/devices/system/cpu/cpu5/ ] ; then
 core_num="5"
elif [ -d /sys/devices/system/cpu/cpu3/ ] ; then
 core_num="3"
fi
if [ $COMMAND = "init" ] ; then
 #Disable userspace boosts
 lock_value "0" /sys/module/migt/parameters/enable
 lock_value "0" /proc/sys/migt/enable
 lock_value "0" /sys/module/migt/parameters/migt_freq
 lock_value "0" /sys/module/migt/parameters/migt_ms
 chmod 0000 /dev/migt
 chmod -R 0000 /sys/module/migt/parameters/
 chmod -R 0000 /proc/sys/migt/
 stop miuibooster 2>/dev/null
 stop oneplus_brain_service 2>/dev/null
 #I want to disable them,but it may cause some device panic.
 #stop vendor.perfservice
 #stop perfd
 #Disable userspace thermal
 for i in $(seq 0 7) ; do
  lock_value "cpu${i} 9500000" /sys/devices/virtual/thermal/thermal_message/cpu_limits
 done
 for i in $(seq 0 7) ; do
  lock_value "cpu${i} 9500000" /sys/class/thermal/thermal_message/cpu_limits
 done
 #Disable HardCoder
 setprop persist.sys.hardcoder.name ""
 #Disable MTK Boosts
 if [ -e /proc/perfmgr/eara_ioctl ] ; then
  chmod 0000 /proc/perfmgr/eara_ioctl
 fi
 if [ -e /proc/perfmgr/eas_ioctl ] ; then
  chmod 0000 /proc/perfmgr/eas_ioctl
 fi
 if [ -e /proc/perfmgr/xgff_ioctl ] ; then
  chmod 0000 /proc/perfmgr/xgff_ioctl
 fi
 if [ -e /sys/devices/system/cpu/sched/sched_boost ] ; then
  lock_value 0 /sys/devices/system/cpu/sched/sched_boost
  lock_value 1 "/sys/devices/system/cpu/eas/enable"
 fi
 lock_value "0" /sys/module/mtk_fpsgo/parameters/boost_affinity
 lock_value "0" /sys/kernel/debug/fpsgo/fbt/switch_idleprefer
 lock_value "0" /sys/kernel/debug/fpsgo/common/force_onoff
 lock_value "1" /sys/kernel/debug/fpsgo/common/stop_boost
 lock_value "0" /sys/kernel/fpsgo/fbt/switch_idleprefer
 lock_value "0" /sys/kernel/fpsgo/common/force_onoff
 lock_value "1" /sys/kernel/fpsgo/common/stop_boost
 lock_value "0" /sys/module/mt_hotplug_mechanism/parameters/g_enable
 lock_value "1" /sys/devices/system/cpu/cpufreq/hotplug/cpu_hotplug_disable
 lock_value "enable=1" /proc/sla/config
 lock_value "1" /proc/perfmgr/syslimiter/syslimiter_force_disable
 lock_value "100" /proc/perfmgr/syslimiter/syslimitertolerance_percent
 lock_value "0" /sys/kernel/eara_thermal/enable
 lock_value "0" /sys/kernel/eara_thermal/fake_throttle
 lock_value "enable: 0" /proc/perfmgr/tchbst/user/usrtch
 lock_value "0" /proc/hps/enabled
 lock_value "0" /proc/ppm/enabled
 lock_value "0" /proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_enable
 lock_value "0" /proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_up_loading
 lock_value "0" /proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_down_loading
 #Disable QTI/Exynos Boosts
 lock_value "0" /sys/kernel/hmp/boost
 lock_value "0" /sys/kernel/hmp/boostpulse_duration
 lock_value "0" /sys/power/cpuhotplug/enabled
 lock_value "0" /sys/kernel/ems/eff_mode
 lock_value "0" /sys/devices/system/cpu/cpuhotplug/enabled
 lock_value "0" /sys/module/msm_thermal/vdd_restriction/enabled
 lock_value "0" /sys/module/msm_thermal/core_control/enabled
 lock_value "N" /sys/module/msm_thermal/parameters/enabled
 lock_value "0" /sys/kernel/intelli_plug/intelli_plug_active
 lock_value "0" /sys/module/blu_plug/parameters/enabled
 lock_value "0" /sys/devices/virtual/misc/mako_hotplug_control/enabled
 lock_value "0" /sys/module/autosmp/parameters/enabled
 lock_value "0" /sys/kernel/zen_decision/enabled
 lock_value "0" /sys/devices/system/cpu/cpu_boost/parameters/input_boost_ms
 lock_value "0" /sys/devices/system/cpu/cpu_boost/parameters/powerkey_input_boost_ms
 lock_value "0" /sys/devices/system/cpu/cpu_boost/input_boost_ms
 lock_value "0" /sys/devices/system/cpu/cpu_boost/powerkey_input_boost_ms
 lock_value "0" /sys/module/cpu_boost/parameters/input_boost_ms
 lock_value "0" /sys/module/msm_performance/parameters/touchboost
 lock_value "0" /sys/module/cpu_boost/parameters/boost_ms
 lock_value "0" /sys/kernel/cpu_input_boost/enabled
 lock_value "0" /sys/kernel/cpu_input_boost/ib_freqs
 lock_value "0" /sys/kernel/cpu_input_boost/ib_duration_m
 lock_value "0" /sys/kernel/cpu_input_boost/ib_duration_ms
 lock_value "0" /sys/module/cpu_boost/parameters/input_boost_enabled
 lock_value "0" /sys/module/cpu_boost/parameters/dynamic_stune_boost
 lock_value "0" /sys/module/cpu_boost/parameters/input_boost_ms
 lock_value "0" /sys/module/cpu_boost/parameters/input_boost_ms_s2
 lock_value "0" /sys/module/dsboost/parameters/input_boost_duration
 lock_value "0" /sys/module/dsboost/parameters/input_stune_boost
 lock_value "0" /sys/module/dsboost/parameters/sched_stune_boost
 lock_value "0" /sys/module/dsboost/parameters/cooldown_boost_duration
 lock_value "0" /sys/module/dsboost/parameters/cooldown_stune_boost
 lock_value "0" /sys/module/cpu_input_boost/parameters/input_boost_duration
 lock_value "0" /sys/module/cpu_input_boost/parameters/dynamic_stune_boost
 lock_value "0" /sys/module/cpu_input_boost/parameters/input_boost_freq_lp
 lock_value "0" /sys/module/cpu_input_boost/parameters/input_boost_freq_hp
 lock_value "0" /sys/module/cpu_input_boost/parameters/input_boost_freq_gold
 lock_value "0" /sys/module/cpu_input_boost/parameters/flex_stune_boost_offset
 lock_value "0" /sys/module/cpu_input_boost/parameters/dynamic_stune_boost
 lock_value "0" /sys/module/cpu_input_boost/parameters/input_stune_boost_offset
 lock_value "0" /sys/module/cpu_input_boost/parameters/max_stune_boost_offset
 lock_value "0" /sys/module/cpu_input_boost/parameters/stune_boost_extender_ms
 lock_value "0" /sys/module/cpu_input_boost/parameters/max_stune_boost_extender_ms
 lock_value "0" /sys/module/cpu_input_boost/parameters/gpu_boost_extender_ms
 lock_value "0" /sys/module/cpu_input_boost/parameters/flex_boost_freq_gold
 lock_value "0" /sys/module/cpu_input_boost/parameters/flex_boost_freq_hp
 lock_value "0" /sys/module/cpu_input_boost/parameters/flex_boost_freq_lp
 lock_value "0" /sys/module/devfreq_boost/parameters/input_boost_duration
 lock_value "0" /sys/module/input_cfboost/parameters/boost_enabled
 lock_value "0" /sys/class/input_booster/level
 lock_value "0" /sys/class/input_booster/head
 lock_value "0" /sys/class/input_booster/tail
 lock_value "N" /sys/module/control_center/parameters/cpu_boost_enable
 lock_value "N" /sys/module/control_center/parameters/ddr_boost_enable
 lock_value "0" /sys/module/aigov/parameters/enable
 lock_value "0" /sys/module/houston/parameters/ais_enable
 lock_value "0" /sys/module/houston/parameters/fps_boost_enable
 lock_value "0" /sys/module/houston/parameters/ht_registed
 lock_value "0" /sys/module/opchain/parameters/chain_on
 lock_value "0" /sys/power/pnpmgr/touch_boost
 lock_value "0" /sys/power/pnpmgr/long_duration_touch_boost
 for i in $(seq 0 7) ; do
  if [ -e /sys/devices/system/cpu/cpu${i}/sched_load_boost ] ; then
   lock_value "0" /sys/devices/system/cpu/cpu${i}/sched_load_boost
  fi
 done
 #Schedule Opt
 for sched_type in walt kernel ; do
  lock_value "0" /proc/sys/${sched_type}/sched_boost
  lock_value "1" /proc/sys/${sched_type}/sched_walt_rotate_big_tasks
  lock_value "20000000" /proc/sys/${sched_type}/sched_coloc_downmigrate_ns
  lock_value "5" /proc/sys/${sched_type}/sched_spill_nr_run
  lock_value "95" /proc/sys/${sched_type}/sched_spill_load
  lock_value "1000000" /proc/sys/${sched_type}/sched_min_granularity_ns
  lock_value "0" /proc/sys/${sched_type}/sched_tunable_scaling
  lock_value "40" /proc/sys/${sched_type}/sched_nr_migrate
  lock_value "0" /proc/sys/${sched_type}/sched_util_clamp_min
  if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
   #UltraCore may have 1.2x performance than SuperCore.
   lock_value "30 65" /proc/sys/${sched_type}/sched_downmigrate
   lock_value "60 80" /proc/sys/${sched_type}/sched_upmigrate
  else
   #SuperCore may have 2x performance than EffiencyCore.
   lock_value "30" /proc/sys/${sched_type}/sched_downmigrate
   lock_value "60" /proc/sys/${sched_type}/sched_upmigrate
  fi
  #Disable Group Migrate.
  lock_value "0" /proc/sys/${sched_type}/sched_group_downmigrate
  lock_value "0" /proc/sys/${sched_type}/sched_group_upmigrate
 done
 if [ -d /dev/stune/ ] ; then
  write_value "0" "/dev/stune/schedtune.boost" 
  write_value "0" "/dev/stune/schedtune.prefer_idle"  
  for stune_dir in /dev/stune/* ; do
   write_value "0" "${stune_dir}/schedtune.prefer_idle"  
   write_value "0" "${stune_dir}/schedtune.boost" 
   write_value "0" "${stune_dir}/schedtune.sched_boost_no_override"
  done
 fi
 if [ -d /dev/cpuctl/ ] ; then
  write_value "0" "/dev/cpuctl/cpu.uclamp.sched_boost_no_override"
  write_value "0" "/dev/cpuctl/cpu.uclamp.min"
  write_value "0" "/dev/cpuctl/cpu.uclamp.latency_sensitive"
  for cpuctl_dir in /dev/cpuctl/* ; do
   write_value "0" "${cpuctl_dir}/cpu.uclamp.latency_sensitive"
   write_value "0" "${cpuctl_dir}/cpu.uclamp.min"
   write_value "0" "${cpuctl_dir}/cpu.uclamp.sched_boost_no_override"
  done
 fi
 if [ $core_num = "7" ] ; then
  lock_value "0-1" /dev/cpuset/background/cpus 
  lock_value "0-3" /dev/cpuset/system-background/cpus 
  lock_value "0-6" /dev/cpuset/foreground/cpus 
  lock_value "0-7" /dev/cpuset/top-app/cpus 
  lock_value "0-7" /dev/cpuset/game/cpus
  lock_value "0-3" /dev/cpuset/restricted/cpus
  lock_value "0-7" /dev/cpuset/vr/cpus
 elif [ $core_num = "5" ] ; then
  lock_value "0-1" /dev/cpuset/background/cpus 
  lock_value "0-3" /dev/cpuset/system-background/cpus 
  lock_value "0-5" /dev/cpuset/foreground/cpus 
  lock_value "0-5" /dev/cpuset/foreground/boost/cpus 
  lock_value "0-5" /dev/cpuset/top-app/cpus 
  lock_value "0-5" /dev/cpuset/top-app/boost/cpus 
  lock_value "0-5" /dev/cpuset/game/cpus
  lock_value "0-3" /dev/cpuset/restricted/cpus
  lock_value "0-5" /dev/cpuset/vr/cpus
 elif [ $core_num = "3" ] ; then
  lock_value "0-1" /dev/cpuset/background/cpus 
  lock_value "0-1" /dev/cpuset/system-background/cpus 
  lock_value "0-3" /dev/cpuset/foreground/cpus 
  lock_value "0-3" /dev/cpuset/foreground/boost/cpus 
  lock_value "0-3" /dev/cpuset/top-app/cpus 
  lock_value "0-3" /dev/cpuset/top-app/boost/cpus 
  lock_value "0-3" /dev/cpuset/game/cpus
  lock_value "0-1" /dev/cpuset/restricted/cpus
  lock_value "0-3" /dev/cpuset/vr/cpus
 fi
 #Bring ALL Cores Online
 for i in $(seq 0 7) ; do
  if [ -e /sys/devices/system/cpu/cpu${i}/online ] ; then
   write_value 1 /sys/devices/system/cpu/cpu${i}/online 
  fi
 done
 #Disable Qualcomm Core Control
 for i in $(seq 0 7) ; do
  if [ -e /sys/devices/system/cpu/cpu${i}/core_ctl/enable ] ; then
   write_value "0" /sys/devices/system/cpu/cpu${i}/core_ctl/enable
  fi
 done
 #Disable Kernel Governor
 for i in $(seq 0 7) ; do
  if [ -e /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_governor ] ; then
   lock_value "performance" /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_governor 
  fi
  if [ -e /sys/devices/system/cpu/cpufreq/policy${i}/scaling_governor ] ; then
   lock_value "performance" /sys/devices/system/cpu/cpufreq/policy${i}/scaling_governor 
  fi
 done
 #Mali unlimit Cache
 for cache_sw in /sys/kernel/debug/mali0/ctx/*/infinite_cache ; do
  write_value "Y" $cache_sw
 done
elif [ $COMMAND = "standby" ] ; then
 write_value "1" /proc/hps/enabled
 write_value "1" /proc/ppm/enabled
 WALT_GOV=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors | grep "walt")
 SCHEDUTIL_GOV=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors | grep "schedutil")
 INTERACTIVE_GOV=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors | grep "interactive")
 if [ -n "$SCHEDUTIL_GOV" ] ; then
  CPU_GOV="schedutil"
 elif [ -n "$WALT_GOV" ] ; then
  CPU_GOV="walt"
 elif [ -n "$INTERACTIVE_GOV" ] ; then
  CPU_GOV="interactive"
 else 
  CPU_GOV="ondemand"
 fi
 for i in $(seq 0 7) ; do
  if [ -e /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_governor ] ; then
   lock_value $CPU_GOV /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_governor 
  fi
  if [ -e /sys/devices/system/cpu/cpufreq/policy${i}/scaling_governor ] ; then
   lock_value $CPU_GOV /sys/devices/system/cpu/cpufreq/policy${i}/scaling_governor 
  fi
 done
else
 MODE=$(echo $COMMAND)
#Mali GPU PowerPolicy Adjust
if [ $MODE != "powersave" ] ; then
 for pwrplcy in /sys/devices/*.mali/power_policy ; do
  write_value "always_on" "$pwrplcy"
 done
 for pwrplcy in /sys/devices/platform/*.mali/power_policy ; do
  write_value "always_on" "$pwrplcy"
 done
 write_value "1" /proc/mali/always_on
else
 for pwrplcy in /sys/devices/*.mali/power_policy ; do
  write_value "coarse_demand" "$pwrplcy"
 done
 for pwrplcy in /sys/devices/platform/*.mali/power_policy ; do
  write_value "coarse_demand" "$pwrplcy"
 done
 write_value "0" /proc/mali/always_on
fi
#Mali Dvfs Enabled
write_value "1" /proc/mali/dvfs_enable
#MTK Powersave underclock the GPU
if [ -d /sys/kernel/ged/hal/ ] ; then
 MTK_OPP_NUM_INFO=$(cat /sys/kernel/ged/hal/total_gpu_freq_level_count)
 MTK_OPP_NUM=$(($MTK_OPP_NUM_INFO-1))
 if [ $MODE != "fast" ] ; then
  lock_value "$MTK_OPP_NUM" "/sys/kernel/ged/hal/custom_boost_gpu_freq"
 else 
  lock_value "0" "/sys/kernel/ged/hal/custom_boost_gpu_freq"
 fi
 if [ $MODE = "powersave" ] ; then
  PWSV_OPP_IDX=$(($MTK_OPP_NUM/2))
  lock_value "$PWSV_OPP_IDX" "/sys/kernel/ged/hal/custom_upbound_gpu_freq"
 else
  lock_value "0" "/sys/kernel/ged/hal/custom_upbound_gpu_freq"
 fi
fi
if [ -d /proc/gpufreq/ ] ; then
 if [ $MODE = "fast" ] ; then
  FREQ_DUMP_OPP=$(cat /proc/gpufreq/gpufreq_opp_dump | head -1 | awk '{print $4}')
  GPU_MAX_FREQ=${FREQ_DUMP_OPP%,*}
  lock_value $GPU_MAX_FREQ "/proc/gpufreq/gpufreq_opp_freq"
 else
  lock_value "0" "/proc/gpufreq/gpufreq_opp_freq"
 fi
fi
 if [ $MODE = "fast" ] ; then
  if [ -d /sys/class/kgsl/kgsl-3d0/ ] ; then
   write_value "msm-adreno-tz" "/sys/class/kgsl/kgsl-3d0/devfreq/governor"
   PWR_NUM=$(cat /sys/class/kgsl/kgsl-3d0/num_pwrlevels)
   write_value "0" "/sys/class/kgsl/kgsl-3d0/max_pwrlevel"
   write_value "0" "/sys/class/kgsl/kgsl-3d0/min_pwrlevel"
   write_value "0" "/sys/class/kgsl/kgsl-3d0/default_pwrlevel"
   write_value "0" "/sys/class/kgsl/kgsl-3d0/force_clk_on"
   write_value "0" "/sys/class/kgsl/kgsl-3d0/force_bus_on"
   write_value "1" "/sys/class/kgsl/kgsl-3d0/force_no_nap"
   write_value "0" "/sys/class/kgsl/kgsl-3d0/force_rail_on"
   write_value "N" "/sys/module/adreno_idler/parameters/adreno_idler_active" 
  fi
  if [ -d /sys/devices/platform/mali.0/ ]; then
   write_value "mali_ondemand" "${MALI_GPU_DIR}/governor"
   write_value 40 ${MALI_GPU_DIR}/mali_ondemand/vsync_upthreshold 
   write_value 20 ${MALI_GPU_DIR}/mali_ondemand/vsync_downdifferential  
   write_value 30 ${MALI_GPU_DIR}/mali_ondemand/no_vsync_upthreshold 
   write_value 10 ${MALI_GPU_DIR}/mali_ondemand/no_vsync_downdifferential 
  fi
  if [ -d /sys/module/ged/parameters/ ] ; then
   write_value "0" "/sys/module/ged/parameters/ged_boost_enable" 
   write_value "0" "/sys/module/ged/parameters/boost_gpu_enable" 
   write_value "0" "/sys/module/ged/parameters/boost_extra" 
   write_value "0" "/sys/module/ged/parameters/enable_cpu_boost" 
   write_value "1" "/sys/module/ged/parameters/enable_gpu_boost" 
   write_value "0" "/sys/module/ged/parameters/enable_game_self_frc_detect" 
   write_value "0" "/sys/module/ged/parameters/ged_force_mdp_enable" 
   write_value "0" "/sys/module/ged/parameters/ged_log_perf_trace_enable" 
   write_value "0" "/sys/module/ged/parameters/ged_log_trace_enable" 
   write_value "0" "/sys/module/ged/parameters/ged_monitor_3D_fence_debug" 
   write_value "0" "/sys/module/ged/parameters/ged_monitor_3D_fence_disable" 
   write_value "0" "/sys/module/ged/parameters/ged_monitor_3D_fence_systrace" 
   write_value "0" "/sys/module/ged/parameters/ged_smart_boost" 
   write_value "0" "/sys/module/ged/parameters/gpu_debug_enable" 
   write_value "1" "/sys/module/ged/parameters/gpu_dvfs_enable" 
   write_value "0" "/sys/module/ged/parameters/gx_3D_benchmark_on" 
   write_value "0" "/sys/module/ged/parameters/gx_dfps" 
   write_value "0" "/sys/module/ged/parameters/gx_force_cpu_boost" 
   write_value "0" "/sys/module/ged/parameters/gx_frc_mode" 
   write_value "1" "/sys/module/ged/parameters/gx_game_mode" 
   write_value "1" "/sys/module/ged/parameters/is_GED_KPI_enabled"
  fi
 elif [ $MODE = "balance" ]||[ $MODE = "performance" ] ; then
  if [ -d /sys/class/kgsl/kgsl-3d0/ ] ; then
   write_value "msm-adreno-tz" "/sys/class/kgsl/kgsl-3d0/devfreq/governor"
   PWR_NUM=$(cat /sys/class/kgsl/kgsl-3d0/num_pwrlevels)
   write_value "0" "/sys/class/kgsl/kgsl-3d0/max_pwrlevel"
   write_value "$(($PWR_NUM-1))" "/sys/class/kgsl/kgsl-3d0/min_pwrlevel"
   write_value "$(($PWR_NUM-1))" "/sys/class/kgsl/kgsl-3d0/default_pwrlevel"
   write_value "0" "/sys/class/kgsl/kgsl-3d0/force_clk_on"
   write_value "0" "/sys/class/kgsl/kgsl-3d0/force_bus_on"
   write_value "1" "/sys/class/kgsl/kgsl-3d0/force_no_nap"
   write_value "0" "/sys/class/kgsl/kgsl-3d0/force_rail_on"
   write_value "N" "/sys/module/adreno_idler/parameters/adreno_idler_active" 
  fi
  if [ -d /sys/devices/platform/mali.0/ ]; then
   write_value "mali_ondemand" "${MALI_GPU_DIR}/governor"
   write_value 90 ${MALI_GPU_DIR}/mali_ondemand/vsync_upthreshold 
   write_value 80 ${MALI_GPU_DIR}/mali_ondemand/vsync_downdifferential 
   write_value 80 ${MALI_GPU_DIR}/mali_ondemand/no_vsync_upthreshold 
   write_value 70 ${MALI_GPU_DIR}/mali_ondemand/no_vsync_downdifferential 
  fi
  if [ -d /sys/module/ged/parameters/ ] ; then
   write_value "0" "/sys/module/ged/parameters/ged_boost_enable" 
   write_value "0" "/sys/module/ged/parameters/boost_gpu_enable" 
   write_value "0" "/sys/module/ged/parameters/boost_extra" 
   write_value "0" "/sys/module/ged/parameters/enable_cpu_boost" 
   write_value "0" "/sys/module/ged/parameters/enable_gpu_boost" 
   write_value "0" "/sys/module/ged/parameters/enable_game_self_frc_detect" 
   write_value "0" "/sys/module/ged/parameters/ged_force_mdp_enable" 
   write_value "0" "/sys/module/ged/parameters/ged_log_perf_trace_enable" 
   write_value "0" "/sys/module/ged/parameters/ged_log_trace_enable" 
   write_value "0" "/sys/module/ged/parameters/ged_monitor_3D_fence_debug" 
   write_value "0" "/sys/module/ged/parameters/ged_monitor_3D_fence_disable" 
   write_value "0" "/sys/module/ged/parameters/ged_monitor_3D_fence_systrace" 
   write_value "0" "/sys/module/ged/parameters/ged_smart_boost" 
   write_value "0" "/sys/module/ged/parameters/gpu_debug_enable" 
   write_value "1" "/sys/module/ged/parameters/gpu_dvfs_enable" 
   write_value "0" "/sys/module/ged/parameters/gx_3D_benchmark_on" 
   write_value "0" "/sys/module/ged/parameters/gx_dfps" 
   write_value "0" "/sys/module/ged/parameters/gx_force_cpu_boost" 
   write_value "0" "/sys/module/ged/parameters/gx_frc_mode" 
   write_value "0" "/sys/module/ged/parameters/gx_game_mode" 
   write_value "1" "/sys/module/ged/parameters/is_GED_KPI_enabled"
  fi
 else
  if [ -d /sys/class/kgsl/kgsl-3d0/ ] ; then
   write_value "msm-adreno-tz" "/sys/class/kgsl/kgsl-3d0/devfreq/governor"
   PWR_NUM=$(cat /sys/class/kgsl/kgsl-3d0/num_pwrlevels)
   write_value "$(($PWR_NUM/2))" "/sys/class/kgsl/kgsl-3d0/max_pwrlevel"
   write_value "$(($PWR_NUM-1))" "/sys/class/kgsl/kgsl-3d0/min_pwrlevel"
   write_value "$(($PWR_NUM-1))" "/sys/class/kgsl/kgsl-3d0/default_pwrlevel"
   write_value "0" "/sys/class/kgsl/kgsl-3d0/force_clk_on"
   write_value "0" "/sys/class/kgsl/kgsl-3d0/force_bus_on"
   write_value "1" "/sys/class/kgsl/kgsl-3d0/force_no_nap"
   write_value "0" "/sys/class/kgsl/kgsl-3d0/force_rail_on"
   write_value "N" "/sys/module/adreno_idler/parameters/adreno_idler_active" 
  fi
  if [ -d /sys/devices/platform/mali.0/ ]; then
   write_value "mali_ondemand" "/sys/devices/platform/mali.0/devfreq/gpufreq/governor"
   write_value 90 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/vsync_upthreshold 
   write_value 80 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/vsync_downdifferential 
   write_value 80 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/no_vsync_upthreshold 
   write_value 70 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/no_vsync_downdifferential 
  fi
  if [ -d /sys/module/ged/parameters/ ] ; then
   write_value "0" "/sys/module/ged/parameters/ged_boost_enable" 
   write_value "0" "/sys/module/ged/parameters/boost_gpu_enable" 
   write_value "0" "/sys/module/ged/parameters/boost_extra" 
   write_value "0" "/sys/module/ged/parameters/enable_cpu_boost" 
   write_value "0" "/sys/module/ged/parameters/enable_gpu_boost" 
   write_value "0" "/sys/module/ged/parameters/enable_game_self_frc_detect" 
   write_value "0" "/sys/module/ged/parameters/ged_force_mdp_enable" 
   write_value "0" "/sys/module/ged/parameters/ged_log_perf_trace_enable" 
   write_value "0" "/sys/module/ged/parameters/ged_log_trace_enable" 
   write_value "0" "/sys/module/ged/parameters/ged_monitor_3D_fence_debug" 
   write_value "0" "/sys/module/ged/parameters/ged_monitor_3D_fence_disable" 
   write_value "0" "/sys/module/ged/parameters/ged_monitor_3D_fence_systrace" 
   write_value "0" "/sys/module/ged/parameters/ged_smart_boost" 
   write_value "0" "/sys/module/ged/parameters/gpu_debug_enable" 
   write_value "1" "/sys/module/ged/parameters/gpu_dvfs_enable" 
   write_value "0" "/sys/module/ged/parameters/gx_3D_benchmark_on" 
   write_value "0" "/sys/module/ged/parameters/gx_dfps" 
   write_value "0" "/sys/module/ged/parameters/gx_force_cpu_boost" 
   write_value "0" "/sys/module/ged/parameters/gx_frc_mode" 
   write_value "0" "/sys/module/ged/parameters/gx_game_mode" 
   write_value "1" "/sys/module/ged/parameters/is_GED_KPI_enabled"
  fi
 fi
 #NVIDIA GPU CONTROL
 if [ -d /sys/kernel/tegra_gpu ] ; then
  RATE_TABLE_NUM=$(grep -o ' ' < /sys/kernel/tegra_gpu/gpu_available_rates | wc -l)
  if [ $MODE = "powersave" ] ; then
   GPU_MIN_LEVEL=1
   GPU_MAX_LEVEL=$(($RATE_TABLE_NUM/2))
  elif [ $MODE = "balance" ]||[ $MODE = "performance" ] ; then
   GPU_MIN_LEVEL=1
   GPU_MAX_LEVEL=$(($RATE_TABLE_NUM+1))
  else
   GPU_MIN_LEVEL=$(($RATE_TABLE_NUM/2))
   GPU_MAX_LEVEL=$(($RATE_TABLE_NUM+1))
  fi
  GPU_MIN_RATE=$(cat /sys/kernel/tegra_gpu/gpu_available_rates | awk '{print $"'$GPU_MIN_LEVEL'"}')
  GPU_MAX_RATE=$(cat /sys/kernel/tegra_gpu/gpu_available_rates | awk '{print $"'$GPU_MAX_LEVEL'"}')
  write_value "1" "/sys/kernel/tegra_gpu/gpu_floor_state"
  write_value "1" "/sys/kernel/tegra_gpu/gpu_cap_state"
  write_value "$GPU_MIN_RATE" "/sys/kernel/tegra_gpu/gpu_floor_rate"
  write_value "$GPU_MAX_RATE" "/sys/kernel/tegra_gpu/gpu_cap_rate"
 fi
 #MALI GPU ADJUST
 MALI_GPU_DIR_NAME=$(ls /sys/class/devfreq/ | grep "mali")
 MALI_GPU_DIR=$(echo /sys/class/devfreq/${MALI_GPU_DIR_NAME})
 if [ -d ${MALI_GPU_DIR} ]&&[ -n "$MALI_GPU_DIR_NAME" ] ; then
  FREQ_TABLE_NUM=$(grep -o ' ' < ${MALI_GPU_DIR}/available_frequencies | wc -l)
  GPU_FIRST_FREQ_INFO=$(cat ${MALI_GPU_DIR}/available_frequencies | awk '{print $1}')
  GPU_SECOND_FREQ_INFO=$(cat ${MALI_GPU_DIR}/available_frequencies | awk '{print $2}')
  GPU_MAX_NUM=$(($FREQ_TABLE_NUM+1))
  GPU_LAST_FREQ=$(cat ${MALI_GPU_DIR}/available_frequencies | awk '{print $"'$GPU_MAX_NUM'"}')
  if [ ! -n "$GPU_LAST_FREQ" ] ; then
   GPU_MAX_NUM=$(echo $FREQ_TABLE_NUM)
  fi
  if [ ${GPU_FIRST_FREQ_INFO} -gt ${GPU_SECOND_FREQ_INFO} ] ; then
   if [ $MODE = "powersave" ] ; then
    GPU_MIN_LEVEL=$(echo $GPU_MAX_NUM)
    GPU_MAX_LEVEL=$(($GPU_MAX_NUM/2-1))
   elif [ $MODE = "balance" ]||[ $MODE = "performance" ] ; then
    GPU_MIN_LEVEL=$(echo $GPU_MAX_NUM)
    GPU_MAX_LEVEL=1
   else
    GPU_MIN_LEVEL=$(($GPU_MAX_NUM/2+1))
    GPU_MAX_LEVEL=1
   fi
  else
   if [ $MODE = "powersave" ] ; then
    GPU_MIN_LEVEL=1
    GPU_MAX_LEVEL=$(($GPU_MAX_NUM/2+1))
   elif [ $MODE = "balance" ]||[ $MODE = "performance" ] ; then
    GPU_MIN_LEVEL=1
    GPU_MAX_LEVEL=$(echo $GPU_MAX_NUM)
   else
    GPU_MIN_LEVEL=$(($GPU_MAX_NUM/2-1))
    GPU_MAX_LEVEL=$(echo $GPU_MAX_NUM)
   fi
  fi
  GPU_MIN_FREQ=$(cat ${MALI_GPU_DIR}/available_frequencies | awk '{print $"'$GPU_MIN_LEVEL'"}')
  GPU_MAX_FREQ=$(cat ${MALI_GPU_DIR}/available_frequencies | awk '{print $"'$GPU_MAX_LEVEL'"}')
  write_value "$GPU_MIN_FREQ" "${MALI_GPU_DIR}/min_freq"
  write_value "$GPU_MAX_FREQ" "${MALI_GPU_DIR}/max_freq"
 fi
fi
exit 0
