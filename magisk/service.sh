#!/system/bin/sh

function get_min_freq() {
	first_freq=$(cat "$1" | tr " " "\n" | head -1)
	last_freq=$(cat "$1" | tr " " "\n" | tail -1)
	if [ $first_freq -lt $last_freq ] ; then
		echo "${first_freq}"
	else
		echo "${last_freq}"
	fi
}

function get_max_freq() {
	first_freq=$(cat "$1" | tr " " "\n" | head -1)
	last_freq=$(cat "$1" | tr " " "\n" | tail -1)
	if [ $first_freq -gt $last_freq ] ; then
		echo "${first_freq}"
	else
		echo "${last_freq}"
	fi
}

function write_value() {
	for file in $2 ; do
    if [ -e "${file}" ] ; then
    	echo "$1" > "${file}"
    fi
  done
}

function lock_value() {
	for file in $2 ; do
  	if [ -e "${file}" ] ; then
  		chown root:root "${file}"
  		chmod 0666 "${file}"
  		echo "$1" > "${file}"
  		chmod 0444 "${file}"
  	fi
  done
}

function change_task_cpuset() {
  task_name=$1
  cgroup=$2
	for task_pid in $(ps -Ao pid,args | grep "${task_name}" | awk '{print $1}') ; do
		echo "${task_pid}" > "/dev/cpuset/${cgroup}/cgroup.procs"
	done
}

function change_task_sched() {
  task_name=$1
  cgroup=$2
	for task_pid in $(ps -Ao pid,args | grep "${task_name}" | awk '{print $1}') ; do
	  if [ -d /dev/stune ] ; then
		  echo "${task_pid}" > "/dev/stune/${cgroup}/cgroup.procs"
		elif [ -d /dev/cpuctl ] ; then
		  echo "${task_pid}" > "/dev/cpuctl/${cgroup}/cgroup.procs"
		fi
	done
}

stop miuibooster 2>/dev/null
stop oneplus_brain_service 2>/dev/null
stop vendor.perfservice 2>/dev/null
stop perfd 2>/dev/null
setprop persist.sys.hardcoder.name "" 2>/dev/null
setprop persist.miui.miperf.enable "false" 2>/dev/null

lock_value "1" /proc/ppm/enabled
if [ -e /proc/ppm/policy_status ] ; then 
	for ppm_policy in $(seq 0 10) ; do
		write_value "${ppm_policy} 1" "/proc/ppm/policy_status"
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
		IDX_INFO=$(cat /proc/ppm/policy_status | grep "${policy}")
		IDX_NUM=$(echo ${IDX_INFO:1:1})
		write_value "${IDX_NUM} 0" "/proc/ppm/policy_status"
	done
fi

lock_value "0" "/sys/module/fbt_cpu/parameters/boost_affinity*"
lock_value "0" "/sys/module/mtk_fpsgo/parameters/boost_affinity*"
lock_value "0" "/proc/hps/enabled"
lock_value "0" "/sys/kernel/ged/hal/dcs_mode"
lock_value "0" "/sys/module/mtk_fpsgo/parameters/perfmgr_enable"
lock_value "0" "/sys/kernel/fpsgo/common/fpsgo_enable"
lock_value "0" "/sys/kernel/debug/fpsgo/common/force_onoff"
lock_value "0" "/sys/kernel/fpsgo/common/force_onoff"
lock_value "enable: 0" "/proc/perfmgr/tchbst/user/usrtch"
lock_value "2" "/sys/devices/system/cpu/eas/enable"

