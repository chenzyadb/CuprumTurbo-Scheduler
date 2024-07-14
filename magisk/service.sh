#!/system/bin/sh
BASE_DIR=$(dirname "$0")

get_min_freq() {
  freq_table=$(tr " " "\n" < "$1")
	first_freq=$(echo "$freq_table" | head -1)
	last_freq=$(echo "$freq_table" | tail -1)
	if [ "$first_freq" -lt "$last_freq" ]; then
		echo "$first_freq"
	else
		echo "$last_freq"
	fi
}

get_max_freq() {
	freq_table=$(tr " " "\n" < "$1")
  first_freq=$(echo "$freq_table" | head -1)
  last_freq=$(echo "$freq_table" | tail -1)
  if [ "$first_freq" -gt "$last_freq" ]; then
  	echo "$first_freq"
  else
  	echo "$last_freq"
  fi
}

write_value() {
	for file in $2; do
		if [ -f "$file" ]; then
			echo "$1" >"$file"
		fi
	done
}

lock_value() {
	for file in $2; do
		if [ -f "$file" ]; then
			chown root:root "$file"
			chmod 0666 "$file"
			echo "$1" > "$file"
			chmod 0444 "$file"
		fi
	done
}

change_task_cpuset() {
	for pid in $(pgrep -f "$1"); do
		echo "$pid" > "/dev/cpuset/${2}/cgroup.procs"
	done
}

change_task_sched() {
	for pid in $(pgrep -f "$1"); do
		if [ -d /dev/stune ]; then
			echo "$pid" > "/dev/stune/${2}/cgroup.procs"
		elif [ -d /dev/cpuctl ]; then
			echo "$pid" > "/dev/cpuctl/${2}/cgroup.procs"
		fi
	done
}

stop horae 2>/dev/null
stop miuibooster 2>/dev/null
stop oneplus_brain_service 2>/dev/null
stop vendor.perfservice 2>/dev/null
stop perfd 2>/dev/null

setprop "persist.sys.hardcoder.name" "" 2>/dev/null
setprop "persist.miui.miperf.enable" "false" 2>/dev/null

lock_value "1" "/proc/ppm/enabled"
lock_value "0" "/proc/hps/enabled"
lock_value "2" "/sys/devices/system/cpu/eas/enable"
lock_value "0" "/sys/module/fbt_cpu/parameters/boost_affinity*"
lock_value "0" "/sys/module/mtk_fpsgo/parameters/boost_affinity*"
lock_value "0" "/sys/module/mtk_fpsgo/parameters/perfmgr_enable"
lock_value "0" "/sys/module/perfmgr/parameters/perfmgr_enable"
lock_value "0" "/sys/module/perfmgr_policy/parameters/perfmgr_enable"
lock_value "0" "/sys/kernel/fpsgo/common/fpsgo_enable"
lock_value "0" "/sys/kernel/fpsgo/common/force_onoff"
lock_value "0" "/sys/kernel/fpsgo/fbt/enable*"
lock_value "0" "/sys/kernel/fpsgo/fbt/limit*"
lock_value "0" "/sys/kernel/fpsgo/fbt/switch_idleprefer"
lock_value "0" "/sys/kernel/debug/fpsgo/common/fpsgo_enable"
lock_value "0" "/sys/kernel/debug/fpsgo/common/force_onoff"
lock_value "0" "/sys/kernel/ged/hal/dcs_mode"
lock_value "enable: 0" "/proc/perfmgr/tchbst/user/usrtch"

