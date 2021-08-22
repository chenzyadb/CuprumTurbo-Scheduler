#!/system/bin/sh
CUPRUM_DIR=${0%/*}
MODE=$(echo $1)
if [ -e /proc/sys/kernel/osrelease ] ; then
 KernelVersionStr=$(cat /proc/sys/kernel/osrelease)
 KernelVersionS=${KernelVersionStr:2:2}
 KernelVersionA=${KernelVersionStr:0:1}
 KernelVersionB=${KernelVersionS%.*}
 if [ $KernelVersionA -ge 4 ]&&[ $KernelVersionB -ge 9 ] ; then
  EAS_SUPPORT_KERNEL="true"
 else
  EAS_SUPPORT_KERNEL="false"
 fi
else
 EAS_SUPPORT_KERNEL="false"
fi
function setlog() {
 echo "$(date +%F) $(date +%T) [$1] $2" >> "/data/Cuprum_Log.txt" ;
}
function write_value() {
 if [ -e $2 ] ; then
  echo "$1" > "$2" 2>/dev/null &
 fi
}
function lock_value() {
 if [ -e $2 ] ; then
  chmod 0666 $2 2>/dev/null ;
  echo "$1" > "$2" 2>/dev/null ;
  chmod 0444 $2 2>/dev/null ;
 fi
}
function main_adjust() {
if [ -d /sys/devices/system/cpu/cpu7/ ] ; then
 core_num="7"
elif [ -d /sys/devices/system/cpu/cpu5/ ] ; then
 core_num="5"
elif [ -d /sys/devices/system/cpu/cpu3/ ] ; then
 core_num="3"
fi
if [ -e /proc/ppm/policy_status ] ; then 
 HARDUSER_IDX_INFO=$(cat /proc/ppm/policy_status | grep "PPM_POLICY_HARD_USER_LIMIT") 
 HARDUSER_IDX_NUM=$(echo ${HARDUSER_IDX_INFO:1:1})
 for ppm_policy in $(seq 0 10) ; do
  write_value "$ppm_policy 0" "/proc/ppm/policy_status"
 done
 #HARD_USER_LIMIT:CPU Freq Control
 write_value "${HARDUSER_IDX_NUM} 1" "/proc/ppm/policy_status"
fi
if [ $MODE = "performance" ] ; then
 if [ -d /dev/stune/ ] ; then
  write_value "0" "/dev/stune/schedtune.boost" 
  write_value "0" "/dev/stune/schedtune.prefer_idle"  
  write_value "1" "/dev/stune/top-app/schedtune.prefer_idle" 
  write_value "0" "/dev/stune/foreground/schedtune.prefer_idle" 
  write_value "0" "/dev/stune/background/schedtune.prefer_idle" 
  write_value "0" "/dev/stune/rt/schedtune.prefer_idle" 
  write_value "0" "/dev/stune/rt/schedtune.boost" 
  write_value "0" "/dev/stune/top-app/schedtune.boost" 
  write_value "0" "/dev/stune/foreground/schedtune.boost" 
  write_value "0" "/dev/stune/background/schedtune.boost"
 fi
 if [ -e /sys/devices/system/cpu/cpufreq/schedutil/down_rate_limit_us ] ; then
  write_value "1000" /sys/devices/system/cpu/cpufreq/schedutil/down_rate_limit_us 
  write_value "1000" /sys/devices/system/cpu/cpufreq/schedutil/up_rate_limit_us
 fi
 if [ -e /sys/devices/system/cpu/sched/sched_boost ] ; then
  write_value 0 /sys/devices/system/cpu/sched/sched_boost
  write_value 2 "/sys/devices/system/cpu/eas/enable"
 fi
 for i in $(seq 0 7) ; do
  if [ -e /sys/devices/system/cpu/cpu${i}/sched_prefer_idle ] ; then
   write_value "0" /sys/devices/system/cpu/cpu${i}/sched_prefer_idle 
  fi
 done
else
 if [ -d /dev/stune/ ] ; then
  write_value "0" "/dev/stune/schedtune.boost" 
  write_value "0" "/dev/stune/schedtune.prefer_idle"  
  write_value "0" "/dev/stune/top-app/schedtune.prefer_idle"  
  write_value "0" "/dev/stune/foreground/schedtune.prefer_idle" 
  write_value "0" "/dev/stune/background/schedtune.prefer_idle" 
  write_value "0" "/dev/stune/rt/schedtune.prefer_idle" 
  write_value "0" "/dev/stune/rt/schedtune.boost" 
  write_value "0" "/dev/stune/top-app/schedtune.boost" 
  write_value "0" "/dev/stune/foreground/schedtune.boost" 
  write_value "0" "/dev/stune/background/schedtune.boost"
 fi
 if [ -e /sys/devices/system/cpu/cpufreq/schedutil/down_rate_limit_us ] ; then
  write_value "1000" /sys/devices/system/cpu/cpufreq/schedutil/down_rate_limit_us 
  write_value "1000" /sys/devices/system/cpu/cpufreq/schedutil/up_rate_limit_us
 fi
 if [ -e /sys/devices/system/cpu/sched/sched_boost ] ; then
  write_value 0 /sys/devices/system/cpu/sched/sched_boost
  write_value 1 "/sys/devices/system/cpu/eas/enable"
 fi
 for i in $(seq 0 7) ; do
  if [ -e /sys/devices/system/cpu/cpu${i}/sched_prefer_idle ] ; then
   write_value "0" /sys/devices/system/cpu/cpu${i}/sched_prefer_idle 
  fi
 done
 for i in $(seq 0 7) ; do
  if [ -e /sys/devices/system/cpu/cpu${i}/sched_load_boost ] ; then
   write_value "-6" "/sys/devices/system/cpu/cpu${i}/sched_load_boost"
  fi
 done
fi
if [ -d /sys/devices/system/cpu/cpu0/core_ctl/ ]&&[ $EAS_SUPPORT_KERNEL = "true" ] ; then
 for i in $(seq 1 7) ; do
  if [ -e /sys/devices/system/cpu/cpu${i}/core_ctl/enabled ] ; then
   lock_value "1" /sys/devices/system/cpu/cpu${i}/core_ctl/enabled
  fi
 done
 if [ $MODE = "performance" ] ; then
  lock_value "0" /sys/devices/system/cpu/cpu0/core_ctl/enabled
  if [ -d /sys/devices/system/cpu/cpufreq/policy4/ ] ; then
   if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
    # Core control parameters for gold
    lock_value "1" /sys/devices/system/cpu/cpu4/core_ctl/enabled
    write_value 3 /sys/devices/system/cpu/cpu4/core_ctl/min_cpus
    write_value 3 /sys/devices/system/cpu/cpu4/core_ctl/max_cpus
    write_value 60 /sys/devices/system/cpu/cpu4/core_ctl/busy_up_thres
    write_value 30 /sys/devices/system/cpu/cpu4/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu4/core_ctl/offline_delay_ms
    write_value "0 0 0" /sys/devices/system/cpu/cpu4/core_ctl/not_preferred
    write_value 4 /sys/devices/system/cpu/cpu4/core_ctl/task_thres
    # Core control parameters for gold+
    lock_value "1" /sys/devices/system/cpu/cpu7/core_ctl/enabled
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/min_cpus
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/max_cpus
    write_value 60 /sys/devices/system/cpu/cpu7/core_ctl/busy_up_thres
    write_value 30 /sys/devices/system/cpu/cpu7/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu7/core_ctl/offline_delay_ms
    write_value "0" /sys/devices/system/cpu/cpu7/core_ctl/not_preferred
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/task_thres
   else
    # Core control parameters for gold
    lock_value "1" /sys/devices/system/cpu/cpu4/core_ctl/enabled
    write_value 4 /sys/devices/system/cpu/cpu4/core_ctl/min_cpus
    write_value 4 /sys/devices/system/cpu/cpu4/core_ctl/max_cpus
    write_value 60 /sys/devices/system/cpu/cpu4/core_ctl/busy_up_thres
    write_value 30 /sys/devices/system/cpu/cpu4/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu4/core_ctl/offline_delay_ms
    write_value "0 0 0 0" /sys/devices/system/cpu/cpu4/core_ctl/not_preferred
    write_value 4 /sys/devices/system/cpu/cpu4/core_ctl/task_thres
   fi
  elif [ -d /sys/devices/system/cpu/cpufreq/policy6/ ] ; then
   # Core control parameters for silver
   lock_value "0" /sys/devices/system/cpu/cpu0/core_ctl/enabled
   write_value 6 /sys/devices/system/cpu/cpu0/core_ctl/min_cpus
   write_value 6 /sys/devices/system/cpu/cpu0/core_ctl/max_cpus
   write_value "0 0 0 0 0 0" /sys/devices/system/cpu/cpu0/core_ctl/not_preferred
   if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
    # Core control parameters for gold
    lock_value "1" /sys/devices/system/cpu/cpu6/core_ctl/enabled
    write_value 1 /sys/devices/system/cpu/cpu6/core_ctl/min_cpus
    write_value 1 /sys/devices/system/cpu/cpu6/core_ctl/max_cpus
    write_value 60 /sys/devices/system/cpu/cpu6/core_ctl/busy_up_thres
    write_value 40 /sys/devices/system/cpu/cpu6/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu6/core_ctl/offline_delay_ms
    write_value "0" /sys/devices/system/cpu/cpu6/core_ctl/not_preferred
    write_value 2 /sys/devices/system/cpu/cpu6/core_ctl/task_thres
    # Core control parameters for gold+
    lock_value "1" /sys/devices/system/cpu/cpu7/core_ctl/enabled
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/min_cpus
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/max_cpus
    write_value 60 /sys/devices/system/cpu/cpu7/core_ctl/busy_up_thres
    write_value 40 /sys/devices/system/cpu/cpu7/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu7/core_ctl/offline_delay_ms
    write_value "0" /sys/devices/system/cpu/cpu7/core_ctl/not_preferred
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/task_thres
   else
    # Core control parameters for gold
    lock_value "1" /sys/devices/system/cpu/cpu6/core_ctl/enabled
    write_value 2 /sys/devices/system/cpu/cpu6/core_ctl/min_cpus
    write_value 2 /sys/devices/system/cpu/cpu6/core_ctl/max_cpus
    write_value 60 /sys/devices/system/cpu/cpu6/core_ctl/busy_up_thres
    write_value 40 /sys/devices/system/cpu/cpu6/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu6/core_ctl/offline_delay_ms
    write_value "0 0" /sys/devices/system/cpu/cpu6/core_ctl/not_preferred
    write_value 2 /sys/devices/system/cpu/cpu6/core_ctl/task_thres
   fi
  fi
 elif [ $MODE = "balance" ] ; then
  lock_value "0" /sys/devices/system/cpu/cpu0/core_ctl/enabled
  if [ -d /sys/devices/system/cpu/cpufreq/policy4/ ] ; then
   if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
    # Core control parameters for gold
    lock_value "1" /sys/devices/system/cpu/cpu4/core_ctl/enabled
    write_value 2 /sys/devices/system/cpu/cpu4/core_ctl/min_cpus
    write_value 3 /sys/devices/system/cpu/cpu4/core_ctl/max_cpus
    write_value 60 /sys/devices/system/cpu/cpu4/core_ctl/busy_up_thres
    write_value 30 /sys/devices/system/cpu/cpu4/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu4/core_ctl/offline_delay_ms
    write_value "0 0 0" /sys/devices/system/cpu/cpu4/core_ctl/not_preferred
    write_value 4 /sys/devices/system/cpu/cpu4/core_ctl/task_thres
    # Core control parameters for gold+
    lock_value "1" /sys/devices/system/cpu/cpu7/core_ctl/enabled
    write_value 0 /sys/devices/system/cpu/cpu7/core_ctl/min_cpus
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/max_cpus
    write_value 60 /sys/devices/system/cpu/cpu7/core_ctl/busy_up_thres
    write_value 30 /sys/devices/system/cpu/cpu7/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu7/core_ctl/offline_delay_ms
    write_value "0" /sys/devices/system/cpu/cpu7/core_ctl/not_preferred
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/task_thres
   else
    # Core control parameters for gold
    lock_value "1" /sys/devices/system/cpu/cpu4/core_ctl/enabled
    write_value 3 /sys/devices/system/cpu/cpu4/core_ctl/min_cpus
    write_value 4 /sys/devices/system/cpu/cpu4/core_ctl/max_cpus
    write_value 60 /sys/devices/system/cpu/cpu4/core_ctl/busy_up_thres
    write_value 30 /sys/devices/system/cpu/cpu4/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu4/core_ctl/offline_delay_ms
    write_value "0 0 0 0" /sys/devices/system/cpu/cpu4/core_ctl/not_preferred
    write_value 4 /sys/devices/system/cpu/cpu4/core_ctl/task_thres
   fi
  elif [ -d /sys/devices/system/cpu/cpufreq/policy6/ ] ; then
   # Core control parameters for silver
   write_value 6 /sys/devices/system/cpu/cpu0/core_ctl/min_cpus
   write_value 6 /sys/devices/system/cpu/cpu0/core_ctl/max_cpus
   write_value "0 0 0 0 0 0" /sys/devices/system/cpu/cpu0/core_ctl/not_preferred
   if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
    # Core control parameters for gold
    lock_value "1" /sys/devices/system/cpu/cpu6/core_ctl/enabled
    write_value 1 /sys/devices/system/cpu/cpu6/core_ctl/min_cpus
    write_value 1 /sys/devices/system/cpu/cpu6/core_ctl/max_cpus
    write_value 60 /sys/devices/system/cpu/cpu6/core_ctl/busy_up_thres
    write_value 40 /sys/devices/system/cpu/cpu6/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu6/core_ctl/offline_delay_ms
    write_value "0" /sys/devices/system/cpu/cpu6/core_ctl/not_preferred
    write_value 2 /sys/devices/system/cpu/cpu6/core_ctl/task_thres
    # Core control parameters for gold+
    lock_value "1" /sys/devices/system/cpu/cpu7/core_ctl/enabled
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/min_cpus
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/max_cpus
    write_value 60 /sys/devices/system/cpu/cpu7/core_ctl/busy_up_thres
    write_value 40 /sys/devices/system/cpu/cpu7/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu7/core_ctl/offline_delay_ms
    write_value "0" /sys/devices/system/cpu/cpu7/core_ctl/not_preferred
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/task_thres
   else
    # Core control parameters for gold
    lock_value "1" /sys/devices/system/cpu/cpu6/core_ctl/enabled
    write_value 2 /sys/devices/system/cpu/cpu6/core_ctl/min_cpus
    write_value 2 /sys/devices/system/cpu/cpu6/core_ctl/max_cpus
    write_value 60 /sys/devices/system/cpu/cpu6/core_ctl/busy_up_thres
    write_value 40 /sys/devices/system/cpu/cpu6/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu6/core_ctl/offline_delay_ms
    write_value "0 0" /sys/devices/system/cpu/cpu6/core_ctl/not_preferred
    write_value 2 /sys/devices/system/cpu/cpu6/core_ctl/task_thres
   fi
  fi
 else
  lock_value "0" /sys/devices/system/cpu/cpu0/core_ctl/enabled
  if [ -d /sys/devices/system/cpu/cpufreq/policy4/ ] ; then
   if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
    # Core control parameters for gold
    lock_value "1" /sys/devices/system/cpu/cpu4/core_ctl/enabled
    write_value 0 /sys/devices/system/cpu/cpu4/core_ctl/min_cpus
    write_value 2 /sys/devices/system/cpu/cpu4/core_ctl/max_cpus
    write_value 80 /sys/devices/system/cpu/cpu4/core_ctl/busy_up_thres
    write_value 30 /sys/devices/system/cpu/cpu4/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu4/core_ctl/offline_delay_ms
    write_value "0 0 0" /sys/devices/system/cpu/cpu4/core_ctl/not_preferred
    write_value 4 /sys/devices/system/cpu/cpu4/core_ctl/task_thres
    # Core control parameters for gold+
    lock_value "1" /sys/devices/system/cpu/cpu7/core_ctl/enabled
    write_value 0 /sys/devices/system/cpu/cpu7/core_ctl/min_cpus
    write_value 0 /sys/devices/system/cpu/cpu7/core_ctl/max_cpus
    write_value 90 /sys/devices/system/cpu/cpu7/core_ctl/busy_up_thres
    write_value 30 /sys/devices/system/cpu/cpu7/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu7/core_ctl/offline_delay_ms
    write_value "0" /sys/devices/system/cpu/cpu7/core_ctl/not_preferred
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/task_thres
   else
    # Core control parameters for gold
    lock_value "1" /sys/devices/system/cpu/cpu4/core_ctl/enabled
    write_value 0 /sys/devices/system/cpu/cpu4/core_ctl/min_cpus
    write_value 2 /sys/devices/system/cpu/cpu4/core_ctl/max_cpus
    write_value 80 /sys/devices/system/cpu/cpu4/core_ctl/busy_up_thres
    write_value 30 /sys/devices/system/cpu/cpu4/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu4/core_ctl/offline_delay_ms
    write_value "0 0 0 0" /sys/devices/system/cpu/cpu4/core_ctl/not_preferred
    write_value 4 /sys/devices/system/cpu/cpu4/core_ctl/task_thres
   fi
 elif [ -d /sys/devices/system/cpu/cpufreq/policy6/ ] ; then
  # Core control parameters for silver
  write_value 6 /sys/devices/system/cpu/cpu0/core_ctl/min_cpus
  write_value 6 /sys/devices/system/cpu/cpu0/core_ctl/max_cpus
  write_value "0 0 0 0 0 0" /sys/devices/system/cpu/cpu0/core_ctl/not_preferred
  if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
    # Core control parameters for gold
    lock_value "1" /sys/devices/system/cpu/cpu6/core_ctl/enabled
    write_value 1 /sys/devices/system/cpu/cpu6/core_ctl/min_cpus
    write_value 1 /sys/devices/system/cpu/cpu6/core_ctl/max_cpus
    write_value 80 /sys/devices/system/cpu/cpu6/core_ctl/busy_up_thres
    write_value 40 /sys/devices/system/cpu/cpu6/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu6/core_ctl/offline_delay_ms
    write_value "0" /sys/devices/system/cpu/cpu6/core_ctl/not_preferred
    write_value 2 /sys/devices/system/cpu/cpu6/core_ctl/task_thres
    # Core control parameters for gold+
    lock_value "1" /sys/devices/system/cpu/cpu7/core_ctl/enabled
    write_value 0 /sys/devices/system/cpu/cpu7/core_ctl/min_cpus
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/max_cpus
    write_value 80 /sys/devices/system/cpu/cpu7/core_ctl/busy_up_thres
    write_value 40 /sys/devices/system/cpu/cpu7/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu7/core_ctl/offline_delay_ms
    write_value "0" /sys/devices/system/cpu/cpu7/core_ctl/not_preferred
    write_value 1 /sys/devices/system/cpu/cpu7/core_ctl/task_thres
   else
    # Core control parameters for gold
    lock_value "1" /sys/devices/system/cpu/cpu6/core_ctl/enabled
    write_value 1 /sys/devices/system/cpu/cpu6/core_ctl/min_cpus
    write_value 2 /sys/devices/system/cpu/cpu6/core_ctl/max_cpus
    write_value 60 /sys/devices/system/cpu/cpu6/core_ctl/busy_up_thres
    write_value 40 /sys/devices/system/cpu/cpu6/core_ctl/busy_down_thres
    write_value 100 /sys/devices/system/cpu/cpu6/core_ctl/offline_delay_ms
    write_value "0 0" /sys/devices/system/cpu/cpu6/core_ctl/not_preferred
    write_value 2 /sys/devices/system/cpu/cpu6/core_ctl/task_thres
   fi
  fi
 fi
elif [ -d /sys/devices/system/cpu/cpu0/core_ctl/ ] ; then
 for i in $(seq 0 7) ; do
  if [ -e /sys/devices/system/cpu/cpu${i}/core_ctl/enabled ] ; then
   lock_value "0" /sys/devices/system/cpu/cpu${i}/core_ctl/enabled
  fi
 done
fi
if [ $MODE = "performance" ] ; then
 if [ -d /proc/perfmgr/boost_ctrl/eas_ctrl/ ] ; then
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_prefer_idle"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_fg_boost"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_ta_boost"
  write_value "-100" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_bg_boost"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_uclamp_min"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_fg_uclamp_min"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_ta_uclamp_min"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_bg_uclamp_min"
  write_value "1" "/proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_enable"
  write_value "60" "/proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_up_loading"
  write_value "80" "/proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_down_loading"
 fi
elif [ $MODE = "balance" ] ; then
 if [ -d /proc/perfmgr/boost_ctrl/eas_ctrl/ ] ; then
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_prefer_idle"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_fg_boost"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_ta_boost"
  write_value "-100" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_bg_boost"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_uclamp_min"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_fg_uclamp_min"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_ta_uclamp_min"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_bg_uclamp_min"
  write_value "1" "/proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_enable"
  write_value "60" "/proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_up_loading"
  write_value "80" "/proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_down_loading"
 fi
else
 if [ -d /proc/perfmgr/boost_ctrl/eas_ctrl/ ] ; then
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_prefer_idle"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_fg_boost"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_ta_boost"
  write_value "-100" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_bg_boost"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_uclamp_min"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_fg_uclamp_min"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_ta_uclamp_min"
  write_value "0" "/proc/perfmgr/boost_ctrl/eas_ctrl/perfserv_bg_uclamp_min"
  write_value "1" "/proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_enable"
  write_value "80" "/proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_up_loading"
  write_value "60" "/proc/perfmgr/boost_ctrl/cpu_ctrl/cfp_down_loading"
 fi
fi
AVAILABLE_GOV=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors)
CPU_GOV="interactive"
case $AVAILABLE_GOV in *"sched"*)
 CPU_GOV="sched"
esac
case $AVAILABLE_GOV in *"schedutil"*)
 CPU_GOV="schedutil"
esac
for i in $(seq 0 7) ; do
 if [ -e /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_governor ] ; then
  write_value "$CPU_GOV" /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_governor 
 fi
done
for i in $(seq 0 7) ; do
 if [ -d /sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/ ] ; then
  write_value 95 "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/go_hispeed_load"
  write_value 95 "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/hispeed_load"
  hispeed_freq=$(cat /sys/devices/system/cpu/cpu${i}/cpufreq/cpuinfo_max_freq)
  write_value "$hispeed_freq" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/hispeed_freq"
  write_value "10000" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/above_hispeed_delay"
  write_value "10000" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/min_sample_time"
  write_value "0" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/boost"
  write_value "0" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/io_is_busy"
  write_value "0" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/target_loads"
  write_value "0" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/boostpulse_duration"
  write_value "999" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/single_enter_load"
  write_value "999" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/single_exit_load"
  write_value "0" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/multi_enter_load"
  write_value "0" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/multi_exit_load"
  if [ $MODE = "powersave" ] ; then
   write_value "100000" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/up_rate_limit_us"
   write_value "0" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/down_rate_limit_us"
  elif [ $MODE = "balance" ] ; then
   write_value "50000" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/up_rate_limit_us"
   write_value "0" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/down_rate_limit_us"
  else
   write_value "0" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/up_rate_limit_us"
   write_value "50000" "/sys/devices/system/cpu/cpu${i}/cpufreq/${CPU_GOV}/down_rate_limit_us"
  fi
 fi
done
if [ -d /sys/devices/system/cpu/cpufreq/${CPU_GOV}/ ] ; then
 write_value 95 "/sys/devices/system/cpu/cpufreq/${CPU_GOV}/go_hispeed_load"
 write_value 95 "/sys/devices/system/cpu/cpufreq/${CPU_GOV}/hispeed_load"
 hispeed_freq=$(cat /sys/devices/system/cpu/cpu${core_num}/cpufreq/cpuinfo_max_freq)
 write_value "$hispeed_freq" "/sys/devices/system/cpu/cpufreq/${CPU_GOV}/hispeed_freq"
 write_value "10000" "/sys/devices/system/cpu/cpufreq/${CPU_GOV}/above_hispeed_delay"
 write_value "10000" "/sys/devices/system/cpu/cpufreq/${CPU_GOV}/min_sample_time"
 write_value "0" "/sys/devices/system/cpu/cpufreq/${CPU_GOV}/boost"
 write_value "0" "/sys/devices/system/cpu/cpufreq/${CPU_GOV}/io_is_busy"
 write_value "0" "/sys/devices/system/cpu/cpufreq/${CPU_GOV}/target_loads"
 write_value "0" "/sys/devices/system/cpu/cpufreq/${CPU_GOV}/boostpulse_duration"
fi
if [ -d /sys/kernel/hmp/ ] ; then
 write_value "0" "/sys/kernel/hmp/boost"
 write_value "0" "/sys/kernel/hmp/boostpulse_duration"
 write_value "500" "/sys/kernel/hmp/up_threshold"
 write_value "300" "/sys/kernel/hmp/down_threshold"
 write_value "400" "/sys/kernel/hmp/sb_up_threshold"
 write_value "200" "/sys/kernel/hmp/sb_down_threshold"
fi
for i in $(seq 0 7) ; do
 if [ -e /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_min_freq ] ; then
  chmod 0666 /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_min_freq 
 fi
 if [ -e /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_max_freq ] ; then
  chmod 0666 /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_max_freq 
 fi
done
if [ $MODE = "performance" ] ; then
  write_value "20" /proc/sys/kernel/sched_spill_nr_run
  write_value "95" /proc/sys/kernel/sched_spill_load
  write_value "2000000" /proc/sys/kernel/sched_min_granularity_ns
  write_value "0" /proc/sys/kernel/sched_tunable_scaling
  write_value "40" /proc/sys/kernel/sched_nr_migrate
  write_value "0" /proc/sys/kernel/sched_util_clamp_min
  write_value "100" /proc/sys/kernel/sched_util_clamp_max
else
  write_value "20" /proc/sys/kernel/sched_spill_nr_run
  write_value "95" /proc/sys/kernel/sched_spill_load
  write_value "1000000" /proc/sys/kernel/sched_min_granularity_ns
  write_value "0" /proc/sys/kernel/sched_tunable_scaling
  write_value "30" /proc/sys/kernel/sched_nr_migrate
  write_value "0" /proc/sys/kernel/sched_util_clamp_min
  write_value "100" /proc/sys/kernel/sched_util_clamp_max
fi
if [ $MODE = "performance" ] ; then
 if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
  write_value "50 70" /proc/sys/kernel/sched_downmigrate
  write_value "50 70" /proc/sys/kernel/sched_upmigrate
 else
  write_value "60" /proc/sys/kernel/sched_downmigrate
  write_value "60" /proc/sys/kernel/sched_upmigrate
 fi
 write_value "90" /proc/sys/kernel/sched_group_downmigrate
 write_value "90" /proc/sys/kernel/sched_group_upmigrate
else
 if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
  write_value "90 80" /proc/sys/kernel/sched_downmigrate
  write_value "90 95" /proc/sys/kernel/sched_upmigrate
 else
  write_value "90" /proc/sys/kernel/sched_downmigrate
  write_value "90" /proc/sys/kernel/sched_upmigrate
 fi
 write_value "110" /proc/sys/kernel/sched_group_downmigrate
 write_value "130" /proc/sys/kernel/sched_group_upmigrate
fi
#eMMC Scaling
if [ -d /sys/class/mmc_host/mmc0/clk_scaling/ ] ; then
 if [ $MODE = "performance" ] ; then
  write_value "0" /sys/class/mmc_host/mmc0/clk_scaling/enable
 elif [ $MODE = "balance" ] ; then
  write_value "1" /sys/class/mmc_host/mmc0/clk_scaling/enable
  write_value "20" /sys/class/mmc_host/mmc0/clk_scaling/up _threshold
  write_value "5" /sys/class/mmc_host/mmc0/clk_scaling/down_threshold
 else
  write_value "1" /sys/class/mmc_host/mmc0/clk_scaling/enable
  write_value "40" /sys/class/mmc_host/mmc0/clk_scaling/up _threshold
  write_value "10" /sys/class/mmc_host/mmc0/clk_scaling/down_threshold
 fi
fi
}
function gpu_adjust() {
#UNIVERSAL GPU CONTROL
if [ -d /sys/kernel/gpu/ ] ; then
 FREQ_TABLE_NUM=$(grep -o ' ' < /sys/kernel/gpu/gpu_freq_table | wc -l)
 GPU_FIRST_FREQ_INFO=$(cat /sys/kernel/gpu/gpu_freq_table | awk '{print $1}')
 GPU_SECOND_FREQ_INFO=$(cat /sys/kernel/gpu/gpu_freq_table | awk '{print $2}')
 GPU_MAX_NUM=$(($FREQ_TABLE_NUM+1))
 GPU_LAST_FREQ=$(cat /sys/class/kgsl/kgsl-3d0/devfreq/available_frequencies | awk '{print $"'$GPU_MAX_NUM'"}')
 if [ ! -n "$GPU_LAST_FREQ" ] ; then
  GPU_MAX_NUM=$(echo $FREQ_TABLE_NUM)
 fi
 if [ ${GPU_FIRST_FREQ_INFO} -gt ${GPU_SECOND_FREQ_INFO} ] ; then
  if [ $MODE = "powersave" ] ; then
   GPU_MIN_LEVEL=$(echo $GPU_MAX_NUM)
   GPU_MAX_LEVEL=$(($GPU_MAX_NUM/2-1))
  elif [ $MODE = "balance" ] ; then
   GPU_MIN_LEVEL=$(echo $GPU_MAX_NUM)
   GPU_MAX_LEVEL=1
  else
   GPU_MIN_LEVEL=1
   GPU_MAX_LEVEL=1
  fi
 else
  if [ $MODE = "powersave" ] ; then
   GPU_MIN_LEVEL=1
   GPU_MAX_LEVEL=$(($GPU_MAX_NUM/2+1))
  elif [ $MODE = "balance" ] ; then
   GPU_MIN_LEVEL=1
   GPU_MAX_LEVEL=$(echo $GPU_MAX_NUM)
  else
   GPU_MIN_LEVEL=$(echo $GPU_MAX_NUM)
   GPU_MAX_LEVEL=$(echo $GPU_MAX_NUM)
  fi
 fi
 GPU_MIN_FREQ=$(cat /sys/kernel/gpu/gpu_freq_table | awk '{print $"'$GPU_MIN_LEVEL'"}')
 GPU_MAX_FREQ=$(cat /sys/kernel/gpu/gpu_freq_table | awk '{print $"'$GPU_MAX_LEVEL'"}')
 lock_value "$GPU_MIN_FREQ" "/sys/kernel/gpu/gpu_min_clock"
 lock_value "$GPU_MAX_FREQ" "/sys/kernel/gpu/gpu_max_clock"
fi
#QUALCOMM GPU CONTROL
if [ -d /sys/class/kgsl/kgsl-3d0/ ] ; then
 lock_value "msm-adreno-tz" "/sys/class/kgsl/kgsl-3d0/devfreq/governor"
 FREQ_TABLE_NUM=$(grep -o ' ' < /sys/class/kgsl/kgsl-3d0/devfreq/available_frequencies | wc -l)
 GPU_FIRST_FREQ_INFO=$(cat /sys/class/kgsl/kgsl-3d0/devfreq/available_frequencies | awk '{print $1}')
 GPU_SECOND_FREQ_INFO=$(cat /sys/class/kgsl/kgsl-3d0/devfreq/available_frequencies | awk '{print $2}')
 GPU_MAX_NUM=$(($FREQ_TABLE_NUM+1))
 GPU_LAST_FREQ=$(cat /sys/class/kgsl/kgsl-3d0/devfreq/available_frequencies | awk '{print $"'$GPU_MAX_NUM'"}')
 if [ ! -n "$GPU_LAST_FREQ" ] ; then
  GPU_MAX_NUM=$(echo $FREQ_TABLE_NUM)
 fi
 if [ ${GPU_FIRST_FREQ_INFO} -gt ${GPU_SECOND_FREQ_INFO} ] ; then
  if [ $MODE = "powersave" ] ; then
   GPU_MIN_LEVEL=$(echo $GPU_MAX_NUM)
   GPU_MAX_LEVEL=$(($GPU_MAX_NUM/2-1))
  elif [ $MODE = "balance" ] ; then
   GPU_MIN_LEVEL=$(echo $GPU_MAX_NUM)
   GPU_MAX_LEVEL=1
  else
   GPU_MIN_LEVEL=1
   GPU_MAX_LEVEL=1
  fi
 else
  if [ $MODE = "powersave" ] ; then
   GPU_MIN_LEVEL=1
   GPU_MAX_LEVEL=$(($GPU_MAX_NUM/2+1))
  elif [ $MODE = "balance" ] ; then
   GPU_MIN_LEVEL=1
   GPU_MAX_LEVEL=$(echo $GPU_MAX_NUM)
  else
   GPU_MIN_LEVEL=$(echo $GPU_MAX_NUM)
   GPU_MAX_LEVEL=$(echo $GPU_MAX_NUM)
  fi
 fi
 GPU_MIN_FREQ=$(cat /sys/class/kgsl/kgsl-3d0/devfreq/available_frequencies | awk '{print $"'$GPU_MIN_LEVEL'"}')
 GPU_MAX_FREQ=$(cat /sys/class/kgsl/kgsl-3d0/devfreq/available_frequencies | awk '{print $"'$GPU_MAX_LEVEL'"}')
 lock_value "$GPU_MIN_FREQ" "/sys/class/kgsl/kgsl-3d0/devfreq/min_freq"
 lock_value "$GPU_MAX_FREQ" "/sys/class/kgsl/kgsl-3d0/devfreq/max_freq"
 lock_value "$GPU_MIN_FREQ" "/sys/class/kgsl/kgsl-3d0/min_clock_mhz"
 lock_value "$GPU_MAX_FREQ" "/sys/class/kgsl/kgsl-3d0/max_clock_mhz"
fi
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
#MTK Performance GPU Lock on the maxium frequency.
if [ $MODE = "performance" ] ; then
 MAX_FREQ_INFO=$(cat /proc/gpufreq/gpufreq_opp_dump | head -1 | awk '{print $4}')
 MTK_GPU_MAX_FREQ=$(echo ${MAX_FREQ_INFO%,*})
 write_value "$MTK_GPU_MAX_FREQ" "/proc/gpufreq/gpufreq_opp_freq"
else
 write_value "0" "/proc/gpufreq/gpufreq_opp_freq"
fi
#MTK Powersave underclock the GPU
if [ -d /sys/kernel/debug/ged/hal/ ] ; then
 MTK_OPP_NUM_INFO=$(cat /sys/kernel/debug/ged/hal/total_gpu_freq_level_count)
 MTK_OPP_NUM=$(($MTK_OPP_NUM_INFO-1))
 write_value "$MTK_OPP_NUM" "/sys/kernel/debug/ged/hal/custom_boost_gpu_freq"
 if [ $MODE = "powersave" ] ; then
  PWSV_OPP_IDX=$(($MTK_OPP_NUM/2))
  write_value "$PWSV_OPP_IDX" "/sys/kernel/debug/ged/hal/custom_upbound_gpu_freq"
 else
  write_value "1" "/sys/kernel/debug/ged/hal/custom_upbound_gpu_freq"
 fi
fi
#Old GPU Control,Keep it to support new GPU Control
if [ $MODE = "performance" ] ; then
 if [ -d /sys/class/kgsl/kgsl-3d0/ ] ; then
  PWR_NUM=$(cat /sys/class/kgsl/kgsl-3d0/num_pwrlevels)
  write_value "0" "/sys/class/kgsl/kgsl-3d0/max_pwrlevel"
  write_value "0" "/sys/class/kgsl/kgsl-3d0/min_pwrlevel"
  write_value "0" "/sys/class/kgsl/kgsl-3d0/default_pwrlevel"
  write_value "0" "/sys/class/kgsl/kgsl-3d0/force_clk_on"
  write_value "0" "/sys/class/kgsl/kgsl-3d0/force_bus_on"
  write_value "1" "/sys/class/kgsl/kgsl-3d0/force_no_nap"
  write_value "0" "/sys/class/kgsl/kgsl-3d0/force_rail_on"
  write_value "0" "/sys/class/kgsl/kgsl-3d0/default_pwrlevel"
  write_value "N" "/sys/module/adreno_idler/parameters/adreno_idler_active" 
 fi
 if [ -d /sys/devices/platform/mali.0/ ]; then
  write_value 40 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/vsync_upthreshold 
  write_value 20 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/vsync_downdifferential  
  write_value 30 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/no_vsync_upthreshold 
  write_value 10 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/no_vsync_downdifferential 
  write_value "1" /proc/mali/dvfs_enable
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
elif [ $MODE = "balance" ] ; then
 if [ -d /sys/class/kgsl/kgsl-3d0/ ] ; then
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
  write_value 80 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/vsync_upthreshold 
  write_value 60 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/vsync_downdifferential 
  write_value 70 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/no_vsync_upthreshold 
  write_value 50 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/no_vsync_downdifferential 
  write_value "1" /proc/mali/dvfs_enable
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
  write_value 90 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/vsync_upthreshold 
  write_value 80 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/vsync_downdifferential 
  write_value 80 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/no_vsync_upthreshold 
  write_value 70 /sys/devices/platform/mali.0/devfreq/gpufreq/mali_ondemand/no_vsync_downdifferential 
  write_value "1" /proc/mali/dvfs_enable
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
 elif [ $MODE = "balance" ] ; then
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
 lock_value "$GPU_MIN_RATE" "/sys/kernel/tegra_gpu/gpu_floor_rate"
 lock_value "$GPU_MAX_RATE" "/sys/kernel/tegra_gpu/gpu_cap_rate"
fi
}
#Start Running
main_adjust ;
gpu_adjust ;
exit 0