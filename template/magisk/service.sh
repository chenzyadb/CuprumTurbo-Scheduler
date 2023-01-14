#!/system/bin/sh
function get_min_freq() {
	first_freq=$(cat "$1" | tr " " "\n" | head -1)
	last_freq=$(cat "$1" | tr " " "\n" | tail -1)
	if [ $first_freq -lt $last_freq ] ; then
		echo $first_freq
	else
		echo $last_freq
	fi
}

function get_max_freq() {
	first_freq=$(cat "$1" | tr " " "\n" | head -1)
	last_freq=$(cat "$1" | tr " " "\n" | tail -1)
	if [ $first_freq -gt $last_freq ] ; then
		echo $first_freq
	else
		echo $last_freq
	fi
}

function write_value() {
	if [ -e "$2" ] ; then
		echo "$1" > "$2" 2>/dev/null;
	fi
}

function lock_value() {
	if [ -e "$2" ] ; then
		chown root:root "$2" 2>/dev/null;
		chmod 0666 "$2" 2>/dev/null;
		echo "$1" > "$2" 2>/dev/null;
		chmod 0444 "$2" 2>/dev/null;
	fi
}

function unlock_value() {
	if [ -e "$2" ] ; then
	    chown root:root "$2" 2>/dev/null;
		chmod 0666 "$2" 2>/dev/null;
		echo "$1" > "$2" 2>/dev/null;
	fi
}

function multi_lock_value() {
	for file in $2 ; do
		if [ -e "$file" ] ; then
			chown root:root "$file" 2>/dev/null;
			chmod 0666 "$file" 2>/dev/null;
			echo "$1" > "$file" 2>/dev/null;
			chmod 0444 "$file" 2>/dev/null;
		fi
	done
}

function unlock() {
	if [ -e "$2" ] ; then
		chown root:root "$2" 2>/dev/null;
		chmod 0666 "$2" 2>/dev/null;
	fi
}

function block() {
    if [ -e "$2" ] ; then
		chmod 0000 "$2" 2>/dev/null;
	fi
}

function multi_block() {
    for file in $1 ; do
        if [ -e "$file" ] ; then
		    chmod 0000 "$file" 2>/dev/null;
	    fi
    done
}

function change_task_cpuset() {
	for task_pid in $(ps -Ao pid,args | grep "$1" | awk '{print $1}') ; do
		echo "$task_pid" > "/dev/cpuset/$2/tasks" 2>/dev/null;
	done
}


if [ -d /sys/devices/system/cpu/cpu9/ ] ; then
	core_num="9"
elif [ -d /sys/devices/system/cpu/cpu7/ ] ; then
	core_num="7"
elif [ -d /sys/devices/system/cpu/cpu5/ ] ; then
	core_num="5"
elif [ -d /sys/devices/system/cpu/cpu3/ ] ; then
	core_num="3"
fi

#Disable Userspace Perf Controller
stop miuibooster 2>/dev/null
stop oneplus_brain_service 2>/dev/null
stop vendor.perfservice 2>/dev/null
stop perfd 2>/dev/null

#Disable Tencent HardCoder
setprop persist.sys.hardcoder.name "" 2>/dev/null

#Disable MTK PPM Perf Controller
lock_value "1" /proc/ppm/enabled
if [ -e /proc/ppm/policy_status ] ; then 
	for ppm_policy in $(seq 0 10) ; do
		write_value "$ppm_policy 1" "/proc/ppm/policy_status"
	done
	disable_policy_list="
	   PPM_POLICY_PTPOD
	   PPM_POLICY_UT
	   PPM_POLICY_FORCE_LIMIT
	   PPM_POLICY_PWR_THRO
	   PPM_POLICY_THERMAL
	   PPM_POLICY_DLPT
	   PPM_POLICY_USER_LIMIT
	   PPM_POLICY_LCM_OFF
	   PPM_POLICY_SYS_BOOST
	   PPM_POLICY_HICA"
	for policy in $disable_policy_list ; do
		IDX_INFO=$(cat /proc/ppm/policy_status | grep "$policy") 
		IDX_NUM=$(echo ${IDX_INFO:1:1})
		write_value "${IDX_NUM} 0" "/proc/ppm/policy_status"
	done
