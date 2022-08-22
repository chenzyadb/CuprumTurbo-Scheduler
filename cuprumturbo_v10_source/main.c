// CuprumTurbo V10
#define check_bit(bit, array) (array[bit / 8] & (1 << (bit % 8)))
#define __USE_GNU
#include <arpa/inet.h>
#include <dirent.h>
#include <linux/input.h>
#include <math.h>
#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
pthread_t thread_info;
cpu_set_t efficiency_mask;
cpu_set_t multi_perf_mask;
cpu_set_t single_perf_mask;
cpu_set_t all_perf_mask;
cpu_set_t comm_mask;
cpu_set_t all_mask;
char path[256];
char mode[16];
long long int sampling_time = 0;
float cpu_usage[8] = {0};
int foreground_pid = -1;
int core_num = 0;
int hint_level = 0;
int SCREEN_OFF = 0;
int cpu_perf_mask[8] = {0};
int sample_time = 20000;
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
long int cluster0_freq_table[11] = {0};
long int cluster1_freq_table[11] = {0};
long int cluster2_freq_table[11] = {0};
int sc_pwr_mask[11] = {0};
int bc_pwr_mask[11] = {0};
int xc_pwr_mask[11] = {0};
int cluster0_cpu = -1;
int cluster1_cpu = -1;
int cluster2_cpu = -1;
int freq_writer_type = -1;
void write_log(const char *format, ...)
{
    FILE *fp;
    fp = fopen("/storage/emulated/0/Android/data/xyz.chenzyadb.cu_toolbox/files/Cuprum_Log.txt", "a");
    va_list arg;
    va_start(arg, format);
    time_t time_log = time(NULL);
    struct tm *tm_log = localtime(&time_log);
    fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d ", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday,
            tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec);
    vfprintf(fp, format, arg);
    va_end(arg);
    fprintf(fp, "\n");
    fflush(fp);
    fclose(fp);
}
void run_libcuprum(char command[64])
{
    char shell_buf[256];
    sprintf(shell_buf, "%s/libcuprum.sh \"%s\" ", path, command);
    system(shell_buf);
}
int get_thread_cpu_usage(int tid)
{
    FILE *fp;
    char thread_stat_url[64];
    char buf[1024];
    char comm[128];
    char state[1];
    long long int user, nice, sys, idle, iowait, irq, softirq, all, used;
    long long int c, thread_time0, all_time0, thread_time1, all_time1;
    long int a, utime, stime, cutime, cstime;
    int fd, b;
    int thread_cpu_usage = 0;
    sprintf(thread_stat_url, "/proc/%d/stat", tid);
    fd = open(thread_stat_url, O_RDONLY);
    if (fd > 0)
    {
        read(fd, buf, sizeof(buf));
        sscanf(buf,
               "%d %s %s %d %d %d %d %d %d %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %d 0 %lld %ld %ld %ld %ld %ld %ld "
               "%ld %ld %ld %ld %ld %ld %ld %ld %ld %d %d %d %d %lld %ld %ld",
               &b, comm, state, &b, &b, &b, &b, &b, &b, &a, &a, &a, &a, &utime, &stime, &cutime, &cstime, &a, &a, &b,
               &c, &a, &a, &a, &a, &a, &a, &a, &a, &a, &a, &a, &a, &a, &a, &a, &b, &b, &b, &b, &c, &a, &a);
        close(fd);
    }
    thread_time0 = utime + stime + cutime + cstime;
    fp = fopen("/proc/stat", "r");
    if (fp != NULL)
    {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "cpu %lld %lld %lld %lld %lld %lld %lld", &user, &nice, &sys, &idle, &iowait, &irq, &softirq);
        fclose(fp);
    }
    all_time0 = user + nice + sys + idle + iowait + irq + softirq;
    usleep(200000);
    fd = open(thread_stat_url, O_RDONLY);
    if (fd > 0)
    {
        read(fd, buf, sizeof(buf));
        sscanf(buf,
               "%d %s %s %d %d %d %d %d %d %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %d 0 %lld %ld %ld %ld %ld %ld %ld "
               "%ld %ld %ld %ld %ld %ld %ld %ld %ld %d %d %d %d %lld %ld %ld",
               &b, comm, state, &b, &b, &b, &b, &b, &b, &a, &a, &a, &a, &utime, &stime, &cutime, &cstime, &a, &a, &b,
               &c, &a, &a, &a, &a, &a, &a, &a, &a, &a, &a, &a, &a, &a, &a, &a, &b, &b, &b, &b, &c, &a, &a);
        close(fd);
    }
    thread_time1 = utime + stime + cutime + cstime;
    fp = fopen("/proc/stat", "r");
    if (fp != NULL)
    {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "cpu %lld %lld %lld %lld %lld %lld %lld", &user, &nice, &sys, &idle, &iowait, &irq, &softirq);
        fclose(fp);
    }
    all_time1 = user + nice + sys + idle + iowait + irq + softirq;
    thread_cpu_usage = (thread_time1 - thread_time0) * (core_num + 1) * 100 / (all_time1 - all_time0);
    return thread_cpu_usage;
}
void get_config(void)
{
    FILE *fp;
    int sc_volt_table[11] = {0};
    int bc_volt_table[11] = {0};
    int xc_volt_table[11] = {0};
    int cpu_pwr_ratio = 0;
    int i, min_diff, diff, cur_freq;
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
    int sc_power_mw = 0;
    int bc_power_mw = 0;
    int xc_power_mw = 0;
    int sc_power_cost = 0;
    int bc_power_cost = 0;
    int xc_power_cost = 0;
    int sc_efficiency = 0;
    int bc_efficiency = 0;
    int xc_efficiency = 0;
    char buf[128];
    char config_url[128];
    char platform_name[32];
    char config_name[32];
    sprintf(config_url, "%s/config.txt", path);
    fp = popen("getprop ro.board.platform", "r");
    fgets(buf, sizeof(buf), fp);
    sscanf(buf, "%s", platform_name);
    pclose(fp);
    fp = fopen(config_url, "r");
    config_exist = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        if (config_exist == 0)
        {
            sscanf(buf, "platform=%s", config_name);
            if (strstr(platform_name, config_name) && strstr(buf, "platform="))
            {
                write_log("[I] platform %s supported, loading custom config.", platform_name);
                config_exist = 1;
            }
        }
        else if (config_exist == 1)
        {
            sscanf(buf, "sc_efficiency=%d", &sc_efficiency);
            sscanf(buf, "bc_efficiency=%d", &bc_efficiency);
            sscanf(buf, "xc_efficiency=%d", &xc_efficiency);
            sscanf(buf, "sc_basic_freq_mhz=%d", &sc_basic_freq_mhz);
            sscanf(buf, "bc_basic_freq_mhz=%d", &bc_basic_freq_mhz);
            sscanf(buf, "xc_basic_freq_mhz=%d", &xc_basic_freq_mhz);
            sscanf(buf, "sc_burst_freq_mhz=%d", &sc_burst_freq_mhz);
            sscanf(buf, "bc_burst_freq_mhz=%d", &bc_burst_freq_mhz);
            sscanf(buf, "xc_burst_freq_mhz=%d", &xc_burst_freq_mhz);
            sscanf(buf, "sc_expect_freq_mhz=%d", &sc_expect_freq_mhz);
            sscanf(buf, "bc_expect_freq_mhz=%d", &bc_expect_freq_mhz);
            sscanf(buf, "xc_expect_freq_mhz=%d", &xc_expect_freq_mhz);
            sscanf(buf, "sc_current_freq_mhz=%d", &sc_current_freq_mhz);
            sscanf(buf, "bc_current_freq_mhz=%d", &bc_current_freq_mhz);
            sscanf(buf, "xc_current_freq_mhz=%d", &xc_current_freq_mhz);
            sscanf(buf, "sc_power_mw=%d", &sc_power_mw);
            sscanf(buf, "bc_power_mw=%d", &bc_power_mw);
            sscanf(buf, "xc_power_mw=%d", &xc_power_mw);
            if (strstr(buf, "platform="))
            {
                break;
            }
        }
    }
    fclose(fp);
    if (config_exist == 0)
    {
        write_log("[E] platform %s not supported.", platform_name);
        exit(0);
    }
    else
    {
        // freq_mhz -> freq_idx
        basic_sc_level = 0;
        min_diff = 9999;
        for (i = 0; i <= 10; i++)
        {
            diff = cluster0_freq_table[i] / 1000 - sc_basic_freq_mhz;
            if (abs(diff) < min_diff)
            {
                basic_sc_level = i;
                min_diff = abs(diff);
            }
        }
        burst_sc_level = 0;
        min_diff = 9999;
        for (i = 1; i <= 10; i++)
        {
            diff = cluster0_freq_table[i] / 1000 - sc_burst_freq_mhz;
            if (abs(diff) < min_diff)
            {
                burst_sc_level = i;
                min_diff = abs(diff);
            }
        }
        expect_sc_level = 0;
        min_diff = 9999;
        for (i = 1; i <= 10; i++)
        {
            diff = cluster0_freq_table[i] / 1000 - sc_expect_freq_mhz;
            if (abs(diff) < min_diff)
            {
                expect_sc_level = i;
                min_diff = abs(diff);
            }
        }
        current_sc_level = 0;
        min_diff = 9999;
        for (i = 1; i <= 10; i++)
        {
            diff = cluster0_freq_table[i] / 1000 - sc_current_freq_mhz;
            if (abs(diff) < min_diff)
            {
                current_sc_level = i;
                min_diff = abs(diff);
            }
        }
        basic_bc_level = 0;
        min_diff = 9999;
        for (i = 0; i <= 10; i++)
        {
            diff = cluster1_freq_table[i] / 1000 - bc_basic_freq_mhz;
            if (abs(diff) < min_diff)
            {
                basic_bc_level = i;
                min_diff = abs(diff);
            }
        }
        burst_bc_level = 0;
        min_diff = 9999;
        for (i = 1; i <= 10; i++)
        {
            diff = cluster1_freq_table[i] / 1000 - bc_burst_freq_mhz;
            if (abs(diff) < min_diff)
            {
                burst_bc_level = i;
                min_diff = abs(diff);
            }
        }
        expect_bc_level = 0;
        min_diff = 9999;
        for (i = 1; i <= 10; i++)
        {
            diff = cluster1_freq_table[i] / 1000 - bc_expect_freq_mhz;
            if (abs(diff) < min_diff)
            {
                expect_bc_level = i;
                min_diff = abs(diff);
            }
        }
        current_bc_level = 0;
        min_diff = 9999;
        for (i = 1; i <= 10; i++)
        {
            diff = cluster1_freq_table[i] / 1000 - bc_current_freq_mhz;
            if (abs(diff) < min_diff)
            {
                current_bc_level = i;
                min_diff = abs(diff);
            }
        }
        basic_xc_level = 0;
        min_diff = 9999;
        for (i = 0; i <= 10; i++)
        {
            diff = cluster2_freq_table[i] / 1000 - xc_basic_freq_mhz;
            if (abs(diff) < min_diff)
            {
                basic_xc_level = i;
                min_diff = abs(diff);
            }
        }
        burst_xc_level = 0;
        min_diff = 9999;
        for (i = 1; i <= 10; i++)
        {
            diff = cluster2_freq_table[i] / 1000 - xc_burst_freq_mhz;
            if (abs(diff) < min_diff)
            {
                burst_xc_level = i;
                min_diff = abs(diff);
            }
        }
        expect_xc_level = 0;
        min_diff = 9999;
        for (i = 1; i <= 10; i++)
        {
            diff = cluster2_freq_table[i] / 1000 - xc_expect_freq_mhz;
            if (abs(diff) < min_diff)
            {
                expect_xc_level = i;
                min_diff = abs(diff);
            }
        }
        current_xc_level = 0;
        min_diff = 9999;
        for (i = 1; i <= 10; i++)
        {
            diff = cluster2_freq_table[i] / 1000 - xc_current_freq_mhz;
            if (abs(diff) < min_diff)
            {
                current_xc_level = i;
                min_diff = abs(diff);
            }
        }
        // Get CPU Volt Table
        for (i = 0; i < burst_sc_level; i++)
        {
            sc_volt_table[i] = 60;
        }
        for (i = burst_sc_level; i < expect_sc_level; i++)
        {
            sc_volt_table[i] = sc_volt_table[i - 1] + 4;
        }
        for (i = expect_sc_level; i <= 10; i++)
        {
            sc_volt_table[i] = sc_volt_table[i - 1] + 6;
        }
        for (i = 0; i < burst_bc_level; i++)
        {
            bc_volt_table[i] = 60;
        }
        for (i = burst_bc_level; i < expect_bc_level; i++)
        {
            bc_volt_table[i] = bc_volt_table[i - 1] + 4;
        }
        for (i = expect_bc_level; i <= 10; i++)
        {
            bc_volt_table[i] = bc_volt_table[i - 1] + 6;
        }
        for (i = 0; i < burst_xc_level; i++)
        {
            xc_volt_table[i] = 60;
        }
        for (i = burst_xc_level; i < expect_xc_level; i++)
        {
            xc_volt_table[i] = xc_volt_table[i - 1] + 4;
        }
        for (i = expect_xc_level; i <= 10; i++)
        {
            xc_volt_table[i] = xc_volt_table[i - 1] + 6;
        }
        // Get Power Model
        sc_power_cost = sc_power_mw * 1000000 /
                        (cluster0_freq_table[current_sc_level] / 1000 * sc_volt_table[current_sc_level] *
                         sc_volt_table[current_sc_level]);
        for (i = 0; i <= 10; i++)
        {
            sc_pwr_mask[i] =
                sc_power_cost * cluster0_freq_table[i] / 1000 * sc_volt_table[i] * sc_volt_table[i] / 1000000;
        }
        bc_power_cost = bc_power_mw * 1000000 /
                        (cluster1_freq_table[current_bc_level] / 1000 * bc_volt_table[current_bc_level] *
                         bc_volt_table[current_bc_level]);
        for (i = 0; i <= 10; i++)
        {
            bc_pwr_mask[i] =
                bc_power_cost * cluster1_freq_table[i] / 1000 * bc_volt_table[i] * bc_volt_table[i] / 1000000;
        }
        xc_power_cost = xc_power_mw * 1000000 /
                        (cluster2_freq_table[current_xc_level] / 1000 * xc_volt_table[current_xc_level] *
                         xc_volt_table[current_xc_level]);
        for (i = 0; i <= 10; i++)
        {
            xc_pwr_mask[i] =
                xc_power_cost * cluster2_freq_table[i] / 1000 * xc_volt_table[i] * xc_volt_table[i] / 1000000;
        }
        // Get Power Ratio
        cpu_pwr_ratio = sc_efficiency * sc_core_num + bc_efficiency * bc_core_num + xc_efficiency * xc_core_num;
        sc_pwr_ratio = sc_efficiency * sc_core_num * 100 / cpu_pwr_ratio;
        bc_pwr_ratio = bc_efficiency * bc_core_num * 100 / cpu_pwr_ratio;
        xc_pwr_ratio = xc_efficiency * xc_core_num * 100 / cpu_pwr_ratio;
    }
}
void get_cpu_mask(void)
{
    FILE *fp;
    char buf[64];
    char URL[256];
    long int cpu_max_freq;
    int i;
    for (i = 0; i <= 7; i++)
    {
        sprintf(URL, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", i);
        fp = fopen(URL, "r");
        if (fp != NULL)
        {
            fgets(buf, sizeof(buf), fp);
            fclose(fp);
            sscanf(buf, "%ld", &cpu_max_freq);
            cpu_perf_mask[i] = 5000000000 / cpu_max_freq;
        }
    }
}
void get_cpu_clusters(void)
{
    if ((access("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq", 0)) != -1)
    {
        cluster0_cpu = 0;
    }
    if ((access("/sys/devices/system/cpu/cpufreq/policy4/scaling_min_freq", 0)) != -1)
    {
        cluster1_cpu = 4;
        if ((access("/sys/devices/system/cpu/cpufreq/policy6/scaling_min_freq", 0)) != -1)
        {
            cluster2_cpu = 6;
        }
        else if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", 0)) != -1)
        {
            cluster2_cpu = 7;
        }
    }
    else if ((access("/sys/devices/system/cpu/cpufreq/policy2/scaling_min_freq", 0)) != -1)
    {
        cluster1_cpu = 2;
    }
    else if ((access("/sys/devices/system/cpu/cpufreq/policy6/scaling_min_freq", 0)) != -1)
    {
        cluster1_cpu = 6;
        if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", 0)) != -1)
        {
            cluster2_cpu = 7;
        }
    }
    else if (cpu_perf_mask[3] != cpu_perf_mask[4] && core_num > 3)
    {
        cluster1_cpu = 4;
    }
    else if (cpu_perf_mask[1] != cpu_perf_mask[2] && core_num == 3)
    {
        cluster1_cpu = 2;
    }
}
void get_core_num(void)
{
    if (cluster1_cpu == -1)
    {
        sc_core_num = core_num + 1;
        bc_core_num = 0;
        xc_core_num = 0;
    }
    else if (cluster2_cpu == -1)
    {
        sc_core_num = cluster1_cpu;
        bc_core_num = core_num - cluster1_cpu + 1;
        xc_core_num = 0;
    }
    else
    {
        sc_core_num = cluster1_cpu;
        bc_core_num = cluster2_cpu - cluster1_cpu;
        xc_core_num = core_num - cluster2_cpu + 1;
    }
}
void get_cpu_group(void)
{
    int i;
    if (cluster1_cpu == 4 && cluster2_cpu == 7)
    {
        write_log("[I] TasksetHelper: efficiency=0-3, all_perf=4-7, multi_perf=4-6, single_perf=7, comm=0-5, all=0-7.");
        CPU_ZERO(&efficiency_mask);
        for (i = 0; i <= 3; i++)
        {
            CPU_SET(i, &efficiency_mask);
        }
        CPU_ZERO(&multi_perf_mask);
        for (i = 4; i <= 6; i++)
        {
            CPU_SET(i, &multi_perf_mask);
        }
        CPU_ZERO(&single_perf_mask);
        for (i = 7; i <= 7; i++)
        {
            CPU_SET(i, &single_perf_mask);
        }
        CPU_ZERO(&all_perf_mask);
        for (i = 4; i <= 7; i++)
        {
            CPU_SET(i, &all_perf_mask);
        }
        CPU_ZERO(&comm_mask);
        for (i = 0; i <= 5; i++)
        {
            CPU_SET(i, &comm_mask);
        }
        CPU_ZERO(&all_mask);
        for (i = 0; i <= core_num; i++)
        {
            CPU_SET(i, &all_mask);
        }
    }
    else if (cluster1_cpu == 4 && cluster2_cpu == 6)
    {
        write_log("[I] TasksetHelper: efficiency=0-3, all_perf=4-7, multi_perf=4-6, single_perf=7, comm=0-5, all=0-7.");
        CPU_ZERO(&efficiency_mask);
        for (i = 0; i <= 3; i++)
        {
            CPU_SET(i, &efficiency_mask);
        }
        CPU_ZERO(&multi_perf_mask);
        for (i = 4; i <= 6; i++)
        {
            CPU_SET(i, &multi_perf_mask);
        }
        CPU_ZERO(&single_perf_mask);
        for (i = 7; i <= 7; i++)
        {
            CPU_SET(i, &single_perf_mask);
        }
        CPU_ZERO(&all_perf_mask);
        for (i = 4; i <= 7; i++)
        {
            CPU_SET(i, &all_perf_mask);
        }
        CPU_ZERO(&comm_mask);
        for (i = 0; i <= 5; i++)
        {
            CPU_SET(i, &comm_mask);
        }
        CPU_ZERO(&all_mask);
        for (i = 0; i <= core_num; i++)
        {
            CPU_SET(i, &all_mask);
        }
    }
    else if (cluster1_cpu == 6 && cluster2_cpu == 7)
    {
        write_log("[I] TasksetHelper: efficiency=0-3, all_perf=6-7, multi_perf=0-6, single_perf=7, comm=0-6, all=0-7.");
        CPU_ZERO(&efficiency_mask);
        for (i = 0; i <= 3; i++)
        {
            CPU_SET(i, &efficiency_mask);
        }
        CPU_ZERO(&multi_perf_mask);
        for (i = 0; i <= 6; i++)
        {
            CPU_SET(i, &multi_perf_mask);
        }
        CPU_ZERO(&single_perf_mask);
        for (i = 7; i <= 7; i++)
        {
            CPU_SET(i, &single_perf_mask);
        }
        CPU_ZERO(&all_perf_mask);
        for (i = 6; i <= 7; i++)
        {
            CPU_SET(i, &all_perf_mask);
        }
        CPU_ZERO(&comm_mask);
        for (i = 0; i <= 6; i++)
        {
            CPU_SET(i, &comm_mask);
        }
        CPU_ZERO(&all_mask);
        for (i = 0; i <= core_num; i++)
        {
            CPU_SET(i, &all_mask);
        }
    }
    else if (cluster1_cpu == 6 && cluster2_cpu == -1)
    {
        write_log("[I] TasksetHelper: efficiency=0-3, all_perf=6-7, multi_perf=0-6, single_perf=7, comm=0-6, all=0-7.");
        CPU_ZERO(&efficiency_mask);
        for (i = 0; i <= 3; i++)
        {
            CPU_SET(i, &efficiency_mask);
        }
        CPU_ZERO(&multi_perf_mask);
        for (i = 0; i <= 6; i++)
        {
            CPU_SET(i, &multi_perf_mask);
        }
        CPU_ZERO(&single_perf_mask);
        for (i = 7; i <= 7; i++)
        {
            CPU_SET(i, &single_perf_mask);
        }
        CPU_ZERO(&all_perf_mask);
        for (i = 6; i <= 7; i++)
        {
            CPU_SET(i, &all_perf_mask);
        }
        CPU_ZERO(&comm_mask);
        for (i = 0; i <= 6; i++)
        {
            CPU_SET(i, &comm_mask);
        }
        CPU_ZERO(&all_mask);
        for (i = 0; i <= core_num; i++)
        {
            CPU_SET(i, &all_mask);
        }
    }
    else if (cluster1_cpu == 4 && cluster2_cpu == -1 && core_num == 7)
    {
        write_log("[I] TasksetHelper: efficiency=0-3, all_perf=4-7, multi_perf=4-6, single_perf=7, comm=0-5, all=0-7.");
        CPU_ZERO(&efficiency_mask);
        for (i = 0; i <= 3; i++)
        {
            CPU_SET(i, &efficiency_mask);
        }
        CPU_ZERO(&multi_perf_mask);
        for (i = 4; i <= 6; i++)
        {
            CPU_SET(i, &multi_perf_mask);
        }
        CPU_ZERO(&single_perf_mask);
        for (i = 7; i <= 7; i++)
        {
            CPU_SET(i, &single_perf_mask);
        }
        CPU_ZERO(&all_perf_mask);
        for (i = 4; i <= 7; i++)
        {
            CPU_SET(i, &all_perf_mask);
        }
        CPU_ZERO(&comm_mask);
        for (i = 0; i <= 5; i++)
        {
            CPU_SET(i, &comm_mask);
        }
        CPU_ZERO(&all_mask);
        for (i = 0; i <= core_num; i++)
        {
            CPU_SET(i, &all_mask);
        }
    }
    else if (cluster1_cpu == 4 && cluster2_cpu == -1 && core_num == 5)
    {
        write_log("[I] TasksetHelper: efficiency=0-3, all_perf=4-5, multi_perf=0-5, single_perf=5, comm=0-5, all=0-5.");
        CPU_ZERO(&efficiency_mask);
        for (i = 0; i <= 3; i++)
        {
            CPU_SET(i, &efficiency_mask);
        }
        CPU_ZERO(&multi_perf_mask);
        for (i = 0; i <= 5; i++)
        {
            CPU_SET(i, &multi_perf_mask);
        }
        CPU_ZERO(&single_perf_mask);
        for (i = 5; i <= 5; i++)
        {
            CPU_SET(i, &single_perf_mask);
        }
        CPU_ZERO(&all_perf_mask);
        for (i = 4; i <= 5; i++)
        {
            CPU_SET(i, &all_perf_mask);
        }
        CPU_ZERO(&comm_mask);
        for (i = 0; i <= 5; i++)
        {
            CPU_SET(i, &comm_mask);
        }
        CPU_ZERO(&all_mask);
        for (i = 0; i <= core_num; i++)
        {
            CPU_SET(i, &all_mask);
        }
    }
    else if (cluster1_cpu == 2 && cluster2_cpu == -1)
    {
        write_log("[I] TasksetHelper: efficiency=0-1, all_perf=2-3, multi_perf=0-3, single_perf=3, comm=0-3, all=0-3.");
        CPU_ZERO(&efficiency_mask);
        for (i = 0; i <= 1; i++)
        {
            CPU_SET(i, &efficiency_mask);
        }
        CPU_ZERO(&multi_perf_mask);
        for (i = 0; i <= 3; i++)
        {
            CPU_SET(i, &multi_perf_mask);
        }
        CPU_ZERO(&single_perf_mask);
        for (i = 3; i <= 3; i++)
        {
            CPU_SET(i, &single_perf_mask);
        }
        CPU_ZERO(&all_perf_mask);
        for (i = 2; i <= 3; i++)
        {
            CPU_SET(i, &all_perf_mask);
        }
        CPU_ZERO(&comm_mask);
        for (i = 0; i <= 3; i++)
        {
            CPU_SET(i, &comm_mask);
        }
        CPU_ZERO(&all_mask);
        for (i = 0; i <= core_num; i++)
        {
            CPU_SET(i, &all_mask);
        }
    }
    else
    {
        write_log("[I] TasksetHelper: Using generic cpu group.");
        CPU_ZERO(&efficiency_mask);
        for (i = 0; i <= core_num; i++)
        {
            CPU_SET(i, &efficiency_mask);
        }
        CPU_ZERO(&multi_perf_mask);
        for (i = 0; i <= core_num; i++)
        {
            CPU_SET(i, &multi_perf_mask);
        }
        CPU_ZERO(&single_perf_mask);
        for (i = 0; i <= core_num; i++)
        {
            CPU_SET(i, &single_perf_mask);
        }
        CPU_ZERO(&all_perf_mask);
        for (i = 0; i <= core_num; i++)
        {
            CPU_SET(i, &all_perf_mask);
        }
        CPU_ZERO(&comm_mask);
        for (i = 0; i <= core_num; i++)
        {
            CPU_SET(i, &comm_mask);
        }
        CPU_ZERO(&all_mask);
        for (i = 0; i <= core_num; i++)
        {
            CPU_SET(i, &all_mask);
        }
    }
}
void get_cpu_tables(void)
{
    FILE *fp;
    char url[256];
    char shell_buf[256];
    char buf[64];
    int i, j, select_freq_table;
    long int cluster0_max_freq, cluster1_max_freq, cluster2_max_freq;
    long int cluster0_min_freq, cluster1_min_freq, cluster2_min_freq;
    long int tables_step_freq, now_table_freq, get_cpu_freq, now_table_num;
    long int minus_freq, min_minus_freq;
    long int cpuinfo_available_freqs[50] = {0};
    if (cluster0_cpu != -1)
    {
        sprintf(url, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cluster0_cpu);
        fp = fopen(url, "r");
        if (fp != NULL)
        {
            fgets(buf, sizeof(buf), fp);
            fclose(fp);
            sscanf(buf, "%ld", &cluster0_max_freq);
        }
        sprintf(url, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_min_freq", cluster0_cpu);
        fp = fopen(url, "r");
        if (fp != NULL)
        {
            fgets(buf, sizeof(buf), fp);
            fclose(fp);
            sscanf(buf, "%ld", &cluster0_min_freq);
        }
        cluster0_freq_table[10] = cluster0_max_freq;
        cluster0_freq_table[0] = cluster0_min_freq;
        tables_step_freq = (cluster0_max_freq - cluster0_min_freq) / 10;
        sprintf(shell_buf, "cat /sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies | tr \" \" \"\n\"",
                cluster0_cpu);
        fp = popen(shell_buf, "r");
        if (fp != NULL)
        {
            now_table_num = 0;
            while (fgets(buf, sizeof(buf), fp) != NULL)
            {
                sscanf(buf, "%ld", &cpuinfo_available_freqs[now_table_num]);
                now_table_num++;
            }
            fclose(fp);
        }
        for (i = 1; i < 10; i++)
        {
            now_table_freq = cluster0_min_freq + i * tables_step_freq;
            min_minus_freq = 9999999;
            select_freq_table = 0;
            for (j = 0; j < 50; j++)
            {
                minus_freq = cpuinfo_available_freqs[j] - now_table_freq;
                if (minus_freq < 0)
                {
                    minus_freq = 0 - minus_freq;
                }
                if (minus_freq < min_minus_freq && cpuinfo_available_freqs[j] != 0)
                {
                    min_minus_freq = minus_freq;
                    select_freq_table = j;
                }
            }
            cluster0_freq_table[i] = cpuinfo_available_freqs[select_freq_table];
        }
    }
    if (cluster1_cpu != -1)
    {
        sprintf(url, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cluster1_cpu);
        fp = fopen(url, "r");
        if (fp != NULL)
        {
            fgets(buf, sizeof(buf), fp);
            fclose(fp);
            sscanf(buf, "%ld", &cluster1_max_freq);
        }
        sprintf(url, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_min_freq", cluster1_cpu);
        fp = fopen(url, "r");
        if (fp != NULL)
        {
            fgets(buf, sizeof(buf), fp);
            fclose(fp);
            sscanf(buf, "%ld", &cluster1_min_freq);
        }
        tables_step_freq = (cluster1_max_freq - cluster1_min_freq) / 10;
        cluster1_freq_table[10] = cluster1_max_freq;
        cluster1_freq_table[0] = cluster1_min_freq;
        sprintf(shell_buf, "cat /sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies | tr \" \" \"\n\"",
                cluster1_cpu);
        fp = popen(shell_buf, "r");
        if (fp != NULL)
        {
            now_table_num = 0;
            while (fgets(buf, sizeof(buf), fp) != NULL)
            {
                sscanf(buf, "%ld", &cpuinfo_available_freqs[now_table_num]);
                now_table_num++;
            }
            fclose(fp);
        }
        for (i = 1; i < 10; i++)
        {
            now_table_freq = cluster1_min_freq + i * tables_step_freq;
            min_minus_freq = 9999999;
            select_freq_table = 0;
            for (j = 0; j < 50; j++)
            {
                minus_freq = cpuinfo_available_freqs[j] - now_table_freq;
                if (minus_freq < 0)
                {
                    minus_freq = 0 - minus_freq;
                }
                if (minus_freq < min_minus_freq && cpuinfo_available_freqs[j] != 0)
                {
                    min_minus_freq = minus_freq;
                    select_freq_table = j;
                }
            }
            cluster1_freq_table[i] = cpuinfo_available_freqs[select_freq_table];
        }
    }
    if (cluster2_cpu != -1)
    {
        sprintf(url, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", cluster2_cpu);
        fp = fopen(url, "r");
        if (fp != NULL)
        {
            fgets(buf, sizeof(buf), fp);
            fclose(fp);
            sscanf(buf, "%ld", &cluster2_max_freq);
        }
        sprintf(url, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_min_freq", cluster2_cpu);
        fp = fopen(url, "r");
        if (fp != NULL)
        {
            fgets(buf, sizeof(buf), fp);
            fclose(fp);
            sscanf(buf, "%ld", &cluster2_min_freq);
        }
        tables_step_freq = (cluster2_max_freq - cluster2_min_freq) / 10;
        cluster2_freq_table[10] = cluster2_max_freq;
        cluster2_freq_table[0] = cluster2_min_freq;
        sprintf(shell_buf, "cat /sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies | tr \" \" \"\n\"",
                cluster2_cpu);
        fp = popen(shell_buf, "r");
        if (fp != NULL)
        {
            now_table_num = 0;
            while (fgets(buf, sizeof(buf), fp) != NULL)
            {
                sscanf(buf, "%ld", &cpuinfo_available_freqs[now_table_num]);
                now_table_num++;
            }
            fclose(fp);
        }
        for (i = 1; i < 10; i++)
        {
            now_table_freq = cluster2_min_freq + i * tables_step_freq;
            min_minus_freq = 9999999;
            select_freq_table = 0;
            for (j = 0; j < 50; j++)
            {
                minus_freq = cpuinfo_available_freqs[j] - now_table_freq;
                if (minus_freq < 0)
                {
                    minus_freq = 0 - minus_freq;
                }
                if (minus_freq < min_minus_freq && cpuinfo_available_freqs[j] != 0)
                {
                    min_minus_freq = minus_freq;
                    select_freq_table = j;
                }
            }
            cluster2_freq_table[i] = cpuinfo_available_freqs[select_freq_table];
        }
    }
}
int get_task_pid(char pkg_name[128])
{
    FILE *fp = NULL;
    DIR *dir = NULL;
    struct dirent *entry;
    char cmdline_url[128];
    char cmdline[128];
    char buf[128];
    int app_pid = -1;
    int tmp_pid = -1;
    dir = opendir("/proc/");
    if (dir)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            sscanf((*entry).d_name, "%d", &tmp_pid);
            sprintf(cmdline_url, "/proc/%d/cmdline", tmp_pid);
            fp = fopen(cmdline_url, "r");
            if (fp)
            {
                fgets(buf, sizeof(buf), fp);
                sscanf(buf, "%s", cmdline);
                fclose(fp);
                if (strcmp(cmdline, pkg_name) == 0)
                {
                    app_pid = tmp_pid;
                    break;
                }
            }
        }
        closedir(dir);
    }
    return app_pid;
}
void set_task_affinity(int pid)
{
    FILE *fp;
    FILE *tid_fp;
    DIR *dir;
    struct sched_param idle;
    struct sched_param normal;
    struct sched_param active;
    struct sched_param game;
    struct dirent *entry;
    char buf[256];
    char tid_dir[128];
    char tid_url[128];
    char tid_name[32];
    char pid_url[128];
    char pkg_name[256];
    int tid;
    idle.sched_priority = 130;
    normal.sched_priority = 110;
    active.sched_priority = 105;
    game.sched_priority = 100;
    sprintf(pid_url, "/proc/%d/cmdline", pid);
    fp = fopen(pid_url, "r");
    if (fp)
    {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "%s", pkg_name);
        fclose(fp);
    }
    sprintf(tid_dir, "/proc/%d/task/", pid);
    dir = opendir(tid_dir);
    if (dir != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            sscanf((*entry).d_name, "%d", &tid);
            sprintf(tid_url, "%s%d/comm", tid_dir, tid);
            tid_fp = fopen(tid_url, "r");
            if (tid_fp != NULL)
            {
                fgets(tid_name, sizeof(tid_name), tid_fp);
                fclose(tid_fp);
                if (tid == foreground_pid)
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "UnityMain"))
                {
                    sched_setaffinity(tid, sizeof(single_perf_mask), &single_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &game);
                }
                else if (strstr(tid_name, "MainThread-UE4"))
                {
                    sched_setaffinity(tid, sizeof(single_perf_mask), &single_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &game);
                }
                else if (strstr(tid_name, "GameThread"))
                {
                    sched_setaffinity(tid, sizeof(single_perf_mask), &single_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &game);
                }
                else if (strstr(tid_name, "RenderThread"))
                {
                    sched_setaffinity(tid, sizeof(single_perf_mask), &single_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &game);
                }
                else if (strstr(tid_name, "MINECRAFT"))
                {
                    sched_setaffinity(tid, sizeof(all_perf_mask), &all_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &game);
                }
                else if (strstr(tid_name, "GLThread"))
                {
                    sched_setaffinity(tid, sizeof(all_perf_mask), &all_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(pkg_name, "netease") && strstr(tid_name, "Thread-"))
                {
                    sched_setaffinity(tid, sizeof(all_perf_mask), &all_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "android.anim"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "Thread-"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "UnityGfx"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "RHIThread"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "JNISurfaceText"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "IJK_External"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "blur"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "android.ui"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "anim"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "Anim"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "android.display"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "UnityMulti"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "UnityPreload"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "UnityChoreograp"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "Worker Thread"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "Job.Worker"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "TaskGraphNP"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "glp"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "Gesture"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, ".gifmaker"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "Binder"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "Async"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "async"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "Vsync"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "vsync"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "ForkJoinPool-"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "UiThread"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "AndroidUI"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "RenderEngine"))
                {
                    sched_setaffinity(tid, sizeof(multi_perf_mask), &multi_perf_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &active);
                }
                else if (strstr(tid_name, "hwui"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "Chrome"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "Gecko"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "Chromium"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "WebView"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "webView"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "Webview"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "JavaScript"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "Javascript"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "javaScript"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "javascript"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "js"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "JS"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "android.fg"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "android.io"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "miui.fg"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "CrGpuMain"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "Compositor"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "CrRendererMain"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "OkHttp"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "work_thread"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "SearchDaemon"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "Profile"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "ThreadPool"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "PoolThread"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "Download"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "download"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "Audio"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "audio"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "Video"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "video"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "Mixer"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "mixer"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "mali-"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "ged-swd"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "GPU completion"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "FramePolicy"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "ScrollPolicy"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "glide-"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "launcher-"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "NativeThread"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "HeapTaskDaemon"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "FinalizerDaemon"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "ReferenceQueue"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "Jit thread pool"))
                {
                    sched_setaffinity(tid, sizeof(comm_mask), &comm_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
                else if (strstr(tid_name, "Timer-"))
                {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &idle);
                }
                else if (strstr(tid_name, "Fresco"))
                {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &idle);
                }
                else if (strstr(tid_name, "log"))
                {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &idle);
                }
                else if (strstr(tid_name, "xcrash"))
                {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &idle);
                }
                else if (strstr(tid_name, "bugly"))
                {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &idle);
                }
                else if (strstr(tid_name, "android.bg"))
                {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &idle);
                }
                else if (strstr(tid_name, "miui.bg"))
                {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &idle);
                }
                else if (strstr(tid_name, "SensorService"))
                {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &idle);
                }
                else if (strstr(tid_name, "HealthService"))
                {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &idle);
                }
                else if (strstr(tid_name, "background"))
                {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &idle);
                }
                else if (strstr(tid_name, "Background"))
                {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &idle);
                }
                else if (strstr(tid_name, "report"))
                {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &idle);
                }
                else if (strstr(tid_name, "tt_pangle"))
                {
                    sched_setaffinity(tid, sizeof(efficiency_mask), &efficiency_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &idle);
                }
                else
                {
                    sched_setaffinity(tid, sizeof(all_mask), &all_mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &normal);
                }
            }
        }
        closedir(dir);
    }
}
void restore_kernel_governor()
{
    FILE *fp;
    int nfp, i;
    char url[256];
    char governor_list[256];
    char cur_governor[32];
    if (access("/sys/devices/system/cpu/cpufreq/policy0/scaling_available_governors", 0) != -1)
    {
        fp = fopen("/sys/devices/system/cpu/cpufreq/policy0/scaling_available_governors", "r");
        if (fp)
        {
            fgets(governor_list, sizeof(governor_list), fp);
            fclose(fp);
        }
        if (strstr(governor_list, "schedutil"))
        {
            sprintf(cur_governor, "schedutil\n");
        }
        else if (strstr(governor_list, "sched"))
        {
            sprintf(cur_governor, "sched\n");
        }
        else if (strstr(governor_list, "sugov_ext"))
        {
            sprintf(cur_governor, "sugov_ext\n");
        }
        else if (strstr(governor_list, "walt"))
        {
            sprintf(cur_governor, "walt\n");
        }
        else if (strstr(governor_list, "interactive"))
        {
            sprintf(cur_governor, "interactive\n");
        }
        else if (strstr(governor_list, "ondemand"))
        {
            sprintf(cur_governor, "ondemand\n");
        }
        for (i = 0; i <= core_num; i++)
        {
            sprintf(url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_governor", i);
            if (access(url, 0) != -1)
            {
                nfp = open(url, O_WRONLY);
                write(nfp, cur_governor, strlen(cur_governor));
                close(nfp);
            }
        }
    }
    else if (access("/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors", 0) != -1)
    {
        fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors", "r");
        if (fp)
        {
            fgets(governor_list, sizeof(governor_list), fp);
            fclose(fp);
        }
        if (strstr(governor_list, "schedutil"))
        {
            sprintf(cur_governor, "schedutil\n");
        }
        else if (strstr(governor_list, "sched"))
        {
            sprintf(cur_governor, "sched\n");
        }
        else if (strstr(governor_list, "sugov_ext"))
        {
            sprintf(cur_governor, "sugov_ext\n");
        }
        else if (strstr(governor_list, "walt"))
        {
            sprintf(cur_governor, "walt\n");
        }
        else if (strstr(governor_list, "interactive"))
        {
            sprintf(cur_governor, "interactive\n");
        }
        else if (strstr(governor_list, "ondemand"))
        {
            sprintf(cur_governor, "ondemand\n");
        }
        for (i = 0; i <= core_num; i++)
        {
            sprintf(url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", i);
            if (access(url, 0) != -1)
            {
                nfp = open(url, O_WRONLY);
                write(nfp, cur_governor, strlen(cur_governor));
                close(nfp);
            }
        }
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
    int nfp;
    char buf[128];
    char file_url[128];
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", cluster0_cpu);
    if ((access(file_url, 0)) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "%ld\n", SC_MIN_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", cluster0_cpu);
    if ((access(file_url, 0)) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "%ld\n", SC_MAX_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", cluster1_cpu);
    if ((access(file_url, 0)) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "%ld\n", MC_MIN_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", cluster1_cpu);
    if ((access(file_url, 0)) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "%ld\n", MC_MAX_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", cluster2_cpu);
    if ((access(file_url, 0)) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "%ld\n", BC_MIN_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", cluster2_cpu);
    if ((access(file_url, 0)) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "%ld\n", BC_MAX_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq", cluster0_cpu);
    if ((access(file_url, 0)) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "%ld\n", SC_MIN_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq", cluster0_cpu);
    if ((access(file_url, 0)) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "%ld\n", SC_MAX_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq", cluster1_cpu);
    if ((access(file_url, 0)) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "%ld\n", MC_MIN_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq", cluster1_cpu);
    if ((access(file_url, 0)) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "%ld\n", MC_MAX_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq", cluster2_cpu);
    if ((access(file_url, 0)) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "%ld\n", BC_MIN_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq", cluster2_cpu);
    if ((access(file_url, 0)) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "%ld\n", BC_MAX_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    if ((access("/proc/ppm/policy/hard_userlimit_cpu_freq", 0)) != -1)
    {
        if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", 0)) != -1)
        {
            nfp = open("/proc/ppm/policy/hard_userlimit_cpu_freq", O_WRONLY);
            sprintf(buf, "%ld %ld %ld %ld %ld %ld\n", SC_MIN_FREQ, SC_MAX_FREQ, MC_MIN_FREQ, MC_MAX_FREQ, BC_MIN_FREQ,
                    BC_MAX_FREQ);
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
        else
        {
            nfp = open("/proc/ppm/policy/hard_userlimit_cpu_freq", O_WRONLY);
            sprintf(buf, "%ld %ld %ld %ld\n", SC_MIN_FREQ, SC_MAX_FREQ, MC_MIN_FREQ, MC_MAX_FREQ);
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
    }
    if (access("/sys/module/msm_performance/parameters/cpu_max_freq", 0) != -1)
    {
        if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", 0)) != -1)
        {
            nfp = open("/sys/module/msm_performance/parameters/cpu_max_freq", O_WRONLY);
            sprintf(buf, "%d:%ld %d:%ld %d:%ld\n", cluster0_cpu, SC_MAX_FREQ, cluster1_cpu, MC_MAX_FREQ, cluster2_cpu,
                    BC_MAX_FREQ);
            write(nfp, buf, strlen(buf));
            close(nfp);
            nfp = open("/sys/module/msm_performance/parameters/cpu_min_freq", O_WRONLY);
            sprintf(buf, "%d:%ld %d:%ld %d:%ld\n", cluster0_cpu, SC_MIN_FREQ, cluster1_cpu, MC_MIN_FREQ, cluster2_cpu,
                    BC_MIN_FREQ);
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
        else
        {
            nfp = open("/sys/module/msm_performance/parameters/cpu_max_freq", O_WRONLY);
            sprintf(buf, "%d:%ld %d:%ld\n", cluster0_cpu, SC_MAX_FREQ, cluster1_cpu, MC_MAX_FREQ);
            write(nfp, buf, strlen(buf));
            close(nfp);
            nfp = open("/sys/module/msm_performance/parameters/cpu_min_freq", O_WRONLY);
            sprintf(buf, "%d:%ld %d:%ld\n", cluster0_cpu, SC_MIN_FREQ, cluster1_cpu, MC_MIN_FREQ);
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
    }
    if ((access("/dev/cluster0_freq_min", 0)) != -1)
    {
        nfp = open("/dev/cluster0_freq_min", O_WRONLY);
        sprintf(buf, "%ld\n", SC_MIN_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    if ((access("/dev/cluster0_freq_max", 0)) != -1)
    {
        nfp = open("/dev/cluster0_freq_max", O_WRONLY);
        sprintf(buf, "%ld\n", SC_MAX_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    if ((access("/dev/cluster1_freq_min", 0)) != -1)
    {
        nfp = open("/dev/cluster1_freq_min", O_WRONLY);
        sprintf(buf, "%ld\n", MC_MIN_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    if ((access("/dev/cluster1_freq_max", 0)) != -1)
    {
        nfp = open("/dev/cluster1_freq_max", O_WRONLY);
        sprintf(buf, "%ld\n", MC_MAX_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    if ((access("/dev/cluster2_freq_min", 0)) != -1)
    {
        nfp = open("/dev/cluster2_freq_min", O_WRONLY);
        sprintf(buf, "%ld\n", BC_MIN_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    if ((access("/dev/cluster2_freq_max", 0)) != -1)
    {
        nfp = open("/dev/cluster2_freq_max", O_WRONLY);
        sprintf(buf, "%ld\n", BC_MAX_FREQ);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
}
void qti_freq_writer(int cpu, long int freq)
{
    int nfp;
    char buf[128];
    char file_url[128];
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_governor", cpu);
    if (access(file_url, 0) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "powersave\n");
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    if (access("/sys/module/msm_performance/parameters/cpu_max_freq", 0) != -1)
    {
        nfp = open("/sys/module/msm_performance/parameters/cpu_max_freq", O_WRONLY);
        sprintf(buf, "%d:%ld\n", cpu, freq);
        write(nfp, buf, strlen(buf));
        close(nfp);
        nfp = open("/sys/module/msm_performance/parameters/cpu_min_freq", O_WRONLY);
        sprintf(buf, "%d:%ld\n", cpu, freq);
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
}
void mtk_freq_writer(int cluster, long int freq)
{
    FILE *fp;
    long int cluster0_cur_freq, cluster1_cur_freq, cluster2_cur_freq;
    int nfp, cpu;
    char buf[128];
    char file_url[128];
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_cur_freq", cluster0_cpu);
    fp = fopen(file_url, "r");
    if (fp)
    {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "%ld", &cluster0_cur_freq);
        fclose(fp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_cur_freq", cluster1_cpu);
    fp = fopen(file_url, "r");
    if (fp)
    {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "%ld", &cluster1_cur_freq);
        fclose(fp);
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_cur_freq", cluster2_cpu);
    fp = fopen(file_url, "r");
    if (fp)
    {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "%ld", &cluster2_cur_freq);
        fclose(fp);
    }
    if (cluster == 0)
    {
        cluster0_cur_freq = freq;
        cpu = cluster0_cpu;
    }
    else if (cluster == 1)
    {
        cluster1_cur_freq = freq;
        cpu = cluster1_cpu;
    }
    else if (cluster == 2)
    {
        cluster2_cur_freq = freq;
        cpu = cluster2_cpu;
    }
    sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_governor", cpu);
    if (access(file_url, 0) != -1)
    {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf, "powersave\n");
        write(nfp, buf, strlen(buf));
        close(nfp);
    }
    if (access("/proc/ppm/policy/hard_userlimit_cpu_freq", 0) != -1)
    {
        if (access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", 0) != -1)
        {
            nfp = open("/proc/ppm/policy/hard_userlimit_cpu_freq", O_WRONLY);
            sprintf(buf, "%ld %ld %ld %ld %ld %ld\n", cluster0_cur_freq, cluster0_cur_freq, cluster1_cur_freq,
                    cluster1_cur_freq, cluster2_cur_freq, cluster2_cur_freq);
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
        else
        {
            nfp = open("/proc/ppm/policy/hard_userlimit_cpu_freq", O_WRONLY);
            sprintf(buf, "%ld %ld %ld %ld\n", cluster0_cur_freq, cluster0_cur_freq, cluster1_cur_freq,
                    cluster1_cur_freq);
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
    }
}
void exynos_freq_writer(int cluster, int cpu, long int freq)
{
    int nfp;
    char buf[128];
    char file_url[128];
    if ((access("/dev/cluster0_freq_min", 0)) != -1)
    {
        sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_governor", cpu);
        if (access(file_url, 0) != -1)
        {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf, "powersave\n");
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
        sprintf(file_url, "/dev/cluster%d_freq_min", cluster);
        if ((access(file_url, 0)) != -1)
        {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf, "%ld\n", freq);
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
        sprintf(file_url, "/dev/cluster%d_freq_max", cluster);
        if ((access(file_url, 0)) != -1)
        {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf, "%ld\n", freq);
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
    }
}
void userspace_freq_writer(int cpu, long int freq)
{
    int nfp;
    char buf[128];
    char file_url[128];
    if ((access("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed", 0)) != -1)
    {
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", cpu);
        if (access(file_url, 0) != -1)
        {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf, "userspace\n");
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_setspeed", cpu);
        if ((access(file_url, 0)) != -1)
        {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf, "%ld\n", freq);
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
    }
}
void eas_freq_writer(int cpu, long int freq)
{
    int nfp;
    char buf[128];
    char file_url[128];
    if ((access("/sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq", 0)) != -1)
    {
        sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_governor", cpu);
        if (access(file_url, 0) != -1)
        {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf, "performance\n");
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
        sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq", cpu);
        if ((access(file_url, 0)) != -1)
        {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf, "%ld\n", freq);
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
        sprintf(file_url, "/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq", cpu);
        if ((access(file_url, 0)) != -1)
        {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf, "%ld\n", freq);
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
    }
}
void hmp_freq_writer(int cpu, long int freq)
{
    int nfp;
    char buf[128];
    char file_url[128];
    if (access("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq", 0) != -1)
    {
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", cpu);
        if (access(file_url, 0) != -1)
        {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf, "performance\n");
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", cpu);
        if ((access(file_url, 0)) != -1)
        {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf, "%ld\n", freq);
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
        sprintf(file_url, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", cpu);
        if ((access(file_url, 0)) != -1)
        {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf, "%ld\n", freq);
            write(nfp, buf, strlen(buf));
            close(nfp);
        }
    }
}
void write_xc_freq(long int freq_level)
{
    pthread_detach(pthread_self());
    if (freq_writer_type == 1)
    {
        qti_freq_writer(cluster2_cpu, cluster2_freq_table[freq_level]);
    }
    else if (freq_writer_type == 2)
    {
        exynos_freq_writer(2, cluster2_cpu, cluster2_freq_table[freq_level]);
    }
    else if (freq_writer_type == 3)
    {
        mtk_freq_writer(2, cluster2_freq_table[freq_level]);
    }
    else if (freq_writer_type == 4)
    {
        userspace_freq_writer(cluster2_cpu, cluster2_freq_table[freq_level]);
    }
    else if (freq_writer_type == 5)
    {
        eas_freq_writer(cluster2_cpu, cluster2_freq_table[freq_level]);
    }
    else if (freq_writer_type == 6)
    {
        hmp_freq_writer(cluster2_cpu, cluster2_freq_table[freq_level]);
    }
    pthread_exit(0);
}
void write_bc_freq(long int freq_level)
{
    pthread_detach(pthread_self());
    if (freq_writer_type == 1)
    {
        qti_freq_writer(cluster1_cpu, cluster1_freq_table[freq_level]);
    }
    else if (freq_writer_type == 2)
    {
        exynos_freq_writer(1, cluster1_cpu, cluster1_freq_table[freq_level]);
    }
    else if (freq_writer_type == 3)
    {
        mtk_freq_writer(1, cluster1_freq_table[freq_level]);
    }
    else if (freq_writer_type == 4)
    {
        userspace_freq_writer(cluster1_cpu, cluster1_freq_table[freq_level]);
    }
    else if (freq_writer_type == 5)
    {
        eas_freq_writer(cluster1_cpu, cluster1_freq_table[freq_level]);
    }
    else if (freq_writer_type == 6)
    {
        hmp_freq_writer(cluster1_cpu, cluster1_freq_table[freq_level]);
    }
    pthread_exit(0);
}
void write_sc_freq(long int freq_level)
{
    pthread_detach(pthread_self());
    if (freq_writer_type == 1)
    {
        qti_freq_writer(cluster0_cpu, cluster0_freq_table[freq_level]);
    }
    else if (freq_writer_type == 2)
    {
        exynos_freq_writer(0, cluster0_cpu, cluster0_freq_table[freq_level]);
    }
    else if (freq_writer_type == 3)
    {
        mtk_freq_writer(0, cluster0_freq_table[freq_level]);
    }
    else if (freq_writer_type == 4)
    {
        userspace_freq_writer(cluster0_cpu, cluster0_freq_table[freq_level]);
    }
    else if (freq_writer_type == 5)
    {
        eas_freq_writer(cluster0_cpu, cluster0_freq_table[freq_level]);
    }
    else if (freq_writer_type == 6)
    {
        hmp_freq_writer(cluster0_cpu, cluster0_freq_table[freq_level]);
    }
    pthread_exit(0);
}
void usage_monitor(void)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "UsageMonitor");
    FILE *fp;
    char buf[256];
    int i, cur_cpu_core;
    long long int user, nice, sys, idle, iowait, irq, softirq, all, used;
    long long int last_all[8] = {0};
    long long int last_used[8] = {0};
    while (1)
    {
        sampling_time++;
        fp = fopen("/proc/stat", "r");
        if (fp)
        {
            fgets(buf, sizeof(buf), fp);
            for (i = 0; i <= core_num; i++)
            {
                fgets(buf, sizeof(buf), fp);
                if (strstr(buf, "cpu"))
                {
                    sscanf(buf, "cpu%d %lld %lld %lld %lld %lld %lld %lld", &cur_cpu_core, &user, &nice, &sys, &idle,
                           &iowait, &irq, &softirq);
                    all = user + nice + sys + idle + iowait + irq + softirq;
                    used = all - idle - iowait;
                    if ((all - last_all[cur_cpu_core]) > 0 && last_all[cur_cpu_core] != 0)
                    {
                        cpu_usage[cur_cpu_core] =
                            (float)(used - last_used[cur_cpu_core]) * 100 / (all - last_all[cur_cpu_core]);
                    }
                    else
                    {
                        cpu_usage[cur_cpu_core] = 0;
                    }
                    last_all[cur_cpu_core] = all;
                    last_used[cur_cpu_core] = used;
                }
            }
            fclose(fp);
        }
        usleep(sample_time);
        if (SCREEN_OFF == 1)
        {
            pthread_exit(0);
        }
    }
}
void cpu_governor(void)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "CPUGovernor");
    int target_loads[11] = {60, 60, 70, 70, 80, 80, 80, 80, 80, 80, 90};
    long long int gov_runtime_ms = 0;
    long long int get_sampling_time = 0;
    long long int sc_freq_up_time = 0;
    long long int bc_freq_up_time = 0;
    long long int xc_freq_up_time = 0;
    long long int sc_freq_down_time = 0;
    long long int bc_freq_down_time = 0;
    long long int xc_freq_down_time = 0;
    long long int fast_state_pool = 0;
    long long int fast_state_used = 0;
    long int cpu_total_pwr = 0;
    long int comm_limit_pwr = 0;
    long int fast_limit_pwr = 0;
    long int cur_limit_pwr = 0;
    long int fast_recover_stair = 0;
    long int total_pwr_ratio = 0;
    float sc_max_usage = 0;
    float bc_max_usage = 0;
    float xc_max_usage = 0;
    float target_level = 0;
    float demand = 0;
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
    int now_sc_level = 1;
    int now_bc_level = 1;
    int now_xc_level = 1;
    int sc_freq_up_delay = 0;
    int bc_freq_up_delay = 0;
    int xc_freq_up_delay = 0;
    int sc_freq_down_delay = 0;
    int bc_freq_down_delay = 0;
    int xc_freq_down_delay = 0;
    int boost_delay_ms = 0;
    int boost_level = 0;
    int boost_value = 0;
    int now_hint_level = 0;
    int i;
    char now_mode[32];
    while (1)
    {
        gov_runtime_ms++;
        if (strcmp(mode, now_mode) != 0)
        {
            // Initizalize Governor Values
            if (strcmp(mode, "powersave") == 0)
            {
                sc_perf_margin = 10;
                bc_perf_margin = 10;
                xc_perf_margin = 10;
                sc_freq_up_delay = 60;
                bc_freq_up_delay = 60;
                xc_freq_up_delay = 60;
                sc_freq_down_delay = 100;
                bc_freq_down_delay = 100;
                xc_freq_down_delay = 100;
                fast_state_pool = 500000;
                fast_recover_stair = 2000;
                comm_limit_pwr = 2000;
                fast_limit_pwr = 3000;
            }
            else if (strcmp(mode, "balance") == 0)
            {
                sc_perf_margin = 10;
                bc_perf_margin = 10;
                xc_perf_margin = 10;
                sc_freq_up_delay = 40;
                bc_freq_up_delay = 40;
                xc_freq_up_delay = 40;
                sc_freq_down_delay = 200;
                bc_freq_down_delay = 200;
                xc_freq_down_delay = 200;
                fast_state_pool = 1000000;
                fast_recover_stair = 3000;
                comm_limit_pwr = 3000;
                fast_limit_pwr = 5000;
            }
            else if (strcmp(mode, "performance") == 0)
            {
                sc_perf_margin = 20;
                bc_perf_margin = 20;
                xc_perf_margin = 20;
                sc_freq_up_delay = 20;
                bc_freq_up_delay = 20;
                xc_freq_up_delay = 20;
                sc_freq_down_delay = 200;
                bc_freq_down_delay = 200;
                xc_freq_down_delay = 200;
                fast_state_pool = 1000000;
                fast_recover_stair = 4000;
                comm_limit_pwr = 4000;
                fast_limit_pwr = 6000;
            }
            else
            {
                sc_perf_margin = 30;
                bc_perf_margin = 30;
                xc_perf_margin = 30;
                sc_freq_up_delay = 0;
                bc_freq_up_delay = 0;
                xc_freq_up_delay = 0;
                sc_freq_down_delay = 200;
                bc_freq_down_delay = 200;
                xc_freq_down_delay = 200;
                fast_state_pool = 0;
                fast_recover_stair = 0;
                comm_limit_pwr = 10000;
                fast_limit_pwr = 10000;
            }
            fast_state_used = 0;
            target_sc_level = 0;
            target_bc_level = 0;
            target_xc_level = 0;
            now_sc_level = 0;
            now_bc_level = 0;
            now_xc_level = 0;
            boost_level = 0;
            boost_delay_ms = 0;
            boost_value = 0;
            sprintf(now_mode, "%s", mode);
        }
        // Boost Switcher
        if (hint_level != now_hint_level)
        {
            if (boost_level <= hint_level)
            {
                boost_level = hint_level;
                if (hint_level == 3)
                {
                    boost_delay_ms = 200;
                }
                else if (hint_level == 2)
                {
                    boost_delay_ms = 500;
                }
                else if (hint_level == 1)
                {
                    boost_delay_ms = 2000;
                }
            }
            else if (boost_delay_ms == 0)
            {
                boost_level = hint_level;
            }
            now_hint_level = hint_level;
        }
        else
        {
            if (boost_delay_ms > 0)
            {
                boost_delay_ms--;
            }
            else
            {
                boost_level = 0;
            }
        }
        if (boost_level == 3)
        {
            boost_value = 20;
        }
        else if (boost_level == 2)
        {
            boost_value = 15;
        }
        else if (boost_level == 1)
        {
            boost_value = 10;
        }
        else
        {
            boost_value = 0;
        }
        if (boost_level > 0)
        {
            sample_time = 20000;
        }
        else
        {
            sample_time = 40000;
        }
        // Get Target Freq Level
        if (get_sampling_time != sampling_time)
        {
            sc_max_usage = 0;
            bc_max_usage = 0;
            xc_max_usage = 0;
            for (i = 0; i <= core_num; i++)
            {
                if (i < cluster1_cpu || cluster1_cpu == -1)
                {
                    if (cpu_usage[i] > sc_max_usage)
                    {
                        sc_max_usage = cpu_usage[i];
                    }
                }
                else if (i < cluster2_cpu || cluster2_cpu == -1)
                {
                    if (cpu_usage[i] > bc_max_usage)
                    {
                        bc_max_usage = cpu_usage[i];
                    }
                }
                else
                {
                    if (cpu_usage[i] > xc_max_usage)
                    {
                        xc_max_usage = cpu_usage[i];
                    }
                }
            }
            demand = (float)xc_max_usage + (100 - xc_max_usage) * (xc_perf_margin + boost_value) / 100;
            if (now_xc_level == 0)
                now_xc_level = 1;
            target_level = (float)now_xc_level * demand / target_loads[now_xc_level];
            if (target_level > 10)
            {
                target_xc_level = 10;
            }
            else
            {
                target_xc_level = (int)round(target_level);
            }
            if (now_xc_level < expect_xc_level && target_xc_level > expect_xc_level)
            {
                if ((gov_runtime_ms - xc_freq_up_time) >= xc_freq_up_delay)
                {
                    xc_freq_up_time = gov_runtime_ms;
                }
                else
                {
                    target_xc_level = expect_xc_level;
                }
            }
            demand = (float)bc_max_usage + (100 - bc_max_usage) * (bc_perf_margin + boost_value) / 100;
            if (now_bc_level == 0)
                now_bc_level = 1;
            target_level = (float)now_bc_level * demand / target_loads[now_bc_level];
            if (target_level > 10)
            {
                target_bc_level = 10;
            }
            else
            {
                target_bc_level = (int)round(target_level);
            }
            if (now_bc_level < expect_bc_level && target_bc_level > expect_bc_level)
            {
                if ((gov_runtime_ms - bc_freq_up_time) >= bc_freq_up_delay)
                {
                    bc_freq_up_time = gov_runtime_ms;
                }
                else
                {
                    target_bc_level = expect_bc_level;
                }
            }
            demand = (float)sc_max_usage + (100 - sc_max_usage) * (sc_perf_margin + boost_value) / 100;
            if (now_sc_level == 0)
                now_sc_level = 1;
            target_level = (float)now_sc_level * demand / target_loads[now_sc_level];
            if (target_level > 10)
            {
                target_sc_level = 10;
            }
            else
            {
                target_sc_level = (int)round(target_level);
            }
            if (now_sc_level < expect_sc_level && target_sc_level > expect_sc_level)
            {
                if ((gov_runtime_ms - sc_freq_up_time) >= sc_freq_up_delay)
                {
                    sc_freq_up_time = gov_runtime_ms;
                }
                else
                {
                    target_sc_level = expect_sc_level;
                }
            }
            if (sc_max_usage > 5 && target_sc_level < basic_sc_level)
            {
                target_sc_level = basic_sc_level;
            }
            if (bc_max_usage > 5 && target_bc_level < basic_bc_level)
            {
                target_bc_level = basic_bc_level;
            }
            if (xc_max_usage > 5 && target_xc_level < basic_xc_level)
            {
                target_xc_level = basic_xc_level;
            }
            get_sampling_time = sampling_time;
        }
        // Fast State Limit
        cpu_total_pwr = sc_pwr_mask[target_sc_level] * sc_core_num + bc_pwr_mask[target_bc_level] * bc_core_num +
                        xc_pwr_mask[target_xc_level] * xc_core_num;
        if (fast_state_used < fast_state_pool)
        {
            if (cpu_total_pwr > fast_limit_pwr)
            {
                cur_limit_pwr = fast_limit_pwr;
                sc_limit_pwr = 0;
                bc_limit_pwr = 0;
                xc_limit_pwr = 0;
                if ((sc_pwr_mask[target_sc_level] * sc_core_num) < (fast_limit_pwr * sc_pwr_ratio / 100))
                {
                    cur_limit_pwr = cur_limit_pwr - sc_pwr_mask[target_sc_level] * sc_core_num;
                    sc_over_limit = 0;
                }
                else
                {
                    sc_over_limit = 1;
                }
                if ((bc_pwr_mask[target_bc_level] * bc_core_num) < (fast_limit_pwr * bc_pwr_ratio / 100))
                {
                    cur_limit_pwr = cur_limit_pwr - bc_pwr_mask[target_bc_level] * bc_core_num;
                    bc_over_limit = 0;
                }
                else
                {
                    bc_over_limit = 1;
                }
                if ((xc_pwr_mask[target_xc_level] * xc_core_num) < (fast_limit_pwr * xc_pwr_ratio / 100))
                {
                    cur_limit_pwr = cur_limit_pwr - xc_pwr_mask[target_xc_level] * xc_core_num;
                    xc_over_limit = 0;
                }
                else
                {
                    xc_over_limit = 1;
                }
                total_pwr_ratio =
                    sc_pwr_ratio * sc_over_limit + bc_pwr_ratio * bc_over_limit + xc_pwr_ratio * xc_over_limit;
                if (sc_over_limit == 1)
                {
                    sc_limit_pwr = cur_limit_pwr * sc_pwr_ratio / total_pwr_ratio;
                    target_sc_level = 0;
                    for (i = 10; i >= 0; i--)
                    {
                        if ((sc_pwr_mask[i] * sc_core_num) <= sc_limit_pwr)
                        {
                            target_sc_level = i;
                            break;
                        }
                    }
                }
                if (bc_over_limit == 1)
                {
                    bc_limit_pwr = cur_limit_pwr * bc_pwr_ratio / total_pwr_ratio;
                    target_bc_level = 0;
                    for (i = 10; i >= 0; i--)
                    {
                        if ((bc_pwr_mask[i] * bc_core_num) <= bc_limit_pwr)
                        {
                            target_bc_level = i;
                            break;
                        }
                    }
                }
                if (xc_over_limit == 1)
                {
                    xc_limit_pwr = cur_limit_pwr * xc_pwr_ratio / total_pwr_ratio;
                    target_xc_level = 0;
                    for (i = 10; i >= 0; i--)
                    {
                        if ((xc_pwr_mask[i] * xc_core_num) <= xc_limit_pwr)
                        {
                            target_xc_level = i;
                            break;
                        }
                    }
                }
                fast_state_used = fast_state_used + fast_limit_pwr;
            }
            else if (cpu_total_pwr > comm_limit_pwr)
            {
                fast_state_used = fast_state_used + cpu_total_pwr;
            }
            else if (cpu_total_pwr < comm_limit_pwr && fast_state_used > 0)
            {
                fast_state_used = fast_state_used - fast_recover_stair;
            }
        }
        else
        {
            if (cpu_total_pwr > comm_limit_pwr)
            {
                cur_limit_pwr = comm_limit_pwr;
                sc_limit_pwr = 0;
                bc_limit_pwr = 0;
                xc_limit_pwr = 0;
                if ((sc_pwr_mask[target_sc_level] * sc_core_num) < (comm_limit_pwr * sc_pwr_ratio / 100))
                {
                    cur_limit_pwr = cur_limit_pwr - sc_pwr_mask[target_sc_level] * sc_core_num;
                    sc_over_limit = 0;
                }
                else
                {
                    sc_over_limit = 1;
                }
                if ((bc_pwr_mask[target_bc_level] * bc_core_num) < (comm_limit_pwr * bc_pwr_ratio / 100))
                {
                    cur_limit_pwr = cur_limit_pwr - bc_pwr_mask[target_bc_level] * bc_core_num;
                    bc_over_limit = 0;
                }
                else
                {
                    bc_over_limit = 1;
                }
                if ((xc_pwr_mask[target_xc_level] * xc_core_num) < (comm_limit_pwr * xc_pwr_ratio / 100))
                {
                    cur_limit_pwr = cur_limit_pwr - xc_pwr_mask[target_xc_level] * xc_core_num;
                    xc_over_limit = 0;
                }
                else
                {
                    xc_over_limit = 1;
                }
                total_pwr_ratio =
                    sc_pwr_ratio * sc_over_limit + bc_pwr_ratio * bc_over_limit + xc_pwr_ratio * xc_over_limit;
                if (sc_over_limit == 1)
                {
                    sc_limit_pwr = cur_limit_pwr * sc_pwr_ratio / total_pwr_ratio;
                    target_sc_level = 0;
                    for (i = 10; i >= 0; i--)
                    {
                        if ((sc_pwr_mask[i] * sc_core_num) <= sc_limit_pwr)
                        {
                            target_sc_level = i;
                            break;
                        }
                    }
                }
                if (bc_over_limit == 1)
                {
                    bc_limit_pwr = cur_limit_pwr * bc_pwr_ratio / total_pwr_ratio;
                    target_bc_level = 0;
                    for (i = 10; i >= 0; i--)
                    {
                        if ((bc_pwr_mask[i] * bc_core_num) <= bc_limit_pwr)
                        {
                            target_bc_level = i;
                            break;
                        }
                    }
                }
                if (xc_over_limit == 1)
                {
                    xc_limit_pwr = cur_limit_pwr * xc_pwr_ratio / total_pwr_ratio;
                    target_xc_level = 0;
                    for (i = 10; i >= 0; i--)
                    {
                        if ((xc_pwr_mask[i] * xc_core_num) <= xc_limit_pwr)
                        {
                            target_xc_level = i;
                            break;
                        }
                    }
                }
            }
            else if (fast_state_used > 0)
            {
                fast_state_used = fast_state_used - fast_recover_stair;
            }
        }
        // Write CPU Freq
        if (target_xc_level < now_xc_level)
        {
            if ((gov_runtime_ms - xc_freq_down_time) >= xc_freq_down_delay)
            {
                pthread_create(&thread_info, NULL, (void *)write_xc_freq, (void *)(long)target_xc_level);
                now_xc_level = target_xc_level;
                xc_freq_down_time = gov_runtime_ms;
            }
        }
        else if (target_xc_level > now_xc_level)
        {
            pthread_create(&thread_info, NULL, (void *)write_xc_freq, (void *)(long)target_xc_level);
            now_xc_level = target_xc_level;
        }
        if (target_bc_level < now_bc_level)
        {
            if ((gov_runtime_ms - bc_freq_down_time) >= bc_freq_down_delay)
            {
                pthread_create(&thread_info, NULL, (void *)write_bc_freq, (void *)(long)target_bc_level);
                now_bc_level = target_bc_level;
                bc_freq_down_time = gov_runtime_ms;
            }
        }
        else if (target_bc_level > now_bc_level)
        {
            pthread_create(&thread_info, NULL, (void *)write_bc_freq, (void *)(long)target_bc_level);
            now_bc_level = target_bc_level;
        }
        if (target_sc_level < now_sc_level)
        {
            if ((gov_runtime_ms - sc_freq_down_time) >= sc_freq_down_delay)
            {
                pthread_create(&thread_info, NULL, (void *)write_sc_freq, (void *)(long)target_sc_level);
                now_sc_level = target_sc_level;
                sc_freq_down_time = gov_runtime_ms;
            }
        }
        else if (target_sc_level > now_sc_level)
        {
            pthread_create(&thread_info, NULL, (void *)write_sc_freq, (void *)(long)target_sc_level);
            now_sc_level = target_sc_level;
        }
        usleep(1000);
        if (SCREEN_OFF == 1)
        {
            pthread_exit(0);
        }
    }
}
void TasksetHelper(void)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "TasksetHelper");
    while (1)
    {
        set_task_affinity(foreground_pid);
        sleep(1);
        if (SCREEN_OFF == 1)
        {
            pthread_exit(0);
        }
    }
}
void event_reader(long int ts_event)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "EventReader");
    struct input_event ts;
    struct input_absinfo ts_x_info;
    struct input_absinfo ts_y_info;
    char touch_screen_url[128];
    long long int start_x = 0;
    long long int start_y = 0;
    long long int end_x = 0;
    long long int end_y = 0;
    long long int swipe_x = 0;
    long long int swipe_y = 0;
    long long int touch_x = 0;
    long long int touch_y = 0;
    long long int swipe_range = 0;
    long long int gesture_left = 0;
    long long int gesture_right = 0;
    long long int gesture_top = 0;
    long long int gesture_bottom = 0;
    int touch_s = 0;
    int last_s = 0;
    int ret;
    sprintf(touch_screen_url, "/dev/input/event%ld", ts_event);
    int fd = open(touch_screen_url, O_RDONLY);
    if (fd < 0)
    {
        write_log("[W] EventReader: Failed to open %s.", touch_screen_url);
        pthread_exit(0);
    }
    ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &ts_x_info);
    ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &ts_y_info);
    write_log("[I] EventReader: listening %s, absinfo: x=%d~%d, y=%d~%d. ", touch_screen_url, ts_x_info.minimum,
              ts_x_info.maximum, ts_y_info.minimum, ts_y_info.maximum);
    swipe_range = ts_x_info.maximum / 10;
    gesture_left = swipe_range;
    gesture_right = ts_x_info.maximum - swipe_range;
    gesture_top = swipe_range;
    gesture_bottom = ts_y_info.maximum - swipe_range;
    while (1)
    {
        ret = read(fd, &ts, sizeof(ts));
        if (ret == -1)
        {
            write_log("[W] EventReader: Failed to get touchScreen input.");
            pthread_exit(0);
        }
        last_s = touch_s;
        if (ts.code == ABS_MT_POSITION_X)
        {
            touch_x = ts.value;
        }
        if (ts.code == ABS_MT_POSITION_Y)
        {
            touch_y = ts.value;
        }
        if (ts.code == BTN_TOUCH)
        {
            touch_s = ts.value;
        }
        if ((touch_s - last_s) == 1)
        {
            start_x = touch_x;
            start_y = touch_y;
            hint_level = 1;
        }
        else if ((touch_s - last_s) == 0 && touch_s == 1)
        {
            swipe_x = touch_x - start_x;
            swipe_y = touch_y - start_y;
            if (llabs(swipe_y) > swipe_range || llabs(swipe_x) > swipe_range)
            {
                hint_level = 2;
            }
        }
        else if ((touch_s - last_s) == -1)
        {
            end_x = touch_x;
            end_y = touch_y;
            swipe_x = end_x - start_x;
            swipe_y = end_y - start_y;
            if (start_x > gesture_right && llabs(swipe_x) > swipe_range)
            {
                hint_level = 3;
            }
            else if (start_x < gesture_left && llabs(swipe_x) > swipe_range)
            {
                hint_level = 3;
            }
            else if (start_y < gesture_top && llabs(swipe_y) > swipe_range)
            {
                hint_level = 3;
            }
            else if (start_y > gesture_bottom && llabs(swipe_y) > swipe_range)
            {
                hint_level = 3;
            }
            else
            {
                hint_level = 0;
            }
        }
    }
    close(fd);
}
void Run_EventReader(void)
{
    char abs_bitmask[(ABS_MAX + 1) / 8] = {0};
    char buf[64];
    char input_url[64];
    DIR *dir = NULL;
    struct dirent *entry;
    long int event;
    int fd;
    dir = opendir("/dev/input");
    if (dir != NULL)
    {
        while ((entry = readdir(dir)) != NULL)
        {
            sprintf(input_url, "/dev/input/%s", (*entry).d_name);
            fd = open(input_url, O_RDONLY);
            if (fd > 0)
            {
                ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
                if (check_bit(ABS_MT_POSITION_X, abs_bitmask) && check_bit(ABS_MT_POSITION_Y, abs_bitmask))
                {
                    sscanf(input_url, "/dev/input/event%ld", &event);
                    pthread_create(&thread_info, NULL, (void *)event_reader, (void *)event);
                }
                close(fd);
            }
        }
    }
    closedir(dir);
}
int freq_writer_test()
{
    FILE *fp;
    char buf[64];
    long int cur_freq;
    restore_kernel_governor();
    write_freq(10, 10, 10, 10, 10, 10);
    usleep(50000);
    qti_freq_writer(0, cluster0_freq_table[5]);
    usleep(10000);
    fp = fopen("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq", "r");
    if (fp)
    {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "%ld", &cur_freq);
        fclose(fp);
        if (cur_freq == cluster0_freq_table[5])
        {
            freq_writer_type = 1;
            write_log("[I] Using QTI FreqWriter.");
            return 0;
        }
    }
    restore_kernel_governor();
    write_freq(10, 10, 10, 10, 10, 10);
    usleep(50000);
    exynos_freq_writer(0, 0, cluster0_freq_table[5]);
    usleep(10000);
    fp = fopen("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq", "r");
    if (fp)
    {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "%ld", &cur_freq);
        fclose(fp);
        if (cur_freq == cluster0_freq_table[5])
        {
            freq_writer_type = 2;
            write_log("[I] Using Exynos FreqWriter.");
            return 0;
        }
    }
    restore_kernel_governor();
    write_freq(10, 10, 10, 10, 10, 10);
    usleep(50000);
    mtk_freq_writer(0, cluster0_freq_table[5]);
    usleep(10000);
    fp = fopen("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq", "r");
    if (fp)
    {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "%ld", &cur_freq);
        fclose(fp);
        if (cur_freq == cluster0_freq_table[5])
        {
            freq_writer_type = 3;
            write_log("[I] Using MTK FreqWriter.");
            return 0;
        }
    }
    restore_kernel_governor();
    write_freq(10, 10, 10, 10, 10, 10);
    usleep(50000);
    userspace_freq_writer(0, cluster0_freq_table[5]);
    usleep(10000);
    fp = fopen("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq", "r");
    if (fp)
    {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "%ld", &cur_freq);
        fclose(fp);
        if (cur_freq == cluster0_freq_table[5])
        {
            freq_writer_type = 4;
            write_log("[I] Using UserSpace FreqWriter.");
            return 0;
        }
    }
    restore_kernel_governor();
    write_freq(10, 10, 10, 10, 10, 10);
    usleep(50000);
    eas_freq_writer(0, cluster0_freq_table[5]);
    usleep(10000);
    fp = fopen("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq", "r");
    if (fp)
    {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "%ld", &cur_freq);
        fclose(fp);
        if (cur_freq == cluster0_freq_table[5])
        {
            freq_writer_type = 5;
            write_log("[I] Using EAS FreqWriter.");
            return 0;
        }
    }
    restore_kernel_governor();
    write_freq(10, 10, 10, 10, 10, 10);
    usleep(50000);
    hmp_freq_writer(0, cluster0_freq_table[5]);
    usleep(10000);
    fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq", "r");
    if (fp)
    {
        fgets(buf, sizeof(buf), fp);
        sscanf(buf, "%ld", &cur_freq);
        fclose(fp);
        if (cur_freq == cluster0_freq_table[5])
        {
            freq_writer_type = 6;
            write_log("[I] Using HMP FreqWriter.");
            return 0;
        }
    }
    restore_kernel_governor();
    write_freq(0, 10, 0, 10, 0, 10);
    write_log("[E] No FreqWriter availiable on your device, Abort.");
    exit(0);
    return 0;
}
int main(int argc, char *argv[])
{
    daemon(0, 0);
    FILE *fp;
    char recv_buf[128];
    char buf[128];
    char shell[256];
    char pkg_name[128];
    char get_mode[16];
    int screen_on_val = 1;
    int nfp;
    int i;
    int sockfd, newfd;
    int recv_num = 0;
    sprintf(path, "%s", argv[0]);
    for (i = strlen(path); i > 0; i--)
    {
        if (path[i] == '/')
        {
            path[i] = '\0';
            break;
        }
    }
    sprintf(mode, "null");
    sprintf(get_mode, "balance");
    system("echo \"CuprumTurbo V10 by chenzyadb@coolapk.com\" > "
           "\"/sdcard/Android/data/xyz.chenzyadb.cu_toolbox/files/Cuprum_Log.txt\" ");
    write_log("[I] Initizalizing.");
    run_libcuprum("init");
    if (access("/sys/devices/system/cpu/cpu7/cpufreq/scaling_cur_freq", 0) != -1)
    {
        core_num = 7;
    }
    else if (access("/sys/devices/system/cpu/cpu5/cpufreq/scaling_cur_freq", 0) != -1)
    {
        core_num = 5;
    }
    else if (access("/sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq", 0) != -1)
    {
        core_num = 3;
    }
    get_cpu_mask();
    get_cpu_clusters();
    write_log("[I] cluster0=cpu%d, cluster1=cpu%d, cluster2=cpu%d. ", cluster0_cpu, cluster1_cpu, cluster2_cpu);
    get_core_num();
    get_cpu_group();
    get_cpu_tables();
    get_config();
    write_log("[I] Cluster0 CPUFreq Table:");
    for (i = 0; i <= 10; i++)
    {
        write_log("[I] idx=%d, freq=%ld KHz, pwr_mask=%d.", i, cluster0_freq_table[i], sc_pwr_mask[i]);
    }
    if (cluster1_cpu != -1)
    {
        write_log("[I] Cluster1 CPUFreq Table:");
        for (i = 0; i <= 10; i++)
        {
            write_log("[I] idx=%d, freq=%ld KHz, pwr_mask=%d.", i, cluster1_freq_table[i], bc_pwr_mask[i]);
        }
    }
    if (cluster2_cpu != -1)
    {
        write_log("[I] Cluster2 CPUFreq Table:");
        for (i = 0; i <= 10; i++)
        {
            write_log("[I] idx=%d, freq=%ld KHz, pwr_mask=%d.", i, cluster2_freq_table[i], xc_pwr_mask[i]);
        }
    }
    write_log("[I] cpu_type=%d+%d+%d, cpu_pwr_ratio=%d/%d/%d.", sc_core_num, bc_core_num, xc_core_num, sc_pwr_ratio,
              bc_pwr_ratio, xc_pwr_ratio);
    set_task_affinity(get_task_pid("/system/bin/surfaceflinger"));
    set_task_affinity(get_task_pid("system_server"));
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(12345);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bind(sockfd, (struct sockaddr *)(&server_addr), sizeof(server_addr));
    write_log("[I] Waiting for Socket Client Connection. ");
    listen(sockfd, 1);
    newfd = accept(sockfd, NULL, NULL);
    if (newfd < 0)
    {
        write_log("[W] Socket Client connection failed.");
        for (i = 1; i <= 5; i++)
        {
            write_log("[I] Try reconnecting (%d/5).", i);
            close(newfd);
            close(sockfd);
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            server_addr.sin_port = htons(12345);
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            bind(sockfd, (struct sockaddr *)(&server_addr), sizeof(server_addr));
            listen(sockfd, 1);
            newfd = accept(sockfd, NULL, NULL);
            if (newfd >= 0)
            {
                write_log("[I] Socket Client Connected.");
                break;
            }
            else if (i < 5)
            {
                write_log("[W] Socket Client reconnection failed.");
                sleep(1);
            }
            else
            {
                write_log("[E] Unable to connect to the Socket Client, Abort.");
                exit(0);
            }
        }
    }
    else
    {
        write_log("[I] Socket Client Connected.");
    }
    freq_writer_test();
    Run_EventReader();
    pthread_create(&thread_info, NULL, (void *)TasksetHelper, NULL);
    pthread_create(&thread_info, NULL, (void *)usage_monitor, NULL);
    pthread_create(&thread_info, NULL, (void *)cpu_governor, NULL);
    usleep(500000);
    write_log("[I] CuDaemon(pid=%d) is running. ", getpid());
    prctl(PR_SET_NAME, "SocketServer");
    while (1)
    {
        memset(recv_buf, 0, sizeof(recv_buf));
        recv_num = recv(newfd, recv_buf, sizeof(recv_buf), 0);
        if (recv_num > 0)
        {
            if (strstr(recv_buf, "mode"))
            {
                sscanf(recv_buf, "mode=%s", get_mode);
            }
            else if (strstr(recv_buf, "foreground_pkg"))
            {
                sscanf(recv_buf, "foreground_pkg=%s", pkg_name);
                foreground_pid = get_task_pid(pkg_name);
                set_task_affinity(foreground_pid);
            }
            else if (strstr(recv_buf, "window_pkg"))
            {
                sscanf(recv_buf, "window_pkg=%s", pkg_name);
                set_task_affinity(get_task_pid(pkg_name));
            }
            else if (strstr(recv_buf, "screen_on"))
            {
                sscanf(recv_buf, "screen_on=%d", &screen_on_val);
            }
        }
        else
        {
            close(newfd);
            newfd = accept(sockfd, NULL, NULL);
            if (newfd < 0)
            {
                write_log("[W] Socket Client connection failed.");
                for (i = 1; i <= 5; i++)
                {
                    write_log("[I] Try reconnecting (%d/5).", i);
                    close(newfd);
                    close(sockfd);
                    server_addr.sin_family = AF_INET;
                    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
                    server_addr.sin_port = htons(12345);
                    sockfd = socket(AF_INET, SOCK_STREAM, 0);
                    bind(sockfd, (struct sockaddr *)(&server_addr), sizeof(server_addr));
                    listen(sockfd, 1);
                    newfd = accept(sockfd, NULL, NULL);
                    if (newfd >= 0)
                    {
                        write_log("[I] Socket Client Connected.");
                        break;
                    }
                    else if (i < 5)
                    {
                        write_log("[W] Socket Client reconnection failed.");
                        sleep(1);
                    }
                    else
                    {
                        write_log("[E] Unable to connect to the Socket Client, Abort.");
                        exit(0);
                    }
                }
            }
        }
        if (screen_on_val == 0)
        {
            if (SCREEN_OFF == 0)
            {
                SCREEN_OFF = 1;
                usleep(500000);
                run_libcuprum("powersave");
                restore_kernel_governor();
                write_freq(0, 10, 0, 10, 0, 10);
            }
        }
        else
        {
            if (SCREEN_OFF == 1)
            {
                write_freq(10, 10, 10, 10, 10, 10);
                run_libcuprum("init");
                run_libcuprum(mode);
                set_task_affinity(get_task_pid("/system/bin/surfaceflinger"));
                set_task_affinity(get_task_pid("system_server"));
                sample_time = 20000;
                foreground_pid = -1;
                SCREEN_OFF = 0;
                pthread_create(&thread_info, NULL, (void *)TasksetHelper, NULL);
                pthread_create(&thread_info, NULL, (void *)usage_monitor, NULL);
                pthread_create(&thread_info, NULL, (void *)cpu_governor, NULL);
            }
            if (strcmp(get_mode, mode) != 0)
            {
                write_log("[I] mode switching %s -> %s.", mode, get_mode);
                run_libcuprum(get_mode);
                sprintf(mode, "%s", get_mode);
            }
        }
    }
}