lock_value "0" "/sys/devices/system/cpu/sched/sched_boost"
lock_value "0" "/sys/kernel/hmp/boost"
lock_value "0" "/sys/kernel/hmp/boostpulse_duration"
lock_value "0" "/sys/power/cpuhotplug/enabled"
lock_value "0" "/sys/kernel/ems/eff_mode"
lock_value "0" "/sys/module/msm_performance/parameters/touchboost"
lock_value "0" "/sys/devices/system/cpu/cpu*/sched_load_boost"
lock_value "0" "/proc/sys/walt/input_boost/*"
lock_value "0" "/sys/devices/system/cpu/cpu_boost/*"
lock_value "0" "/sys/devices/system/cpu/cpu_boost/parameters/*"
lock_value "0" "/sys/module/cpu_boost/parameters/*"
lock_value "0" "/sys/module/aigov/parameters/enable"
lock_value "0" "/sys/module/opchain/parameters/chain_on"
lock_value "0" "/sys/power/pnpmgr/touch_boost"
lock_value "0" "/sys/power/pnpmgr/long_duration_touch_boost"
lock_value "0" "/proc/sys/fbg/frame_boost_enabled"
lock_value "0" "/proc/sys/fbg/input_boost_enabled"
lock_value "0" "/proc/sys/fbg/slide_boost_enabled"
lock_value "0" "/sys/module/houston/parameters/*"
lock_value "N" "/sys/module/control_center/parameters/*"
lock_value "0" "/sys/kernel/cpu_input_boost/*"
lock_value "0" "/sys/module/dsboost/parameters/*"
lock_value "0" "/sys/module/cpu_input_boost/parameters/*"
lock_value "0" "/sys/module/input_cfboost/parameters/*"
lock_value "0" "/sys/class/input_booster/*"
lock_value "0" "/sys/devices/system/cpu/cpu*/sched_load_boost"

lock_value "1" "/sys/devices/system/cpu/cpufreq/hotplug/cpu_hotplug_disable"
lock_value "0" "/sys/devices/system/cpu/cpuhotplug/enabled"
lock_value "0" "/sys/module/msm_thermal/vdd_restriction/enabled"
lock_value "0" "/sys/module/msm_thermal/core_control/enabled"
lock_value "N" "/sys/module/msm_thermal/parameters/enabled"
lock_value "0" "/sys/kernel/intelli_plug/intelli_plug_active"
lock_value "0" "/sys/module/blu_plug/parameters/enabled"
lock_value "0" "/sys/devices/virtual/misc/mako_hotplug_control/enabled"
lock_value "0" "/sys/module/autosmp/parameters/enabled"
lock_value "0" "/sys/kernel/zen_decision/enabled"
lock_value "0" "/sys/devices/system/cpu/hyp_core_ctl/enable"

for sched_type in walt kernel ; do
	lock_value "0" "/proc/sys/${sched_type}/sched_boost"
	lock_value "0" "/proc/sys/${sched_type}/sched_util_clamp_min"
	lock_value "" "/proc/sys/${sched_type}/sched_lib_name"
done

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
		lock_value "0" "${perfserv_boost}"
	done
	for perfserv_uclamp_min in /proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_*_uclamp_min ; do
		lock_value "0" "${perfserv_uclamp_min}"
	done
fi

all_cpus=$(cat /dev/cpuset/cpus)
for cpuset_cpus in /dev/cpuset/*/cpus ; do
  write_value "${all_cpus}" "${cpuset_cpus}"
done

lock_value "1" "/sys/devices/system/cpu/cpu*/online"
lock_value "1" "/sys/devices/system/cpu/cpu*/core_ctl/enable"   
if [ -d /sys/devices/system/cpu/cpufreq/policy2 ] ; then
	lock_value "2" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
	lock_value "2" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
	lock_value "2" "/sys/devices/system/cpu/cpu2/core_ctl/max_cpus"
	lock_value "2" "/sys/devices/system/cpu/cpu2/core_ctl/min_cpus"
elif [ -d /sys/devices/system/cpu/cpufreq/policy3 ] ; then
  lock_value "3" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
  lock_value "3" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
  lock_value "4" "/sys/devices/system/cpu/cpu3/core_ctl/max_cpus"
  lock_value "4" "/sys/devices/system/cpu/cpu3/core_ctl/min_cpus"
  lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/max_cpus"
  lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/min_cpus"
elif [ -d /sys/devices/system/cpu/cpufreq/policy4 ] ; then
	if [ -d /sys/devices/system/cpu/cpufreq/policy7 ] ; then
		lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
		lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
		lock_value "3" "/sys/devices/system/cpu/cpu4/core_ctl/max_cpus"
		lock_value "3" "/sys/devices/system/cpu/cpu4/core_ctl/min_cpus"
		lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/max_cpus"
		lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/min_cpus"
	else
		if [ -d /sys/devices/system/cpu/cpu7 ] ; then
			lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
			lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
			lock_value "4" "/sys/devices/system/cpu/cpu4/core_ctl/max_cpus"
			lock_value "4" "/sys/devices/system/cpu/cpu4/core_ctl/min_cpus"
		else
			lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
			lock_value "4" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
			lock_value "2" "/sys/devices/system/cpu/cpu4/core_ctl/max_cpus"
			lock_value "2" "/sys/devices/system/cpu/cpu4/core_ctl/min_cpus"
		fi
	fi
