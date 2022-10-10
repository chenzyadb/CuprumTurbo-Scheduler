#include <dirent.h>
#include <math.h>
#include <regex.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#define CAP_MAX_NUM (CAP_TO_INDEX(CAP_LAST_CAP) + 1)

int libcu(void)
{
    return 0;
}

int write_value(const char* url, const char* format, ...)
{
    int fd;
    char buffer[1024];
    va_list arg;
    va_start(arg, format);
    vsprintf(buffer, format, arg);
    va_end(arg);
    fd = open(url, O_WRONLY);
    if (fd > 0) {
        write(fd, buffer, sizeof(buffer));
        close(fd);
    }
    return 0;
}

char* read_value(const char* url)
{
    int fd;
    char buffer[1024];
    static char ret[1024];
    fd = open(url, O_RDONLY);
    if (fd > 0) {
        read(fd, buffer, sizeof(buffer));
        close(fd);
    }
    sprintf(ret, "%s", buffer);
    return ret;
}

char* cmd_exec(const char* cmd, const char* format, ...)
{
    FILE* fp;
    char option[128];
    char script[256];
    char buffer[1024];
    static char ret[1024];
    va_list arg;
    va_start(arg, format);
    vsprintf(option, format, arg);
    va_end(arg);
    sprintf(script, "%s %s", cmd, option);
    fp = popen(script, "r");
    if (fp) {
        fgets(buffer, sizeof(buffer), fp);
        pclose(fp);
    }
    sprintf(ret, "%s", buffer);
    return ret;
}

char* str_grep(const char* string, const char* key_wd)
{
    char buffer[16 * 1024];
    char line_str[1024];
    static char ret[1024];
    long int i, j;
    long int line_start = -1;
    long int line_end = -1;
    sprintf(buffer, "%s", string);
    for (i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == '\n') {
            memset(line_str, 0, sizeof(line_str));
            line_start = line_end + 1;
            line_end = i;
            for (j = 0; j <= (line_end - line_start); j++) {
                line_str[j] = buffer[line_start + j];
            }
            if (strstr(line_str, key_wd)) {
                sprintf(ret, "%s", line_str);
                break;
            }
        }
    }
    return ret;
}

char* str_cut(const char* string, int cut_space)
{
    char buffer[1024];
    char cut_str[1024];
    static char ret[1024];
    int i, j;
    int space_num = 0;
    int cut_start = -1;
    int cut_end = -1;
    sprintf(buffer, "%s", string);
    i = 0;
    while (i < strlen(buffer)) {
        if (buffer[i] == ' ') {
            space_num++;
            cut_start = i;
            while (buffer[cut_start] == ' ') {
                cut_start++;
            }
            cut_end = cut_start + 1;
            while (buffer[cut_end] != ' ' && buffer[cut_end] != '\0') {
                cut_end++;
            }
            if (space_num == cut_space) {
                for (j = 0; j < (cut_end - cut_start); j++) {
                    cut_str[j] = buffer[cut_start + j];
                }
                sprintf(ret, "%s", cut_str);
                break;
            }
            i = cut_end;
        } else {
            i++;
        }
    }
    return ret;
}

int check_regex(const char* str, const char* regex)
{
    int ret = -1;
    regex_t comment;
    regcomp(&comment, regex, REG_EXTENDED | REG_NEWLINE | REG_NOSUB);
    if (regexec(&comment, str, 0, NULL, 0) != REG_NOMATCH) {
        ret = 1;
    } else {
        ret = 0;
    }
    regfree(&comment);
    return ret;
}

char* get_top_activity(void)
{
    FILE* fp;
    char buffer[1024];
    static char ret[1024];
    fp = popen("/system/bin/dumpsys activity o", "r");
    if (fp) {
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            if (strstr(buffer, "top-activity")) {
                sprintf(ret, "%s", buffer);
                break;
            }
        }
        pclose(fp);
    }
    return ret;
}

int get_space_num(const char* input_str)
{
    char buffer[1024];
    int i;
    int space_num = 0;
    sprintf(buffer, "%s", input_str);
    for (i = 0; i < strlen(buffer); i++) {
        if (buffer[i] == ' ') {
            space_num++;
        }
    }
    return space_num;
}

