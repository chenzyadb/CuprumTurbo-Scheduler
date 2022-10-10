// CuprumTurbo V12
// Format Code use clang-format -style=WebKit -i main.c
#define check_bit(bit, array) (array[bit / 8] & (1 << (bit % 8)))
#define MAX_INT (((unsigned int)(-1)) >> 1)
#define __USE_GNU
#include <dirent.h>
#include <linux/input.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "cJSON.h"
#include "cJSON_Utils.h"
#include "libcu.h"

struct governor_config {
    int sc_perf_margin;
    int bc_perf_margin;
    int xc_perf_margin;
    int sc_freq_up_delay;
    int bc_freq_up_delay;
    int xc_freq_up_delay;
    int sc_freq_down_delay;
    int bc_freq_down_delay;
    int xc_freq_down_delay;
    int comm_limit_pwr;
    int boost_limit_pwr;
};
struct hint_config {
    char boost_type[16];
    int boost_duration;
};
struct boost_config {
    int gov_boost;
};
struct sched_config {
    int efficiency_cpu_start;
    int efficiency_cpu_end;
    int common_cpu_start;
    int common_cpu_end;
    int other_cpu_start;
    int other_cpu_end;
    int multi_perf_cpu_start;
    int multi_perf_cpu_end;
    int single_perf_cpu_start;
    int single_perf_cpu_end;
    int efficiency_sched_prio;
    int common_sched_prio;
    int other_sched_prio;
    int multi_perf_sched_prio;
    int single_perf_sched_prio;
};