elif [ -d /sys/devices/system/cpu/cpufreq/policy6 ] ; then
	if [ -d /sys/devices/system/cpu/cpufreq/policy7 ] ; then
		lock_value "6" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
		lock_value "6" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
		lock_value "1" "/sys/devices/system/cpu/cpu6/core_ctl/max_cpus"
		lock_value "1" "/sys/devices/system/cpu/cpu6/core_ctl/min_cpus"
		lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/max_cpus"
		lock_value "1" "/sys/devices/system/cpu/cpu7/core_ctl/min_cpus"
	else
		lock_value "6" "/sys/devices/system/cpu/cpu0/core_ctl/max_cpus"
		lock_value "6" "/sys/devices/system/cpu/cpu0/core_ctl/min_cpus"
		lock_value "2" "/sys/devices/system/cpu/cpu6/core_ctl/max_cpus"
		lock_value "2" "/sys/devices/system/cpu/cpu6/core_ctl/min_cpus"
	fi
fi
lock_value "0" "/sys/devices/system/cpu/cpu*/core_ctl/enable"   
lock_value "0" "/sys/devices/system/cpu/cpu*/core_ctl/core_ctl_boost"

if [ -d /sys/class/devfreq/gpufreq/ ] ; then
  write_value "mali_ondemand" "/sys/class/devfreq/gpufreq/governor"
  write_value "0" "/sys/class/devfreq/gpufreq/animation_boost"
  write_value "0" "/sys/class/devfreq/gpufreq/cl_boost"
  write_value "1" "/sys/class/devfreq/gpufreq/vsync"
  write_value "80" "/sys/class/devfreq/gpufreq/upthreshold"
  write_value "20" "/sys/class/devfreq/gpufreq/downdifferential"
  gpu_max_freq=$(get_max_freq "/sys/class/devfreq/gpufreq/available_frequencies")
  gpu_min_freq=$(get_min_freq "/sys/class/devfreq/gpufreq/available_frequencies")
  write_value "${gpu_max_freq}" "/sys/class/devfreq/gpufreq/max_freq"
  write_value "${gpu_min_freq}" "/sys/class/devfreq/gpufreq/min_freq"
fi

for GPU_DIR in /sys/class/devfreq/*gpu* ; do
  if [ -d ${GPU_DIR} ] ; then
	  gpu_max_freq=$(get_max_freq "${GPU_DIR}/available_frequencies")
	  gpu_min_freq=$(get_min_freq "${GPU_DIR}/available_frequencies")
	  write_value "${gpu_max_freq}" "${GPU_DIR}/max_freq"
	  write_value "${gpu_min_freq}" "${GPU_DIR}/min_freq"
	fi
done

change_task_cpuset "surfaceflinger" ""
change_task_sched "surfaceflinger" ""
change_task_cpuset "system_server" ""
change_task_sched "system_server" ""
change_task_cpuset "android.hardware.graphics.composer" ""
change_task_sched "android.hardware.graphics.composer" ""
change_task_cpuset "vendor.qti.hardware.display.composer-service" ""
change_task_sched "vendor.qti.hardware.display.composer-service" ""

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
while [ ! -e /sdcard/.test_file ] ; do
	true > /sdcard/.test_file
	sleep 1
done
rm -f /sdcard/.test_file

# Create CT Dir.
if [ ! -d /sdcard/Android/ct/ ] ; then
	mkdir -p /sdcard/Android/ct/
	echo "balance" > /sdcard/Android/ct/cur_mode.txt
fi

SCRIPT_PATH=$(dirname $0)
#CuDaemon -R [config] [mode] [log]
${SCRIPT_PATH}/CuDaemon -R "${SCRIPT_PATH}/config.json" "/sdcard/Android/ct/cur_mode.txt" "/sdcard/Android/ct/scheduler.log"

exit 0
