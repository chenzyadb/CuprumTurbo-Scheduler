#pragma once

#define CHECK_BIT(bit, array) (array[bit / 8] & (1 << (bit % 8)))

#define SCREEN_ON 1
#define SCREEN_OFF 0

#define TASK_FOREGROUND 0
#define TASK_VISIBLE 1
#define TASK_SERVICE 2
#define TASK_SYSTEM 3
#define TASK_OTHER -1

int WriteFile(const char* filePath, const char* format, ...);
char* ReadFile(char* ret, const char* format, ...);
int CheckRegex(const char* str, const char* regex);
char* GetDirName(const char* filePath);
char* DumpTopActivityInfo(void);
char* CutString(const char* string, const int cut_space);
char* StrMerge(const char* format, ...);
long int GetThreadRuntime(const int pid, const int tid);
int MakeDir(const char* format, ...);
int IsDirExist(const char* format, ...);
int IsFileExist(const char* format, ...);
int GetThreadPid(const int tid);
int GetTaskType(const int pid);
int GetScreenState(void);
long int GetCompileDateCode(const char *compileDate);
int RoundNum(const float num);
int GetAndroidSDKVersion(void);