#!/system/bin/sh
set +e
set +u
set +x
CUPRUM_DIR=${0%/*}
cd $CUPRUM_DIR ;
chmod -R 7777 $CUPRUM_DIR
if [ ! -d /data/Cuprum_Custom/ ] ; then
 mkdir -p /data/Cuprum_Custom/
 echo "balance" > /data/Cuprum_Custom/mode
fi
echo "--- CuprumTurbo Adjustment V3" > "/data/Cuprum_Log.txt"
echo "--- Author: Chenzyadb@Coolapk" >> "/data/Cuprum_Log.txt"
echo "--- $(date) Log Start." >> "/data/Cuprum_Log.txt"
echo ": CuLoader Start." >> "/data/Cuprum_Log.txt"
if [ -e /data/Cuprum_Custom/run_success ] ; then
 LAST_RUN_SUCCESS=$(cat /data/Cuprum_Custom/run_success)
else
 LAST_RUN_SUCCESS="1"
fi
if [ $LAST_RUN_SUCCESS = "1" ] ; then
 echo "0" > /data/Cuprum_Custom/run_success
else
 echo "[CuprumTurbo SafetyMode] Last Load may cause system crashed." >> "/data/Cuprum_Log.txt"
 echo "Please upload the log to author and reflash module to exit SafetyMode." >> "/data/Cuprum_Log.txt"
 true > /data/adb/modules/cuprumai/disable
 exit 0
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
function get_bc_mask() {
 CPU_NUM=$1
 if [ -e /sys/devices/system/cpu/cpu${CPU_NUM}/cpufreq/cpuinfo_max_freq ] ; then
  CPU_FREQ=$(cat /sys/devices/system/cpu/cpu${CPU_NUM}/cpufreq/cpuinfo_max_freq)
  CPU_CLK_MHZ=$(($CPU_FREQ/1000))
  CPU_MASK=$((5000000/${CPU_CLK_MHZ}))
  echo "${CPU_MASK}" 
 fi
}
function get_sc_mask() {
 CPU_NUM=$1
 if [ -e /sys/devices/system/cpu/cpu${CPU_NUM}/cpufreq/cpuinfo_max_freq ] ; then
  CPU_FREQ=$(cat /sys/devices/system/cpu/cpu${CPU_NUM}/cpufreq/cpuinfo_max_freq)
  CPU_CLK_MHZ=$(($CPU_FREQ/1000))
  CPU_MASK=$((3200000/${CPU_CLK_MHZ}))
  echo "${CPU_MASK}" 
 fi
}
function get_cpufreq_tables() {
#Stop Thermal
write_value 0 /sys/module/msm_thermal/vdd_restriction/enabled
write_value "N" /sys/module/msm_thermal/parameters/enabled
write_value 0 /sys/module/msm_thermal/core_control/enabled
#Stop MTK Perfservices
if [ -e /proc/hps/enabled ] ; then
 write_value 0 "/proc/hps/enabled"
fi
if [ -e /proc/ppm/enabled ] ; then
 write_value "0" "/proc/ppm/enabled"
fi
#Bring Cores online
if [ -d /sys/devices/system/cpu/cpu7/ ] ; then
 core_num="7"
elif [ -d /sys/devices/system/cpu/cpu5/ ] ; then
 core_num="5"
elif [ -d /sys/devices/system/cpu/cpu3/ ] ; then
 core_num="3"
fi
#Bring ALL Cores Online
 for i in $(seq 0 7) ; do
  if [ -e /sys/devices/system/cpu/cpu${i}/online ] ; then
   lock_value 1 /sys/devices/system/cpu/cpu${i}/online 
  fi
 done
#get CPU Freq
for tables in $(seq 0 10) ; do
 for i in 0 2 4 6 7 ; do
  if [ -e /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_available_frequencies ] ; then
   AVAILABLE_FREQS=$(cat /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_available_frequencies)
   FREQ_TABLE_NUM=$(grep -o ' ' < /sys/devices/system/cpu/cpu${i}/cpufreq/scaling_available_frequencies | wc -l)
   MIN_DIFF_VALUE=4000000
   CPU_INFO_MAX_FREQ=$(cat /sys/devices/system/cpu/cpu${i}/cpufreq/cpuinfo_max_freq)
   TARGET_FREQ_STEP=$(($CPU_INFO_MAX_FREQ/9))
   NOW_TARGET_FREQ=$(($TARGET_FREQ_STEP*$tables))
   for j in $(seq 1 $FREQ_TABLE_NUM) ; do
    GET_FREQ=$(echo $AVAILABLE_FREQS | awk '{print $"'$j'"}')
    DIFF_FREQ_VALUE=$(($GET_FREQ-$NOW_TARGET_FREQ))
    if [ ${DIFF_FREQ_VALUE} -lt ${MIN_DIFF_VALUE} ]&&[ ${DIFF_FREQ_VALUE} -gt 0 ] ; then
     SYS_PROV_FREQ=$(echo $GET_FREQ)
     MIN_DIFF_VALUE=$(echo $DIFF_FREQ_VALUE)
    fi
   done
   if [ ! -d /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ] ; then
    mkdir -p /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ;
   fi
   echo "$SYS_PROV_FREQ" > /data/Cuprum_Custom/cpufreq_tables/level${tables}/cpu${i}_freq ;
  fi
 done 
done
for tables in $(seq 0 10) ; do
 #Samsung Exynos Freq Table
 if [ -e /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster0_freq_table ] ; then
  if [ -e /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster2_freq_table ] ; then
    #Cluster0
    AVAILABLE_FREQS=$(cat /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster0_freq_table)
    FREQ_TABLE_NUM=$(grep -o ' ' < /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster0_freq_table | wc -l)
    MIN_DIFF_VALUE=4000000
   CPU_INFO_MAX_FREQ=$(cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq)
   TARGET_FREQ_STEP=$(($CPU_INFO_MAX_FREQ/9))
   NOW_TARGET_FREQ=$(($TARGET_FREQ_STEP*$tables))
   for j in $(seq 1 $FREQ_TABLE_NUM) ; do
    GET_FREQ=$(echo $AVAILABLE_FREQS | awk '{print $"'$j'"}')
    DIFF_FREQ_VALUE=$(($GET_FREQ-$NOW_TARGET_FREQ))
    if [ ${DIFF_FREQ_VALUE} -lt ${MIN_DIFF_VALUE} ]&&[ ${DIFF_FREQ_VALUE} -gt 0 ] ; then
     SYS_PROV_FREQ=$(echo $GET_FREQ)
     MIN_DIFF_VALUE=$(echo $DIFF_FREQ_VALUE)
    fi
   done
   if [ ! -d /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ] ; then
    mkdir -p /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ;
   fi
   echo "$SYS_PROV_FREQ" > /data/Cuprum_Custom/cpufreq_tables/level${tables}/cpu0_freq ;
    #Cluster1
    AVAILABLE_FREQS=$(cat /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster1_freq_table)
    FREQ_TABLE_NUM=$(grep -o ' ' < /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster1_freq_table | wc -l)
    MIN_DIFF_VALUE=4000000
   CPU_INFO_MAX_FREQ=$(cat /sys/devices/system/cpu/cpu4/cpufreq/cpuinfo_max_freq)
   TARGET_FREQ_STEP=$(($CPU_INFO_MAX_FREQ/9))
   NOW_TARGET_FREQ=$(($TARGET_FREQ_STEP*$tables))
   for j in $(seq 1 $FREQ_TABLE_NUM) ; do
    GET_FREQ=$(echo $AVAILABLE_FREQS | awk '{print $"'$j'"}')
    DIFF_FREQ_VALUE=$(($GET_FREQ-$NOW_TARGET_FREQ))
    if [ ${DIFF_FREQ_VALUE} -lt ${MIN_DIFF_VALUE} ]&&[ ${DIFF_FREQ_VALUE} -gt 0 ] ; then
     SYS_PROV_FREQ=$(echo $GET_FREQ)
     MIN_DIFF_VALUE=$(echo $DIFF_FREQ_VALUE)
    fi
   done
   if [ ! -d /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ] ; then
    mkdir -p /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ;
   fi
   echo "$SYS_PROV_FREQ" > /data/Cuprum_Custom/cpufreq_tables/level${tables}/cpu4_freq ;
    if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
     #Cluster2
     AVAILABLE_FREQS=$(cat /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster2_freq_table)
     FREQ_TABLE_NUM=$(grep -o ' ' < /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster2_freq_table | wc -l)
     MIN_DIFF_VALUE=4000000
     CPU_INFO_MAX_FREQ=$(cat /sys/devices/system/cpu/cpu7/cpufreq/cpuinfo_max_freq)
     TARGET_FREQ_STEP=$(($CPU_INFO_MAX_FREQ/9))
     NOW_TARGET_FREQ=$(($TARGET_FREQ_STEP*$tables))
     for j in $(seq 1 $FREQ_TABLE_NUM) ; do
      GET_FREQ=$(echo $AVAILABLE_FREQS | awk '{print $"'$j'"}')
      DIFF_FREQ_VALUE=$(($GET_FREQ-$NOW_TARGET_FREQ))
      if [ ${DIFF_FREQ_VALUE} -lt ${MIN_DIFF_VALUE} ]&&[ ${DIFF_FREQ_VALUE} -gt 0 ] ; then
       SYS_PROV_FREQ=$(echo $GET_FREQ)
       MIN_DIFF_VALUE=$(echo $DIFF_FREQ_VALUE)
      fi
     done
     if [ ! -d /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ] ; then
      mkdir -p /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ;
     fi
     echo "$SYS_PROV_FREQ" > /data/Cuprum_Custom/cpufreq_tables/level${tables}/cpu7_freq ;
    elif [ -d /sys/devices/system/cpu/cpufreq/policy6/ ] ; then
     #Cluster2
     AVAILABLE_FREQS=$(cat /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster2_freq_table)
     FREQ_TABLE_NUM=$(grep -o ' ' < /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster2_freq_table | wc -l)
     MIN_DIFF_VALUE=4000000
     CPU_INFO_MAX_FREQ=$(cat /sys/devices/system/cpu/cpu6/cpufreq/cpuinfo_max_freq)
     TARGET_FREQ_STEP=$(($CPU_INFO_MAX_FREQ/9))
     NOW_TARGET_FREQ=$(($TARGET_FREQ_STEP*$tables))
     for j in $(seq 1 $FREQ_TABLE_NUM) ; do
      GET_FREQ=$(echo $AVAILABLE_FREQS | awk '{print $"'$j'"}')
      DIFF_FREQ_VALUE=$(($GET_FREQ-$NOW_TARGET_FREQ))
      if [ ${DIFF_FREQ_VALUE} -lt ${MIN_DIFF_VALUE} ]&&[ ${DIFF_FREQ_VALUE} -gt 0 ] ; then
       SYS_PROV_FREQ=$(echo $GET_FREQ)
       MIN_DIFF_VALUE=$(echo $DIFF_FREQ_VALUE)
      fi
     done
     if [ ! -d /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ] ; then
      mkdir -p /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ;
     fi
     echo "$SYS_PROV_FREQ" > /data/Cuprum_Custom/cpufreq_tables/level${tables}/cpu6_freq ;
    fi
  else
   #Cluster0
    AVAILABLE_FREQS=$(cat /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster0_freq_table)
    FREQ_TABLE_NUM=$(grep -o ' ' < /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster0_freq_table | wc -l)
    MIN_DIFF_VALUE=4000000
    CPU_INFO_MAX_FREQ=$(cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq)
    TARGET_FREQ_STEP=$(($CPU_INFO_MAX_FREQ/9))
    NOW_TARGET_FREQ=$(($TARGET_FREQ_STEP*$tables))
    for j in $(seq 1 $FREQ_TABLE_NUM) ; do
     GET_FREQ=$(echo $AVAILABLE_FREQS | awk '{print $"'$j'"}')
     DIFF_FREQ_VALUE=$(($GET_FREQ-$NOW_TARGET_FREQ))
     if [ ${DIFF_FREQ_VALUE} -lt ${MIN_DIFF_VALUE} ]&&[ ${DIFF_FREQ_VALUE} -gt 0 ] ; then
      SYS_PROV_FREQ=$(echo $GET_FREQ)
      MIN_DIFF_VALUE=$(echo $DIFF_FREQ_VALUE)
     fi
    done
    if [ ! -d /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ] ; then
     mkdir -p /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ;
    fi
    echo "$SYS_PROV_FREQ" > /data/Cuprum_Custom/cpufreq_tables/level${tables}/cpu0_freq ;
    #Cluster1
    AVAILABLE_FREQS=$(cat /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster1_freq_table)
    FREQ_TABLE_NUM=$(grep -o ' ' < /sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster1_freq_table | wc -l)
    MIN_DIFF_VALUE=4000000
    CPU_INFO_MAX_FREQ=$(cat /sys/devices/system/cpu/cpu4/cpufreq/cpuinfo_max_freq)
    TARGET_FREQ_STEP=$(($CPU_INFO_MAX_FREQ/9))
    NOW_TARGET_FREQ=$(($TARGET_FREQ_STEP*$tables))
    for j in $(seq 1 $FREQ_TABLE_NUM) ; do
     GET_FREQ=$(echo $AVAILABLE_FREQS | awk '{print $"'$j'"}')
     DIFF_FREQ_VALUE=$(($GET_FREQ-$NOW_TARGET_FREQ))
     if [ ${DIFF_FREQ_VALUE} -lt ${MIN_DIFF_VALUE} ]&&[ ${DIFF_FREQ_VALUE} -gt 0 ] ; then
      SYS_PROV_FREQ=$(echo $GET_FREQ)
      MIN_DIFF_VALUE=$(echo $DIFF_FREQ_VALUE)
     fi
    done
    if [ ! -d /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ] ; then
     mkdir -p /data/Cuprum_Custom/cpufreq_tables/level${tables}/ ;
    fi
    echo "$SYS_PROV_FREQ" > /data/Cuprum_Custom/cpufreq_tables/level${tables}/cpu4_freq ;
  fi
 fi
done
chmod -R 0444 /data/Cuprum_Custom/cpufreq_tables/
if [ -e /proc/ppm/enabled ] ; then
 write_value "1" "/proc/ppm/enabled"
fi
}
#Get CPU Freq Tables
if [ ! -d /data/Cuprum_Custom/cpufreq_tables/ ] ; then
 setlog "I" "CuLoader: Create new CPU freq tables."
 get_cpufreq_tables;
else
 setlog "I" "CuLoader: Load last CPU freq tables."
fi
if [ -d /sys/devices/system/cpu/cpu7/ ] ; then
 core_num="7"
elif [ -d /sys/devices/system/cpu/cpu5/ ] ; then
 core_num="5"
elif [ -d /sys/devices/system/cpu/cpu3/ ] ; then
 core_num="3"
fi
#Get CPU Perf Mask
if [ $core_num = 7 ] ; then
 cpu_max_freq0=$(cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq)
 cpu_max_freq1=$(cat /sys/devices/system/cpu/cpu1/cpufreq/cpuinfo_max_freq)
 cpu_max_freq2=$(cat /sys/devices/system/cpu/cpu2/cpufreq/cpuinfo_max_freq)
 cpu_max_freq3=$(cat /sys/devices/system/cpu/cpu3/cpufreq/cpuinfo_max_freq)
 cpu_max_freq4=$(cat /sys/devices/system/cpu/cpu4/cpufreq/cpuinfo_max_freq)
 cpu_max_freq5=$(cat /sys/devices/system/cpu/cpu5/cpufreq/cpuinfo_max_freq)
 cpu_max_freq6=$(cat /sys/devices/system/cpu/cpu6/cpufreq/cpuinfo_max_freq)
 cpu_max_freq7=$(cat /sys/devices/system/cpu/cpu7/cpufreq/cpuinfo_max_freq)
elif [ $core_num = 5 ] ; then
 cpu_max_freq0=$(cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq)
 cpu_max_freq1=$(cat /sys/devices/system/cpu/cpu1/cpufreq/cpuinfo_max_freq)
 cpu_max_freq2=$(cat /sys/devices/system/cpu/cpu2/cpufreq/cpuinfo_max_freq)
 cpu_max_freq3=$(cat /sys/devices/system/cpu/cpu3/cpufreq/cpuinfo_max_freq)
 cpu_max_freq4=$(cat /sys/devices/system/cpu/cpu4/cpufreq/cpuinfo_max_freq)
 cpu_max_freq5=$(cat /sys/devices/system/cpu/cpu5/cpufreq/cpuinfo_max_freq)
else
 cpu_max_freq0=$(cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq)
 cpu_max_freq1=$(cat /sys/devices/system/cpu/cpu1/cpufreq/cpuinfo_max_freq)
 cpu_max_freq2=$(cat /sys/devices/system/cpu/cpu2/cpufreq/cpuinfo_max_freq)
 cpu_max_freq3=$(cat /sys/devices/system/cpu/cpu3/cpufreq/cpuinfo_max_freq)
fi
#CPUMASK: The IMPORTANCE of each CPU Core.(Not the PERFORMANCE of each core.
if [ ! -e /data/Cuprum_Custom/cpu_perf_mask ] ; then
 if [ $core_num = 7 ] ; then
  if [ "$cpu_max_freq3" != "$cpu_max_freq4" ] ; then
   echo "$(get_sc_mask 0) $(get_sc_mask 1) $(get_sc_mask 2) $(get_sc_mask 3) $(get_bc_mask 4) $(get_bc_mask 5) $(get_bc_mask 6) $(get_bc_mask 7)" > /data/Cuprum_Custom/cpu_perf_mask
  elif [ "$cpu_max_freq0" = "$cpu_max_freq7" ] ; then
   echo "$(get_bc_mask 0) $(get_bc_mask 1) $(get_bc_mask 2) $(get_bc_mask 3) $(get_bc_mask 4) $(get_bc_mask 5) $(get_bc_mask 6) $(get_bc_mask 7) " > /data/Cuprum_Custom/cpu_perf_mask
  fi
 elif [ $core_num = 5 ] ; then
  echo "$(get_sc_mask 0) $(get_sc_mask 1) $(get_sc_mask 2) $(get_sc_mask 3) $(get_bc_mask 4) $(get_bc_mask 6) 0 0" > /data/Cuprum_Custom/cpu_perf_mask
 elif [ $core_num = 3 ] ; then
  if [ "$cpu_max_freq1" != "$cpu_max_freq2" ] ; then
   echo "$(get_bc_mask 0) $(get_bc_mask 1) $(get_bc_mask 2) $(get_bc_mask 3) 0 0 0 0" > /data/Cuprum_Custom/cpu_perf_mask
  else
   echo "$(get_bc_mask 0) $(get_bc_mask 1) $(get_bc_mask 2) $(get_bc_mask 3) 0 0 0 0" > /data/Cuprum_Custom/cpu_perf_mask
  fi
 fi
 if [ -d /sys/devices/system/cpu/cpufreq/policy4/ ] ; then
  if [ -d /sys/devices/system/cpu/cpufreq/policy6/ ] ; then
   echo "$(get_sc_mask 0) $(get_sc_mask 1) $(get_sc_mask 2) $(get_sc_mask 3) $(get_bc_mask 4) $(get_bc_mask 5) $(get_bc_mask 6) $(get_bc_mask 7)" > /data/Cuprum_Custom/cpu_perf_mask
  elif [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
   echo "$(get_sc_mask 0) $(get_sc_mask 1) $(get_sc_mask 2) $(get_sc_mask 3) $(get_bc_mask 4) $(get_bc_mask 5) $(get_bc_mask 6) $(get_bc_mask 7)" > /data/Cuprum_Custom/cpu_perf_mask
  else
   echo "$(get_sc_mask 0) $(get_sc_mask 1) $(get_sc_mask 2) $(get_sc_mask 3) $(get_bc_mask 4) $(get_bc_mask 5) $(get_bc_mask 6) $(get_bc_mask 7)" > /data/Cuprum_Custom/cpu_perf_mask
  fi
 elif [ -d /sys/devices/system/cpu/cpufreq/policy6/ ] ; then
  if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
   echo "$(get_sc_mask 0) $(get_sc_mask 1) $(get_sc_mask 2) $(get_sc_mask 3) $(get_sc_mask 4) $(get_sc_mask 5) $(get_bc_mask 6) $(get_bc_mask 7)" > /data/Cuprum_Custom/cpu_perf_mask
  else
   echo "$(get_sc_mask 0) $(get_sc_mask 1) $(get_sc_mask 2) $(get_sc_mask 3) $(get_sc_mask 4) $(get_sc_mask 5) $(get_bc_mask 6) $(get_bc_mask 7)" > /data/Cuprum_Custom/cpu_perf_mask
  fi
 fi
fi
#Wait for first unlock screen
setlog "I" "CuLoader: Wait for /data/media unlock."
while [ ! -e /data/media/0/.cu_test ] ; do
 echo "this is a test file" > /data/media/0/.cu_test 
 sleep 1 
done
rm -rf /data/media/0/.cu_test
setlog "I" "CuLoader: /data/media unlocked."
#Disable Boosts & Hotplugs
su -c stop "perfd" 2>/dev/null & 
su -c stop "perf-hal-1-0" 2>/dev/null &
su -c stop "mpdecision" 2>/dev/null &
su -c stop "vendor.qti.hardware.perf@1.0-service" 2>/dev/null & 
su -c stop "vendor.qti.hardware.perf@2.0-service" 2>/dev/null & 
su -c stop "vendor.perfservice" 2>/dev/null &
su -c stop "oneplus_brain_service" 2>/dev/null &
for bclmode in /sys/devices/soc.0/qcom,bcl.*/mode ; do
 lock_value "0" "$bclmode" ;
done
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
lock_value "0" /sys/class/usb_boost/cpu_core/enable
lock_value "0" /sys/class/usb_boost/cpu_freq/enable
lock_value "0" /sys/class/usb_boost/dram_vcore/enable
lock_value "0" /sys/kernel/debug/fpsgo/common/force_onoff
lock_value "0" /sys/module/mt_hotplug_mechanism/parameters/g_enable
lock_value "1" /sys/devices/system/cpu/cpufreq/hotplug/cpu_hotplug_disable
lock_value "0" /proc/hps/enabled
lock_value "1" /proc/ppm/enabled
if [ -e /proc/ppm/policy_status ] ; then 
 POLICY_IDX_INFO=$(cat /proc/ppm/policy_status | grep "PPM_POLICY_HARD_USER_LIMIT") 
 IDX_NUM=$(echo ${POLICY_IDX_INFO:1:1})
 for ppm_policy in $(seq 0 10) ; do
  write_value "$ppm_policy 0" "/proc/ppm/policy_status"
 done
 write_value "$IDX_NUM 1" "/proc/ppm/policy_status"
fi
if [ $core_num = "7" ] ; then
 lock_value "0-3" /dev/cpuset/background/cpus 
 if [ -d /sys/devices/system/cpu/cpufreq/policy6/ ] ; then
  lock_value "0-5" /dev/cpuset/system-background/cpus 
 else
  lock_value "0-3" /dev/cpuset/system-background/cpus 
 fi
 if [ -d /sys/devices/system/cpu/cpufreq/policy7/ ] ; then
  lock_value "0-6" /dev/cpuset/foreground/cpus 
 else
  lock_value "0-7" /dev/cpuset/foreground/cpus 
 fi
 lock_value "0-7" /dev/cpuset/top-app/cpus 
elif [ $core_num = "5" ] ; then
 lock_value "0-3" /dev/cpuset/background/cpus 
 lock_value "0-3" /dev/cpuset/system-background/cpus 
 lock_value "0-5" /dev/cpuset/foreground/cpus 
 lock_value "0-5" /dev/cpuset/top-app/cpus 
elif [ $core_num = "3" ] ; then
 lock_value "0-1" /dev/cpuset/background/cpus 
 lock_value "0-1" /dev/cpuset/system-background/cpus 
 lock_value "0-3" /dev/cpuset/foreground/cpus 
 lock_value "0-3" /dev/cpuset/top-app/cpus 
fi
setlog "I" "CuLoader: Kernel & Userspace boost & Hotplug disabled."
#Run CuAware
setlog "I" "CuLoader: Run CuAware Service."
nohup $CUPRUM_DIR/frame_analysis 2>/dev/null &
nohup $CUPRUM_DIR/CuAware 2>/dev/null &
sleep 2
#Run taskclassify
setlog "I" "CuLoader: Run Taskclassify Service."
nohup $CUPRUM_DIR/taskclassify 2>/dev/null &
sleep 30
echo "1" > /data/Cuprum_Custom/run_success;
exit 0