lock_value "0" "/sys/power/cpuhotplug/enabled"
lock_value "0" "/sys/power/pnpmgr/touch_boost"
lock_value "0" "/sys/power/pnpmgr/long_duration_touch_boost"
lock_value "0" "/sys/kernel/ems/eff_mode"
lock_value "0" "/sys/kernel/hmp/boost"
lock_value "0" "/sys/kernel/hmp/boostpulse_duration"
lock_value "0" "/sys/kernel/cpu_input_boost/*"
lock_value "0" "/sys/kernel/intelli_plug/intelli_plug_active"
lock_value "0" "/sys/kernel/zen_decision/enabled"
lock_value "0" "/sys/devices/system/cpu/cpu*/sched_load_boost"
lock_value "0" "/sys/devices/system/cpu/sched/sched_boost"
lock_value "0" "/sys/devices/system/cpu/cpu_boost/*"
lock_value "0" "/sys/devices/system/cpu/cpu_boost/parameters/*"
lock_value "1" "/sys/devices/system/cpu/cpufreq/hotplug/cpu_hotplug_disable"
lock_value "0" "/sys/devices/system/cpu/cpuhotplug/enabled"
lock_value "0" "/sys/devices/system/cpu/hyp_core_ctl/enable"
lock_value "0" "/sys/devices/virtual/misc/mako_hotplug_control/enabled"
lock_value "0" "/sys/module/msm_performance/parameters/touchboost"
lock_value "0" "/sys/module/msm_thermal/vdd_restriction/enabled"
lock_value "0" "/sys/module/msm_thermal/core_control/enabled"
lock_value "N" "/sys/module/msm_thermal/parameters/enabled"
lock_value "0" "/sys/module/cpu_boost/parameters/*"
lock_value "0" "/sys/module/aigov/parameters/enable"
lock_value "0" "/sys/module/opchain/parameters/chain_on"
lock_value "0" "/sys/module/houston/parameters/*"
lock_value "N" "/sys/module/control_center/parameters/*"
lock_value "0" "/sys/module/dsboost/parameters/*"
lock_value "0" "/sys/module/cpu_input_boost/parameters/*"
lock_value "0" "/sys/module/input_cfboost/parameters/*"
lock_value "0" "/sys/module/blu_plug/parameters/enabled"
lock_value "0" "/sys/module/autosmp/parameters/enabled"
lock_value "0" "/sys/class/input_booster/*"
lock_value "0" "/proc/mz_thermal_boost/sched_boost_enabled"
lock_value "0" "/proc/mz_thermal_boost/boost_enabled"
lock_value "0" "/proc/mz_scheduler/vip_task/enabled"
lock_value "0" "/proc/oplus_scheduler/sched_assist/sched_assist_enabled"
lock_value "1" "/proc/game_opt/disable_cpufreq_limit"
lock_value "-1" "/proc/game_opt/game_pid"
lock_value "0" "/proc/sys/fbg/frame_boost_enabled"
lock_value "0" "/proc/sys/fbg/input_boost_enabled"
lock_value "0" "/proc/sys/fbg/slide_boost_enabled"
lock_value "0" "/proc/sys/kernel/sched_util_clamp_min"
lock_value "1024" "/proc/sys/kernel/sched_util_clamp_max"
lock_value "0" "/proc/sys/kernel/*boost*"
lock_value "" "/proc/sys/kernel/sched_lib_name"
lock_value "0" "/proc/sys/walt/*boost*"
lock_value "0" "/proc/sys/walt/input_boost/*"
lock_value "" "/proc/sys/walt/sched_lib_name"

if [ -d "/dev/stune/" ]; then
    lock_value "0" "/dev/stune/schedtune.boost"
    lock_value "0" "/dev/stune/schedtune.prefer_idle"
    lock_value "0" "/dev/stune/*/schedtune.prefer_idle"
    lock_value "0" "/dev/stune/*/schedtune.boost"
    lock_value "0" "/dev/stune/*/schedtune.sched_boost_no_override"
fi

if [ -d "/dev/cpuctl/" ]; then
    lock_value "0" "/dev/cpuctl/cpu.idle"
    lock_value "1024" "/dev/cpuctl/cpu.shares"
    lock_value "0" "/dev/cpuctl/*/cpu.uclamp.latency_sensitive"
    lock_value "0" "/dev/cpuctl/*/cpu.uclamp.sched_boost_no_override"
    lock_value "0" "/dev/cpuctl/*/cpu.uclamp.min"
    lock_value "max" "/dev/cpuctl/*/cpu.uclamp.max"
    lock_value "0" "/dev/cpuctl/*/cpu.idle"
    lock_value "1024" "/dev/cpuctl/*/cpu.shares"