int get_freq_volt(int freq, int burst_freq, int expect_freq)
{
    /*  cpu_freq_mhz -> cpu_voltage
             cpu_freq_mhz <= cpu_burst_freq -> 0.4-0.5v
             cpu_freq_mhz > cpu_burst_freq && cpu_freq_mhz < cpu_expect_freq -> 0.5-0.7v
             cpu_freq_mhz > cpu_expect_freq -> 0.7-1.0v
             max_freq -> 3000MHz max_voltage -> 1.0v */
    float low_volt_stair, comm_volt_stair, high_volt_stair;
    int cur_volt, cur_idx;
    int low_volt_idx = 0;
    int high_volt_idx = 0;
    low_volt_idx = burst_freq / 100;
    high_volt_idx = expect_freq / 100;
    low_volt_stair = (500 - 400) / low_volt_idx;
    comm_volt_stair = (700 - 500) / (high_volt_idx - low_volt_idx);
    high_volt_stair = (1000 - 700) / (30 - high_volt_idx);
    cur_idx = freq / 100;
    if (cur_idx <= low_volt_idx) {
        cur_volt = 400 + low_volt_idx * low_volt_stair;
    } else if (cur_idx <= high_volt_idx) {
        cur_volt = 500 + (cur_idx - low_volt_idx) * comm_volt_stair;
    } else {
        cur_volt = 700 + (cur_idx - high_volt_idx) * high_volt_stair;
    }
    return cur_volt;
}

int get_task_pid(const char* pkg_name)
{
    FILE* fp = NULL;
    DIR* dir = NULL;
    struct dirent* entry;
    char cmdline_url[128];
    char cmdline[128];
    char buf[128];
    int task_pid = -1;
    int tmp_pid = -1;
    dir = opendir("/proc/");
    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            sscanf((*entry).d_name, "%d", &tmp_pid);
            sprintf(cmdline_url, "/proc/%d/cmdline", tmp_pid);
            fp = fopen(cmdline_url, "r");
            if (fp) {
                fgets(buf, sizeof(buf), fp);
                sscanf(buf, "%s", cmdline);
                fclose(fp);
                if (strcmp(cmdline, pkg_name) == 0) {
                    task_pid = tmp_pid;
                    break;
                }
            }
        }
        closedir(dir);
    }
    return task_pid;
}

float level_to_load(int freq_level, int perf_margin)
{
    // target_level = now_level * cpu_load / target_load;
    // if cpu_load > (100 - perf_margin) then freq_level++ else freq_level--.
    float target_load = 100 - perf_margin;
    if (freq_level > 0 && freq_level < 10) {
        target_load = (float)freq_level * (100 - perf_margin) / (freq_level + 1);
    }
    return target_load;
}

long long int get_thread_runtime(int pid, int tid)
{
    char buf[1024];
    char thread_stat_url[128];
    char comm[32];
    char state[32];
    long long int runtime;
    long int utime, stime, cutime, cstime;
    int fd;
    sprintf(thread_stat_url, "/proc/%d/task/%d/stat", pid, tid);
    fd = open(thread_stat_url, O_RDONLY);
    if (fd > 0) {
        read(fd, buf, sizeof(buf));
        sscanf(buf, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %ld %ld %ld %ld %*d %*d %*d %*d %*u %*lu %*ld", &utime, &stime, &cutime, &cstime);
        close(fd);
    }
    runtime = utime + stime + cutime + cstime;
    return runtime;
}

int reset_task_nice(int tid)
{
    struct __user_cap_header_struct cap_header;
    struct __user_cap_data_struct cap_data[CAP_MAX_NUM];
    int cap_sys_nice_idx = CAP_TO_INDEX(CAP_SYS_NICE);
    unsigned int cap_sys_nice_mask = CAP_TO_MASK(CAP_SYS_NICE);
    cap_header.version = _LINUX_CAPABILITY_VERSION_3;
    cap_header.pid = tid;
    if (capget(&cap_header, cap_data)) {
        return -1;
    }
    cap_data[cap_sys_nice_idx].effective &= ~cap_sys_nice_mask;
    cap_data[cap_sys_nice_idx].permitted &= ~cap_sys_nice_mask;
    cap_data[cap_sys_nice_idx].inheritable &= ~cap_sys_nice_mask;
    capset(&cap_header, cap_data);
    setpriority(PRIO_PROCESS, tid, 0);
    return 0;
}