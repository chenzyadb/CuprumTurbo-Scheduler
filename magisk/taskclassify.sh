#!/system/bin/sh
CUPRUM_DIR=${0%/*}
CU_TASKSET=$(echo "${CUPRUM_DIR}/taskset")
chmod 7777 $CU_TASKSET ;
function bind_task_sc() {
 for pid in $(ps -eo pid,name | grep "$1" | awk '{print $1}') ; do
  echo "$pid" > "/dev/cpuset/background/tasks" &
  echo "$pid" > "/dev/stune/background/tasks" &
 done
}
function bind_task_system() {
 for pid in $(ps -eo pid,name | grep "$1" | awk '{print $1}') ; do
  echo "$pid" > "/dev/cpuset/top-app/tasks" &
  echo "$pid" > "/dev/stune/top-app/tasks" &
  if [ -d /sys/devices/system/cpu/cpu7 ] ; then
   if [ -d /sys/devices/system/cpu/cpufreq/policy4/ ] ; then
    if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
     su -c $CU_TASKSET -pc 4-6 $pid >/dev/null
    else
     su -c $CU_TASKSET -pc 4-7 $pid >/dev/null
    fi
   elif [ -d /sys/devices/system/cpu/cpufreq/policy6/ ] ; then
    su -c $CU_TASKSET -pc 6-7 $pid >/dev/null
   else
    su -c $CU_TASKSET -pc 4-7 $pid >/dev/null
   fi
  elif [ -d /sys/devices/system/cpu/cpu5 ] ; then
   su -c $CU_TASKSET -pc 4-5 $pid >/dev/null
  elif [ -d /sys/devices/system/cpu/cpu3 ] ; then
   su -c $CU_TASKSET -pc 0-3 $pid >/dev/null
  fi
 done
}
function bind_task_powersave() {
 for pid in $(ps -eo pid,name | grep "$1" | awk '{print $1}') ; do
  echo "$pid" > "/dev/cpuset/top-app/tasks" &
  echo "$pid" > "/dev/stune/top-app/tasks" &
  if [ -d /sys/devices/system/cpu/cpu7 ] ; then
   if [ -d /sys/devices/system/cpu/cpufreq/policy4/ ] ; then
    if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
     su -c $CU_TASKSET -pc 2-5 $pid >/dev/null
    else
     su -c $CU_TASKSET -pc 2-5 $pid >/dev/null
    fi
   elif [ -d /sys/devices/system/cpu/cpufreq/policy6/ ] ; then
    su -c $CU_TASKSET -pc 0-5 $pid >/dev/null
   else
    su -c $CU_TASKSET -pc 0-5 $pid >/dev/null
   fi
  elif [ -d /sys/devices/system/cpu/cpu5 ] ; then
   su -c $CU_TASKSET -pc 0-4 $pid >/dev/null
  elif [ -d /sys/devices/system/cpu/cpu3 ] ; then
   su -c $CU_TASKSET -pc 0-3 $pid >/dev/null
  fi
 done
}
function bind_task_balance() {
 for pid in $(ps -eo pid,name | grep "$1" | awk '{print $1}') ; do
  echo "$pid" > "/dev/cpuset/top-app/tasks" &
  echo "$pid" > "/dev/stune/top-app/tasks" &
  if [ -d /sys/devices/system/cpu/cpu7 ] ; then
   if [ -d /sys/devices/system/cpu/cpufreq/policy4/ ] ; then
    if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
     su -c $CU_TASKSET -pc 0-6 $pid >dev/null &
    else
     su -c $CU_TASKSET -pc 0-7 $pid >/dev/null
    fi
   elif [ -d /sys/devices/system/cpu/cpufreq/policy6/ ] ; then
    su -c $CU_TASKSET -pc 4-7 $pid >/dev/null
   else
    su -c $CU_TASKSET -pc 0-7 $pid >/dev/null
   fi
  elif [ -d /sys/devices/system/cpu/cpu5 ] ; then
   su -c $CU_TASKSET -pc 0-5 $pid >/dev/null
  elif [ -d /sys/devices/system/cpu/cpu3 ] ; then
   su -c $CU_TASKSET -pc 0-3 $pid >/dev/null
  fi
 done
}
function bind_task_performance() {
 for pid in $(ps -eo pid,name | grep "$1" | awk '{print $1}') ; do
  echo "$pid" > "/dev/cpuset/top-app/tasks" &
  echo "$pid" > "/dev/stune/top-app/tasks" &
  if [ -d /sys/devices/system/cpu/cpu7 ] ; then
   if [ -d /sys/devices/system/cpu/cpufreq/policy4/ ] ; then
    if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
     su -c $CU_TASKSET -pc 4-7 $pid >/dev/null
    else
     su -c $CU_TASKSET -pc 4-7 $pid >/dev/null
    fi
   elif [ -d /sys/devices/system/cpu/cpufreq/policy6/ ] ; then
    su -c $CU_TASKSET -pc 0-7 $pid >/dev/null
   else
    su -c $CU_TASKSET -pc 4-7 $pid >/dev/null
   fi
  elif [ -d /sys/devices/system/cpu/cpu5 ] ; then
   su -c $CU_TASKSET -pc 0-5 $pid >/dev/null
  elif [ -d /sys/devices/system/cpu/cpu3 ] ; then
   su -c $CU_TASKSET -pc 0-3 $pid >/dev/null
  fi
 done
}
function set_task_prio() {
 for pid in $(ps -eo pid,name | grep "$1" | awk '{print $1}') ; do
  renice -n $2 -p $pid 2>/dev/null
 done
}
function setlog() {
 echo "$(date +%F) $(date +%T) [$1] $2" >> "/data/Cuprum_Log.txt" ;
}
function set_cg_log() {
 LINE_NUM=$(cat /data/taskclassify_log.txt | wc -l)
 if [ ${LINE_NUM} -lt 1000 ] ; then
  echo "$(date +%F) $(date +%T) $1" >> "/data/taskclassify_log.txt" ;
 else
  echo "$(date +%F) $(date +%T) $1" > "/data/taskclassify_log.txt" ;
 fi
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
if [ ! -e /dev/cpuset/sched_relax_domain_level ] ; then
 setlog "W" "taskclassify: Kernel doesn't support cpuset.";
else
 #Process Adjust
 # su -c stop "logd"  **Some APP need it. 
 bind_task_system "surfaceflinger"
 set_task_prio "surfaceflinger" "-8"
 bind_task_system "system_server" 
 set_task_prio "system_server" "-4"
 bind_task_system "com.android.systemui"
 set_task_prio "com.android.systemui" "-4"
 bind_task_system "com.android.bluetooth"
 bind_task_system "android.hardware.wifi"
 bind_task_system "android.hardware.bluetooth"
 bind_task_system "android.hardware.usb"
 bind_task_system "android.hardware.keymaster"
 bind_task_system "android.hardware.audio"
 bind_task_system "android.hardware.camera"
 bind_task_system "audioserver"
 bind_task_system "netd"
 bind_task_sc "logd"
 bind_task_sc "lmkd"
 bind_task_sc "com.android.nfc"
 bind_task_sc "android.hardware.gnss"
 bind_task_sc "android.hardware.health"
 bind_task_sc "android.hardware.thermal"
 bind_task_sc "android.hardware.nfc"
 bind_task_sc "android.hardware.vibrator"
 bind_task_sc "android.hardware.light"
 bind_task_sc "android.hardware.boot"
 bind_task_sc "hardware.sensors"
 bind_task_sc "sensorservice"
 bind_task_sc "vendor.mediatek.hardware.pq"
 setlog "I" "taskclassify: System Process bind success."
 echo "$(date +%F) $(date +%T)" > "/data/taskclassify_log.txt" ;
 FOREGROUND_APP="null"
 LAST_FOREGROUND_APP="null"
 TASK_CLASSIFYED_APP="null"
 TASK_CLASSIFYED_MODE="null"
 setlog "I" "taskclassify: Service Running."
 while true ; do
  MODE=$(cat /data/Cuprum_Costom/mode)
  LAST_FOREGROUND_APP=$(echo $FOREGROUND_APP)
  FOREGROUND_APP_INFO=$(su -c dumpsys window | grep "mCurrentFocus" | awk '{print $3}' )
  FOREGROUND_APP=$(echo ${FOREGROUND_APP_INFO%/*})
  if [ ! -d /data/data/$FOREGROUND_APP/ ]||[ ! -n "$FOREGROUND_APP" ] ; then
   FOREGROUND_APP_INFO=$(su -c dumpsys activity window | grep "mCurrentFocus" | awk '{print $5}' )
   FOREGROUND_APP=$(echo ${FOREGROUND_APP_INFO%/*})
   if [ ! -d /data/data/$FOREGROUND_APP/ ]||[ ! -n "$FOREGROUND_APP" ] ; then
    FOREGROUND_APP_INFO=$(su -c dumpsys activity activities | grep "mCurrentFocus" | awk '{print $3}' )
    FOREGROUND_APP=$(echo ${FOREGROUND_APP_INFO%/*})
    if [ ! -d /data/data/$FOREGROUND_APP/ ]||[ ! -n "$FOREGROUND_APP" ] ; then
     FOREGROUND_APP_INFO=$(su -c dumpsys activity activities | grep "mCurrentFocus" | awk '{print $5}' )
     FOREGROUND_APP=$(echo ${FOREGROUND_APP_INFO%/*})
     if [ ! -d /data/data/$FOREGROUND_APP/ ]||[ ! -n "$FOREGROUND_APP" ] ; then
      FOREGROUND_APP=$(echo $LAST_FOREGROUND_APP)
     fi
    fi
   fi
  fi
  if [ "$FOREGROUND_APP" != "$TASK_CLASSIFYED_APP" ]||[ "$MODE" != "$TASK_CLASSIFYED_MODE" ] ; then
   bind_task_sc "$TASK_CLASSIFYED_APP"
   set_task_prio "$TASK_CLASSIFYED_APP" "5"
   # set_cg_log "${TASK_CLASSIFYED_APP} bind to small core."
   if [ $MODE = "performance" ] ; then
    bind_task_performance "$FOREGROUND_APP"
    set_task_prio "$FOREGROUND_APP" "-12"
    # set_cg_log "${FOREGROUND_APP} bind to big core."
   elif [ $MODE = "balance" ] ; then
    bind_task_balance "$FOREGROUND_APP"
    set_task_prio "$FOREGROUND_APP" "-8"
    # set_cg_log "${FOREGROUND_APP} bind to big core."
   else 
    bind_task_powersave "$FOREGROUND_APP"
    set_task_prio "$FOREGROUND_APP" "-4"
    # set_cg_log "${FOREGROUND_APP} bind to big core."
   fi
   TASK_CLASSIFYED_APP=$(echo $FOREGROUND_APP)
   TASK_CLASSIFYED_MODE=$(echo $MODE)
  fi
  sleep 5
 done
fi
exit 0