fi

#Disable MTK Kernel Perf Controller
lock_value "0" "/sys/devices/system/cpu/sched/sched_boost"
lock_value "1" "/sys/devices/system/cpu/eas/enable"
lock_value "0" "/sys/kernel/debug/fpsgo/fbt/switch_idleprefer"
lock_value "0" "/sys/kernel/debug/fpsgo/common/force_onoff"
lock_value "1" "/sys/kernel/debug/fpsgo/common/stop_boost"
lock_value "0" "/sys/kernel/fpsgo/fbt/boost_ta"
lock_value "0" "/sys/kernel/fpsgo/fbt/switch_idleprefer"
lock_value "0" "/sys/kernel/fpsgo/common/force_onoff"
lock_value "1" "/sys/kernel/fpsgo/common/stop_boost"
lock_value "0" "/sys/kernel/fpsgo/minitop/enable"
lock_value "0" "/sys/module/mt_hotplug_mechanism/parameters/g_enable"
lock_value "1" "/sys/devices/system/cpu/cpufreq/hotplug/cpu_hotplug_disable"
lock_value "1" "/proc/perfmgr/syslimiter/syslimiter_force_disable"
lock_value "100" "/proc/perfmgr/syslimiter/syslimitertolerance_percent"
lock_value "1" "/sys/module/mtk_core_ctl/parameters/policy_enable"
lock_value "95" "/sys/kernel/fpsgo/fbt/thrm_temp_th"
lock_value "-1" "/sys/kernel/fpsgo/fbt/thrm_limit_cpu"
lock_value "-1" "/sys/kernel/fpsgo/fbt/thrm_sub_cpu"
lock_value "0" "/sys/kernel/eara_thermal/enable"
lock_value "0" "/sys/kernel/eara_thermal/fake_throttle"
lock_value "enable: 0" "/proc/perfmgr/tchbst/user/usrtch"
lock_value "0" "/proc/hps/enabled"
lock_value "0" "/proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_enable"
lock_value "0" "/proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_up_loading"
lock_value "0" "/proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_down_loading"
lock_value "0" "/sys/module/fbt_cpu/parameters/boost_affinity"
lock_value "0" "/sys/module/mtk_fpsgo/parameters/boost_affinity"
lock_value "0" "/sys/module/mtk_fpsgo/parameters/cfp_onoff"
lock_value "0" "/sys/module/mtk_fpsgo/parameters/cfp_up_loading"
lock_value "0" "/sys/module/mtk_fpsgo/parameters/cfp_down_loading"
multi_lock_value "9999999" "/sys/kernel/fpsgo/fbt/limit_*"
block "/proc/perfmgr/eara_ioctl"
block "/proc/perfmgr/perf_ioctl"
block "/proc/perfmgr/eas_ioctl"
block "/proc/perfmgr/xgff_ioctl"

#Disable Exynos Kernel Perf Controller
lock_value "0" "/sys/kernel/hmp/boost"
lock_value "0" "/sys/kernel/hmp/boostpulse_duration"
lock_value "0" "/sys/power/cpuhotplug/enabled"
lock_value "0" "/sys/kernel/ems/eff_mode"
lock_value "0" "/sys/devices/system/cpu/cpuhotplug/enabled"