fi

if [ -d "/proc/perfmgr/boost_ctrl/eas_ctrl/" ]; then
    lock_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_*_boost"
    lock_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_*_uclamp_min"
fi

lock_value "$(cat /dev/cpuset/cpus)" "/dev/cpuset/*/cpus"

lock_value "1" "/sys/devices/system/cpu/cpu*/online"
lock_value "1" "/sys/devices/system/cpu/cpu*/core_ctl/enable"
POLICY2_PATH="/sys/devices/system/cpu/cpu2/core_ctl"
POLICY3_PATH="/sys/devices/system/cpu/cpu3/core_ctl"
POLICY4_PATH="/sys/devices/system/cpu/cpu4/core_ctl"
POLICY5_PATH="/sys/devices/system/cpu/cpu5/core_ctl"
POLICY6_PATH="/sys/devices/system/cpu/cpu6/core_ctl"
POLICY7_PATH="/sys/devices/system/cpu/cpu7/core_ctl"
if [ -d "$POLICY2_PATH" ] && [ -d "$POLICY5_PATH" ] && [ -d "$POLICY7_PATH" ]; then #2+3+2+1
    lock_value "2" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
    lock_value "2" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
    lock_value "3" "/sys/devices/system/cpu/cpu2/core_ctl/max_cpus"
    lock_value "3" "/sys/devices/system/cpu/cpu2/core_ctl/min_cpus"
    lock_value "2" "/sys/devices/system/cpu/cpu5/core_ctl/max_cpus"
    lock_value "2" "/sys/devices/system/cpu/cpu5/core_ctl/min_cpus"
    lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/max_cpus"
    lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/min_cpus"
elif [ -d "$POLICY2_PATH" ]; then #2+2
    lock_value "2" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
    lock_value "2" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
    lock_value "2" "/sys/devices/system/cpu/cpu2/core_ctl/max_cpus"
    lock_value "2" "/sys/devices/system/cpu/cpu2/core_ctl/min_cpus"
elif [ -d "$POLICY3_PATH" ] && [ -d "$POLICY7_PATH" ]; then #3+4+1
    lock_value "3" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
    lock_value "3" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
    lock_value "4" "/sys/devices/system/cpu/cpu3/core_ctl/max_cpus"
    lock_value "4" "/sys/devices/system/cpu/cpu3/core_ctl/min_cpus"
    lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/max_cpus"
    lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/min_cpus"
elif [ -d "$POLICY4_PATH" ] && [ -d "$POLICY7_PATH" ]; then #4+3+1
    lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
    lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
    lock_value "3" "/sys/devices/system/cpu/cpu4/core_ctl/max_cpus"
    lock_value "3" "/sys/devices/system/cpu/cpu4/core_ctl/min_cpus"
    lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/max_cpus"
    lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/min_cpus"
elif [ -d "$POLICY4_PATH" ]; then #4+4
    lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
    lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
    lock_value "4" "/sys/devices/system/cpu/cpu4/core_ctl/max_cpus"
    lock_value "4" "/sys/devices/system/cpu/cpu4/core_ctl/min_cpus"
elif [ -d "$POLICY6_PATH" ] && [ -d "$POLICY7_PATH" ]; then #6+1+1
    lock_value "6" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
    lock_value "6" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
    lock_value "1" "/sys/devices/system/cpu/cpu6/core_ctl/max_cpus"
    lock_value "1" "/sys/devices/system/cpu/cpu6/core_ctl/min_cpus"
    lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/max_cpus"
    lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/min_cpus"
elif [ -d "$POLICY6_PATH" ]; then #6+2
    lock_value "6" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
    lock_value "6" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
    lock_value "2" "/sys/devices/system/cpu/cpu6/core_ctl/max_cpus"
    lock_value "2" "/sys/devices/system/cpu/cpu6/core_ctl/min_cpus"
fi
lock_value "0" "/sys/devices/system/cpu/cpu*/core_ctl/enable"
lock_value "0" "/sys/devices/system/cpu/cpu*/core_ctl/core_ctl_boost"

