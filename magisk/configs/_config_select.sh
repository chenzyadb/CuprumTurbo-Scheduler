#!/system/bin/sh

get_pineapple_name() {
    cpu_max_freq=$(cat /sys/devices/system/cpu/cpufreq/policy7/cpuinfo_max_freq)
    if [ "$cpu_max_freq" -gt 3200000 ]; then
        echo "sdm8gen3"
    else
        echo "sdm7+gen3"
    fi
}

get_taro_name() {
    cpu7_max_freq=$(cat /sys/devices/system/cpu/cpufreq/policy7/cpuinfo_max_freq)
    gpu_max_freq=$(cat /sys/class/kgsl/kgsl-3d0/max_clock_mhz)
    if [ "$cpu7_max_freq" -gt 3100000 ]; then
        echo "sdm8+gen1"
    elif [ "$cpu7_max_freq" -gt 2950000 ]; then
        if [ "$gpu_max_freq" -gt 850 ]; then
            echo "sdm8+gen1"
        else
            echo "sdm8gen1"
        fi
    elif [ "$cpu7_max_freq" -gt 2900000 ]; then
        echo "sdm7+gen2"
    else
        echo "sdm7gen1"
    fi
}

get_lahaina_name() {
    cpu7_max_freq=$(cat /sys/devices/system/cpu/cpufreq/policy7/cpuinfo_max_freq)
    cpu4_max_freq=$(cat /sys/devices/system/cpu/cpufreq/policy4/cpuinfo_max_freq)
    if [ "$cpu7_max_freq" -gt 2800000 ]; then
        echo "sdm888"
    elif [ "$cpu4_max_freq" -gt 2300000 ]; then
        echo "sdm778"
    else
        echo "sdm780"
    fi
}

get_lito_name() {
    cpu_max_freq=$(cat /sys/devices/system/cpu/cpufreq/policy7/cpuinfo_max_freq)
    if [ "$cpu_max_freq" -gt 2300000 ]; then
        echo "sdm765"
    else
        echo "sdm750"
    fi
}

get_sm6150_name() {
    cpu_max_freq=$(cat /sys/devices/system/cpu/cpufreq/policy7/cpuinfo_max_freq)
    if [ "$cpu_max_freq" -gt 2200000 ]; then
        echo "sdm730"
    else
        echo "sdm675"
    fi
}

get_mt6895_name() {
    cpu_max_freq=$(cat /sys/devices/system/cpu/cpufreq/policy7/cpuinfo_max_freq)
    if [ "$cpu_max_freq" -gt 3000000 ]; then
        echo "dimensity8200"
    else
        echo "dimensity8100"
    fi
}

get_bengal_name() {
    cpu_max_freq=$(cat /sys/devices/system/cpu/cpufreq/policy7/cpuinfo_max_freq)
    if [ "$cpu_max_freq" -gt 2300000 ]; then
        echo "sdm680"
    else
        echo "sdm665"
    fi
}

get_config_name() {
    case "$1" in
    crow*)
        echo "sdm7gen3"
        ;;
    garnet*)
        echo "sdm6gen1"
        ;;
    parrot*)
        echo "sdm6gen1"
        ;;
    pineapple*)
        get_pineapple_name
        ;;
    sunstone*)
        echo "sdm4gen1"
        ;;
    sky*)
        echo "sdm4gen2"
        ;;
    kalama*)
        echo "sdm8gen2"
        ;;
    taro*)
        get_taro_name
        ;;
    lahaina*)
        get_lahaina_name
        ;;
    shima*)
        get_lahaina_name
        ;;
    yupik*)
        get_lahaina_name
        ;;
    kona*)
        echo "sdm865"
        ;;
    msmnile*)
        echo "sdm855"
        ;;
    sdm845*)
        echo "sdm845"
        ;;
    lito*)
        get_lito_name
        ;;
    sm7150*)
        echo "sdm730"
        ;;
    sm6150*)
        get_sm6150_name
        ;;
    sdm670*)
        echo "sdm710"
        ;;
    sdm710*)
        echo "sdm710"
        ;;
    sdm439*)
        echo "sdm439"
        ;;
    sdm450*)
        echo "sdm625"
        ;;
    sdm4350*)
        echo "sdm730"
        ;;
    msm8953*)
        echo "sdm625"
        ;;
    sdm660*)
        echo "sdm660"
        ;;
    sdm636*)
        echo "sdm660"
        ;;
    sdm632*)
        echo "sdm660"
        ;;
    sdm630*)
        echo "sdm630"
        ;;
    trinket*)
        echo "sdm665"
        ;;
    bengal*)
        get_bengal_name
        ;;
    holi*)
        echo "sdm4gen1"
        ;;
    msm8998*)
        echo "sdm835"
        ;;
    msm8996*)
        echo "sdm820"
        ;;
    mt6771*)
        echo "helio_p60"
        ;;
    mt6779*)
        echo "helio_g80"
        ;;
    mt6762*)
        echo "helio_p35"
        ;;
    mt6765*)
        echo "helio_p35"
        ;;
    mt6768*)
        echo "helio_g80"
        ;;
    mt6785*)
        echo "helio_g90"
        ;;
    mt6789*)
        echo "helio_g99"
        ;;
    mt6833*)
        echo "dimensity700"
        ;;
    mt6835*)
        echo "helio_g99"
        ;;
    mt6853*)
        echo "dimensity700"
        ;;
    mt6873*)
        echo "dimensity820"
        ;;
    mt6875*)
        echo "dimensity820"
        ;;
    mt6877*)
        echo "dimensity900"
        ;;
    mt6885*)
        echo "dimensity1000"
        ;;
    mt6886*)
        echo "dimensity7200"
        ;;
    mt6889*)
        echo "dimensity1000"
        ;;
    mt6891*)
        echo "dimensity1100"
        ;;
    mt6893*)
        echo "dimensity1100"
        ;;
    mt6895*)
        get_mt6895_name
        ;;
    mt6983*)
        echo "dimensity9000"
        ;;
    mt6985*)
        echo "dimensity9200"
        ;;
    mt6989*)
        echo "dimensity9300"
        ;;
    kirin970*)
        echo "kirin970"
        ;;
    hi3670*)
        echo "kirin970"
        ;;
    hi3660*)
        echo "kirin960"
        ;;
    hi3650*)
        echo "kirin950"
        ;;
    kirin710*)
        echo "kirin710"
        ;;
    hi6250*)
        echo "kirin650"
        ;;
    sp9863a*)
        echo "sc9863a"
        ;;
    ums512*)
        echo "unisoc_t618"
        ;;
    ud710*)
        echo "unisoc_t740"
        ;;
    ums9620*)
        echo "unisoc_t770"
        ;;
    ums9230*)
        echo "unisoc_t618"
        ;;
    *)
        echo "universal"
        ;;
    esac
}

platform_name=$(getprop "ro.board.platform")
config_name=$(get_config_name "$platform_name")

echo "$config_name"