#Disable Qualcomm Kernel Perf Controller
lock_value "0" "/sys/module/msm_thermal/vdd_restriction/enabled"
lock_value "0" "/sys/module/msm_thermal/core_control/enabled"
lock_value "N" "/sys/module/msm_thermal/parameters/enabled"
lock_value "0" "/sys/module/msm_performance/parameters/touchboost"
multi_lock_value "0" "/sys/devices/system/cpu/cpu*/sched_load_boost"
multi_lock_value "0" "/proc/sys/walt/input_boost/*"
multi_lock_value "0" "/sys/devices/system/cpu/cpu_boost/*"
multi_lock_value "0" "/sys/devices/system/cpu/cpu_boost/parameters/*"
multi_lock_value "0" "/sys/module/cpu_boost/parameters/*"

#Disable Custom Kernel Perf Controller
lock_value "0" "/sys/kernel/intelli_plug/intelli_plug_active"
lock_value "0" "/sys/module/blu_plug/parameters/enabled"
lock_value "0" "/sys/devices/virtual/misc/mako_hotplug_control/enabled"
lock_value "0" "/sys/module/autosmp/parameters/enabled"
lock_value "0" "/sys/kernel/zen_decision/enabled"
lock_value "0" "/sys/module/aigov/parameters/enable"
lock_value "0" "/sys/module/opchain/parameters/chain_on"
lock_value "0" "/sys/power/pnpmgr/touch_boost"
lock_value "0" "/sys/power/pnpmgr/long_duration_touch_boost"
lock_value "0" "/sys/devices/system/cpu/hang_detect_core/enable"
lock_value "0" "/sys/devices/system/cpu/hyp_core_ctl/enable"
lock_value "0" "/sys/module/core_hang_detect/parameters/enable"
lock_value "0 0 0 0" "/proc/oplus_frame_boost/stune_boost"
lock_value "0 0 0 0" "/proc/oppo_frame_boost/stune_boost"
lock_value "0" "/proc/oplus_scheduler/sched_assist/sched_assist_enabled"
lock_value "0" "/proc/oppo_scheduler/sched_assist/sched_assist_enabled"
lock_value "0" "/proc/sys/fbg/frame_boost_enabled"
lock_value "0" "/proc/sys/fbg/input_boost_enabled"
lock_value "0" "/proc/sys/fbg/slide_boost_enabled"
multi_lock_value "0" "/sys/module/houston/parameters/*"
multi_lock_value "N" "/sys/module/control_center/parameters/*"
multi_lock_value "0" "/sys/kernel/cpu_input_boost/*"
multi_lock_value "0" "/sys/module/dsboost/parameters/*"
multi_lock_value "0" "/sys/module/cpu_input_boost/parameters/*"
multi_lock_value "0" "/sys/module/devfreq_boost/parameters/*"
multi_lock_value "0" "/sys/module/input_cfboost/parameters/*"
multi_lock_value "0" "/sys/class/input_booster/*"
multi_lock_value "0" "/sys/devices/system/cpu/cpu*/sched_load_boost"
block "/dev/migt"
multi_block "/sys/module/migt/parameters/*"
multi_block "/proc/sys/migt/*"
multi_block "/sys/module/huawei_hung_task/*"
multi_block "/proc/oplus_scheduler/sched_assist/*"
multi_block "/proc/oppo_scheduler/sched_assist/*"