if [ -d "/sys/class/devfreq/gpufreq/" ]; then
    lock_value "mali_ondemand" "/sys/class/devfreq/gpufreq/governor"
    lock_value "0" "/sys/class/devfreq/gpufreq/animation_boost"
    lock_value "0" "/sys/class/devfreq/gpufreq/cl_boost"
    lock_value "1" "/sys/class/devfreq/gpufreq/vsync"
    lock_value "90" "/sys/class/devfreq/gpufreq/upthreshold"
    lock_value "10" "/sys/class/devfreq/gpufreq/downdifferential"
    gpu_max_freq=$(get_max_freq "/sys/class/devfreq/gpufreq/available_frequencies")
    gpu_min_freq=$(get_min_freq "/sys/class/devfreq/gpufreq/available_frequencies")
    lock_value "$gpu_max_freq" "/sys/class/devfreq/gpufreq/max_freq"
    lock_value "$gpu_min_freq" "/sys/class/devfreq/gpufreq/min_freq"
fi

for GPU_DIR in /sys/class/devfreq/*gpu*; do
    if [ -d "$GPU_DIR" ]; then
        gpu_max_freq=$(get_max_freq "${GPU_DIR}/available_frequencies")
        gpu_min_freq=$(get_min_freq "${GPU_DIR}/available_frequencies")
        lock_value "$gpu_max_freq" "${GPU_DIR}/max_freq"
        lock_value "$gpu_min_freq" "${GPU_DIR}/min_freq"
    fi
done

if [ -d "/sys/class/kgsl/kgsl-3d0/" ]; then
    lock_value "0" "/sys/class/kgsl/kgsl-3d0/max_pwrlevel"
    MIN_PWRLEVEL=$(($(cat /sys/class/kgsl/kgsl-3d0/num_pwrlevels) - 1))
    lock_value "$MIN_PWRLEVEL" "/sys/class/kgsl/kgsl-3d0/min_pwrlevel"
    lock_value "$MIN_PWRLEVEL" "/sys/class/kgsl/kgsl-3d0/default_pwrlevel"
fi

change_task_cpuset "surfaceflinger" "top-app"
change_task_sched "surfaceflinger" ""
change_task_cpuset "system_server" "top-app"
change_task_sched "system_server" ""
change_task_cpuset "android.hardware.graphics.composer" "top-app"
change_task_sched "android.hardware.graphics.composer" ""
change_task_cpuset "vendor.qti.hardware.display.composer-service" "top-app"
change_task_sched "vendor.qti.hardware.display.composer-service" ""

change_task_cpuset "adbd" "system-background"
change_task_sched "adbd" ""
change_task_cpuset "logd" "system-background"
change_task_sched "logd" ""
change_task_cpuset "lmkd" "system-background"
change_task_sched "lmkd" ""
change_task_cpuset "mdnsd" "system-background"
change_task_sched "mdnsd" ""
change_task_cpuset "tombstoned" "system-background"
change_task_sched "tombstoned" ""
change_task_cpuset "traced" "system-background"
change_task_sched "traced" ""
change_task_cpuset "swapd" "system-background"
change_task_sched "swapd" ""
change_task_cpuset "compactd" "system-background"
change_task_sched "compactd" ""

### Run CuprumTurbo-Scheduler Daemon.

# Check if /sdcard is ready.
while [ ! -f /sdcard/.test_file ]; do
	true >/sdcard/.test_file
	sleep 1
done
rm -f /sdcard/.test_file

# Create CT Dir.
if [ ! -d /sdcard/Android/ct/ ]; then
	mkdir -p /sdcard/Android/ct/
	echo "balance" >/sdcard/Android/ct/cur_mode.txt
fi

#CuDaemon -R [config] [mode] [log]
"${BASE_DIR}/CuDaemon" -R "${BASE_DIR}/config.json" "/sdcard/Android/ct/cur_mode.txt" "/sdcard/Android/ct/scheduler.log"

exit 0
