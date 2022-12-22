#ifndef LIBCU_H
#define LIBCU_H
int libcu(void);
int write_value(const char* url, const char* format, ...);
char* read_value(const char* url);
char* cmd_exec(const char* cmd, const char* format, ...);
char* str_cut(const char* str, int cut_space);
char* get_top_activity(void);
int check_regex(const char* str, const char* regex);
int get_space_num(const char* input_str);
int get_freq_volt(int freq, int burst_freq, int expect_freq);
int get_task_pid(const char* pkg_name);
float level_to_load(int freq_level, int perf_margin);
long long int get_thread_runtime(int pid, int tid);
int reset_task_nice(int tid);
#endif