#Unify Kernel Scheduler
for sched_type in walt kernel ; do
	lock_value "0" "/proc/sys/${sched_type}/sched_freq_aggregate"
	lock_value "0" "/proc/sys/${sched_type}/sched_boost"
	lock_value "1" "/proc/sys/${sched_type}/sched_walt_rotate_big_tasks"
	lock_value "20000000" "/proc/sys/${sched_type}/sched_coloc_downmigrate_ns"
	lock_value "5" "/proc/sys/${sched_type}/sched_spill_nr_run"
	lock_value "95" "/proc/sys/${sched_type}/sched_spill_load"
	lock_value "1000000" "/proc/sys/${sched_type}/sched_min_granularity_ns"
	lock_value "0" "/proc/sys/${sched_type}/sched_tunable_scaling"
	lock_value "40" "/proc/sys/${sched_type}/sched_nr_migrate"
	lock_value "0" "/proc/sys/${sched_type}/sched_util_clamp_min"
	lock_value "0" "/proc/sys/${sched_type}/sched_force_lb_enable"
	lock_value "255" "/proc/sys/${sched_type}/sched_busy_hysteresis_enable_cpus"
	lock_value "2000000" "/proc/sys/${sched_type}/sched_busy_hyst_ns"
	#sched_lib_name will cause TasksetHelper to lose effect.
	lock_value "" "/proc/sys/${sched_type}/sched_lib_name"
	if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
		lock_value "40 30" "/proc/sys/${sched_type}/sched_downmigrate"
		lock_value "80 90" "/proc/sys/${sched_type}/sched_upmigrate"
	else
		lock_value "40" "/proc/sys/${sched_type}/sched_downmigrate"
		lock_value "80" "/proc/sys/${sched_type}/sched_upmigrate"
	fi
	lock_value "0" "/proc/sys/${sched_type}/sched_group_downmigrate"
	lock_value "0" "/proc/sys/${sched_type}/sched_group_upmigrate"
done

#Disable stune_boost & uclamp_min 
if [ -d /dev/stune/ ] ; then
	lock_value "0" "/dev/stune/schedtune.boost"
	lock_value "0" "/dev/stune/schedtune.prefer_idle"
	for stune_dir in /dev/stune/* ; do
		lock_value "0" "${stune_dir}/schedtune.prefer_idle"
		lock_value "0" "${stune_dir}/schedtune.boost"
		lock_value "0" "${stune_dir}/schedtune.sched_boost_no_override"
	done
fi
if [ -d /dev/cpuctl/ ] ; then
	lock_value "0" "/dev/cpuctl/cpu.uclamp.sched_boost_no_override"
	lock_value "0" "/dev/cpuctl/cpu.uclamp.min"
	lock_value "0" "/dev/cpuctl/cpu.uclamp.latency_sensitive"
	for cpuctl_dir in /dev/cpuctl/* ; do
		lock_value "0" "${cpuctl_dir}/cpu.uclamp.latency_sensitive"
		lock_value "0" "${cpuctl_dir}/cpu.uclamp.min"
		lock_value "0" "${cpuctl_dir}/cpu.uclamp.sched_boost_no_override"
	done
fi
if [ -d /proc/perfmgr/boost_ctrl/eas_ctrl/ ] ; then
	for perfserv_boost in /proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_*_boost ; do
		lock_value "0" "$perfserv_boost"
	done
	for perfserv_uclamp_min in /proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_*_uclamp_min ; do
		lock_value "0" "$perfserv_uclamp_min"
	done
fi

#Reset CPUSET
if [ $core_num = "9" ] ; then
	lock_value "0-3" "/dev/cpuset/background/cpus"
	lock_value "0-3" "/dev/cpuset/system-background/cpus"
	lock_value "0-9" "/dev/cpuset/foreground/cpus"
	lock_value "0-9" "/dev/cpuset/foreground/boost/cpus" 
	lock_value "0-9" "/dev/cpuset/top-app/cpus" 
	lock_value "0-9" "/dev/cpuset/top-app/boost/cpus" 
	lock_value "0-9" "/dev/cpuset/game/cpus"
	lock_value "0-7" "/dev/cpuset/restricted/cpus"
	lock_value "0-9" "/dev/cpuset/vr/cpus"
elif [ $core_num = "7" ] ; then
	lock_value "0-1" "/dev/cpuset/background/cpus"
	lock_value "0-3" "/dev/cpuset/system-background/cpus"
	lock_value "0-7" "/dev/cpuset/foreground/cpus"
	lock_value "0-7" "/dev/cpuset/foreground/boost/cpus" 
	lock_value "0-7" "/dev/cpuset/top-app/cpus" 
	lock_value "0-7" "/dev/cpuset/top-app/boost/cpus" 
	lock_value "0-7" "/dev/cpuset/game/cpus"
	lock_value "0-3" "/dev/cpuset/restricted/cpus"
	lock_value "0-7" "/dev/cpuset/vr/cpus"
elif [ $core_num = "5" ] ; then
	lock_value "0-1" "/dev/cpuset/background/cpus"
	lock_value "0-3" "/dev/cpuset/system-background/cpus" 
	lock_value "0-5" "/dev/cpuset/foreground/cpus" 
	lock_value "0-5" "/dev/cpuset/foreground/boost/cpus" 
	lock_value "0-5" "/dev/cpuset/top-app/cpus" 
	lock_value "0-5" "/dev/cpuset/top-app/boost/cpus" 
	lock_value "0-5" "/dev/cpuset/game/cpus"
	lock_value "0-3" "/dev/cpuset/restricted/cpus"
	lock_value "0-5" "/dev/cpuset/vr/cpus"
elif [ $core_num = "3" ] ; then
	lock_value "0-1" "/dev/cpuset/background/cpus"
	lock_value "0-1" "/dev/cpuset/system-background/cpus" 
	lock_value "0-3" "/dev/cpuset/foreground/cpus" 
	lock_value "0-3" "/dev/cpuset/foreground/boost/cpus" 
	lock_value "0-3" "/dev/cpuset/top-app/cpus" 
	lock_value "0-3" "/dev/cpuset/top-app/boost/cpus" 
	lock_value "0-3" "/dev/cpuset/game/cpus"
	lock_value "0-1" "/dev/cpuset/restricted/cpus"
	lock_value "0-3" "/dev/cpuset/vr/cpus"
fi

#Bring ALL Cores Online
multi_lock_value "1" "/sys/devices/system/cpu/cpu*/online" 