pthread_t thread_info;
pthread_t taskset_tid;
struct governor_config powersave_conf;
struct governor_config balance_conf;
struct governor_config performance_conf;
struct governor_config fast_conf;
struct hint_config touch_hint;
struct hint_config swipe_hint;
struct hint_config gesture_hint;
struct hint_config top_activity_changed;
struct boost_config touch_boost;
struct boost_config swipe_boost;
struct boost_config gesture_boost;
struct boost_config heavyload_boost;
struct boost_config no_boost;
struct sched_config normal_sched;
struct sched_config boost_sched;
char config_url[1024];
char mode_url[1024];
char log_url[1024];
char path[256];
char mode[16];
char boost[32];
float cpu_usage[8] = { 0 };
int core_num = 0;
int governor_boost = 0;
int boost_duration_ms = 0;
int SCREEN_OFF = 0;
int cpu_perf_mask[8] = { 0 };
int basic_sc_level = -1;
int basic_bc_level = -1;
int basic_xc_level = -1;
int burst_sc_level = -1;
int burst_bc_level = -1;
int burst_xc_level = -1;
int expect_sc_level = -1;
int expect_bc_level = -1;
int expect_xc_level = -1;
int current_sc_level = -1;
int current_bc_level = -1;
int current_xc_level = -1;
int sc_pwr_ratio = 0;
int bc_pwr_ratio = 0;
int xc_pwr_ratio = 0;
int sc_core_num = 0;
int bc_core_num = 0;
int xc_core_num = 0;
long int cluster0_freq_table[11] = { 0 };
long int cluster1_freq_table[11] = { 0 };
long int cluster2_freq_table[11] = { 0 };
int sc_pwr_mask[11] = { 0 };
int bc_pwr_mask[11] = { 0 };
int xc_pwr_mask[11] = { 0 };
int cluster0_cpu = -1;
int cluster1_cpu = -1;
int cluster2_cpu = -1;
int freq_writer_type = -1;
int foreground_pid_num = 0;
void init_log(void)
{
    FILE* fp;
    fp = fopen(log_url, "w");
    fprintf(fp, "CuprumTurbo Scheduler V12 by chenzyadb.\n");
    fflush(fp);
    fclose(fp);
}
void write_log(const char* format, ...)
{
    FILE* fp;
    fp = fopen(log_url, "a");
    va_list arg;
    va_start(arg, format);
    time_t time_log = time(NULL);
    struct tm* tm_log = localtime(&time_log);
    fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d ", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday,
        tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec);
    vfprintf(fp, format, arg);
    va_end(arg);
    fprintf(fp, "\n");
    fflush(fp);
    fclose(fp);
}
void get_config(void)
{
    FILE* fp;
    cJSON* json_buffer = NULL;
    cJSON* json_name = NULL;
    cJSON* json_author = NULL;
    cJSON* item_buffer = NULL;
    cJSON* item_folder = NULL;
    cJSON* item_folder2 = NULL;
    cJSON* item_info = NULL;
    double sc_power_cost = 0;
    double bc_power_cost = 0;
    double xc_power_cost = 0;
    float cpu_pwr_ratio;
    int sc_volt_table[11] = { 0 };
    int bc_volt_table[11] = { 0 };
    int xc_volt_table[11] = { 0 };
    int i, j, min_diff, diff, cur_freq, fd;
    int config_exist = 0;
    int sc_basic_freq_mhz = 0;
    int bc_basic_freq_mhz = 0;
    int xc_basic_freq_mhz = 0;
    int sc_burst_freq_mhz = 0;
    int bc_burst_freq_mhz = 0;
    int xc_burst_freq_mhz = 0;
    int sc_expect_freq_mhz = 0;
    int bc_expect_freq_mhz = 0;
    int xc_expect_freq_mhz = 0;
    int sc_current_freq_mhz = 0;
    int bc_current_freq_mhz = 0;
    int xc_current_freq_mhz = 0;
    int sc_current_volt = 0;
    int bc_current_volt = 0;
    int xc_current_volt = 0;
    int sc_power_mw = 0;
    int bc_power_mw = 0;
    int xc_power_mw = 0;
    int sc_capacity = 0;
    int bc_capacity = 0;
    int xc_capacity = 0;
    char config_text[128 * 1024];
    char buf[1024];
    char file_url[256];
    fd = open(config_url, O_RDONLY);
    if (fd <= 0) {
        close(fd);
        write_log("[E] Can't open \"%s\".", config_url);
        exit(0);
    } else {
        read(fd, config_text, sizeof(config_text));
        close(fd);
        // Get JSON Buffer & Info
        json_buffer = cJSON_Parse(config_text);
        if (json_buffer == NULL) {
            write_log("[E] Can't read config file.");
            exit(0);
        }
        json_name = cJSON_GetObjectItem(json_buffer, "name");
        json_author = cJSON_GetObjectItem(json_buffer, "author");
        write_log("[I] Config \"%s\" by \"%s\".", json_name->valuestring, json_author->valuestring);
        // Get PowerModel Value
        item_buffer = cJSON_GetObjectItem(json_buffer, "power_model");
        item_info = cJSON_GetObjectItem(item_buffer, "sc_capacity");
        sc_capacity = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "bc_capacity");
        bc_capacity = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "xc_capacity");
        xc_capacity = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "sc_basic_freq_mhz");
        sc_basic_freq_mhz = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "bc_basic_freq_mhz");
        bc_basic_freq_mhz = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "xc_basic_freq_mhz");
        xc_basic_freq_mhz = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "sc_burst_freq_mhz");
        sc_burst_freq_mhz = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "bc_burst_freq_mhz");
        bc_burst_freq_mhz = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "xc_burst_freq_mhz");
        xc_burst_freq_mhz = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "sc_expect_freq_mhz");
        sc_expect_freq_mhz = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "bc_expect_freq_mhz");
        bc_expect_freq_mhz = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "xc_expect_freq_mhz");
        xc_expect_freq_mhz = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "sc_current_freq_mhz");
        sc_current_freq_mhz = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "bc_current_freq_mhz");
        bc_current_freq_mhz = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "xc_current_freq_mhz");
        xc_current_freq_mhz = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "sc_power_mw");
        sc_power_mw = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "bc_power_mw");
        bc_power_mw = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_buffer, "xc_power_mw");
        xc_power_mw = item_info->valueint;
        // Get governor value
        item_buffer = cJSON_GetObjectItem(json_buffer, "governor_config");
        item_folder = cJSON_GetObjectItem(item_buffer, "powersave");
        item_info = cJSON_GetObjectItem(item_folder, "sc_perf_margin");
        powersave_conf.sc_perf_margin = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "bc_perf_margin");
        powersave_conf.bc_perf_margin = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "xc_perf_margin");
        powersave_conf.xc_perf_margin = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "sc_freq_up_delay");
        powersave_conf.sc_freq_up_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "bc_freq_up_delay");
        powersave_conf.bc_freq_up_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "xc_freq_up_delay");
        powersave_conf.xc_freq_up_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "sc_freq_down_delay");
        powersave_conf.sc_freq_down_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "bc_freq_down_delay");
        powersave_conf.bc_freq_down_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "xc_freq_down_delay");
        powersave_conf.xc_freq_down_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "comm_limit_pwr");
        powersave_conf.comm_limit_pwr = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "boost_limit_pwr");
        powersave_conf.boost_limit_pwr = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "balance");
        item_info = cJSON_GetObjectItem(item_folder, "sc_perf_margin");
        balance_conf.sc_perf_margin = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "bc_perf_margin");
        balance_conf.bc_perf_margin = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "xc_perf_margin");
        balance_conf.xc_perf_margin = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "sc_freq_up_delay");
        balance_conf.sc_freq_up_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "bc_freq_up_delay");
        balance_conf.bc_freq_up_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "xc_freq_up_delay");
        balance_conf.xc_freq_up_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "sc_freq_down_delay");
        balance_conf.sc_freq_down_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "bc_freq_down_delay");
        balance_conf.bc_freq_down_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "xc_freq_down_delay");
        balance_conf.xc_freq_down_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "comm_limit_pwr");
        balance_conf.comm_limit_pwr = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "boost_limit_pwr");
        balance_conf.boost_limit_pwr = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "performance");
        item_info = cJSON_GetObjectItem(item_folder, "sc_perf_margin");
        performance_conf.sc_perf_margin = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "bc_perf_margin");
        performance_conf.bc_perf_margin = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "xc_perf_margin");
        performance_conf.xc_perf_margin = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "sc_freq_up_delay");
        performance_conf.sc_freq_up_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "bc_freq_up_delay");
        performance_conf.bc_freq_up_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "xc_freq_up_delay");
        performance_conf.xc_freq_up_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "sc_freq_down_delay");
        performance_conf.sc_freq_down_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "bc_freq_down_delay");
        performance_conf.bc_freq_down_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "xc_freq_down_delay");
        performance_conf.xc_freq_down_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "comm_limit_pwr");
        performance_conf.comm_limit_pwr = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "boost_limit_pwr");
        performance_conf.boost_limit_pwr = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "fast");
        item_info = cJSON_GetObjectItem(item_folder, "sc_perf_margin");
        fast_conf.sc_perf_margin = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "bc_perf_margin");
        fast_conf.bc_perf_margin = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "xc_perf_margin");
        fast_conf.xc_perf_margin = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "sc_freq_up_delay");
        fast_conf.sc_freq_up_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "bc_freq_up_delay");
        fast_conf.bc_freq_up_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "xc_freq_up_delay");
        fast_conf.xc_freq_up_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "sc_freq_down_delay");
        fast_conf.sc_freq_down_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "bc_freq_down_delay");
        fast_conf.bc_freq_down_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "xc_freq_down_delay");
        fast_conf.xc_freq_down_delay = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "comm_limit_pwr");
        fast_conf.comm_limit_pwr = item_info->valueint;
        item_info = cJSON_GetObjectItem(item_folder, "boost_limit_pwr");
        fast_conf.boost_limit_pwr = item_info->valueint;
        // get hint config
        item_buffer = cJSON_GetObjectItem(json_buffer, "hint_config");
        item_folder = cJSON_GetObjectItem(item_buffer, "touch");
        item_info = cJSON_GetObjectItem(item_folder, "boost_type");
        sprintf(touch_hint.boost_type, "%s", item_info->valuestring);
        item_info = cJSON_GetObjectItem(item_folder, "boost_duration");
        touch_hint.boost_duration = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "swipe");
        item_info = cJSON_GetObjectItem(item_folder, "boost_type");
        sprintf(swipe_hint.boost_type, "%s", item_info->valuestring);
        item_info = cJSON_GetObjectItem(item_folder, "boost_duration");
        swipe_hint.boost_duration = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "gesture");
        item_info = cJSON_GetObjectItem(item_folder, "boost_type");
        sprintf(gesture_hint.boost_type, "%s", item_info->valuestring);
        item_info = cJSON_GetObjectItem(item_folder, "boost_duration");
        gesture_hint.boost_duration = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "top_activity_changed");
        item_info = cJSON_GetObjectItem(item_folder, "boost_type");
        sprintf(top_activity_changed.boost_type, "%s", item_info->valuestring);
        item_info = cJSON_GetObjectItem(item_folder, "boost_duration");
        top_activity_changed.boost_duration = item_info->valueint;
        // get boost config
        item_buffer = cJSON_GetObjectItem(json_buffer, "boost_config");
        item_folder = cJSON_GetObjectItem(item_buffer, "touch");
        item_info = cJSON_GetObjectItem(item_folder, "governor.boost");
        touch_boost.gov_boost = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "swipe");
        item_info = cJSON_GetObjectItem(item_folder, "governor.boost");
        swipe_boost.gov_boost = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "gesture");
        item_info = cJSON_GetObjectItem(item_folder, "governor.boost");
        gesture_boost.gov_boost = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "heavyload");
        item_info = cJSON_GetObjectItem(item_folder, "governor.boost");
        heavyload_boost.gov_boost = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "none");
        item_info = cJSON_GetObjectItem(item_folder, "governor.boost");
        no_boost.gov_boost = item_info->valueint;
        // TasksetHelper config
        item_buffer = cJSON_GetObjectItem(json_buffer, "TasksetHelper_config");
        item_folder = cJSON_GetObjectItem(item_buffer, "efficiency");
        item_folder2 = cJSON_GetObjectItem(item_folder, "normal");
        item_info = cJSON_GetObjectItem(item_folder2, "cpu_set_t");
        sscanf(item_info->valuestring, "%d-%d", &normal_sched.efficiency_cpu_start, &normal_sched.efficiency_cpu_end);
        item_info = cJSON_GetObjectItem(item_folder2, "sched_priority");
        normal_sched.efficiency_sched_prio = item_info->valueint;
        item_folder2 = cJSON_GetObjectItem(item_folder, "boost");
        item_info = cJSON_GetObjectItem(item_folder2, "cpu_set_t");
        sscanf(item_info->valuestring, "%d-%d", &boost_sched.efficiency_cpu_start, &boost_sched.efficiency_cpu_end);
        item_info = cJSON_GetObjectItem(item_folder2, "sched_priority");
        boost_sched.efficiency_sched_prio = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "common");
        item_folder2 = cJSON_GetObjectItem(item_folder, "normal");
        item_info = cJSON_GetObjectItem(item_folder2, "cpu_set_t");
        sscanf(item_info->valuestring, "%d-%d", &normal_sched.common_cpu_start, &normal_sched.common_cpu_end);
        item_info = cJSON_GetObjectItem(item_folder2, "sched_priority");
        normal_sched.common_sched_prio = item_info->valueint;
        item_folder2 = cJSON_GetObjectItem(item_folder, "boost");
        item_info = cJSON_GetObjectItem(item_folder2, "cpu_set_t");
        sscanf(item_info->valuestring, "%d-%d", &boost_sched.common_cpu_start, &boost_sched.common_cpu_end);
        item_info = cJSON_GetObjectItem(item_folder2, "sched_priority");
        boost_sched.common_sched_prio = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "multi_perf");
        item_folder2 = cJSON_GetObjectItem(item_folder, "normal");
        item_info = cJSON_GetObjectItem(item_folder2, "cpu_set_t");
        sscanf(item_info->valuestring, "%d-%d", &normal_sched.multi_perf_cpu_start, &normal_sched.multi_perf_cpu_end);
        item_info = cJSON_GetObjectItem(item_folder2, "sched_priority");
        normal_sched.multi_perf_sched_prio = item_info->valueint;
        item_folder2 = cJSON_GetObjectItem(item_folder, "boost");
        item_info = cJSON_GetObjectItem(item_folder2, "cpu_set_t");
        sscanf(item_info->valuestring, "%d-%d", &boost_sched.multi_perf_cpu_start, &boost_sched.multi_perf_cpu_end);
        item_info = cJSON_GetObjectItem(item_folder2, "sched_priority");
        boost_sched.multi_perf_sched_prio = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "single_perf");
        item_folder2 = cJSON_GetObjectItem(item_folder, "normal");
        item_info = cJSON_GetObjectItem(item_folder2, "cpu_set_t");
        sscanf(item_info->valuestring, "%d-%d", &normal_sched.single_perf_cpu_start, &normal_sched.single_perf_cpu_end);
        item_info = cJSON_GetObjectItem(item_folder2, "sched_priority");
        normal_sched.single_perf_sched_prio = item_info->valueint;
        item_folder2 = cJSON_GetObjectItem(item_folder, "boost");
        item_info = cJSON_GetObjectItem(item_folder2, "cpu_set_t");
        sscanf(item_info->valuestring, "%d-%d", &boost_sched.single_perf_cpu_start, &boost_sched.single_perf_cpu_end);
        item_info = cJSON_GetObjectItem(item_folder2, "sched_priority");
        boost_sched.single_perf_sched_prio = item_info->valueint;
        item_folder = cJSON_GetObjectItem(item_buffer, "other");
        item_folder2 = cJSON_GetObjectItem(item_folder, "normal");
        item_info = cJSON_GetObjectItem(item_folder2, "cpu_set_t");
        sscanf(item_info->valuestring, "%d-%d", &normal_sched.other_cpu_start, &normal_sched.other_cpu_end);
        item_info = cJSON_GetObjectItem(item_folder2, "sched_priority");
        normal_sched.other_sched_prio = item_info->valueint;
        item_folder2 = cJSON_GetObjectItem(item_folder, "boost");
        item_info = cJSON_GetObjectItem(item_folder2, "cpu_set_t");
        sscanf(item_info->valuestring, "%d-%d", &boost_sched.other_cpu_start, &boost_sched.other_cpu_end);
        item_info = cJSON_GetObjectItem(item_folder2, "sched_priority");
        boost_sched.other_sched_prio = item_info->valueint;
    }
    // get freq_table
    long int origin_freq_table[50] = { 0 };
    long int cluster_min_freq, cluster_max_freq, freq_stair, target_freq, min_freq_diff;
    int freq_table_num, start_p, end_p, target_freq_idx;
    char freq_buf[16];
    if (cluster0_cpu != -1) {
        memset(origin_freq_table, 0, sizeof(origin_freq_table));
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies", cluster0_cpu);
        sprintf(buf, "%s", read_value(file_url));
        freq_table_num = 0;
        start_p = -1;
        end_p = -1;
        for (i = 0; i < strlen(buf); i++) {
            if (buf[i] == ' ') {
                memset(freq_buf, 0, sizeof(freq_buf));
                start_p = end_p + 1;
                end_p = i;
                for (j = 0; j <= end_p - start_p; j++) {
                    freq_buf[j] = buf[start_p + j];
                }
                sscanf(freq_buf, "%ld", &origin_freq_table[freq_table_num]);
                freq_table_num++;
            }
        }
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_min_freq", cluster0_cpu);
        sscanf(read_value(file_url), "%ld", &cluster_min_freq);
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cluster0_cpu);
        sscanf(read_value(file_url), "%ld", &cluster_max_freq);
        cluster0_freq_table[0] = cluster_min_freq;
        cluster0_freq_table[10] = cluster_max_freq;
        freq_stair = (cluster_max_freq / 1000 - sc_basic_freq_mhz) / 10;
        for (i = 1; i <= 9; i++) {
            target_freq_idx = 0;
            min_freq_diff = 9999999;
            target_freq = (sc_basic_freq_mhz + (i - 1) * freq_stair) * 1000;
            for (j = 0; j <= freq_table_num; j++) {
                if (labs(origin_freq_table[j] - target_freq) <= min_freq_diff) {
                    min_freq_diff = labs(origin_freq_table[j] - target_freq);
                    target_freq_idx = j;
                }
            }
            cluster0_freq_table[i] = origin_freq_table[target_freq_idx];
        }
    }
    if (cluster1_cpu != -1) {
        memset(origin_freq_table, 0, sizeof(origin_freq_table));
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies", cluster1_cpu);
        sprintf(buf, "%s", read_value(file_url));
        freq_table_num = 0;
        start_p = -1;
        end_p = -1;
        for (i = 0; i <= strlen(buf); i++) {
            if (buf[i] == ' ') {
                memset(freq_buf, 0, sizeof(freq_buf));
                start_p = end_p + 1;
                end_p = i;
                for (j = 0; j <= end_p - start_p; j++) {
                    freq_buf[j] = buf[start_p + j];
                }
                sscanf(freq_buf, "%ld", &origin_freq_table[freq_table_num]);
                freq_table_num++;
            }
        }
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_min_freq", cluster1_cpu);
        sscanf(read_value(file_url), "%ld", &cluster_min_freq);
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cluster1_cpu);
        sscanf(read_value(file_url), "%ld", &cluster_max_freq);
        cluster1_freq_table[0] = cluster_min_freq;
        cluster1_freq_table[10] = cluster_max_freq;
        freq_stair = (cluster_max_freq / 1000 - sc_basic_freq_mhz) / 10;
        for (i = 1; i <= 9; i++) {
            target_freq_idx = 0;
            min_freq_diff = 9999999;
            target_freq = (bc_basic_freq_mhz + (i - 1) * freq_stair) * 1000;
            for (j = 0; j <= freq_table_num; j++) {
                if (labs(origin_freq_table[j] - target_freq) <= min_freq_diff) {
                    min_freq_diff = labs(origin_freq_table[j] - target_freq);
                    target_freq_idx = j;
                }
            }
            cluster1_freq_table[i] = origin_freq_table[target_freq_idx];
        }
    }
    if (cluster2_cpu != -1) {
        memset(origin_freq_table, 0, sizeof(origin_freq_table));
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies", cluster2_cpu);
        sprintf(buf, "%s", read_value(file_url));
        freq_table_num = 0;
        start_p = -1;
        end_p = -1;
        for (i = 0; i <= strlen(buf); i++) {
            if (buf[i] == ' ') {
                memset(freq_buf, 0, sizeof(freq_buf));
                start_p = end_p + 1;
                end_p = i;
                for (j = 0; j <= end_p - start_p; j++) {
                    freq_buf[j] = buf[start_p + j];
                }
                sscanf(freq_buf, "%ld", &origin_freq_table[freq_table_num]);
                freq_table_num++;
            }
        }
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_min_freq", cluster2_cpu);
        sscanf(read_value(file_url), "%ld", &cluster_min_freq);
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cluster2_cpu);
        sscanf(read_value(file_url), "%ld", &cluster_max_freq);
        cluster2_freq_table[0] = cluster_min_freq;
        cluster2_freq_table[10] = cluster_max_freq;
        freq_stair = (cluster_max_freq / 1000 - sc_basic_freq_mhz) / 10;
        for (i = 1; i <= 9; i++) {
            target_freq_idx = 0;
            min_freq_diff = 9999999;
            target_freq = (xc_basic_freq_mhz + (i - 1) * freq_stair) * 1000;
            for (j = 0; j <= freq_table_num; j++) {
                if (labs(origin_freq_table[j] - target_freq) <= min_freq_diff) {
                    min_freq_diff = labs(origin_freq_table[j] - target_freq);
                    target_freq_idx = j;
                }
            }
            cluster2_freq_table[i] = origin_freq_table[target_freq_idx];
        }
    }
    // freq_mhz -> freq_idx
    basic_sc_level = 0;
    min_diff = 9999;
    for (i = 0; i <= 10; i++) {
        diff = cluster0_freq_table[i] / 1000 - sc_basic_freq_mhz;
        if (abs(diff) < min_diff) {
            basic_sc_level = i;
            min_diff = abs(diff);
        }
    }
    burst_sc_level = 0;
    min_diff = 9999;
    for (i = 1; i <= 10; i++) {
        diff = cluster0_freq_table[i] / 1000 - sc_burst_freq_mhz;
        if (abs(diff) < min_diff) {
            burst_sc_level = i;
            min_diff = abs(diff);
        }
    }
    expect_sc_level = 0;
    min_diff = 9999;
    for (i = 1; i <= 10; i++) {
        diff = cluster0_freq_table[i] / 1000 - sc_expect_freq_mhz;
        if (abs(diff) < min_diff) {
            expect_sc_level = i;
            min_diff = abs(diff);
        }
    }
    current_sc_level = 0;
    min_diff = 9999;
    for (i = 1; i <= 10; i++) {
        diff = cluster0_freq_table[i] / 1000 - sc_current_freq_mhz;
        if (abs(diff) < min_diff) {
            current_sc_level = i;
            min_diff = abs(diff);
        }
    }
    basic_bc_level = 0;
    min_diff = 9999;
    for (i = 0; i <= 10; i++) {
        diff = cluster1_freq_table[i] / 1000 - bc_basic_freq_mhz;
        if (abs(diff) < min_diff) {
            basic_bc_level = i;
            min_diff = abs(diff);
        }
    }
    burst_bc_level = 0;
    min_diff = 9999;
    for (i = 1; i <= 10; i++) {
        diff = cluster1_freq_table[i] / 1000 - bc_burst_freq_mhz;
        if (abs(diff) < min_diff) {
            burst_bc_level = i;
            min_diff = abs(diff);
        }
    }
    expect_bc_level = 0;
    min_diff = 9999;
    for (i = 1; i <= 10; i++) {
        diff = cluster1_freq_table[i] / 1000 - bc_expect_freq_mhz;
        if (abs(diff) < min_diff) {
            expect_bc_level = i;
            min_diff = abs(diff);
        }
    }
    current_bc_level = 0;
    min_diff = 9999;
    for (i = 1; i <= 10; i++) {
        diff = cluster1_freq_table[i] / 1000 - bc_current_freq_mhz;
        if (abs(diff) < min_diff) {
            current_bc_level = i;
            min_diff = abs(diff);
        }
    }
    basic_xc_level = 0;
    min_diff = 9999;
    for (i = 0; i <= 10; i++) {
        diff = cluster2_freq_table[i] / 1000 - xc_basic_freq_mhz;
        if (abs(diff) < min_diff) {
            basic_xc_level = i;
            min_diff = abs(diff);
        }
    }
    burst_xc_level = 0;
    min_diff = 9999;
    for (i = 1; i <= 10; i++) {
        diff = cluster2_freq_table[i] / 1000 - xc_burst_freq_mhz;
        if (abs(diff) < min_diff) {
            burst_xc_level = i;
            min_diff = abs(diff);
        }
    }
    expect_xc_level = 0;
    min_diff = 9999;
    for (i = 1; i <= 10; i++) {
        diff = cluster2_freq_table[i] / 1000 - xc_expect_freq_mhz;
        if (abs(diff) < min_diff) {
            expect_xc_level = i;
            min_diff = abs(diff);
        }
    }
    current_xc_level = 0;
    min_diff = 9999;
    for (i = 1; i <= 10; i++) {
        diff = cluster2_freq_table[i] / 1000 - xc_current_freq_mhz;
        if (abs(diff) < min_diff) {
            current_xc_level = i;
            min_diff = abs(diff);
        }
    }
    // Get CPU Volt Table
    for (i = 0; i <= 10; i++) {
        sc_volt_table[i] = get_freq_volt(cluster0_freq_table[i] / 1000, sc_burst_freq_mhz, sc_expect_freq_mhz);
    }
    for (i = 0; i <= 10; i++) {
        bc_volt_table[i] = get_freq_volt(cluster1_freq_table[i] / 1000, bc_burst_freq_mhz, bc_expect_freq_mhz);
    }
    for (i = 0; i <= 10; i++) {
        xc_volt_table[i] = get_freq_volt(cluster2_freq_table[i] / 1000, xc_burst_freq_mhz, xc_expect_freq_mhz);
    }
    sc_current_volt = get_freq_volt(sc_current_freq_mhz, sc_burst_freq_mhz, sc_expect_freq_mhz);
    bc_current_volt = get_freq_volt(bc_current_freq_mhz, bc_burst_freq_mhz, bc_expect_freq_mhz);
    xc_current_volt = get_freq_volt(xc_current_freq_mhz, xc_burst_freq_mhz, xc_expect_freq_mhz);
    // Get Power Model
    sc_power_cost = (double)sc_power_mw * 10000 / ((long long int)sc_current_freq_mhz * sc_current_volt * sc_current_volt);
    for (i = 0; i <= 10; i++) {
        sc_pwr_mask[i] = sc_power_cost * cluster0_freq_table[i] / 1000 * sc_volt_table[i] * sc_volt_table[i] / 10000;
    }
    bc_power_cost = (double)bc_power_mw * 10000 / ((long long int)bc_current_freq_mhz * bc_current_volt * bc_current_volt);
    for (i = 0; i <= 10; i++) {
        bc_pwr_mask[i] = bc_power_cost * cluster1_freq_table[i] / 1000 * bc_volt_table[i] * bc_volt_table[i] / 10000;
    }
    xc_power_cost = (double)xc_power_mw * 10000 / ((long long int)xc_current_freq_mhz * xc_current_volt * xc_current_volt);
    for (i = 0; i <= 10; i++) {
        xc_pwr_mask[i] = xc_power_cost * cluster2_freq_table[i] / 1000 * xc_volt_table[i] * xc_volt_table[i] / 10000;
    }
    // Get Power Ratio
    cpu_pwr_ratio = (float)sc_capacity * sc_core_num + bc_capacity * bc_core_num + xc_capacity * xc_core_num;
    sc_pwr_ratio = sc_capacity * sc_core_num * 100 / cpu_pwr_ratio;
    bc_pwr_ratio = bc_capacity * bc_core_num * 100 / cpu_pwr_ratio;
    xc_pwr_ratio = xc_capacity * xc_core_num * 100 / cpu_pwr_ratio;
}
void get_cpu_clusters(void)
{
    FILE* fp;
    char buf[128];
    char url[1024];
    long int cpu_freq, last_cpu_freq;
    int i;
    cluster0_cpu = 0;
    if ((access("/sys/devices/system/cpu/cpufreq/policy4/scaling_min_freq", 0)) != -1) {
        cluster1_cpu = 4;
        if ((access("/sys/devices/system/cpu/cpufreq/policy6/scaling_min_freq", 0)) != -1) {
            cluster2_cpu = 6;
        } else if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", 0)) != -1) {
            cluster2_cpu = 7;
        }
    } else if ((access("/sys/devices/system/cpu/cpufreq/policy2/scaling_min_freq", 0)) != -1) {
        cluster1_cpu = 2;
    } else if ((access("/sys/devices/system/cpu/cpufreq/policy6/scaling_min_freq", 0)) != -1) {
        cluster1_cpu = 6;
        if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", 0)) != -1) {
            cluster2_cpu = 7;
        }
    } else {
        cluster0_cpu = -1;
        cluster1_cpu = -1;
        cluster2_cpu = -1;
        for (i = 0; i <= core_num; i++) {
            sprintf(url, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", i);
            fp = fopen(url, "r");
            if (fp) {
                fgets(buf, sizeof(buf), fp);
                fclose(fp);
                last_cpu_freq = cpu_freq;
                sscanf(buf, "%ld", &cpu_freq);
                if (cpu_freq != last_cpu_freq) {
                    if (cluster0_cpu == -1) {
                        cluster0_cpu = i;
                    } else if (cluster1_cpu == -1) {
                        cluster1_cpu = i;
                    } else if (cluster2_cpu == -1) {
                        cluster2_cpu = i;
                    }
                }
            }
        }
    }
}
void get_core_num(void)
{
    if (cluster1_cpu == -1) {
        sc_core_num = core_num + 1;
        bc_core_num = 0;
        xc_core_num = 0;
    } else if (cluster2_cpu == -1) {
        sc_core_num = cluster1_cpu;
        bc_core_num = core_num - cluster1_cpu + 1;
        xc_core_num = 0;
    } else {
        sc_core_num = cluster1_cpu;
        bc_core_num = cluster2_cpu - cluster1_cpu;
        xc_core_num = core_num - cluster2_cpu + 1;
    }
}
void restore_task_affinity(int pid)
{
    DIR* dir;
    struct dirent* entry;
    cpu_set_t cpu_mask;
    struct sched_param prio;
    char dir_url[128];
    int tid, i;
    prio.sched_priority = 120;
    CPU_ZERO(&cpu_mask);
    for (i = 0; i <= core_num; i++) {
        CPU_SET(i, &cpu_mask);
    }
    sprintf(dir_url, "/proc/%d/task/", pid);
    dir = opendir(dir_url);
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            sscanf((*entry).d_name, "%d", &tid);
            sched_setaffinity(tid, sizeof(cpu_mask), &cpu_mask);
            sched_setscheduler(tid, SCHED_NORMAL, &prio);
        }
    }
}
void set_boost_affinity(int pid)
{
    FILE* fp;
    DIR* dir;
    cpu_set_t efficiency_mask;
    cpu_set_t multi_perf_mask;
    cpu_set_t single_perf_mask;
    cpu_set_t comm_mask;
    cpu_set_t other_mask;
    struct sched_param efficiency_prio;
    struct sched_param comm_prio;
    struct sched_param multi_perf_prio;
    struct sched_param single_perf_prio;
    struct sched_param other_prio;
    struct dirent* entry;
    char buf[256];
    char tid_dir[128];
    char tid_url[128];
    char tid_name[32];
    int tid, i;
    CPU_ZERO(&efficiency_mask);
    for (i = boost_sched.efficiency_cpu_start; i <= boost_sched.efficiency_cpu_end; i++) {
        CPU_SET(i, &efficiency_mask);
    }
    CPU_ZERO(&comm_mask);
    for (i = boost_sched.common_cpu_start; i <= boost_sched.common_cpu_end; i++) {
        CPU_SET(i, &comm_mask);
    }
    CPU_ZERO(&other_mask);
    for (i = boost_sched.other_cpu_start; i <= boost_sched.other_cpu_end; i++) {
        CPU_SET(i, &other_mask);
    }
    CPU_ZERO(&multi_perf_mask);
    for (i = boost_sched.multi_perf_cpu_start; i <= boost_sched.multi_perf_cpu_end; i++) {
        CPU_SET(i, &multi_perf_mask);
    }
    CPU_ZERO(&single_perf_mask);
    for (i = boost_sched.single_perf_cpu_start; i <= boost_sched.single_perf_cpu_end; i++) {
        CPU_SET(i, &single_perf_mask);
    }
    efficiency_prio.sched_priority = boost_sched.efficiency_sched_prio;
    comm_prio.sched_priority = boost_sched.common_sched_prio;
    other_prio.sched_priority = boost_sched.other_sched_prio;
    multi_perf_prio.sched_priority = boost_sched.multi_perf_sched_prio;
    single_perf_prio.sched_priority = boost_sched.single_perf_sched_prio;
    sprintf(tid_dir, "/proc/%d/task/", pid);
    dir = opendir(tid_dir);
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            sscanf((*entry).d_name, "%d", &tid);
            sprintf(tid_url, "%s%d/comm", tid_dir, tid);
            fp = fopen(tid_url, "r");
            if (fp != NULL) {
                memset(tid_name, 0, sizeof(tid_name));
                fgets(tid_name, sizeof(tid_name), fp);
                fclose(fp);
                reset_task_nice(tid);
                if (tid == pid) {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &multi_perf_prio);
                } else if (check_regex(tid_name, "^(UnityMain|MainThread-UE4|GameThread|SDLThread|RenderThread|MINECRAFT|GLThread|Thread-)")) {
                    sched_setaffinity(tid, sizeof(single_perf_mask), &single_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &single_perf_prio);
                } else if (strstr(tid_name, "^(UnityMulti|UnityPreload|UnityChoreograp|UnityGfx|Worker Thread)")) {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &multi_perf_prio);
                } else if (check_regex(tid_name, "^(LoadingThread|RHIThread|FrameThread|Job.Worker|CmpJob|TaskGraphNP|glp)")) {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &multi_perf_prio);
                } else if (check_regex(tid_name, "^(JNISurfaceText|IJK_External|ForkJoinPool-|UiThread|AndroidUI|RenderEngine|[.]raster|Compositor)")) {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &multi_perf_prio);
                } else if (check_regex(tid_name, "^([Gg]esture|.gifmaker|Binder|mali-|[Aa]sync|[Vv]sync|android.anim|android.ui|[Bb]lur|[Aa]nim|Chrome_|Viz)")) {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &multi_perf_prio);
                } else if (check_regex(tid_name, "^(Chrome_InProc|Chromium|Gecko|[Ww]eb[Vv]iew|[Jj]ava[Ss]cript|js|JS|android.fg|android.io)")) {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &comm_prio);
                } else if (check_regex(tid_name, "^(CrGpuMain|CrRendererMain|work_thread|NativeThread|[Dd]ownload|[Mm]ixer|[Aa]udio|[Vv]ideo)")) {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &comm_prio);
                } else if (check_regex(tid_name, "^(OkHttp|ThreadPool|PoolThread|glide-|pool-|launcher-|Fresco)")) {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &comm_prio);
                } else if (check_regex(tid_name, "^(SearchDaemon|Profile|ged-swd|GPU completion|FramePolicy|ScrollPolicy|HeapTaskDaemon|FinalizerDaemon)")) {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &efficiency_prio);
                } else if (check_regex(tid_name, "^(ReferenceQueue|Jit thread pool|Timer-|log|xcrash|Ysa|Xqa|Rx|APM|TVKDL-|tp-|cgi-|ODCP-|xlog_|[Bb]ugly|android.bg|SensorService)")) {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &efficiency_prio);
                } else if (check_regex(tid_name, "^(HealthService|[Bb]ackground|[Rr]eport|tt_pangle|xg_vip_service|default_matrix|FrameDecoder|FrameSeq|hwui)")) {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &efficiency_prio);
                } else {
                    sched_setaffinity(tid, sizeof(other_mask), &other_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &other_prio);
                }
            }
        }
        closedir(dir);
    }
}
void write_freq(int sc_min, int sc_max, int bc_min, int bc_max, int xc_min, int xc_max)
{
    long int SC_MIN_FREQ = cluster0_freq_table[sc_min];
    long int SC_MAX_FREQ = cluster0_freq_table[sc_max];
    long int MC_MIN_FREQ = cluster1_freq_table[bc_min];
    long int MC_MAX_FREQ = cluster1_freq_table[bc_max];
    long int BC_MIN_FREQ = cluster2_freq_table[xc_min];
    long int BC_MAX_FREQ = cluster2_freq_table[xc_max];
    char file_url[128];
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", cluster0_cpu);
    write_value(file_url, "%ld\n", SC_MIN_FREQ);
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", cluster0_cpu);
    write_value(file_url, "%ld\n", SC_MAX_FREQ);
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", cluster1_cpu);
    write_value(file_url, "%ld\n", MC_MIN_FREQ);
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", cluster1_cpu);
    write_value(file_url, "%ld\n", MC_MAX_FREQ);
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", cluster2_cpu);
    write_value(file_url, "%ld\n", BC_MIN_FREQ);
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", cluster2_cpu);
    write_value(file_url, "%ld\n", BC_MAX_FREQ);
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq", cluster0_cpu);
    write_value(file_url, "%ld\n", SC_MIN_FREQ);
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq", cluster0_cpu);
    write_value(file_url, "%ld\n", SC_MAX_FREQ);
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq", cluster1_cpu);
    write_value(file_url, "%ld\n", MC_MIN_FREQ);
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq", cluster1_cpu);
    write_value(file_url, "%ld\n", MC_MAX_FREQ);
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq", cluster2_cpu);
    write_value(file_url, "%ld\n", BC_MIN_FREQ);
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq", cluster2_cpu);
    write_value(file_url, "%ld\n", BC_MAX_FREQ);
    if ((access("/proc/ppm/policy/hard_userlimit_cpu_freq", 0)) != -1) {
        if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", 0)) != -1) {
            write_value("/proc/ppm/policy/hard_userlimit_cpu_freq", "%ld %ld %ld %ld %ld %ld\n", SC_MIN_FREQ,
                SC_MAX_FREQ, MC_MIN_FREQ, MC_MAX_FREQ, BC_MIN_FREQ, BC_MAX_FREQ);
        } else {
            write_value("/proc/ppm/policy/hard_userlimit_cpu_freq", "%ld %ld %ld %ld\n", SC_MIN_FREQ, SC_MAX_FREQ,
                MC_MIN_FREQ, MC_MAX_FREQ);
        }
    }
    if (access("/sys/module/msm_performance/parameters/cpu_max_freq", 0) != -1) {
        if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", 0)) != -1) {
            write_value("/sys/module/msm_performance/parameters/cpu_max_freq", "%d:%ld %d:%ld %d:%ld\n", cluster0_cpu,
                SC_MAX_FREQ, cluster1_cpu, MC_MAX_FREQ, cluster2_cpu, BC_MAX_FREQ);
            write_value("/sys/module/msm_performance/parameters/cpu_min_freq", "%d:%ld %d:%ld %d:%ld\n", cluster0_cpu,
                SC_MIN_FREQ, cluster1_cpu, MC_MIN_FREQ, cluster2_cpu, BC_MIN_FREQ);
        } else {
            write_value("/sys/module/msm_performance/parameters/cpu_max_freq", "%d:%ld %d:%ld\n", cluster0_cpu,
                SC_MAX_FREQ, cluster1_cpu, MC_MAX_FREQ);
            write_value("/sys/module/msm_performance/parameters/cpu_min_freq", "%d:%ld %d:%ld\n", cluster0_cpu,
                SC_MIN_FREQ, cluster1_cpu, MC_MIN_FREQ);
        }
    }
    if ((access("/dev/cluster0_freq_min", 0)) != -1) {
        write_value("/dev/cluster0_freq_min", "%ld\n", SC_MIN_FREQ);
    }
    if ((access("/dev/cluster0_freq_max", 0)) != -1) {
        write_value("/dev/cluster0_freq_max", "%ld\n", SC_MAX_FREQ);
    }
    if ((access("/dev/cluster1_freq_min", 0)) != -1) {
        write_value("/dev/cluster1_freq_min", "%ld\n", MC_MIN_FREQ);
    }
    if ((access("/dev/cluster1_freq_max", 0)) != -1) {
        write_value("/dev/cluster1_freq_max", "%ld\n", MC_MAX_FREQ);
    }
    if ((access("/dev/cluster2_freq_min", 0)) != -1) {
        write_value("/dev/cluster2_freq_min", "%ld\n", BC_MIN_FREQ);
    }
    if ((access("/dev/cluster2_freq_max", 0)) != -1) {
        write_value("/dev/cluster2_freq_max", "%ld\n", BC_MAX_FREQ);
    }
}
void qti_freq_writer(int cpu, long int freq)
{
    if (access("/sys/module/msm_performance/parameters/cpu_max_freq", 0) != -1) {
        write_value("/sys/module/msm_performance/parameters/cpu_max_freq", "%d:%ld\n", cpu, freq);
        write_value("/sys/module/msm_performance/parameters/cpu_min_freq", "%d:%ld\n", cpu, freq);
    }
}
void mtk_freq_writer(int cluster, long int freq)
{
    FILE* fp;
    long int cluster0_cur_freq, cluster1_cur_freq, cluster2_cur_freq;
    char buf[128];
    char file_url[128];
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_cur_freq", cluster0_cpu);
    fp = fopen(file_url, "r");
    if (fp) {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "%ld", &cluster0_cur_freq);
        fclose(fp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_cur_freq", cluster1_cpu);
    fp = fopen(file_url, "r");
    if (fp) {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "%ld", &cluster1_cur_freq);
        fclose(fp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_cur_freq", cluster2_cpu);
    fp = fopen(file_url, "r");
    if (fp) {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "%ld", &cluster2_cur_freq);
        fclose(fp);
    }
    if (cluster == 0) {
        cluster0_cur_freq = freq;
    } else if (cluster == 1) {
        cluster1_cur_freq = freq;
    } else if (cluster == 2) {
        cluster2_cur_freq = freq;
    }
    if (access("/proc/ppm/policy/hard_userlimit_cpu_freq", 0) != -1) {
        if (access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", 0) != -1) {
            write_value("/proc/ppm/policy/hard_userlimit_cpu_freq", "%ld %ld %ld %ld %ld %ld\n", cluster0_cur_freq,
                cluster0_cur_freq, cluster1_cur_freq, cluster1_cur_freq, cluster2_cur_freq, cluster2_cur_freq);
        } else {
            write_value("/proc/ppm/policy/hard_userlimit_cpu_freq", "%ld %ld %ld %ld\n", cluster0_cur_freq,
                cluster0_cur_freq, cluster1_cur_freq, cluster1_cur_freq);
        }
    }
}
void exynos_freq_writer(int cluster, long int freq)
{
    char file_url[128];
    sprintf(file_url, "/dev/cluster%d_freq_min", cluster);
    write_value(file_url, "%ld\n", freq);
    sprintf(file_url, "/dev/cluster%d_freq_max", cluster);
    write_value(file_url, "%ld\n", freq);
}
void userspace_freq_writer(int cpu, long int freq)
{
    char file_url[128];
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_setspeed", cpu);
    write_value(file_url, "%ld\n", freq);
}
void eas_freq_writer(int cpu, long int freq)
{
    char file_url[128];
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq", cpu);
    write_value(file_url, "%ld\n", freq);
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq", cpu);
    write_value(file_url, "%ld\n", freq);
}
void hmp_freq_writer(int cpu, long int freq)
{
    char file_url[128];
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", cpu);
    write_value(file_url, "%ld\n", freq);
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", cpu);
    write_value(file_url, "%ld\n", freq);
}
void write_xc_freq(long int freq_level)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "CpuFreqWriter0");
    if (freq_writer_type == 1) {
        qti_freq_writer(cluster2_cpu, cluster2_freq_table[freq_level]);
    } else if (freq_writer_type == 2) {
        exynos_freq_writer(2, cluster2_freq_table[freq_level]);
    } else if (freq_writer_type == 3) {
        mtk_freq_writer(2, cluster2_freq_table[freq_level]);
    } else if (freq_writer_type == 4) {
        userspace_freq_writer(cluster2_cpu, cluster2_freq_table[freq_level]);
    } else if (freq_writer_type == 5) {
        eas_freq_writer(cluster2_cpu, cluster2_freq_table[freq_level]);
    } else if (freq_writer_type == 6) {
        hmp_freq_writer(cluster2_cpu, cluster2_freq_table[freq_level]);
    }
    pthread_exit(0);
}
void write_bc_freq(long int freq_level)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "CpuFreqWriter1");
    if (freq_writer_type == 1) {
        qti_freq_writer(cluster1_cpu, cluster1_freq_table[freq_level]);
    } else if (freq_writer_type == 2) {
        exynos_freq_writer(1, cluster1_freq_table[freq_level]);
    } else if (freq_writer_type == 3) {
        mtk_freq_writer(1, cluster1_freq_table[freq_level]);
    } else if (freq_writer_type == 4) {
        userspace_freq_writer(cluster1_cpu, cluster1_freq_table[freq_level]);
    } else if (freq_writer_type == 5) {
        eas_freq_writer(cluster1_cpu, cluster1_freq_table[freq_level]);
    } else if (freq_writer_type == 6) {
        hmp_freq_writer(cluster1_cpu, cluster1_freq_table[freq_level]);
    }
    pthread_exit(0);
}
void write_sc_freq(int freq_level)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "CpuFreqWriter2");
    if (freq_writer_type == 1) {
        qti_freq_writer(cluster0_cpu, cluster0_freq_table[freq_level]);
    } else if (freq_writer_type == 2) {
        exynos_freq_writer(0, cluster0_freq_table[freq_level]);
    } else if (freq_writer_type == 3) {
        mtk_freq_writer(0, cluster0_freq_table[freq_level]);
    } else if (freq_writer_type == 4) {
        userspace_freq_writer(cluster0_cpu, cluster0_freq_table[freq_level]);
    } else if (freq_writer_type == 5) {
        eas_freq_writer(cluster0_cpu, cluster0_freq_table[freq_level]);
    } else if (freq_writer_type == 6) {
        hmp_freq_writer(cluster0_cpu, cluster0_freq_table[freq_level]);
    }
    pthread_exit(0);
}
void cpu_governor(void)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "CpuGovernor");
    FILE* fp;
    long long int user, nice, sys, idle, iowait, irq, softirq, all, used;
    long long int last_all[8] = { 0 };
    long long int last_used[8] = { 0 };
    long long int gov_runtime_ms = 0;
    long long int get_sampling_time = 0;
    long long int sc_freq_up_time = 0;
    long long int bc_freq_up_time = 0;
    long long int xc_freq_up_time = 0;
    long long int sc_freq_down_time = 0;
    long long int bc_freq_down_time = 0;
    long long int xc_freq_down_time = 0;
    float prev_cpu_usage[8] = { 0 };
    float nowa_cpu_usage;
    float sc_max_usage = 0;
    float bc_max_usage = 0;
    float xc_max_usage = 0;
    float target_level = 0;
    float cpu_load = 0;
    float cpu_total_pwr = 0;
    int comm_limit_pwr = 0;
    int boost_limit_pwr = 0;
    int cur_limit_pwr = 0;
    int total_pwr_ratio = 0;
    int i, cur_cpu_core;
    int gov_sample_ms = 0;
    int sc_limit_pwr = 0;
    int bc_limit_pwr = 0;
    int xc_limit_pwr = 0;
    int sc_over_limit = 0;
    int bc_over_limit = 0;
    int xc_over_limit = 0;
    int sc_perf_margin = 0;
    int bc_perf_margin = 0;
    int xc_perf_margin = 0;
    int target_sc_level = 10;
    int target_bc_level = 10;
    int target_xc_level = 10;
    int prev_target_level = 0;
    int now_sc_level = 1;
    int now_bc_level = 1;
    int now_xc_level = 1;
    int sc_freq_up_delay = 0;
    int bc_freq_up_delay = 0;
    int xc_freq_up_delay = 0;
    int sc_freq_down_delay = 0;
    int bc_freq_down_delay = 0;
    int xc_freq_down_delay = 0;
    int sc_latency_ms[11] = { 0 };
    int bc_latency_ms[11] = { 0 };
    int xc_latency_ms[11] = { 0 };
    int target_latency_ms = 0;
    int sc_cur_latency_ms = 0;
    int bc_cur_latency_ms = 0;
    int xc_cur_latency_ms = 0;
    char buf[1024];
    char now_mode[16];
    while (SCREEN_OFF == 0) {
        if (strcmp(boost, "null") != 0) {
            gov_sample_ms = 10;
        } else {
            gov_sample_ms = 40;
        }
        if (strcmp(mode, now_mode) != 0) {
            // Get Governor Values
            if (strcmp(mode, "powersave") == 0) {
                sc_perf_margin = powersave_conf.sc_perf_margin;
                bc_perf_margin = powersave_conf.bc_perf_margin;
                xc_perf_margin = powersave_conf.xc_perf_margin;
                sc_freq_up_delay = powersave_conf.sc_freq_up_delay;
                bc_freq_up_delay = powersave_conf.bc_freq_up_delay;
                xc_freq_up_delay = powersave_conf.xc_freq_up_delay;
                sc_freq_down_delay = powersave_conf.sc_freq_down_delay;
                bc_freq_down_delay = powersave_conf.bc_freq_down_delay;
                xc_freq_down_delay = powersave_conf.xc_freq_down_delay;
                comm_limit_pwr = powersave_conf.comm_limit_pwr;
                boost_limit_pwr = powersave_conf.boost_limit_pwr;
            } else if (strcmp(mode, "balance") == 0) {
                sc_perf_margin = balance_conf.sc_perf_margin;
                bc_perf_margin = balance_conf.bc_perf_margin;
                xc_perf_margin = balance_conf.xc_perf_margin;
                sc_freq_up_delay = balance_conf.sc_freq_up_delay;
                bc_freq_up_delay = balance_conf.bc_freq_up_delay;
                xc_freq_up_delay = balance_conf.xc_freq_up_delay;
                sc_freq_down_delay = balance_conf.sc_freq_down_delay;
                bc_freq_down_delay = balance_conf.bc_freq_down_delay;
                xc_freq_down_delay = balance_conf.xc_freq_down_delay;
                comm_limit_pwr = balance_conf.comm_limit_pwr;
                boost_limit_pwr = balance_conf.boost_limit_pwr;
            } else if (strcmp(mode, "performance") == 0) {
                sc_perf_margin = performance_conf.sc_perf_margin;
                bc_perf_margin = performance_conf.bc_perf_margin;
                xc_perf_margin = performance_conf.xc_perf_margin;
                sc_freq_up_delay = performance_conf.sc_freq_up_delay;
                bc_freq_up_delay = performance_conf.bc_freq_up_delay;
                xc_freq_up_delay = performance_conf.xc_freq_up_delay;
                sc_freq_down_delay = performance_conf.sc_freq_down_delay;
                bc_freq_down_delay = performance_conf.bc_freq_down_delay;
                xc_freq_down_delay = performance_conf.xc_freq_down_delay;
                comm_limit_pwr = performance_conf.comm_limit_pwr;
                boost_limit_pwr = performance_conf.boost_limit_pwr;
            } else {
                sc_perf_margin = fast_conf.sc_perf_margin;
                bc_perf_margin = fast_conf.bc_perf_margin;
                xc_perf_margin = fast_conf.xc_perf_margin;
                sc_freq_up_delay = fast_conf.sc_freq_up_delay;
                bc_freq_up_delay = fast_conf.bc_freq_up_delay;
                xc_freq_up_delay = fast_conf.xc_freq_up_delay;
                sc_freq_down_delay = fast_conf.sc_freq_down_delay;
                bc_freq_down_delay = fast_conf.bc_freq_down_delay;
                xc_freq_down_delay = fast_conf.xc_freq_down_delay;
                comm_limit_pwr = fast_conf.comm_limit_pwr;
                boost_limit_pwr = fast_conf.boost_limit_pwr;
            }
            // Get Latency MS
            for (i = 0; i <= 10; i++) {
                if (i < expect_sc_level) {
                    sc_latency_ms[i] = sc_freq_up_delay;
                } else {
                    target_latency_ms = (i - expect_sc_level + 1) * 20;
                    if (target_latency_ms > 100) {
                        sc_latency_ms[i] = 100;
                    } else if (target_latency_ms > sc_freq_up_delay) {
                        sc_latency_ms[i] = target_latency_ms;
                    } else {
                        sc_latency_ms[i] = sc_freq_up_delay;
                    }
                }
                if (i < expect_bc_level) {
                    bc_latency_ms[i] = bc_freq_up_delay;
                } else {
                    target_latency_ms = (i - expect_bc_level + 1) * 20;
                    if (target_latency_ms > 100) {
                        bc_latency_ms[i] = 100;
                    } else if (target_latency_ms > bc_freq_up_delay) {
                        bc_latency_ms[i] = target_latency_ms;
                    } else {
                        bc_latency_ms[i] = bc_freq_up_delay;
                    }
                }
                if (i < expect_xc_level) {
                    xc_latency_ms[i] = xc_freq_up_delay;
                } else {
                    target_latency_ms = (i - expect_xc_level + 1) * 20;
                    if (target_latency_ms > 100) {
                        xc_latency_ms[i] = 100;
                    } else if (target_latency_ms > xc_freq_up_delay) {
                        xc_latency_ms[i] = target_latency_ms;
                    } else {
                        xc_latency_ms[i] = xc_freq_up_delay;
                    }
                }
            }
            // Init Governor
            target_sc_level = 0;
            target_bc_level = 0;
            target_xc_level = 0;
            now_sc_level = 0;
            now_bc_level = 0;
            now_xc_level = 0;
            sprintf(now_mode, "%s", mode);
        }
        // Get CPU Usage
        fp = fopen("/proc/stat", "r");
        if (fp) {
            fgets(buf, sizeof(buf), fp);
            for (i = 0; i <= core_num; i++) {
                fgets(buf, sizeof(buf), fp);
                if (strstr(buf, "cpu")) {
                    sscanf(buf, "cpu%d %lld %lld %lld %lld %lld %lld %lld", &cur_cpu_core, &user, &nice, &sys, &idle, &iowait, &irq, &softirq);
                    all = user + nice + sys + idle + iowait + irq + softirq;
                    used = all - idle - iowait;
                    if ((all - last_all[cur_cpu_core]) > 0 && last_all[cur_cpu_core] != 0) {
                        nowa_cpu_usage = (float)(used - last_used[cur_cpu_core]) * 100 / (all - last_all[cur_cpu_core]);
                        cpu_usage[cur_cpu_core] = (prev_cpu_usage[cur_cpu_core] + nowa_cpu_usage) / 2;
                        prev_cpu_usage[cur_cpu_core] = nowa_cpu_usage;
                    } else {
                        cpu_usage[cur_cpu_core] = 0;
                    }
                    last_all[cur_cpu_core] = all;
                    last_used[cur_cpu_core] = used;
                }
            }
            fclose(fp);
        }
        // Get Target Freq Level
        sc_max_usage = 0;
        bc_max_usage = 0;
        xc_max_usage = 0;
        for (i = 0; i <= core_num; i++) {
            if (i < cluster1_cpu || cluster1_cpu == -1) {
                if (cpu_usage[i] > sc_max_usage) {
                    sc_max_usage = cpu_usage[i];
                }
            } else if (i < cluster2_cpu || cluster2_cpu == -1) {
                if (cpu_usage[i] > bc_max_usage) {
                    bc_max_usage = cpu_usage[i];
                }
            } else {
                if (cpu_usage[i] > xc_max_usage) {
                    xc_max_usage = cpu_usage[i];
                }
            }
        }
        cpu_load = (float)xc_max_usage + (100 - xc_max_usage) * governor_boost / 100;
        if (cpu_load > 100) {
            cpu_load = 100;
        }
        if (now_xc_level == 0) {
            now_xc_level = 1;
        }
        target_level = (float)now_xc_level * cpu_load / level_to_load(now_xc_level, xc_perf_margin);
        if (target_level > 10) {
            target_xc_level = 10;
        } else if (target_level >= 1) {
            target_xc_level = (int)target_level;
        } else if (xc_max_usage > 5) {
            target_xc_level = 1;
        }
        cpu_load = (float)bc_max_usage + (100 - bc_max_usage) * governor_boost / 100;
        if (cpu_load > 100) {
            cpu_load = 100;
        }
        if (now_bc_level == 0) {
            now_bc_level = 1;
        }
        target_level = (float)now_bc_level * cpu_load / level_to_load(now_bc_level, bc_perf_margin);
        if (target_level > 10) {
            target_bc_level = 10;
        } else if (target_level >= 1) {
            target_bc_level = (int)target_level;
        } else if (bc_max_usage > 5) {
            target_bc_level = 1;
        }
        cpu_load = (float)sc_max_usage + (100 - sc_max_usage) * governor_boost / 100;
        if (cpu_load > 100) {
            cpu_load = 100;
        }
        if (now_sc_level == 0) {
            now_sc_level = 1;
        }
        target_level = (float)now_sc_level * cpu_load / level_to_load(now_sc_level, sc_perf_margin);
        if (target_level > 10) {
            target_sc_level = 10;
        } else if (target_level >= 1) {
            target_sc_level = (int)target_level;
        } else if (sc_max_usage > 5) {
            target_sc_level = 1;
        }
        // CPU Power Limit
        cpu_total_pwr = sc_pwr_mask[target_sc_level] * sc_core_num + bc_pwr_mask[target_bc_level] * bc_core_num + xc_pwr_mask[target_xc_level] * xc_core_num;
        if (strcmp(boost, "null") != 0) {
            if (cpu_total_pwr > boost_limit_pwr) {
                cur_limit_pwr = boost_limit_pwr;
                sc_limit_pwr = 0;
                bc_limit_pwr = 0;
                xc_limit_pwr = 0;
                if ((sc_pwr_mask[target_sc_level] * sc_core_num) < (boost_limit_pwr * sc_pwr_ratio / 100)) {
                    cur_limit_pwr = cur_limit_pwr - sc_pwr_mask[target_sc_level] * sc_core_num;
                    sc_over_limit = 0;
                } else {
                    sc_over_limit = 1;
                }
                if ((bc_pwr_mask[target_bc_level] * bc_core_num) < (boost_limit_pwr * bc_pwr_ratio / 100)) {
                    cur_limit_pwr = cur_limit_pwr - bc_pwr_mask[target_bc_level] * bc_core_num;
                    bc_over_limit = 0;
                } else {
                    bc_over_limit = 1;
                }
                if ((xc_pwr_mask[target_xc_level] * xc_core_num) < (boost_limit_pwr * xc_pwr_ratio / 100)) {
                    cur_limit_pwr = cur_limit_pwr - xc_pwr_mask[target_xc_level] * xc_core_num;
                    xc_over_limit = 0;
                } else {
                    xc_over_limit = 1;
                }
                total_pwr_ratio = sc_pwr_ratio * sc_over_limit + bc_pwr_ratio * bc_over_limit + xc_pwr_ratio * xc_over_limit;
                if (sc_over_limit == 1) {
                    sc_limit_pwr = cur_limit_pwr * sc_pwr_ratio / total_pwr_ratio;
                    prev_target_level = target_sc_level;
                    target_sc_level = 0;
                    for (i = prev_target_level; i >= 0; i--) {
                        if ((sc_pwr_mask[i] * sc_core_num) <= sc_limit_pwr) {
                            target_sc_level = i;
                            break;
                        }
                    }
                }
                if (bc_over_limit == 1) {
                    bc_limit_pwr = cur_limit_pwr * bc_pwr_ratio / total_pwr_ratio;
                    prev_target_level = target_bc_level;
                    target_bc_level = 0;
                    for (i = prev_target_level; i >= 0; i--) {
                        if ((bc_pwr_mask[i] * bc_core_num) <= bc_limit_pwr) {
                            target_bc_level = i;
                            break;
                        }
                    }
                }
                if (xc_over_limit == 1) {
                    xc_limit_pwr = cur_limit_pwr * xc_pwr_ratio / total_pwr_ratio;
                    prev_target_level = target_xc_level;
                    target_xc_level = 0;
                    for (i = prev_target_level; i >= 0; i--) {
                        if ((xc_pwr_mask[i] * xc_core_num) <= xc_limit_pwr) {
                            target_xc_level = i;
                            break;
                        }
                    }
                }
            }
        } else {
            if (cpu_total_pwr > comm_limit_pwr) {
                cur_limit_pwr = comm_limit_pwr;
                sc_limit_pwr = 0;
                bc_limit_pwr = 0;
                xc_limit_pwr = 0;
                if ((sc_pwr_mask[target_sc_level] * sc_core_num) < (comm_limit_pwr * sc_pwr_ratio / 100)) {
                    cur_limit_pwr = cur_limit_pwr - sc_pwr_mask[target_sc_level] * sc_core_num;
                    sc_over_limit = 0;
                } else {
                    sc_over_limit = 1;
                }
                if ((bc_pwr_mask[target_bc_level] * bc_core_num) < (comm_limit_pwr * bc_pwr_ratio / 100)) {
                    cur_limit_pwr = cur_limit_pwr - bc_pwr_mask[target_bc_level] * bc_core_num;
                    bc_over_limit = 0;
                } else {
                    bc_over_limit = 1;
                }
                if ((xc_pwr_mask[target_xc_level] * xc_core_num) < (comm_limit_pwr * xc_pwr_ratio / 100)) {
                    cur_limit_pwr = cur_limit_pwr - xc_pwr_mask[target_xc_level] * xc_core_num;
                    xc_over_limit = 0;
                } else {
                    xc_over_limit = 1;
                }
                total_pwr_ratio = sc_pwr_ratio * sc_over_limit + bc_pwr_ratio * bc_over_limit + xc_pwr_ratio * xc_over_limit;
                if (sc_over_limit == 1) {
                    sc_limit_pwr = cur_limit_pwr * sc_pwr_ratio / total_pwr_ratio;
                    prev_target_level = target_sc_level;
                    target_sc_level = 0;
                    for (i = prev_target_level; i >= 0; i--) {
                        if ((sc_pwr_mask[i] * sc_core_num) <= sc_limit_pwr) {
                            target_sc_level = i;
                            break;
                        }
                    }
                }
                if (bc_over_limit == 1) {
                    bc_limit_pwr = cur_limit_pwr * bc_pwr_ratio / total_pwr_ratio;
                    prev_target_level = target_bc_level;
                    target_bc_level = 0;
                    for (i = prev_target_level; i >= 0; i--) {
                        if ((bc_pwr_mask[i] * bc_core_num) <= bc_limit_pwr) {
                            target_bc_level = i;
                            break;
                        }
                    }
                }
                if (xc_over_limit == 1) {
                    xc_limit_pwr = cur_limit_pwr * xc_pwr_ratio / total_pwr_ratio;
                    prev_target_level = target_xc_level;
                    target_xc_level = 0;
                    for (i = prev_target_level; i >= 0; i--) {
                        if ((xc_pwr_mask[i] * xc_core_num) <= xc_limit_pwr) {
                            target_xc_level = i;
                            break;
                        }
                    }
                }
            }
        }
        // Get Current Latency MS.
        if (strcmp(boost, "null") == 0) {
            sc_cur_latency_ms = sc_latency_ms[target_sc_level];
            bc_cur_latency_ms = bc_latency_ms[target_bc_level];
            xc_cur_latency_ms = xc_latency_ms[target_xc_level];
        } else {
            sc_cur_latency_ms = 0;
            bc_cur_latency_ms = 0;
            xc_cur_latency_ms = 0;
        }
        // submit CPU Freq
        if (target_xc_level < now_xc_level) {
            if ((gov_runtime_ms - xc_freq_down_time) >= xc_freq_down_delay) {
                pthread_create(&thread_info, NULL, (void*)write_xc_freq, (void*)(long)target_xc_level);
                now_xc_level = target_xc_level;
                xc_freq_down_time = gov_runtime_ms;
            }
        } else if (target_xc_level > now_xc_level) {
            if ((gov_runtime_ms - xc_freq_up_time) >= xc_cur_latency_ms) {
                pthread_create(&thread_info, NULL, (void*)write_xc_freq, (void*)(long)target_xc_level);
                now_xc_level = target_xc_level;
                xc_freq_up_time = gov_runtime_ms;
            }
        }
        if (target_bc_level < now_bc_level) {
            if ((gov_runtime_ms - bc_freq_down_time) >= bc_freq_down_delay) {
                pthread_create(&thread_info, NULL, (void*)write_bc_freq, (void*)(long)target_bc_level);
                now_bc_level = target_bc_level;
                bc_freq_down_time = gov_runtime_ms;
            }
        } else if (target_bc_level > now_bc_level) {
            if ((gov_runtime_ms - bc_freq_up_time) >= bc_cur_latency_ms) {
                pthread_create(&thread_info, NULL, (void*)write_bc_freq, (void*)(long)target_bc_level);
                now_bc_level = target_bc_level;
                bc_freq_up_time = gov_runtime_ms;
            }
        }
        if (target_sc_level < now_sc_level) {
            if ((gov_runtime_ms - sc_freq_down_time) >= sc_freq_down_delay) {
                pthread_create(&thread_info, NULL, (void*)write_sc_freq, (void*)(long)target_sc_level);
                now_sc_level = target_sc_level;
                sc_freq_down_time = gov_runtime_ms;
            }
        } else if (target_sc_level > now_sc_level) {
            if ((gov_runtime_ms - sc_freq_up_time) >= sc_cur_latency_ms) {
                pthread_create(&thread_info, NULL, (void*)write_sc_freq, (void*)(long)target_sc_level);
                now_sc_level = target_sc_level;
                sc_freq_up_time = gov_runtime_ms;
            }
        }
        gov_runtime_ms += gov_sample_ms;
        usleep(gov_sample_ms * 1000);
    }
    pthread_exit(0);
}
void boost_timer(void)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "BoostTimer");
    while (boost_duration_ms > 0) {
        boost_duration_ms--;
        usleep(1000);
    }
    governor_boost = no_boost.gov_boost;
    sprintf(boost, "none");
    pthread_exit(0);
}
void submit_hint(char hint[32])
{
    int prev_boost_duration_ms = boost_duration_ms;
    if (strcmp(hint, "touch") == 0) {
        if (strcmp(touch_hint.boost_type, "touch") == 0) {
            if (strcmp(boost, "none") == 0 || strcmp(boost, "touch") == 0) {
                sprintf(boost, "%s", touch_hint.boost_type);
                boost_duration_ms = touch_hint.boost_duration;
            }
        } else if (strcmp(touch_hint.boost_type, "swipe") == 0) {
            if (strcmp(boost, "none") == 0 || strcmp(boost, "touch") == 0 || strcmp(boost, "swipe") == 0) {
                sprintf(boost, "%s", touch_hint.boost_type);
                boost_duration_ms = touch_hint.boost_duration;
            }
        } else if (strcmp(touch_hint.boost_type, "gesture") == 0) {
            if (strcmp(boost, "none") == 0 || strcmp(boost, "touch") == 0 || strcmp(boost, "swipe") == 0 || strcmp(boost, "gesture") == 0) {
                sprintf(boost, "%s", touch_hint.boost_type);
                boost_duration_ms = touch_hint.boost_duration;
            }
        } else if (strcmp(touch_hint.boost_type, "heavyload") == 0) {
            sprintf(boost, "%s", touch_hint.boost_type);
            boost_duration_ms = touch_hint.boost_duration;
        }
    } else if (strcmp(hint, "swipe") == 0) {
        if (strcmp(swipe_hint.boost_type, "touch") == 0) {
            if (strcmp(boost, "none") == 0 || strcmp(boost, "touch") == 0) {
                sprintf(boost, "%s", swipe_hint.boost_type);
                boost_duration_ms = swipe_hint.boost_duration;
            }
        } else if (strcmp(swipe_hint.boost_type, "swipe") == 0) {
            if (strcmp(boost, "none") == 0 || strcmp(boost, "touch") == 0 || strcmp(boost, "swipe") == 0) {
                sprintf(boost, "%s", swipe_hint.boost_type);
                boost_duration_ms = swipe_hint.boost_duration;
            }
        } else if (strcmp(swipe_hint.boost_type, "gesture") == 0) {
            if (strcmp(boost, "none") == 0 || strcmp(boost, "touch") == 0 || strcmp(boost, "swipe") == 0 || strcmp(boost, "gesture") == 0) {
                sprintf(boost, "%s", swipe_hint.boost_type);
                boost_duration_ms = swipe_hint.boost_duration;
            }
        } else if (strcmp(swipe_hint.boost_type, "heavyload") == 0) {
            sprintf(boost, "%s", swipe_hint.boost_type);
            boost_duration_ms = swipe_hint.boost_duration;
        }
    } else if (strcmp(hint, "gesture") == 0) {
        if (strcmp(gesture_hint.boost_type, "touch") == 0) {
            if (strcmp(boost, "none") == 0 || strcmp(boost, "touch") == 0) {
                sprintf(boost, "%s", gesture_hint.boost_type);
                boost_duration_ms = gesture_hint.boost_duration;
            }
        } else if (strcmp(gesture_hint.boost_type, "swipe") == 0) {
            if (strcmp(boost, "none") == 0 || strcmp(boost, "touch") == 0 || strcmp(boost, "swipe") == 0) {
                sprintf(boost, "%s", gesture_hint.boost_type);
                boost_duration_ms = gesture_hint.boost_duration;
            }
        } else if (strcmp(gesture_hint.boost_type, "gesture") == 0) {
            if (strcmp(boost, "none") == 0 || strcmp(boost, "touch") == 0 || strcmp(boost, "swipe") == 0 || strcmp(boost, "gesture") == 0) {
                sprintf(boost, "%s", gesture_hint.boost_type);
                boost_duration_ms = gesture_hint.boost_duration;
            }
        } else if (strcmp(gesture_hint.boost_type, "heavyload") == 0) {
            sprintf(boost, "%s", gesture_hint.boost_type);
            boost_duration_ms = gesture_hint.boost_duration;
        }
    } else if (strcmp(hint, "topActivityChanged") == 0) {
        if (strcmp(top_activity_changed.boost_type, "touch") == 0) {
            if (strcmp(boost, "none") == 0 || strcmp(boost, "touch") == 0) {
                sprintf(boost, "%s", top_activity_changed.boost_type);
                boost_duration_ms = top_activity_changed.boost_duration;
            }
        } else if (strcmp(top_activity_changed.boost_type, "swipe") == 0) {
            if (strcmp(boost, "none") == 0 || strcmp(boost, "touch") == 0 || strcmp(boost, "swipe") == 0) {
                sprintf(boost, "%s", top_activity_changed.boost_type);
                boost_duration_ms = top_activity_changed.boost_duration;
            }
        } else if (strcmp(top_activity_changed.boost_type, "gesture") == 0) {
            if (strcmp(boost, "none") == 0 || strcmp(boost, "touch") == 0 || strcmp(boost, "swipe") == 0 || strcmp(boost, "gesture") == 0) {
                sprintf(boost, "%s", top_activity_changed.boost_type);
                boost_duration_ms = top_activity_changed.boost_duration;
            }
        } else if (strcmp(top_activity_changed.boost_type, "heavyload") == 0) {
            sprintf(boost, "%s", top_activity_changed.boost_type);
            boost_duration_ms = top_activity_changed.boost_duration;
        }
    }
    if (prev_boost_duration_ms == 0) {
        pthread_create(&thread_info, NULL, (void*)boost_timer, NULL);
    }
}
void taskset_helper(long int pid)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "TasksetHelper");
    FILE* fp;
    DIR* dir;
    cpu_set_t efficiency_mask;
    cpu_set_t multi_perf_mask;
    cpu_set_t single_perf_mask;
    cpu_set_t comm_mask;
    cpu_set_t other_mask;
    struct sched_param efficiency_prio;
    struct sched_param comm_prio;
    struct sched_param other_prio;
    struct sched_param multi_perf_prio;
    struct sched_param single_perf_prio;
    struct dirent* entry;
    char buf[256];
    char tid_dir[128];
    char tid_url[128];
    char tid_name[32];
    long long int prev_task_runtime[256] = { 0 };
    long long int nowa_task_runtime = 0;
    long int cur_task_runtime = 0;
    long int max_task_runtime = 0;
    int heavy_load_task = -1;
    int tid, i;
    int single_perf_tasks[256] = { 0 };
    int single_perf_task_num = 0;
    CPU_ZERO(&efficiency_mask);
    for (i = normal_sched.efficiency_cpu_start; i <= normal_sched.efficiency_cpu_end; i++) {
        CPU_SET(i, &efficiency_mask);
    }
    CPU_ZERO(&comm_mask);
    for (i = normal_sched.common_cpu_start; i <= normal_sched.common_cpu_end; i++) {
        CPU_SET(i, &comm_mask);
    }
    CPU_ZERO(&other_mask);
    for (i = normal_sched.other_cpu_start; i <= normal_sched.other_cpu_end; i++) {
        CPU_SET(i, &other_mask);
    }
    CPU_ZERO(&multi_perf_mask);
    for (i = normal_sched.multi_perf_cpu_start; i <= normal_sched.multi_perf_cpu_end; i++) {
        CPU_SET(i, &multi_perf_mask);
    }
    CPU_ZERO(&single_perf_mask);
    for (i = normal_sched.single_perf_cpu_start; i <= normal_sched.single_perf_cpu_end; i++) {
        CPU_SET(i, &single_perf_mask);
    }
    efficiency_prio.sched_priority = normal_sched.efficiency_sched_prio;
    comm_prio.sched_priority = normal_sched.common_sched_prio;
    other_prio.sched_priority = normal_sched.other_sched_prio;
    multi_perf_prio.sched_priority = normal_sched.multi_perf_sched_prio;
    single_perf_prio.sched_priority = normal_sched.single_perf_sched_prio;
    sprintf(tid_dir, "/proc/%ld/task/", pid);
    dir = opendir(tid_dir);
    if (dir != NULL) {
        single_perf_task_num = 0;
        while ((entry = readdir(dir)) != NULL) {
            sscanf((*entry).d_name, "%d", &tid);
            sprintf(tid_url, "%s%d/comm", tid_dir, tid);
            fp = fopen(tid_url, "r");
            if (fp != NULL) {
                memset(tid_name, 0, sizeof(tid_name));
                fgets(tid_name, sizeof(tid_name), fp);
                fclose(fp);
                reset_task_nice(tid);
                if (tid == pid) {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &multi_perf_prio);
                } else if (check_regex(tid_name, "^(UnityMain|MainThread-UE4|GameThread|SDLThread|RenderThread|MINECRAFT|GLThread|Thread-)")) {
                    single_perf_task_num++;
                    single_perf_tasks[single_perf_task_num] = tid;
                } else if (strstr(tid_name, "^(UnityMulti|UnityPreload|UnityChoreograp|UnityGfx|Worker Thread)")) {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &multi_perf_prio);
                } else if (check_regex(tid_name, "^(LoadingThread|RHIThread|FrameThread|Job.Worker|CmpJob|TaskGraphNP|glp)")) {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &multi_perf_prio);
                } else if (check_regex(tid_name, "^(JNISurfaceText|IJK_External|ForkJoinPool-|UiThread|AndroidUI|RenderEngine|[.]raster|Compositor)")) {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &multi_perf_prio);
                } else if (check_regex(tid_name, "^([Gg]esture|.gifmaker|Binder|mali-|[Aa]sync|[Vv]sync|android.anim|android.ui|[Bb]lur|[Aa]nim|Chrome_|Viz)")) {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &multi_perf_prio);
                } else if (check_regex(tid_name, "^(Chrome_InProc|Chromium|Gecko|[Ww]eb[Vv]iew|[Jj]ava[Ss]cript|js|JS|android.fg|android.io)")) {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &comm_prio);
                } else if (check_regex(tid_name, "^(CrGpuMain|CrRendererMain|work_thread|NativeThread|[Dd]ownload|[Mm]ixer|[Aa]udio|[Vv]ideo)")) {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &comm_prio);
                } else if (check_regex(tid_name, "^(OkHttp|ThreadPool|PoolThread|glide-|pool-|launcher-|Fresco)")) {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &comm_prio);
                } else if (check_regex(tid_name, "^(SearchDaemon|Profile|ged-swd|GPU completion|FramePolicy|ScrollPolicy|HeapTaskDaemon|FinalizerDaemon)")) {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &efficiency_prio);
                } else if (check_regex(tid_name, "^(ReferenceQueue|Jit thread pool|Timer-|log|xcrash|Ysa|Xqa|Rx|APM|TVKDL-|tp-|cgi-|ODCP-|xlog_|[Bb]ugly|android.bg|SensorService)")) {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &efficiency_prio);
                } else if (check_regex(tid_name, "^(HealthService|[Bb]ackground|[Rr]eport|tt_pangle|xg_vip_service|default_matrix|FrameDecoder|FrameSeq|hwui)")) {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &efficiency_prio);
                } else {
                    sched_setaffinity(tid, sizeof(other_mask), &other_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &other_prio);
                }
            }
        }
        closedir(dir);
    }
    while (taskset_tid == pthread_self()) {
        max_task_runtime = 0;
        for (i = 1; i <= single_perf_task_num; i++) {
            nowa_task_runtime = get_thread_runtime((int)pid, single_perf_tasks[i]);
            cur_task_runtime = nowa_task_runtime - prev_task_runtime[i];
            prev_task_runtime[i] = nowa_task_runtime;
            if (cur_task_runtime > max_task_runtime) {
                heavy_load_task = single_perf_tasks[i];
                max_task_runtime = cur_task_runtime;
            }
        }
        for (i = 1; i <= single_perf_task_num; i++) {
            if (single_perf_tasks[i] != heavy_load_task) {
                sched_setaffinity(single_perf_tasks[i], sizeof(multi_perf_mask), &multi_perf_mask);
                sched_setscheduler(single_perf_tasks[i], SCHED_NORMAL, &multi_perf_prio);
            } else {
                sched_setaffinity(single_perf_tasks[i], sizeof(single_perf_mask), &single_perf_mask);
                sched_setscheduler(single_perf_tasks[i], SCHED_NORMAL, &single_perf_prio);
            }
        }
        sleep(1);
    }
    pthread_exit(0);
}
void get_standby_state()
{
    FILE* fp;
    char buf[128];
    int task_num = 0;
    fp = fopen("/dev/cpuset/restricted/tasks", "r");
    if (fp) {
        while (fgets(buf, sizeof(buf), fp) != NULL) {
            task_num++;
            if (task_num > 10) {
                break;
            }
        }
        fclose(fp);
    }
    if (task_num > 10) {
        if (SCREEN_OFF == 0) {
            sprintf(boost, "none");
            boost_duration_ms = 0;
            taskset_tid = 0;
            SCREEN_OFF = 1;
            usleep(500000);
            write_freq(0, 10, 0, 10, 0, 10);
        }
    } else {
        if (SCREEN_OFF == 1) {
            set_boost_affinity(get_task_pid("/system/bin/surfaceflinger"));
            set_boost_affinity(get_task_pid("system_server"));
            set_boost_affinity(get_task_pid("com.android.systemui"));
            SCREEN_OFF = 0;
            pthread_create(&thread_info, NULL, (void*)cpu_governor, NULL);
            submit_hint("topActivityChanged");
        }
    }
}
void cgroup_listener(void)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "CgroupListener");
    struct inotify_event* watch_event;
    char buf[1024];
    char prev_pkg_name[256];
    char nowa_pkg_name[256];
    int prev_app_pid = -1;
    int nowa_app_pid = -1;
    int fd, ta_wd, fg_wd, bg_wd, re_wd;
    fd = inotify_init();
    if (fd < 0) {
        write_log("[E] CgroupListener: Failed to init inotify.");
        pthread_exit(0);
    }
    ta_wd = inotify_add_watch(fd, "/dev/cpuset/top-app/tasks", IN_MODIFY);
    if (ta_wd < 0) {
        write_log("[W] CgroupListener: Failed to open top-app cgroup.");
    }
    fg_wd = inotify_add_watch(fd, "/dev/cpuset/foreground/tasks", IN_MODIFY);
    if (fg_wd < 0) {
        write_log("[W] CgroupListener: Failed to open foreground cgroup.");
    }
    bg_wd = inotify_add_watch(fd, "/dev/cpuset/background/tasks", IN_MODIFY);
    if (bg_wd < 0) {
        write_log("[W] CgroupListener: Failed to open background cgroup.");
    }
    re_wd = inotify_add_watch(fd, "/dev/cpuset/restricted/tasks", IN_MODIFY);
    if (re_wd < 0) {
        write_log("[W] CgroupListener: Failed to open restricted cgroup.");
    }
    while (1) {
        read(fd, buf, sizeof(buf));
        watch_event = (struct inotify_event*)buf;
        if ((*watch_event).mask == IN_MODIFY) {
            if (SCREEN_OFF == 0) {
                sprintf(prev_pkg_name, "%s", nowa_pkg_name);
                prev_app_pid = nowa_app_pid;
                sscanf(str_cut(get_top_activity(), 9), "%d:%s", &nowa_app_pid, nowa_pkg_name);
                if (strstr(nowa_pkg_name, "systemui")) {
                    // Ignore Android SystemUI.
                    sprintf(nowa_pkg_name, "%s", prev_pkg_name);
                }
                if (strcmp(nowa_pkg_name, prev_pkg_name) != 0 || nowa_app_pid != prev_app_pid) {
                    submit_hint("topAtivityChanged");
                    set_boost_affinity(nowa_app_pid);
                    usleep(top_activity_changed.boost_duration * 1000);
                }
                pthread_create(&taskset_tid, NULL, (void*)taskset_helper, (void*)(long)nowa_app_pid);
            }
            get_standby_state();
        }
        usleep(100000);
    }
    inotify_rm_watch(fd, ta_wd);
    inotify_rm_watch(fd, fg_wd);
    inotify_rm_watch(fd, bg_wd);
    inotify_rm_watch(fd, re_wd);
    close(fd);
    pthread_exit(0);
}
void input_listener(long int ts_event)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "InputListener");
    struct input_event ts;
    struct input_absinfo ts_x_info;
    struct input_absinfo ts_y_info;
    char touch_screen_url[128];
    int start_x = 0;
    int start_y = 0;
    int end_x = 0;
    int end_y = 0;
    int swipe_x = 0;
    int swipe_y = 0;
    int touch_x = 0;
    int touch_y = 0;
    int swipe_range = 0;
    int gesture_left = 0;
    int gesture_right = 0;
    int gesture_top = 0;
    int gesture_bottom = 0;
    int touch_s = 0;
    int last_s = 0;
    int ret;
    sprintf(touch_screen_url, "/dev/input/event%ld", ts_event);
    int fd = open(touch_screen_url, O_RDONLY);
    if (fd < 0) {
        write_log("[E] InputListener: Failed to open %s.", touch_screen_url);
        pthread_exit(0);
    }
    ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &ts_x_info);
    ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &ts_y_info);
    if (ts_y_info.maximum > MAX_INT || ts_x_info.maximum > MAX_INT) {
        write_log("[E] InputListener: TouchScreen Position too large.");
        pthread_exit(0);
    } else {
        write_log("[I] InputListener: listening %s, absinfo: x=%d~%d, y=%d~%d. ", touch_screen_url, ts_x_info.minimum, ts_x_info.maximum, ts_y_info.minimum, ts_y_info.maximum);
    }
    swipe_range = ts_x_info.maximum / 10;
    gesture_left = swipe_range;
    gesture_right = ts_x_info.maximum - swipe_range;
    gesture_top = swipe_range;
    gesture_bottom = ts_y_info.maximum - swipe_range;
    while (1) {
        ret = read(fd, &ts, sizeof(ts));
        if (ret == -1) {
            write_log("[W] InputListener: Failed to get touchScreen input.");
            pthread_exit(0);
        }
        last_s = touch_s;
        if (ts.code == ABS_MT_POSITION_X) {
            touch_x = ts.value;
        }
        if (ts.code == ABS_MT_POSITION_Y) {
            touch_y = ts.value;
        }
        if (ts.code == BTN_TOUCH) {
            touch_s = ts.value;
        }
        if ((touch_s - last_s) == 1) {
            start_x = touch_x;
            start_y = touch_y;
            submit_hint("touch");
        } else if ((touch_s - last_s) == 0 && touch_s == 1) {
            swipe_x = touch_x - start_x;
            swipe_y = touch_y - start_y;
            if (abs(swipe_y) > swipe_range || abs(swipe_x) > swipe_range) {
                submit_hint("swipe");
            }
        } else if ((touch_s - last_s) == -1) {
            end_x = touch_x;
            end_y = touch_y;
            swipe_x = end_x - start_x;
            swipe_y = end_y - start_y;
            if (start_x > gesture_right && abs(swipe_x) > swipe_range) {
                submit_hint("gesture");
            } else if (start_x < gesture_left && abs(swipe_x) > swipe_range) {
                submit_hint("gesture");
            } else if (start_y < gesture_top && abs(swipe_y) > swipe_range) {
                submit_hint("gesture");
            } else if (start_y > gesture_bottom && abs(swipe_y) > swipe_range) {
                submit_hint("gesture");
            } else {
                submit_hint("touch");
            }
        }
    }
    close(fd);
}
void start_input_listener(void)
{
    char abs_bitmask[(ABS_MAX + 1) / 8] = { 0 };
    char buf[64];
    char input_url[64];
    DIR* dir = NULL;
    struct dirent* entry;
    int event;
    int fd;
    dir = opendir("/dev/input");
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            sprintf(input_url, "/dev/input/%s", (*entry).d_name);
            fd = open(input_url, O_RDONLY);
            if (fd > 0) {
                memset(abs_bitmask, 0, sizeof(abs_bitmask));
                ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
                if (check_bit(ABS_MT_POSITION_X, abs_bitmask) && check_bit(ABS_MT_POSITION_Y, abs_bitmask)) {
                    sscanf(input_url, "/dev/input/event%d", &event);
                    pthread_create(&thread_info, NULL, (void*)input_listener, (void*)(long)event);
                }
                close(fd);
            }
        }
    }
    closedir(dir);
}
int freq_writer_test()
{
    /*  freqwriter: 1.qti_freq_writer 2.exynos_freq_writer 3.mtk_freq_writer
              4.userspace_freq_writer 5.eas_freq_writer 6.hmp_freq_writer
              Target: freq_switch_ms <= 10ms(100hz). */
    long int current_freq0, current_freq1, current_freq2;
    // Try QTI CpuFreqWriter
    write_freq(10, 10, 10, 10, 10, 10);
    usleep(10000);
    qti_freq_writer(0, cluster0_freq_table[2]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq"), "%ld", &current_freq0);
    qti_freq_writer(0, cluster0_freq_table[4]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq"), "%ld", &current_freq1);
    qti_freq_writer(0, cluster0_freq_table[6]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq"), "%ld", &current_freq2);
    if (current_freq0 == cluster0_freq_table[2] && current_freq1 == cluster0_freq_table[4] && current_freq2 == cluster0_freq_table[6]) {
        freq_writer_type = 1;
        write_log("[I] Using QTI CpuFreqWriter.");
        return 0;
    }
    // Try Exynos CpuFreqWriter
    write_freq(10, 10, 10, 10, 10, 10);
    usleep(10000);
    exynos_freq_writer(0, cluster0_freq_table[2]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq"), "%ld", &current_freq0);
    exynos_freq_writer(0, cluster0_freq_table[4]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq"), "%ld", &current_freq1);
    exynos_freq_writer(0, cluster0_freq_table[6]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq"), "%ld", &current_freq2);
    if (current_freq0 == cluster0_freq_table[2] && current_freq1 == cluster0_freq_table[4] && current_freq2 == cluster0_freq_table[6]) {
        freq_writer_type = 2;
        write_log("[I] Using Exynos CpuFreqWriter.");
        return 0;
    }
    // Try MTK CpuFreqWriter
    write_freq(10, 10, 10, 10, 10, 10);
    usleep(10000);
    mtk_freq_writer(0, cluster0_freq_table[2]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq"), "%ld", &current_freq0);
    mtk_freq_writer(0, cluster0_freq_table[4]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq"), "%ld", &current_freq1);
    mtk_freq_writer(0, cluster0_freq_table[6]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq"), "%ld", &current_freq2);
    if (current_freq0 == cluster0_freq_table[2] && current_freq1 == cluster0_freq_table[4] && current_freq2 == cluster0_freq_table[6]) {
        freq_writer_type = 3;
        write_log("[I] Using MTK CpuFreqWriter.");
        return 0;
    }
    // Try EAS CpuFreqWriter
    write_freq(10, 10, 10, 10, 10, 10);
    usleep(10000);
    eas_freq_writer(0, cluster0_freq_table[2]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq"), "%ld", &current_freq0);
    eas_freq_writer(0, cluster0_freq_table[4]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq"), "%ld", &current_freq1);
    eas_freq_writer(0, cluster0_freq_table[6]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq"), "%ld", &current_freq2);
    if (current_freq0 == cluster0_freq_table[2] && current_freq1 == cluster0_freq_table[4] && current_freq2 == cluster0_freq_table[6]) {
        freq_writer_type = 5;
        write_log("[I] Using EAS CpuFreqWriter.");
        return 0;
    }
    // Try HMP CpuFreqWriter
    write_freq(10, 10, 10, 10, 10, 10);
    usleep(10000);
    hmp_freq_writer(0, cluster0_freq_table[2]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"), "%ld", &current_freq0);
    hmp_freq_writer(0, cluster0_freq_table[4]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"), "%ld", &current_freq1);
    hmp_freq_writer(0, cluster0_freq_table[6]);
    usleep(10000);
    sscanf(read_value("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"), "%ld", &current_freq2);
    if (current_freq0 == cluster0_freq_table[2] && current_freq1 == cluster0_freq_table[4] && current_freq2 == cluster0_freq_table[6]) {
        freq_writer_type = 6;
        write_log("[I] Using HMP CpuFreqWriter.");
        return 0;
    }
    freq_writer_type = 6;
    write_freq(0, 10, 0, 10, 0, 10);
    write_log("[W] CpuFreqWriter may be unstable.");
    write_log("[I] Using Default CpuFreqWriter.");
    return 0;
}
int main(int argc, char* argv[])
{
    FILE* fp;
    char cur_mode[16];
    char buf[1024];
    struct inotify_event* watch_event;
    int fd, wd;
    int i;
    sprintf(path, "%s", argv[0]);
    sprintf(argv[0], "CuDaemon");
    for (i = strlen(path); i > 0; i--) {
        if (path[i] == '/') {
            path[i] = '\0';
            break;
        }
    }
    if (argc < 2) {
        printf("Wrong input.\n");
        printf("Try CuDaemon --help\n");
        exit(0);
    } else if (strstr(argv[1], "-R")) {
        if (argc == 5) {
            sprintf(config_url, "%s", argv[2]);
            sprintf(mode_url, "%s", argv[3]);
            sprintf(log_url, "%s", argv[4]);
            printf("Daemon Running.\n");
            daemon(0, 0);
        } else {
            printf("Wrong input.\n");
            printf("Try CuDaemon --help\n");
            exit(0);
        }
    } else if (strstr(argv[1], "-V")) {
        printf("CuprumTurbo Scheduler Daemon\n");
        printf("Version: 12\n");
        printf("Author: chenzyadb(chenzyyzd)\n");
        printf("Repo: https://github.com/chenzyyzd/CuprumTurbo-Scheduler\n");
        exit(0);
    } else if (strstr(argv[1], "--help")) {
        printf("CuDaemon Helper\n");
        printf("Options:                      Usage:\n");
        printf("-R [config] [mode] [log]      Run CuprumTurbo Scheduler\n");
        printf("-V                            Show CuDaemon Version\n");
        printf("--help                        Show CuDaemon Helper\n");
        exit(0);
    } else {
        printf("Wrong input.\n");
        printf("Try CuDaemon --help\n");
        exit(0);
    }
    init_log();
    write_log("[I] Initizalizing.");
    if (access("/sys/devices/system/cpu/cpu7/cpufreq/scaling_cur_freq", 0) != -1) {
        core_num = 7;
    } else if (access("/sys/devices/system/cpu/cpu5/cpufreq/scaling_cur_freq", 0) != -1) {
        core_num = 5;
    } else if (access("/sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq", 0) != -1) {
        core_num = 3;
    }
    get_cpu_clusters();
    write_log("[I] cluster0=cpu%d, cluster1=cpu%d, cluster2=cpu%d. ", cluster0_cpu, cluster1_cpu, cluster2_cpu);
    get_core_num();
    get_config();
    write_log("[I] CPU Cluster0 PowerModel:");
    for (i = 0; i <= 10; i++) {
        write_log("[I] Idx=%d, Freq=%ld MHz, Power=%d mW.", i, cluster0_freq_table[i] / 1000, sc_pwr_mask[i]);
    }
    if (cluster1_cpu != -1) {
        write_log("[I] CPU Cluster1 PowerModel:");
        for (i = 0; i <= 10; i++) {
            write_log("[I] Idx=%d, Freq=%ld MHz, Power=%d mW.", i, cluster1_freq_table[i] / 1000, bc_pwr_mask[i]);
        }
    }
    if (cluster2_cpu != -1) {
        write_log("[I] CPU Cluster2 PowerModel:");
        for (i = 0; i <= 10; i++) {
            write_log("[I] Idx=%d, Freq=%ld MHz, Power=%d mW.", i, cluster2_freq_table[i] / 1000, xc_pwr_mask[i]);
        }
    }
    write_log("[I] cpu_type=%d+%d+%d, cpu_pwr_ratio=%d/%d/%d.", sc_core_num, bc_core_num, xc_core_num, sc_pwr_ratio,
        bc_pwr_ratio, xc_pwr_ratio);
    set_boost_affinity(get_task_pid("/system/bin/surfaceflinger"));
    set_boost_affinity(get_task_pid("system_server"));
    set_boost_affinity(get_task_pid("com.android.systemui"));
    freq_writer_test();
    start_input_listener();
    pthread_create(&thread_info, NULL, (void*)cpu_governor, NULL);
    pthread_create(&thread_info, NULL, (void*)cgroup_listener, NULL);
    usleep(500000);
    write_log("[I] CuDaemon(pid=%d) is running. ", getpid());
    prctl(PR_SET_NAME, "ModeWatcher");
    fd = inotify_init();
    if (fd < 0) {
        write_log("[E] CuDaemon: Failed to init inotify.");
        exit(0);
    }
    wd = inotify_add_watch(fd, mode_url, IN_MODIFY);
    if (wd < 0) {
        write_log("[E] CuDaemon: Failed to open %s.", mode_url);
        exit(0);
    }
    sprintf(mode, "null");
    sscanf(read_value(mode_url), "%s", cur_mode);
    if (strcmp(cur_mode, mode) != 0) {
        write_log("[I] mode switching %s -> %s.", mode, cur_mode);
        sprintf(mode, "%s", cur_mode);
    }
    while (1) {
        read(fd, buf, sizeof(buf));
        watch_event = (struct inotify_event*)buf;
        if ((*watch_event).mask == IN_MODIFY) {
            sscanf(read_value(mode_url), "%s", cur_mode);
            if (strcmp(cur_mode, mode) != 0) {
                write_log("[I] mode switching %s -> %s.", mode, cur_mode);
                sprintf(mode, "%s", cur_mode);
            }
            usleep(500000);
        }
    }
    inotify_rm_watch(fd, wd);
    close(fd);
    return 0;
}