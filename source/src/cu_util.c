#include "cu_util.h"

int WriteFile(const char* filePath, const char* format, ...)
{
    int ret = -1;

    char buffer[4096];
    va_list arg;
    va_start(arg, format);
    vsprintf(buffer, format, arg);
    va_end(arg);

    int fd = open(filePath, O_WRONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd <= 0) {
        chmod(filePath, 0666);
        fd = open(filePath, O_WRONLY | O_NONBLOCK | O_CLOEXEC);
    }
    if (fd > 0) {
        ret = write(fd, buffer, strlen(buffer));
        close(fd);
    }

    return ret;
}

char* ReadFile(char* ret, const char* format, ...)
{
    char filePath[256];
    va_list arg;
    va_start(arg, format);
    vsprintf(filePath, format, arg);
    va_end(arg);

    static char buffer[4096];
    memset(buffer, 0, sizeof(buffer));

    int fd = open(filePath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd <= 0) {
        chmod(filePath, 0444);
        fd = open(filePath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    }
    if (fd > 0) {
        int len = read(fd, buffer, sizeof(buffer));
        if (len >= 0) {
            buffer[len] = '\0';
        } else {
            buffer[0] = '\0';
        }
        close(fd);
    }

    if (ret) {
        strcpy(ret, buffer);
    }

    return buffer;
}

char* ReadFileEx(const char* format, ...)
{
    char filePath[256];
    va_list arg;
    va_start(arg, format);
    vsprintf(filePath, format, arg);
    va_end(arg);

    static char buffer[16 * 1024];
    memset(buffer, 0, sizeof(buffer));

    FILE* fp = fopen(filePath, "r");
    if (!fp) {
        chmod(filePath, 0444);
        fp = fopen(filePath, "r");
    }
    if (fp) {
        int len = fread(buffer, sizeof(char), sizeof(buffer), fp);
        if (len >= 0) {
            buffer[len] = '\0';
        } else {
            buffer[0] = '\0';
        }
        fclose(fp);
    }

    return buffer;
}

char* CutString(const char* string, const int cut_space)
{
    static char ret[256];
    memset(ret, 0, sizeof(ret));

    char buffer[256];
    char cut_str[256];
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
                for (j = 0; j <= (cut_end - cut_start); j++) {
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

char* StrMerge(const char* format, ...)
{
    static char ret[256];
    memset(ret, 0, sizeof(ret));

    va_list arg;
    va_start(arg, format);
    vsprintf(ret, format, arg);
    va_end(arg);

    return ret;
}

int CheckRegex(const char* str, const char* regex)
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

char* GetDirName(const char* filePath)
{
    static char ret[256];
    memset(ret, 0, sizeof(ret));

    sprintf(ret, "%s", filePath);

    int i;
    for (i = strlen(ret); i > 0; i--) {
        if (ret[i] == '/') {
            ret[i] = '\0';
            break;
        }
    }

    return ret;
}

char* DumpTopActivityInfo(void)
{
    static char ret[128];
    memset(ret, 0, sizeof(ret));

    char buffer[128];

    FILE* fp = popen("/system/bin/dumpsys activity o", "r");
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

long int GetThreadRuntime(const int pid, const int tid)
{
    char buf[512];
    long int utime, stime, cutime, cstime;

    char statPath[128];
    sprintf(statPath, "/proc/%d/task/%d/stat", pid, tid);

    int fd = open(statPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd > 0) {
        read(fd, buf, sizeof(buf));
        sscanf(buf, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %ld %ld %ld %ld %*d %*d %*d %*d %*u %*lu %*ld", &utime, &stime, &cutime, &cstime);
        close(fd);
    }

    return utime + stime + cutime + cstime;
}

int MakeDir(const char* format, ...)
{
    int ret = -1;

    char dirPath[256];
    va_list arg;
    va_start(arg, format);
    vsprintf(dirPath, format, arg);
    va_end(arg);

    if (access(dirPath, 0) != -1) {
        ret = 1;
    } else {
        ret = mkdir(dirPath, 0666);
    }

    return ret;
}

int IsDirExist(const char* format, ...)
{
    int dirExist = -1;

    char dirPath[256];
    va_list arg;
    va_start(arg, format);
    vsprintf(dirPath, format, arg);
    va_end(arg);

    if (access(dirPath, 0) != -1) {
        dirExist = 1;
    } else {
        dirExist = 0;
    }

    return dirExist;
}

int IsFileExist(const char* format, ...)
{
    int fileExist = -1;

    char filePath[256];
    va_list arg;
    va_start(arg, format);
    vsprintf(filePath, format, arg);
    va_end(arg);

    if (access(filePath, 0) != -1) {
        fileExist = 1;
    } else {
        fileExist = 0;
    }

    return fileExist;
}

int GetThreadPid(const int tid)
{
    int pid = -1;
    char buffer[1024];

    char statusPath[128];
    sprintf(statusPath, "/proc/%d/status", tid);

    int fd = open(statusPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd > 0) {
        char lineStr[128];
        int len = read(fd, buffer, sizeof(buffer));
        int start_p = 0;
        int i;
        for (i = 0; i < len; i++) {
            if (buffer[i] != '\n') {
                lineStr[i - start_p] = buffer[i];
            } else {
                lineStr[i - start_p] = '\0';
                if (strstr(lineStr, "Tgid:")) {
                    sscanf(lineStr, "Tgid: %d", &pid);
                    break;
                }
                start_p = i + 1;
            }
        }
        close(fd);
    }

    return pid;
}

int GetTaskType(const int pid)
{
    int taskType = TASK_OTHER;
    int oomAdj = 0;
    char buffer[16];

    char oomAdjPath[128];
    sprintf(oomAdjPath, "/proc/%d/oom_adj", pid);

    int fd = open(oomAdjPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd > 0) {
        read(fd, buffer, sizeof(buffer));
        sscanf(buffer, "%d", &oomAdj);

        if (oomAdj == 0) {
            taskType = TASK_FOREGROUND;
        } else if (oomAdj == 1) {
            taskType = TASK_VISIBLE;
        } else if (oomAdj > 1) {
            taskType = TASK_SERVICE;
        } else if (oomAdj < 0) {
            taskType = TASK_SYSTEM;
        }

        close(fd);
    }

    return taskType;
}

char* GetTaskName(const int pid)
{
    static char taskName[128];
    memset(taskName, 0, sizeof(taskName));

    char cmdlinePath[128];
    sprintf(cmdlinePath, "/proc/%d/cmdline", pid);
    
    int fd = open(cmdlinePath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd > 0) {
        int len = read(fd, taskName, sizeof(taskName));
        if (len >= 0) {
            taskName[len] = '\0';
        } else {
            taskName[0] = '\0';
        }
        close(fd);
    }

    return taskName;
}

int GetScreenState(void)
{
    int state = SCREEN_ON;

    char buffer[128];
    int restrictedTaskNum = 0;

    int fd = open("/dev/cpuset/restricted/tasks", O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd > 0) {
        int len = read(fd, buffer, sizeof(buffer));
        int i;
        for (i = 0; i < len; i++) {
            if (buffer[i] == '\n') {
                restrictedTaskNum++;
                if (restrictedTaskNum > 10) {
                    state = SCREEN_OFF;
                    break;
                }
            }
        }
        close(fd);
    }

    return state;
}

long int GetCompileDateCode(const char* compileDate)
{
    char monthStr[3];
    int year, month, day;

    sscanf(compileDate, "%s %d %d", monthStr, &day, &year);

    if (strcmp(monthStr, "Jan") == 0) {
        month = 1;
    } else if (strcmp(monthStr, "Feb") == 0) {
        month = 2;
    } else if (strcmp(monthStr, "Mar") == 0) {
        month = 3;
    } else if (strcmp(monthStr, "Apr") == 0) {
        month = 4;
    } else if (strcmp(monthStr, "May") == 0) {
        month = 5;
    } else if (strcmp(monthStr, "Jun") == 0) {
        month = 6;
    } else if (strcmp(monthStr, "Jul") == 0) {
        month = 7;
    } else if (strcmp(monthStr, "Aug") == 0) {
        month = 8;
    } else if (strcmp(monthStr, "Sep") == 0) {
        month = 9;
    } else if (strcmp(monthStr, "Oct") == 0) {
        month = 10;
    } else if (strcmp(monthStr, "Nov") == 0) {
        month = 11;
    } else if (strcmp(monthStr, "Dec") == 0) {
        month = 12;
    }

    return (long int)year * 10000 + month * 100 + day;
}

int RoundNum(const float num)
{
    int ret = 0;

    int dec = (int)(num * 10) % 10;
    if (dec >= 5) {
        ret = (int)num + 1;
    } else {
        ret = (int)num;
    }

    return ret;
}

void SetThreadName(const char* name) {
    prctl(PR_SET_NAME, name);
}

int GetAndroidSDKVersion(void) {
    int AndroidSDKVersion = -1;

    FILE* fp = popen("/system/bin/getprop \"ro.build.version.sdk\"", "r");
    if (fp) {
        char buffer[128];
        fgets(buffer, sizeof(buffer), fp);
        sscanf(buffer, "%d", &AndroidSDKVersion);
        pclose(fp);
    }

    return AndroidSDKVersion;
}