#Disable Qualcomm&MediaTek Core Control
multi_lock_value "1" "/sys/devices/system/cpu/cpu*/core_ctl/enable"   
if [ -d /sys/devices/system/cpu/cpufreq/policy2 ] ; then
	#Type 2+2
	lock_value "2" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
	lock_value "2" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
	lock_value "2" "/sys/devices/system/cpu/cpu2/core_ctl/max_cpus"
	lock_value "2" "/sys/devices/system/cpu/cpu2/core_ctl/min_cpus"
elif [ -d /sys/devices/system/cpu/cpufreq/policy4 ] ; then
	if [ -d /sys/devices/system/cpu/cpufreq/policy7 ] ; then
		#Type 4+3+1 
		lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
		lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
		lock_value "3" "/sys/devices/system/cpu/cpu4/core_ctl/max_cpus"
		lock_value "3" "/sys/devices/system/cpu/cpu4/core_ctl/min_cpus"
		lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/max_cpus"
		lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/min_cpus"
	else
		if [ -d /sys/devices/system/cpu/cpu7 ] ; then
			#Type 4+4 
			lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
			lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
			lock_value "4" "/sys/devices/system/cpu/cpu4/core_ctl/max_cpus"
			lock_value "4" "/sys/devices/system/cpu/cpu4/core_ctl/min_cpus"
		else
			#Type  4+2 
			lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
			lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
			lock_value "2" "/sys/devices/system/cpu/cpu4/core_ctl/max_cpus"
			lock_value "2" "/sys/devices/system/cpu/cpu4/core_ctl/min_cpus"
		fi
	fi
elif [ -d /sys/devices/system/cpu/cpufreq/policy6 ] ; then
	if [ -d /sys/devices/system/cpu/cpufreq/policy7 ] ; then		
	    #Type 6+1+1
		lock_value "6" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
		lock_value "6" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
		lock_value "1" "/sys/devices/system/cpu/cpu6/core_ctl/max_cpus"
		lock_value "1" "/sys/devices/system/cpu/cpu6/core_ctl/min_cpus"
		lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/max_cpus"
		lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/min_cpus"
	else
		#Type 6+2
		lock_value "6" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
		lock_value "6" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
		lock_value "2" "/sys/devices/system/cpu/cpu6/core_ctl/max_cpus"
		lock_value "2" "/sys/devices/system/cpu/cpu6/core_ctl/min_cpus"
	fi
