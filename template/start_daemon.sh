#!/system/bin/sh
SCRIPT_PATH=$(readlink -f "$0")
DIR_PATH=$(dirname "$SCRIPT_PATH")

get_pineapple_name() {
    gpu_model=$(cat /sys/class/kgsl/kgsl-3d0/gpu_model)
    case "$gpu_model" in
        Adreno750*)
            echo "sdm8gen3"
            ;;
        Adreno735*)
            echo "sdm7+gen3"
            ;;
        Adreno732*)
            echo "sdm7+gen3"
            ;;
    esac
}

get_taro_name() {
    gpu_model=$(cat /sys/class/kgsl/kgsl-3d0/gpu_model)
    case "$gpu_model" in
        Adreno730*)
            gpu_max_freq=$(cat /sys/class/kgsl/kgsl-3d0/max_clock_mhz)
            if [ "$gpu_max_freq" -gt 850 ]; then
                echo "sdm8+gen1"
            else
                echo "sdm8gen1"
            fi
            ;;
        Adreno725*)
            echo "sdm7+gen2"
            ;;
        Adreno644*)
            echo "sdm7gen1"
            ;;
    esac
}

get_lahaina_name() {
    gpu_model=$(cat /sys/class/kgsl/kgsl-3d0/gpu_model)
    case "$gpu_model" in
        Adreno660*)
            echo "sdm888"
            ;;
        Adreno642*)
            echo "sdm780"
            ;;
        Adreno642L*)
            echo "sdm778"
            ;;
    esac
}

get_lito_name() {
    gpu_model=$(cat /sys/class/kgsl/kgsl-3d0/gpu_model)
    case "$gpu_model" in
        Adreno620*)
            echo "sdm765"
            ;;
        Adreno619*)
            echo "sdm750"
            ;;
    esac
}

get_sm6150_name() {
    gpu_model=$(cat /sys/class/kgsl/kgsl-3d0/gpu_model)
    case "$gpu_model" in
        Adreno618*)
            echo "sdm730"
            ;;
        Adreno612*)
            echo "sdm675"
            ;;
    esac
}

get_sun_name() {
    if [ -d "/sys/devices/system/cpu/cpufreq/policy6" ]; then
        echo "sdm8elite"
    else
        echo "sdm8sgen4"
    fi
}

get_bengal_name() {
    cpu_max_freq=$(cat /sys/devices/system/cpu/cpu4/cpufreq/cpuinfo_max_freq)
    if [ "$cpu_max_freq" -gt 2300000 ]; then
        echo "sdm680"
    else
        echo "sdm665"
    fi
}

get_mt6895_name() {
    cpu_max_freq=$(cat /sys/devices/system/cpu/cpu4/cpufreq/cpuinfo_max_freq)
    if [ "$cpu_max_freq" -gt 3000000 ]; then
        echo "dimensity8200"
    else
        echo "dimensity8100"
    fi
}

get_config_name() {
    case "$1" in
        canoe*)
            echo "sdm8elite2"
            ;;
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
        sun*)
            get_sun_name
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
        mt6799*)
            echo "helio_x30"
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
        mt6878*)
            echo "dimensity7300"
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
        mt6897*)
            echo "dimensity8300"
            ;;
        mt6899*)
            echo "dimensity8400"
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
        mt6991*)
            echo "dimensity9400"
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

if [ ! -f "${DIR_PATH}/mode.txt" ]; then
    echo "balance" > "${DIR_PATH}/mode.txt"
fi

"${DIR_PATH}/CuDaemon" --run \
"${DIR_PATH}/configs/${config_name}.ccf" \
"${DIR_PATH}/mode.txt" \
"${DIR_PATH}/scheduler_log.txt"