fi
multi_lock_value "0" "/sys/devices/system/cpu/cpu*/core_ctl/enable"   
multi_lock_value "0" "/sys/devices/system/cpu/cpu*/core_ctl/core_ctl_boost"  

#Change important system task cpuset
change_task_cpuset "surfaceflinger" "top-app"
change_task_cpuset "system_server" "top-app"
change_task_cpuset "android.hardware.graphics.composer" "top-app"
change_task_cpuset "vendor.qti.hardware.display.composer-service" "top-app"
change_task_cpuset "netd" "foreground"

#Reduce big-core awake (system-background:cpu0-3)
change_task_cpuset "logd" "system-background"
change_task_cpuset "lmkd" "system-background"
change_task_cpuset "mdnsd" "system-background"

#Reset Kernel CPU Governor
if [ -n "$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors | grep 'walt')" ] ; then
	CPU_GOV="walt"
elif [ -n "$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors | grep 'schedutil')" ] ; then
	CPU_GOV="schedutil"
elif [ -n "$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors | grep 'sched')" ] ; then
	CPU_GOV="sched"
elif [ -n "$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors | grep 'interactive')" ] ; then
	CPU_GOV="interactive"
else
	CPU_GOV="ondemand"
fi
for i in $(seq 0 ${core_num}) ; do
	lock_value "$CPU_GOV" "/sys/devices/system/cpu/cpu${i}/cpufreq/scaling_governor"
	lock_value "$CPU_GOV" "/sys/devices/system/cpu/cpufreq/policy${i}/scaling_governor"
done

#Fix OpenKirin/GSI GPU Problems.
write_value "mali_ondemand" "/sys/class/devfreq/gpufreq/governor"
write_value "0" "/sys/class/devfreq/gpufreq/animation_boost"
write_value "0" "/sys/class/devfreq/gpufreq/cl_boost"
write_value "1" "/sys/class/devfreq/gpufreq/vsync"
write_value "80" "/sys/class/devfreq/gpufreq/upthreshold"
write_value "20" "/sys/class/devfreq/gpufreq/downdifferential"
for GPU_DIR in /sys/class/devfreq/*gpu* ; do 
	gpu_max_freq=$(get_max_freq "${GPU_DIR}/available_frequencies")
	gpu_min_freq=$(get_min_freq "${GPU_DIR}/available_frequencies")
	write_value "$gpu_max_freq" "${GPU_DIR}/max_freq"
	write_value "$gpu_min_freq" "${GPU_DIR}/min_freq"
done
for GPU_DIR in /sys/class/devfreq/*mali* ; do 
	gpu_max_freq=$(get_max_freq "${GPU_DIR}/available_frequencies")
	gpu_min_freq=$(get_min_freq "${GPU_DIR}/available_frequencies")
	write_value "$gpu_max_freq" "${GPU_DIR}/max_freq"
	write_value "$gpu_min_freq" "${GPU_DIR}/min_freq"
done

while [ ! -e /sdcard/.test_file ] ; do
	true > /sdcard/.test_file
	sleep 1
done
rm -f /sdcard/.test_file

if [ ! -d /sdcard/Android/ct/ ] ; then
	mkdir -p /sdcard/Android/ct/
	echo "balance" > /sdcard/Android/ct/cur_mode.txt
fi

SCRIPT_PATH=$(dirname $0)
#CuDaemon -R [config] [mode] [log]
${SCRIPT_PATH}/CuDaemon -R "${SCRIPT_PATH}/config.json" "/sdcard/Android/ct/cur_mode.txt" "/sdcard/Android/ct/scheduler.log"

exit 0