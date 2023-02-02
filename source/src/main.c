// CuprumTurbo Scheduler V13
// Author: chenzyadb

#define __USE_GNU
#include <dirent.h>
#include <limits.h>
#include <linux/input.h>
#include <math.h>
#include <pthread.h>
#include <regex.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "cJSON.h"
#include "cu_util.h"

#define FREQ_WRITER_GOV 0
#define FREQ_WRITER_PPM 1
#define FREQ_WRITER_EPIC 2

#define BOOST_NONE 0
#define BOOST_TOUCH 1
#define BOOST_SWIPE 2
#define BOOST_GESTURE 3
#define BOOST_HEAVYLOAD 4

char daemonPath[256];
char configPath[256];
char modePath[256];
char logPath[256];

char configData[16 * 1024];

int screenState = SCREEN_ON;

char curMode[16] = "balance";

pthread_t threadsTid = 0;
pthread_t tasksetTid = 0;

int cpuCoreNum = 0;
int cpuClusterNum = 0;

int curCpuFreqWriter = FREQ_WRITER_GOV;

int govBoost = 0;
int boostDurationTime = 0;

void InitLogWriter(void)
{
    FILE* fp = fopen(logPath, "w");
    if (!fp) {
        exit(0);
    }
    fprintf(fp, "");
    fflush(fp);
    fclose(fp);

    chmod(logPath, 0666);
}

void WriteLog(const char* logLevel, const char* format, ...)
{
    char logText[128];
    va_list arg;
    va_start(arg, format);
    vsprintf(logText, format, arg);
    va_end(arg);

    time_t timeInfo = time(NULL);
    struct tm* logTime = localtime(&timeInfo);

    FILE* fp = fopen(logPath, "a");
    if (fp) {
        fprintf(fp, "%02d-%02d %02d:%02d:%02d [%s] %s\n", logTime->tm_mon + 1, logTime->tm_mday, logTime->tm_hour, logTime->tm_min, logTime->tm_sec, logLevel, logText);
        fflush(fp);
        fclose(fp);
    }
}

void InitConfigReader(void)
{
    int fd = open(configPath, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd <= 0) {
        WriteLog("E", "Can't read config file.");
        exit(0);
    }
    read(fd, configData, sizeof(configData));
    close(fd);

    cJSON* jsonBuffer = cJSON_Parse(configData);
    if (jsonBuffer == NULL) {
        WriteLog("E", "cJson can't parse this config.");
        exit(0);
    }

    cJSON* itemBuffer = NULL;
    char configName[64];
    char configAuthor[16];
    int configVersion = 0;
    itemBuffer = cJSON_GetObjectItem(jsonBuffer, "name");
    sscanf(itemBuffer->valuestring, "%s", configName);
    itemBuffer = cJSON_GetObjectItem(jsonBuffer, "author");
    sscanf(itemBuffer->valuestring, "%s", configAuthor);
    itemBuffer = cJSON_GetObjectItem(jsonBuffer, "configVersion");
    configVersion = itemBuffer->valueint;
    if (configVersion != 3) {
        WriteLog("E", "Config version does not meet the requirements.");
        WriteLog("E", "Need config version: 3, your config version: %d.", configVersion);
        exit(0);
    }
    WriteLog("I", "Config \"%s\" by \"%s\".", configName, configAuthor);

    memset(configData, 0, sizeof(configData));
    cJSON_PrintPreallocated(jsonBuffer, configData, sizeof(configData), 0);
    cJSON_Delete(jsonBuffer);
}

struct CoCpuGovernor_Data {
    int enable;
    int policy;
    int firstCpu;
    int lastCpu;
    int cpuCapacity;
    long int freqTable[50];
    long int lowPowerFreq;
    long int basicFreq;
    long int expectFreq;
    long int modelFreq;
    int powerTable[50];
    int freqTableItemNum;
} govData[10];

struct Dynamic_CoCpuGovernor_Data {
    int powerLimit;
    int upRateLatency[10];
    int downRateDelay[10];
    int govBoost[5];
    int boostDurationTime[5];
    int boostDurationSleepTime;
} dynamicGovData;

void WriteCpuFreqViaMSM(int cpuCore, long int minFreq, long int maxFreq)
{
    WriteFile("/sys/module/msm_performance/parameters/cpu_max_freq", "%d:%ld\n", cpuCore, maxFreq);
    WriteFile("/sys/module/msm_performance/parameters/cpu_min_freq", "%d:%ld\n", cpuCore, minFreq);
}

void WriteCpuFreqViaPPM(int targetCluster, long int minFreq, long int maxFreq)
{
    long int minCpuFreq[3] = { 0 };
    long int maxCpuFreq[3] = { 0 };

    if (cpuClusterNum > 3) {
        return;
    }

    int i;
    for (i = 0; i < cpuClusterNum; i++) {
        if (i == targetCluster) {
            minCpuFreq[i] = minFreq;
            maxCpuFreq[i] = maxFreq;
        } else {
            sscanf(ReadFile(NULL, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", govData[i].firstCpu), "%ld", &minCpuFreq[i]);
            sscanf(ReadFile(NULL, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", govData[i].firstCpu), "%ld", &maxCpuFreq[i]);
        }
    }

    if (cpuClusterNum == 1) {
        WriteFile("/proc/ppm/policy/hard_userlimit_cpu_freq", "%ld %ld\n", minCpuFreq[0], maxCpuFreq[0]);
    } else if (cpuClusterNum == 2) {
        WriteFile("/proc/ppm/policy/hard_userlimit_cpu_freq", "%ld %ld %ld %ld\n", minCpuFreq[0], maxCpuFreq[0], minCpuFreq[1], maxCpuFreq[1]);
    } else if (cpuClusterNum == 3) {
        WriteFile("/proc/ppm/policy/hard_userlimit_cpu_freq", "%ld %ld %ld %ld %ld %ld\n", minCpuFreq[0], maxCpuFreq[0], minCpuFreq[1], maxCpuFreq[1], minCpuFreq[2], maxCpuFreq[2]);
    }
}

void WriteCpuFreqViaEpic(int cluster, long int minFreq, long int maxFreq)
{
    WriteFile(StrMerge("/dev/cluster%d_freq_max", cluster), "%ld\n", maxFreq);
    WriteFile(StrMerge("/dev/cluster%d_freq_min", cluster), "%ld\n", minFreq);
}

void WriteCpuFreqViaGovernor(int cpuCore, long int minFreq, long int maxFreq)
{
    WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq", cpuCore), "%ld\n", maxFreq);
    WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq", cpuCore), "%ld\n", minFreq);
}

void CheckCpuFreqWriter(void)
{
    if (IsFileExist("/proc/ppm/policy/hard_userlimit_cpu_freq")) {
        curCpuFreqWriter = FREQ_WRITER_PPM;
        WriteLog("I", "Use CpuFreqWriterPPM.");
    } else if (IsFileExist("/dev/cluster0_freq_min") && IsFileExist("/dev/cluster0_freq_max")) {
        curCpuFreqWriter = FREQ_WRITER_EPIC;
        WriteLog("I", "Use CpuFreqWriterEPIC.");
    } else if (IsFileExist("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq") && IsFileExist("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq")) {
        curCpuFreqWriter = FREQ_WRITER_GOV;
        WriteLog("I", "Use CpuFreqWriterGovernor.");
    } else {
        WriteLog("E", "Can't find availiable CpuFreqWriter on your device.");
        exit(0);
    }
}

void SetGovHispeed(int cpuCore, long int cpuFreq)
{
    char curCpuGovernor[16];
    sscanf(ReadFile(NULL, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", cpuCore), "%s", curCpuGovernor);

    char govHispeedPath[128];
    sprintf(govHispeedPath, "/sys/devices/system/cpu/cpu%d/cpufreq/%s/hispeed_freq", cpuCore, curCpuGovernor);
    if (IsFileExist(govHispeedPath)) {
        WriteFile(govHispeedPath, "%ld\n", cpuFreq);
    }
}

void CoCpuGovernor(void)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "CoCpuGovernor");

    long int govRunTime = 0;

    int i, j;

    cJSON* jsonBuffer = cJSON_Parse(configData);
    cJSON* objectBuffer = cJSON_GetObjectItem(jsonBuffer, "CoCpuGovernor_Config");
    cJSON* itemBuffer = cJSON_GetObjectItem(objectBuffer, "fastSampleTime");
    int fastSampleTime = itemBuffer->valueint;
    itemBuffer = cJSON_GetObjectItem(objectBuffer, "slowSampleTime");
    int slowSampleTime = itemBuffer->valueint;
    cJSON_Delete(jsonBuffer);

    FILE* fp = NULL;
    char statBuffer[128];
    int curCpuCore;
    long int user, nice, sys, idle, iowait, irq, softirq;
    long int nowaSumtime, nowaRuntime;
    long int prevRuntime[10] = { 0 };
    long int prevSumtime[10] = { 0 };

    int cpuLoad[10] = { 0 };
    int clusterMaxLoad = 0;
    int clusterDemand = 0;

    long int nowaMinFreq[10] = { 0 };
    long int nowaMaxFreq[10] = { 0 };
    long int prevMinFreq[10] = { 0 };
    long int prevMaxFreq[10] = { 0 };

    int prevCurFreqIdx[10] = { 0 };
    int nowaCurFreqIdx[10] = { 0 };

    int totalPower, pendingPower, pendingTotalCapRatio;
    int clusterPower[10] = { 0 };
    int overPowerLimit[10] = { 0 };
    float clusterCapRatio[10] = { 0 };

    long int totalCapacity = 0;
    for (i = 0; i < cpuClusterNum; i++) {
        totalCapacity += govData[i].cpuCapacity * (govData[i].lastCpu - govData[i].firstCpu + 1);
    }
    for (i = 0; i < cpuClusterNum; i++) {
        clusterCapRatio[i] = (float)govData[i].cpuCapacity * (govData[i].lastCpu - govData[i].firstCpu + 1) * 100 / totalCapacity;
    }

    long int freqDownTime = 0;
    long int freqUpTime = 0;

    int upRateDelay = 0;
    float delayRatio = 0;
    long int unexpectedFreq = 0;

    int clusterMaxPower = 0;
    long int clusterIdleMaxFreq[10] = { 0 };

    while (screenState == SCREEN_ON) {
        for (i = 0; i < cpuCoreNum; i++) {
            cpuLoad[i] = 0;
        }

        fp = fopen("/proc/stat", "r");
        if (fp) {
            while (fgets(statBuffer, sizeof(statBuffer), fp) != NULL) {
                if (CheckRegex(statBuffer, "^cpu[0-9]")) {
                    sscanf(statBuffer, "cpu%d %ld %ld %ld %ld %ld %ld %ld", &curCpuCore, &user, &nice, &sys, &idle, &iowait, &irq, &softirq);
                    nowaSumtime = user + nice + sys + idle + iowait + irq + softirq;
                    nowaRuntime = nowaSumtime - idle;
                    cpuLoad[curCpuCore] = (nowaRuntime - prevRuntime[curCpuCore]) * 100 / (nowaSumtime - prevSumtime[curCpuCore]);
                    prevRuntime[curCpuCore] = nowaRuntime;
                    prevSumtime[curCpuCore] = nowaSumtime;
                } else if (CheckRegex(statBuffer, "^intr")) {
                    break;
                }
            }
            fclose(fp);
        }

        for (i = 0; i < cpuClusterNum; i++) {
            clusterMaxLoad = 0;
            for (j = govData[i].firstCpu; j <= govData[i].lastCpu; j++) {
                if (cpuLoad[j] > clusterMaxLoad) {
                    clusterMaxLoad = cpuLoad[j];
                }
            }

            clusterDemand = clusterMaxLoad + (100 - clusterMaxLoad) * govBoost / 100;

            prevCurFreqIdx[i] = nowaCurFreqIdx[i];
            nowaCurFreqIdx[i] = RoundNum((float)(govData[i].freqTableItemNum - 1) * clusterDemand / 100);

            if (nowaCurFreqIdx[i] < prevCurFreqIdx[i]) {
                if ((govRunTime - freqDownTime) < dynamicGovData.downRateDelay[i]) {
                    nowaCurFreqIdx[i] = prevCurFreqIdx[i];
                } else {
                    freqDownTime = govRunTime;
                }
            } else if (nowaCurFreqIdx[i] > prevCurFreqIdx[i] && govData[i].freqTable[nowaCurFreqIdx[i]] > govData[i].expectFreq) {
                unexpectedFreq = (govData[i].freqTable[nowaCurFreqIdx[i]] - govData[i].expectFreq);
                delayRatio = unexpectedFreq / (govData[i].freqTable[govData[i].freqTableItemNum - 1] - govData[i].expectFreq);
                upRateDelay = 20 + dynamicGovData.upRateLatency[i] * delayRatio;
                if ((govRunTime - freqUpTime) < upRateDelay) {
                    nowaCurFreqIdx[i] = prevCurFreqIdx[i];
                } else {
                    freqUpTime = govRunTime;
                }
            }
        }

        totalPower = 0;
        for (i = 0; i < cpuClusterNum; i++) {
            clusterPower[i] = govData[i].powerTable[nowaCurFreqIdx[i]] * (govData[i].lastCpu - govData[i].firstCpu + 1);
            totalPower += clusterPower[i];
        }
        if (totalPower > dynamicGovData.powerLimit) {
            pendingTotalCapRatio = 0;
            for (i = 0; i < cpuClusterNum; i++) {
                if (clusterPower[i] > (dynamicGovData.powerLimit * clusterCapRatio[i] / 100)) {
                    overPowerLimit[i] = 1;
                    pendingTotalCapRatio += clusterCapRatio[i];
                } else {
                    overPowerLimit[i] = 0;
                }
            }

            pendingPower = dynamicGovData.powerLimit;
            for (i = 0; i < cpuClusterNum; i++) {
                pendingPower -= clusterPower[i] * (1 - overPowerLimit[i]);
            }

            for (i = 0; i < cpuClusterNum; i++) {
                if (overPowerLimit[i] == 1) {
                    nowaCurFreqIdx[i] = 0;
                    for (j = (govData[i].freqTableItemNum - 1); j > 0; j--) {
                        if ((govData[i].powerTable[j] * (govData[i].lastCpu - govData[i].firstCpu + 1)) <= (pendingPower * clusterCapRatio[i] / pendingTotalCapRatio)) {
                            nowaCurFreqIdx[i] = j;
                            break;
                        }
                    }
                }
            }
        }

        for (i = 0; i < cpuClusterNum; i++) {
            clusterMaxPower = dynamicGovData.powerLimit * clusterCapRatio[i] / 100;
            clusterIdleMaxFreq[i] = govData[i].freqTable[0];
            for (j = (govData[i].freqTableItemNum - 1); j > 0; j--) {
                if (govData[i].powerTable[j] < (clusterMaxPower / (govData[i].lastCpu - govData[i].firstCpu + 1))) {
                    clusterIdleMaxFreq[i] = govData[i].freqTable[j];
                    break;
                }
            }

            if (clusterIdleMaxFreq[i] > govData[i].expectFreq) {
                clusterIdleMaxFreq[i] = govData[i].expectFreq;
            }
        }

        for (i = 0; i < cpuClusterNum; i++) {
            prevMinFreq[i] = nowaMinFreq[i];
            prevMaxFreq[i] = nowaMaxFreq[i];

            if (govBoost > 0) {
                if (govData[i].freqTable[nowaCurFreqIdx[i]] < govData[i].basicFreq) {
                    nowaMaxFreq[i] = clusterIdleMaxFreq[i];
                    nowaMinFreq[i] = govData[i].basicFreq;
                } else if (govData[i].freqTable[nowaCurFreqIdx[i]] < clusterIdleMaxFreq[i]) {
                    nowaMaxFreq[i] = clusterIdleMaxFreq[i];
                    nowaMinFreq[i] = govData[i].freqTable[nowaCurFreqIdx[i]];
                } else {
                    nowaMaxFreq[i] = govData[i].freqTable[nowaCurFreqIdx[i]];
                    nowaMinFreq[i] = govData[i].freqTable[nowaCurFreqIdx[i]];
                }
            } else {
                if (govData[i].freqTable[nowaCurFreqIdx[i]] < clusterIdleMaxFreq[i]) {
                    nowaMaxFreq[i] = clusterIdleMaxFreq[i];
                    nowaMinFreq[i] = govData[i].lowPowerFreq;
                } else {
                    nowaMaxFreq[i] = govData[i].freqTable[nowaCurFreqIdx[i]];
                    nowaMinFreq[i] = govData[i].lowPowerFreq;
                }
            }

            if (govData[i].enable) {
                if (prevMinFreq[i] != nowaMinFreq[i] || prevMaxFreq[i] != nowaMaxFreq[i]) {
                    if (curCpuFreqWriter == FREQ_WRITER_PPM) {
                        WriteCpuFreqViaPPM(govData[i].policy, nowaMinFreq[i], nowaMaxFreq[i]);
                    } else if (curCpuFreqWriter == FREQ_WRITER_EPIC) {
                        WriteCpuFreqViaEpic(govData[i].policy, nowaMinFreq[i], nowaMaxFreq[i]);
                    } else if (curCpuFreqWriter == FREQ_WRITER_GOV) {
                        WriteCpuFreqViaGovernor(govData[i].firstCpu, nowaMinFreq[i], nowaMaxFreq[i]);
                    }
                }
                if (prevMaxFreq[i] != nowaMaxFreq[i] && curCpuFreqWriter == FREQ_WRITER_GOV) {
                    SetGovHispeed(govData[i].firstCpu, nowaMaxFreq[i]);
                }
            }
        }

        if (govBoost == 0) {
            govRunTime += slowSampleTime;
            usleep(slowSampleTime * 1000);
        } else {
            govRunTime += fastSampleTime;
            usleep(fastSampleTime * 1000);
        }
    }

    pthread_exit(0);
}

void KernelGovernorOpt(void)
{
    int i, j;
    char curCpuGovernor[16];

    char targetLoads[1024];
    int cpuTargetLoad = 0;

    if (IsFileExist("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor")) {
        sscanf(ReadFile(NULL, "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor"), "%s", curCpuGovernor);
    } else {
        WriteFile("W", "KernelGovernorOpt may not availiable on your device.");
    }

    for (i = 0; i < cpuClusterNum; i++) {
        if (IsDirExist("/sys/devices/system/cpu/cpufreq/policy%d/%s", govData[i].firstCpu, curCpuGovernor)) {
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/rate_limit_us", govData[i].firstCpu, curCpuGovernor), "1000");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/up_rate_limit_us", govData[i].firstCpu, curCpuGovernor), "1000");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/down_rate_limit_us", govData[i].firstCpu, curCpuGovernor), "1000");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/hispeed_load", govData[i].firstCpu, curCpuGovernor), "95");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/go_hispeed_load", govData[i].firstCpu, curCpuGovernor), "95");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/above_hispeed_delay", govData[i].firstCpu, curCpuGovernor), "0");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/boost", govData[i].firstCpu, curCpuGovernor), "0");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/boostpulse_duration", govData[i].firstCpu, curCpuGovernor), "0");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/io_is_busy", govData[i].firstCpu, curCpuGovernor), "0");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/timer_rate", govData[i].firstCpu, curCpuGovernor), "20000");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/timer_slack", govData[i].firstCpu, curCpuGovernor), "40000");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/min_sample_time", govData[i].firstCpu, curCpuGovernor), "20000");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/pl", govData[i].firstCpu, curCpuGovernor), "0");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/rtg_boost_freq", govData[i].firstCpu, curCpuGovernor), "0");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/policy%d/%s/target_loads", govData[i].firstCpu, curCpuGovernor), "80");
        }

        if (IsDirExist("/sys/devices/system/cpu/cpu%d/cpufreq/%s", govData[i].firstCpu, curCpuGovernor)) {
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/rate_limit_us", govData[i].firstCpu, curCpuGovernor), "1000");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/up_rate_limit_us", govData[i].firstCpu, curCpuGovernor), "1000");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/down_rate_limit_us", govData[i].firstCpu, curCpuGovernor), "1000");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/hispeed_load", govData[i].firstCpu, curCpuGovernor), "95");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/go_hispeed_load", govData[i].firstCpu, curCpuGovernor), "95");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/above_hispeed_delay", govData[i].firstCpu, curCpuGovernor), "0");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/boost", govData[i].firstCpu, curCpuGovernor), "0");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/boostpulse_duration", govData[i].firstCpu, curCpuGovernor), "0");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/io_is_busy", govData[i].firstCpu, curCpuGovernor), "0");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/timer_rate", govData[i].firstCpu, curCpuGovernor), "20000");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/timer_slack", govData[i].firstCpu, curCpuGovernor), "40000");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/min_sample_time", govData[i].firstCpu, curCpuGovernor), "20000");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/pl", govData[i].firstCpu, curCpuGovernor), "0");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/rtg_boost_freq", govData[i].firstCpu, curCpuGovernor), "0");
            WriteFile(StrMerge("/sys/devices/system/cpu/cpu%d/cpufreq/%s/target_loads", govData[i].firstCpu, curCpuGovernor), "80");
        }
    }

    if (IsDirExist("/sys/devices/system/cpu/cpufreq/%s", curCpuGovernor)) {
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/rate_limit_us", curCpuGovernor), "1000");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/up_rate_limit_us", curCpuGovernor), "1000");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/down_rate_limit_us", curCpuGovernor), "1000");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/hispeed_load", curCpuGovernor), "95");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/go_hispeed_load", curCpuGovernor), "95");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/above_hispeed_delay", curCpuGovernor), "0");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/boost", curCpuGovernor), "0");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/boostpulse_duration", curCpuGovernor), "0");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/io_is_busy", curCpuGovernor), "0");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/timer_rate", curCpuGovernor), "20000");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/timer_slack", curCpuGovernor), "40000");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/min_sample_time", curCpuGovernor), "20000");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/pl", curCpuGovernor), "0");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/rtg_boost_freq", curCpuGovernor), "0");
        WriteFile(StrMerge("/sys/devices/system/cpu/cpufreq/%s/target_loads", curCpuGovernor), "80");
    }
}

void InitPolicyData(int policy, int firstCpu, int lastCpu)
{
    int i;

    cJSON* jsonBuffer = cJSON_Parse(configData);

    char objectName[32];
    sprintf(objectName, "policy%d_Config", policy);
    cJSON* objectBuffer = cJSON_GetObjectItem(jsonBuffer, objectName);

    cJSON* itemBuffer = NULL;

    itemBuffer = cJSON_GetObjectItem(objectBuffer, "enable");
    govData[policy].enable = itemBuffer->valueint;

    govData[policy].policy = policy;
    govData[policy].firstCpu = firstCpu;
    govData[policy].lastCpu = lastCpu;

    itemBuffer = cJSON_GetObjectItem(objectBuffer, "cpu_capacity");
    govData[policy].cpuCapacity = itemBuffer->valueint;

    char freqTablePath[128];
    itemBuffer = cJSON_GetObjectItem(objectBuffer, "freqTablePath");
    sscanf(itemBuffer->valuestring, "%s", freqTablePath);
    if (!IsFileExist(freqTablePath)) {
        WriteLog("E", "File \"%s\" doesn't exist.", freqTablePath);
    }
    char freqTableBuffer[4096];
    char freqBuffer[16];
    int idx;
    int startIdx = 0;
    int endIdx = 0;
    ReadFile(freqTableBuffer, freqTablePath);
    govData[policy].freqTableItemNum = 0;
    for (idx = 1; idx < strlen(freqTableBuffer); idx++) {
        if (freqTableBuffer[idx - 1] == ' ' && freqTableBuffer[idx] != ' ') {
            startIdx = idx;
        } else if (freqTableBuffer[idx - 1] != ' ' && freqTableBuffer[idx] == ' ' || freqTableBuffer[idx - 1] != ' ' && freqTableBuffer[idx] == '\n') {
            endIdx = idx - 1;
            memset(freqBuffer, 0, sizeof(freqBuffer));
            for (i = 0; i <= (endIdx - startIdx); i++) {
                freqBuffer[i] = freqTableBuffer[startIdx + i];
            }
            sscanf(freqBuffer, "%ld", &govData[policy].freqTable[govData[policy].freqTableItemNum]);
            govData[policy].freqTableItemNum++;
        }
    }

    long int tempFreq;
    if (govData[policy].freqTable[govData[policy].freqTableItemNum - 1] < govData[policy].freqTable[0]) {
        for (i = 0; i < (govData[policy].freqTableItemNum / 2); i++) {
            tempFreq = govData[policy].freqTable[i];
            govData[policy].freqTable[i] = govData[policy].freqTable[govData[policy].freqTableItemNum - 1 - i];
            govData[policy].freqTable[govData[policy].freqTableItemNum - i - 1] = tempFreq;
        }
    }

    long int cpuInfoMaxFreq;
    sscanf(ReadFile(NULL, "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq", firstCpu), "%ld", &cpuInfoMaxFreq);
    if (cpuInfoMaxFreq > govData[policy].freqTable[govData[policy].freqTableItemNum - 1]) {
        govData[policy].freqTable[govData[policy].freqTableItemNum] = cpuInfoMaxFreq;
        govData[policy].freqTableItemNum++;
    }

    long int minDiffFreq = INT_MAX;
    int targetFreqIdx = 0;
    itemBuffer = cJSON_GetObjectItem(objectBuffer, "lowPowerFreq");
    govData[policy].lowPowerFreq = (*itemBuffer).valueint * 1000;
    for (i = 0; i < govData[policy].freqTableItemNum; i++) {
        if (labs(govData[policy].freqTable[i] - govData[policy].lowPowerFreq) < minDiffFreq) {
            minDiffFreq = labs(govData[policy].freqTable[i] - govData[policy].lowPowerFreq);
            targetFreqIdx = i;
        }
    }
    govData[policy].lowPowerFreq = govData[policy].freqTable[targetFreqIdx];

    minDiffFreq = INT_MAX;
    targetFreqIdx = 0;
    itemBuffer = cJSON_GetObjectItem(objectBuffer, "basicFreq");
    govData[policy].basicFreq = (*itemBuffer).valueint * 1000;
    for (i = 0; i < govData[policy].freqTableItemNum; i++) {
        if (labs(govData[policy].freqTable[i] - govData[policy].basicFreq) < minDiffFreq) {
            minDiffFreq = labs(govData[policy].freqTable[i] - govData[policy].basicFreq);
            targetFreqIdx = i;
        }
    }
    govData[policy].basicFreq = govData[policy].freqTable[targetFreqIdx];

    minDiffFreq = INT_MAX;
    targetFreqIdx = 0;
    itemBuffer = cJSON_GetObjectItem(objectBuffer, "expectFreq");
    govData[policy].expectFreq = (*itemBuffer).valueint * 1000;
    for (i = 0; i < govData[policy].freqTableItemNum; i++) {
        if (labs(govData[policy].freqTable[i] - govData[policy].expectFreq) < minDiffFreq) {
            minDiffFreq = labs(govData[policy].freqTable[i] - govData[policy].expectFreq);
            targetFreqIdx = i;
        }
    }
    govData[policy].expectFreq = govData[policy].freqTable[targetFreqIdx];

    if (govData[policy].expectFreq < govData[policy].basicFreq || govData[policy].basicFreq < govData[policy].lowPowerFreq) {
        WriteLog("E", "Policy config does not meet the requirements.");
        exit(0);
    }

    itemBuffer = cJSON_GetObjectItem(objectBuffer, "modelFreq");
    govData[policy].modelFreq = (*itemBuffer).valueint * 1000;
    itemBuffer = cJSON_GetObjectItem(objectBuffer, "modelPower");
    int modelPower = itemBuffer->valueint;
    float powerConst = (float)modelPower / (govData[policy].modelFreq / 1000);
    float lowPowerVoltStair = (float)(800 - 400) / ((govData[policy].expectFreq - govData[policy].freqTable[0]) / 1000);
    float expectVoltStair = (float)(1000 - 800) / ((govData[policy].modelFreq - govData[policy].expectFreq) / 1000);
    float unexpectVoltStair = 0.5f;
    float cpuVolt = 0;
    for (i = 0; i < govData[policy].freqTableItemNum; i++) {
        if (govData[policy].freqTable[i] <= govData[policy].expectFreq) {
            cpuVolt = 400 + (govData[policy].freqTable[i] - govData[policy].freqTable[0]) / 1000 * lowPowerVoltStair;
        } else if (govData[policy].freqTable[i] <= (govData[policy].modelFreq * 1000)) {
            cpuVolt = 800 + (govData[policy].freqTable[i] - govData[policy].expectFreq) / 1000 * expectVoltStair;
        } else {
            cpuVolt = 1000 + (govData[policy].freqTable[i] - govData[policy].modelFreq) / 1000 * unexpectVoltStair;
        }
        govData[policy].powerTable[i] = powerConst * ((long long int)(govData[policy].freqTable[i] / 1000) * (cpuVolt * cpuVolt)) / 1000000;
    }

    cJSON_Delete(jsonBuffer);
}

void InitCoCpuGovernor(void)
{
    cJSON* jsonBuffer = cJSON_Parse(configData);
    cJSON* objectBuffer = cJSON_GetObjectItem(jsonBuffer, "cpuPolicy");

    cJSON* itemBuffer = NULL;

    int i, j;
    int firstCpu = 0;
    int lastCpu = 0;
    int nowaPolicy = 0;
    int prevPolicy = 0;
    for (i = 0; i <= 9; i++) {
        itemBuffer = cJSON_GetObjectItem(objectBuffer, StrMerge("cpu%d", i));
        prevPolicy = nowaPolicy;
        nowaPolicy = itemBuffer->valueint;
        if (nowaPolicy != prevPolicy) {
            lastCpu = i - 1;
            InitPolicyData(prevPolicy, firstCpu, lastCpu);
            firstCpu = i;
        }
        if (nowaPolicy == -1) {
            cpuCoreNum = i;
            cpuClusterNum = prevPolicy + 1;
            break;
        }
    }

    for (i = 0; i < cpuClusterNum; i++) {
        WriteLog("I", "CPU Policy%d (%d-%d) PowerModel:", govData[i].policy, govData[i].firstCpu, govData[i].lastCpu);
        for (j = 0; j < govData[i].freqTableItemNum; j++) {
            WriteLog("I", "Idx=%d, Freq=%ld MHz, Power=%d mW.", j, govData[i].freqTable[j] / 1000, govData[i].powerTable[j]);
        }
    }

    CheckCpuFreqWriter();

    cJSON_Delete(jsonBuffer);
}

void BoostTimer(void)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "BoostTimer");

    while (boostDurationTime > 0) {
        usleep(dynamicGovData.boostDurationSleepTime * 1000);
        boostDurationTime -= dynamicGovData.boostDurationSleepTime;
    }

    boostDurationTime = 0;
    govBoost = 0;

    pthread_exit(0);
}

void TriggerBoost(int boostType)
{
    int nowaBoostDurationTime = boostDurationTime;
    if (dynamicGovData.govBoost[boostType] >= govBoost) {
        govBoost = dynamicGovData.govBoost[boostType];
        boostDurationTime = dynamicGovData.boostDurationTime[boostType];
        if (nowaBoostDurationTime == 0) {
            pthread_create(&threadsTid, NULL, (void*)BoostTimer, NULL);
        }
    }
}

void InputListener(long int ts_event)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "InputListener");

    struct input_event ts;
    struct input_absinfo ts_x_info;
    struct input_absinfo ts_y_info;
    char touch_screen_path[128];
    int start_x = 0;
    int start_y = 0;
    int swipe_x = 0;
    int swipe_y = 0;
    int touch_x = 0;
    int touch_y = 0;
    int touch_s = 0;
    int last_s = 0;
    int ret = -1;

    sprintf(touch_screen_path, "/dev/input/event%ld", ts_event);

    int fd = open(touch_screen_path, O_RDONLY);
    if (fd <= 0) {
        WriteLog("W", "Failed to open \"%s\".", touch_screen_path);
        pthread_exit(0);
    }

    ioctl(fd, EVIOCGABS(ABS_MT_POSITION_X), &ts_x_info);
    ioctl(fd, EVIOCGABS(ABS_MT_POSITION_Y), &ts_y_info);
    if (ts_y_info.maximum > INT_MAX || ts_x_info.maximum > INT_MAX) {
        WriteLog("W", "TouchScreen Position too large.");
        pthread_exit(0);
    } else {
        WriteLog("I", "Listening \"%s\", absinfo: x=%d-%d, y=%d-%d.", touch_screen_path, ts_x_info.minimum, ts_x_info.maximum, ts_y_info.minimum, ts_y_info.maximum);
    }

    int touch_screen_width = ts_x_info.maximum - ts_x_info.minimum + 1;
    int touch_screen_height = ts_y_info.maximum - ts_y_info.minimum + 1;
    int swipe_range = touch_screen_width / 20;
    int gesture_range = touch_screen_width / 10;
    if (touch_screen_height < touch_screen_width) {
        swipe_range = touch_screen_height / 20;
        gesture_range = touch_screen_height / 10;
    }
    int gesture_left = gesture_range;
    int gesture_right = touch_screen_width - gesture_range;
    int gesture_top = gesture_range;
    int gesture_bottom = touch_screen_height - gesture_range;

    while (1) {
        ret = read(fd, &ts, sizeof(ts));
        if (ret > 0) {
            last_s = touch_s;
            if (ts.code == BTN_TOUCH) {
                touch_s = ts.value;
            } else if (ts.code == ABS_MT_POSITION_X) {
                touch_x = ts.value;
            } else if (ts.code == ABS_MT_POSITION_Y) {
                touch_y = ts.value;
            }
            if ((touch_s - last_s) == 1) {
                start_x = touch_x;
                start_y = touch_y;
                TriggerBoost(BOOST_TOUCH);
            } else if ((touch_s - last_s) == 0 && touch_s == 1) {
                swipe_x = touch_x - start_x;
                swipe_y = touch_y - start_y;
                if (abs(swipe_y) > swipe_range || abs(swipe_x) > swipe_range) {
                    TriggerBoost(BOOST_SWIPE);
                }
            } else if ((touch_s - last_s) == -1) {
                swipe_x = touch_x - start_x;
                swipe_y = touch_y - start_y;
                if (start_x > gesture_right && abs(swipe_x) > gesture_range) {
                    TriggerBoost(BOOST_GESTURE);
                } else if (start_x < gesture_left && abs(swipe_x) > gesture_range) {
                    TriggerBoost(BOOST_GESTURE);
                } else if (start_y < gesture_top && abs(swipe_y) > gesture_range) {
                    TriggerBoost(BOOST_GESTURE);
                } else if (start_y > gesture_bottom && abs(swipe_y) > gesture_range) {
                    TriggerBoost(BOOST_GESTURE);
                } else {
                    TriggerBoost(BOOST_TOUCH);
                }
            }
        } else {
            touch_s = 0;
            touch_x = 0;
            touch_y = 0;
            usleep(50000);
        }
    }

    close(fd);
}

void RunInputListener(void)
{
    char abs_bitmask[(ABS_MAX + 1) / 8];
    struct dirent* entry;
    int ts_event;
    int fd;

    DIR* dir = opendir("/dev/input");
    if (dir != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            fd = open(StrMerge("/dev/input/%s", (*entry).d_name), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
            if (fd > 0) {
                memset(abs_bitmask, 0, sizeof(abs_bitmask));
                ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask);
                if (CHECK_BIT(ABS_MT_POSITION_X, abs_bitmask) && CHECK_BIT(ABS_MT_POSITION_Y, abs_bitmask)) {
                    sscanf((*entry).d_name, "event%d", &ts_event);
                    pthread_create(&threadsTid, NULL, (void*)InputListener, (void*)(long)ts_event);
                }
                close(fd);
            }
        }
        closedir(dir);
    }
}

int TasksetHelper_Enabled = 0;
cpu_set_t MainThread_Mask;
cpu_set_t GameSingleThread_Mask;
cpu_set_t GameMultiThread_Mask;
cpu_set_t RenderThread_Mask;
cpu_set_t UIThread_Mask;
cpu_set_t MediaThread_Mask;
cpu_set_t WebviewThread_Mask;
cpu_set_t ProcessThread_Mask;
cpu_set_t NonRealTimeThread_Mask;
cpu_set_t OtherThread_Mask;
struct sched_param MainThread_Prio;
struct sched_param GameSingleThread_Prio;
struct sched_param GameMultiThread_Prio;
struct sched_param RenderThread_Prio;
struct sched_param UIThread_Prio;
struct sched_param MediaThread_Prio;
struct sched_param WebviewThread_Prio;
struct sched_param ProcessThread_Prio;
struct sched_param NonRealTimeThread_Prio;
struct sched_param OtherThread_Prio;

void InitTasksetHelper(void)
{
    cJSON* jsonBuffer = cJSON_Parse(configData);
    cJSON* objectBuffer = cJSON_GetObjectItem(jsonBuffer, "TasksetHelper_Config");

    cJSON* folderBuffer = NULL;
    cJSON* itemBuffer = NULL;

    itemBuffer = cJSON_GetObjectItem(objectBuffer, "enable");
    TasksetHelper_Enabled = itemBuffer->valueint;

    char cpuMask[10];
    char cpuName[1];
    int i, cpuCore;

    folderBuffer = cJSON_GetObjectItem(objectBuffer, "MainThread");
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "cpus");
    sscanf(itemBuffer->valuestring, "%s", cpuMask);
    CPU_ZERO(&MainThread_Mask);
    for (i = 0; i < strlen(cpuMask); i++) {
        cpuName[0] = cpuMask[i];
        cpuCore = atoi(cpuName);
        CPU_SET(cpuCore, &MainThread_Mask);
    }
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "nice");
    MainThread_Prio.sched_priority = 120 + itemBuffer->valueint;

    folderBuffer = cJSON_GetObjectItem(objectBuffer, "GameSingleThread");
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "cpus");
    sscanf(itemBuffer->valuestring, "%s", cpuMask);
    CPU_ZERO(&GameSingleThread_Mask);
    for (i = 0; i < strlen(cpuMask); i++) {
        cpuName[0] = cpuMask[i];
        cpuCore = atoi(cpuName);
        CPU_SET(cpuCore, &GameSingleThread_Mask);
    }
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "nice");
    GameSingleThread_Prio.sched_priority = 120 + itemBuffer->valueint;

    folderBuffer = cJSON_GetObjectItem(objectBuffer, "GameMultiThread");
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "cpus");
    sscanf(itemBuffer->valuestring, "%s", cpuMask);
    CPU_ZERO(&GameMultiThread_Mask);
    for (i = 0; i < strlen(cpuMask); i++) {
        cpuName[0] = cpuMask[i];
        cpuCore = atoi(cpuName);
        CPU_SET(cpuCore, &GameMultiThread_Mask);
    }
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "nice");
    GameMultiThread_Prio.sched_priority = 120 + itemBuffer->valueint;

    folderBuffer = cJSON_GetObjectItem(objectBuffer, "RenderThread");
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "cpus");
    sscanf(itemBuffer->valuestring, "%s", cpuMask);
    CPU_ZERO(&RenderThread_Mask);
    for (i = 0; i < strlen(cpuMask); i++) {
        cpuName[0] = cpuMask[i];
        cpuCore = atoi(cpuName);
        CPU_SET(cpuCore, &RenderThread_Mask);
    }
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "nice");
    RenderThread_Prio.sched_priority = 120 + itemBuffer->valueint;

    folderBuffer = cJSON_GetObjectItem(objectBuffer, "UIThread");
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "cpus");
    sscanf(itemBuffer->valuestring, "%s", cpuMask);
    CPU_ZERO(&UIThread_Mask);
    for (i = 0; i < strlen(cpuMask); i++) {
        cpuName[0] = cpuMask[i];
        cpuCore = atoi(cpuName);
        CPU_SET(cpuCore, &UIThread_Mask);
    }
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "nice");
    UIThread_Prio.sched_priority = 120 + itemBuffer->valueint;

    folderBuffer = cJSON_GetObjectItem(objectBuffer, "MediaThread");
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "cpus");
    sscanf(itemBuffer->valuestring, "%s", cpuMask);
    CPU_ZERO(&MediaThread_Mask);
    for (i = 0; i < strlen(cpuMask); i++) {
        cpuName[0] = cpuMask[i];
        cpuCore = atoi(cpuName);
        CPU_SET(cpuCore, &MediaThread_Mask);
    }
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "nice");
    MediaThread_Prio.sched_priority = 120 + itemBuffer->valueint;

    folderBuffer = cJSON_GetObjectItem(objectBuffer, "WebviewThread");
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "cpus");
    sscanf(itemBuffer->valuestring, "%s", cpuMask);
    CPU_ZERO(&WebviewThread_Mask);
    for (i = 0; i < strlen(cpuMask); i++) {
        cpuName[0] = cpuMask[i];
        cpuCore = atoi(cpuName);
        CPU_SET(cpuCore, &WebviewThread_Mask);
    }
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "nice");
    WebviewThread_Prio.sched_priority = 120 + itemBuffer->valueint;

    folderBuffer = cJSON_GetObjectItem(objectBuffer, "ProcessThread");
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "cpus");
    sscanf(itemBuffer->valuestring, "%s", cpuMask);
    CPU_ZERO(&ProcessThread_Mask);
    for (i = 0; i < strlen(cpuMask); i++) {
        cpuName[0] = cpuMask[i];
        cpuCore = atoi(cpuName);
        CPU_SET(cpuCore, &ProcessThread_Mask);
    }
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "nice");
    ProcessThread_Prio.sched_priority = 120 + itemBuffer->valueint;

    folderBuffer = cJSON_GetObjectItem(objectBuffer, "NonRealTimeThread");
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "cpus");
    sscanf(itemBuffer->valuestring, "%s", cpuMask);
    CPU_ZERO(&NonRealTimeThread_Mask);
    for (i = 0; i < strlen(cpuMask); i++) {
        cpuName[0] = cpuMask[i];
        cpuCore = atoi(cpuName);
        CPU_SET(cpuCore, &NonRealTimeThread_Mask);
    }
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "nice");
    NonRealTimeThread_Prio.sched_priority = 120 + itemBuffer->valueint;

    folderBuffer = cJSON_GetObjectItem(objectBuffer, "OtherThread");
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "cpus");
    sscanf(itemBuffer->valuestring, "%s", cpuMask);
    CPU_ZERO(&OtherThread_Mask);
    for (i = 0; i < strlen(cpuMask); i++) {
        cpuName[0] = cpuMask[i];
        cpuCore = atoi(cpuName);
        CPU_SET(cpuCore, &OtherThread_Mask);
    }
    itemBuffer = cJSON_GetObjectItem(folderBuffer, "nice");
    OtherThread_Prio.sched_priority = 120 + itemBuffer->valueint;

    cJSON_Delete(jsonBuffer);
}

void TasksetHelper(void)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "TasksetHelper");

    char buf[8];
    char threadName[32];
    char packageName[128];
    int pid, tid, taskType;

    long int nowaThreadRuntime = 0;
    long int prevThreadRuntime[128] = { 0 };
    int threadRuntimeDiff[128] = { 0 };
    int maxThreadRuntimeDiff = 0;
    int singlePerfThreadsTidList[128] = { 0 };
    int singlePerfThreadsPidList[128] = { 0 };
    int singlePerfThreadsNum = 0;
    int prevMaxUsageThreadTid = -1;
    int nowaMaxUsageThreadTid = -1;

    FILE* fp = fopen("/dev/cpuset/top-app/tasks", "r");
    if (fp) {
        while (fgets(buf, sizeof(buf), fp) != NULL) {
            sscanf(buf, "%d", &tid);
            pid = GetThreadPid(tid);
            taskType = GetTaskType(pid);
            if (taskType == TASK_FOREGROUND || taskType == TASK_VISIBLE) {
                ReadFile(threadName, "/proc/%d/task/%d/comm", pid, tid);
                if (CheckRegex(threadName, "^(UnityMain|MainThread|GameThread|SDLThread|MINECRAFT|GLThread|CoreThread)")) {
                    if (singlePerfThreadsNum < 128) {
                        singlePerfThreadsTidList[singlePerfThreadsNum] = tid;
                        singlePerfThreadsPidList[singlePerfThreadsNum] = pid;
                        singlePerfThreadsNum++;
                    }
                    sched_setscheduler(tid, SCHED_NORMAL, &GameSingleThread_Prio);
                } else if (CheckRegex(threadName, "^(UnityMulti|UnityGfx|UnityPreload|UnityChoreograp|Worker Thread|Job.Worker|CmpJob|glp|glt|TaskGraph|LoadingThread|Program Thread|Es2Thread|RHIThread)")) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &GameMultiThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &GameMultiThread_Prio);
                } else if (CheckRegex(threadName, "^(RenderThread|RenderEngine)")) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &RenderThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &RenderThread_Prio);
                } else if (CheckRegex(threadName, "^(UiThread|[Vv]sync|[Bb]lur|[Aa]nim|JNISurfaceText|Msg.GLThread|mali-|GNaviMap-GL)|([.]raster|[.][Uu][Ii]|[Rr]ender|[Aa]nim|[Bb]lur)$")) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &UIThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &UIThread_Prio);
                } else if (CheckRegex(threadName, "^([Aa]udio|[Mm]ixer|[Vv]ideo|[Mm]edia|FMedia|Vlc|IjkMediaPlayer|IJK_External|GVoice|ExoPlayer|SuperPlayer|[Dd]ecode|[Cc]odec)|([Dd]ecode|[Cc]odec)$")) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &MediaThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &MediaThread_Prio);
                } else if (CheckRegex(threadName, "^(Chrome|Chromium|Gecko|[Ww]eb[Vv]iew|Compositor|CrGpuMain|CrRenderer|Viz)|([Ww]eb[Vv]iew)$")) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &WebviewThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &WebviewThread_Prio);
                } else if (CheckRegex(threadName, "^(ForkJoinPool-|[Gg]esture|.gifmaker|Binder|work_thread|OkHttp|ThreadPool|Busy-|[Mm]ap-|.navi.mainframe|Camera)|(.fg|.io)$")) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &ProcessThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &ProcessThread_Prio);
                } else if (CheckRegex(threadName, "^(launcher-|Fresco|ResolvePool|ExecutorDispatc|XYThread|RecallManager|Apollo-|[Rr]untime|ImageCache|DefaultDispatch|pcdn_udp)")) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &ProcessThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &ProcessThread_Prio);
                } else if (CheckRegex(threadName, "^(FAsync|[Aa]sync|mqt_|LoadingActivity|queued-work|[Jj]ava[Ss]cript|JS|[Nn]et[Ww]ork|[Dd]ownload)|([Dd]ownload|[Aa]sync)$")) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &ProcessThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &ProcessThread_Prio);
                } else if (CheckRegex(threadName, "^(ged-|GPU completion|SmartThread|MIHOYO_NETWORK|master_engine|MemoryInfra|AILocalThread|FrameThread|TXMap|FileObserver)")) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &ProcessThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &ProcessThread_Prio);
                } else if (CheckRegex(threadName, "^(glide-|pool-|FramePolicy|ScrollPolicy|Turing|hwui|WeexJsBridge|V8 DefaultWork|libweexjsb|Socket Thread|TXTextureThread|PoolThread)")) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &ProcessThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &ProcessThread_Prio);
                } else if (CheckRegex(threadName, "^(URL Classifier|PlaceStorage|Cache2 I/O|JavaBridge|WorkHandler|UNET|UNet|NativeThread|hippy.js|transThread|netmod|FMOD|APM|IL2CPP)")) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &ProcessThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &ProcessThread_Prio);
                } else if (CheckRegex(threadName, "^(HeapTaskDaemon|FinalizerDaemon|FinalizerWatch|Jit thread pool|Signal Catcher|crashhunter|xg_vip_service|default_matrix|matrix)")) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &NonRealTimeThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &NonRealTimeThread_Prio);
                } else if (CheckRegex(threadName, "^(ReferenceQueue|Timer-|log|xcrash|xlog|[Bb]ugly|[Bb]ackground|thread-ad|tt_pangle|[Rr]eport|Profile|SearchDaemon)|([Bb]ackground|.bg)$")) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &NonRealTimeThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &NonRealTimeThread_Prio);
                } else if (CheckRegex(threadName, "^Thread-")) {
                    if (strstr(ReadFile(NULL, "/proc/%d/cmdline", pid), "netease")) {
                        if (singlePerfThreadsNum < 128) {
                            singlePerfThreadsTidList[singlePerfThreadsNum] = tid;
                            singlePerfThreadsPidList[singlePerfThreadsNum] = pid;
                            singlePerfThreadsNum++;
                        }
                        sched_setscheduler(tid, SCHED_NORMAL, &GameSingleThread_Prio);
                    } else {
                        sched_setaffinity(tid, sizeof(cpu_set_t), &GameMultiThread_Mask);
                        sched_setscheduler(tid, SCHED_NORMAL, &GameMultiThread_Prio);
                    }
                } else if (strcmp(threadName, ReadFile(NULL, "/proc/%d/comm", pid)) == 0) {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &MainThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &MainThread_Prio);
                } else {
                    sched_setaffinity(tid, sizeof(cpu_set_t), &OtherThread_Mask);
                    sched_setscheduler(tid, SCHED_NORMAL, &OtherThread_Prio);
                }
            }
        }
        fclose(fp);
    }

    if (singlePerfThreadsNum < 2) {
        if (singlePerfThreadsNum == 1) {
            sched_setaffinity(singlePerfThreadsTidList[0], sizeof(cpu_set_t), &GameSingleThread_Mask);
        }
        pthread_exit(0);
    }

    int i;
    for (i = 1; i < singlePerfThreadsNum; i++) {
        sched_setaffinity(singlePerfThreadsTidList[i], sizeof(cpu_set_t), &GameMultiThread_Mask);
    }
    sched_setaffinity(singlePerfThreadsTidList[0], sizeof(cpu_set_t), &GameSingleThread_Mask);

    prevMaxUsageThreadTid = singlePerfThreadsTidList[0];
    nowaMaxUsageThreadTid = singlePerfThreadsTidList[0];

    while (tasksetTid == pthread_self()) {
        for (i = 0; i < singlePerfThreadsNum; i++) {
            nowaThreadRuntime = GetThreadRuntime(singlePerfThreadsPidList[i], singlePerfThreadsTidList[i]);
            threadRuntimeDiff[i] = nowaThreadRuntime - prevThreadRuntime[i];
            prevThreadRuntime[i] = nowaThreadRuntime;
        }

        prevMaxUsageThreadTid = nowaMaxUsageThreadTid;
        maxThreadRuntimeDiff = 0;
        for (i = 0; i < singlePerfThreadsNum; i++) {
            if (threadRuntimeDiff[i] > maxThreadRuntimeDiff) {
                nowaMaxUsageThreadTid = singlePerfThreadsTidList[i];
                maxThreadRuntimeDiff = threadRuntimeDiff[i];
            }
        }

        if (nowaMaxUsageThreadTid != prevMaxUsageThreadTid) {
            sched_setaffinity(prevMaxUsageThreadTid, sizeof(cpu_set_t), &GameMultiThread_Mask);
            sched_setaffinity(nowaMaxUsageThreadTid, sizeof(cpu_set_t), &GameSingleThread_Mask);
        }

        usleep(500000);
    }

    pthread_exit(0);
}

void CgroupWatcher(void)
{
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, "CgroupWatcher");

    int i;

    int nowaTopActivityPid = -1;
    int prevTopActivityPid = -1;
    char topActivityName[128];

    int fd = inotify_init();
    if (fd <= 0) {
        WriteLog("E", "Failed to init inotify.");
        pthread_exit(0);
    }
    int ta_wd, fg_wd, bg_wd, re_wd;
    if (GetAndroidSDKVersion() < 33) {
        ta_wd = inotify_add_watch(fd, "/dev/cpuset/top-app/tasks", IN_MODIFY);
        if (ta_wd <= 0) {
            WriteLog("W", "Failed to watch top-app cgroup.");
        }
        fg_wd = inotify_add_watch(fd, "/dev/cpuset/foreground/tasks", IN_MODIFY);
        if (fg_wd <= 0) {
            WriteLog("W", "Failed to watch foreground cgroup.");
        }
        bg_wd = inotify_add_watch(fd, "/dev/cpuset/background/tasks", IN_MODIFY);
        if (bg_wd <= 0) {
            WriteLog("W", "Failed to watch background cgroup.");
        }
        re_wd = inotify_add_watch(fd, "/dev/cpuset/restricted/tasks", IN_MODIFY);
        if (re_wd <= 0) {
            WriteLog("W", "Failed to watch restricted cgroup.");
        }
    } else {
        ta_wd = inotify_add_watch(fd, "/dev/cpuset/top-app/cgroup.procs", IN_MODIFY);
        if (ta_wd <= 0) {
            WriteLog("W", "Failed to watch top-app cgroup.");
        }
        fg_wd = inotify_add_watch(fd, "/dev/cpuset/foreground/cgroup.procs", IN_MODIFY);
        if (fg_wd <= 0) {
            WriteLog("W", "Failed to watch foreground cgroup.");
        }
        bg_wd = inotify_add_watch(fd, "/dev/cpuset/background/cgroup.procs", IN_MODIFY);
        if (bg_wd <= 0) {
            WriteLog("W", "Failed to watch background cgroup.");
        }
        re_wd = inotify_add_watch(fd, "/dev/cpuset/restricted/cgroup.procs", IN_MODIFY);
        if (re_wd <= 0) {
            WriteLog("W", "Failed to watch restricted cgroup.");
        }
    }

    char inotify_buf[16];
    struct inotify_event* watchEvent;

    while (1) {
        read(fd, inotify_buf, sizeof(inotify_buf));
        watchEvent = (struct inotify_event*)inotify_buf;
        if (watchEvent->mask == IN_MODIFY) {
            if (screenState == SCREEN_ON) {
                prevTopActivityPid = nowaTopActivityPid;
                if (watchEvent->wd == ta_wd || watchEvent->wd == fg_wd) {
                    sscanf(CutString(DumpTopActivityInfo(), 9), "%d:%s", &nowaTopActivityPid, topActivityName);
                    if (strstr(topActivityName, "systemui")) {
                        nowaTopActivityPid = prevTopActivityPid;
                    }
                }
                if (prevTopActivityPid != nowaTopActivityPid) {
                    TriggerBoost(BOOST_HEAVYLOAD);
                }

                if (TasksetHelper_Enabled) {
                    if (watchEvent->wd == ta_wd || watchEvent->wd == fg_wd) {
                        pthread_create(&tasksetTid, NULL, (void*)TasksetHelper, NULL);
                    }
                }

                if ((screenState = GetScreenState()) == SCREEN_OFF) {
                    tasksetTid = -1;
                    usleep(100000);
                    for (i = 0; i < cpuClusterNum; i++) {
                        WriteCpuFreqViaMSM(govData[i].firstCpu, govData[i].freqTable[0], govData[i].freqTable[govData[i].freqTableItemNum - 1]);
                        WriteCpuFreqViaPPM(govData[i].policy, govData[i].freqTable[0], govData[i].freqTable[govData[i].freqTableItemNum - 1]);
                        WriteCpuFreqViaEpic(govData[i].policy, govData[i].freqTable[0], govData[i].freqTable[govData[i].freqTableItemNum - 1]);
                        WriteCpuFreqViaGovernor(govData[i].firstCpu, govData[i].freqTable[0], govData[i].freqTable[govData[i].freqTableItemNum - 1]);
                    }
                }
            } else {
                if ((screenState = GetScreenState()) == SCREEN_ON) {
                    KernelGovernorOpt();
                    pthread_create(&threadsTid, NULL, (void*)CoCpuGovernor, NULL);
                    TriggerBoost(BOOST_HEAVYLOAD);
                }
            }
        }
        usleep(20000);
    }
    close(fd);

    pthread_exit(0);
}

void GetModeDynamicData(void)
{
    cJSON* jsonBuffer = cJSON_Parse(configData);
    cJSON* objectBuffer = cJSON_GetObjectItem(jsonBuffer, "modeSwitcher");
    cJSON* modeObjectBuffer = cJSON_GetObjectItem(objectBuffer, curMode);

    cJSON* folderBuffer = NULL;
    cJSON* secFolderBuffer = NULL;
    cJSON* itemBuffer = NULL;

    itemBuffer = cJSON_GetObjectItem(modeObjectBuffer, "powerLimit");
    dynamicGovData.powerLimit = itemBuffer->valueint;

    int i, j;
    for (i = 0; i < cpuClusterNum; i++) {
        folderBuffer = cJSON_GetObjectItem(modeObjectBuffer, StrMerge("policy%d", i));
        itemBuffer = cJSON_GetObjectItem(folderBuffer, "upRateLatency");
        dynamicGovData.upRateLatency[i] = itemBuffer->valueint;
        itemBuffer = cJSON_GetObjectItem(folderBuffer, "downRateDelay");
        dynamicGovData.downRateDelay[i] = itemBuffer->valueint;
    }

    folderBuffer = cJSON_GetObjectItem(modeObjectBuffer, "boostConfig");

    dynamicGovData.govBoost[BOOST_NONE] = 0;
    dynamicGovData.boostDurationTime[BOOST_NONE] = 0;

    secFolderBuffer = cJSON_GetObjectItem(folderBuffer, "touch");
    itemBuffer = cJSON_GetObjectItem(secFolderBuffer, "boost");
    dynamicGovData.govBoost[BOOST_TOUCH] = itemBuffer->valueint;
    itemBuffer = cJSON_GetObjectItem(secFolderBuffer, "durationTime");
    dynamicGovData.boostDurationTime[BOOST_TOUCH] = itemBuffer->valueint;

    secFolderBuffer = cJSON_GetObjectItem(folderBuffer, "swipe");
    itemBuffer = cJSON_GetObjectItem(secFolderBuffer, "boost");
    dynamicGovData.govBoost[BOOST_SWIPE] = itemBuffer->valueint;
    itemBuffer = cJSON_GetObjectItem(secFolderBuffer, "durationTime");
    dynamicGovData.boostDurationTime[BOOST_SWIPE] = itemBuffer->valueint;

    secFolderBuffer = cJSON_GetObjectItem(folderBuffer, "gesture");
    itemBuffer = cJSON_GetObjectItem(secFolderBuffer, "boost");
    dynamicGovData.govBoost[BOOST_GESTURE] = itemBuffer->valueint;
    itemBuffer = cJSON_GetObjectItem(secFolderBuffer, "durationTime");
    dynamicGovData.boostDurationTime[BOOST_GESTURE] = itemBuffer->valueint;

    secFolderBuffer = cJSON_GetObjectItem(folderBuffer, "heavyload");
    itemBuffer = cJSON_GetObjectItem(secFolderBuffer, "boost");
    dynamicGovData.govBoost[BOOST_HEAVYLOAD] = itemBuffer->valueint;
    itemBuffer = cJSON_GetObjectItem(secFolderBuffer, "durationTime");
    dynamicGovData.boostDurationTime[BOOST_HEAVYLOAD] = itemBuffer->valueint;

    cJSON_Delete(jsonBuffer);

    int minDurationTime = INT_MAX;
    for (i = 0; i < 5; i++) {
        if (dynamicGovData.boostDurationTime[i] < minDurationTime && dynamicGovData.boostDurationTime[i] != 0) {
            minDurationTime = dynamicGovData.boostDurationTime[i];
        }
    }
    for (i = minDurationTime; i > 0; i--) {
        int matchNum = 0;
        for (j = 0; j < 5; j++) {
            if (dynamicGovData.boostDurationTime[j] % i == 0 || dynamicGovData.boostDurationTime[j] == 0) {
                matchNum++;
            }
        }
        if (matchNum == 5) {
            dynamicGovData.boostDurationSleepTime = i;
            break;
        }
    }
}

int main(int argc, char* argv[])
{
    const char compileDate[12] = __DATE__;

    sprintf(daemonPath, "%s", GetDirName(argv[0]));
    sprintf(argv[0], "CuDaemon");

    if (argc < 2) {
        printf("Wrong input.\n");
        printf("Try CuDaemon --help\n");
        exit(0);
    } else if (strcmp(argv[1], "-R") == 0) {
        if (argc == 5) {
            sprintf(configPath, "%s", argv[2]);
            sprintf(modePath, "%s", argv[3]);
            sprintf(logPath, "%s", argv[4]);
            printf("Daemon Running.\n");
            daemon(0, 0);
        } else {
            printf("Wrong input.\n");
            printf("Try CuDaemon --help\n");
            exit(0);
        }
    } else if (strcmp(argv[1], "-V") == 0) {
        printf("CuprumTurbo Scheduler Daemon\n");
        printf("Version: 13\n");
        printf("ComplieDate: %s\n", compileDate);
        printf("Author: chenzyadb(chenzyyzd)\n");
        printf("Repo: https://github.com/chenzyyzd/CuprumTurbo-Scheduler\n");
        exit(0);
    } else if (strcmp(argv[1], "--help") == 0) {
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

    InitLogWriter();
    WriteLog("I", "CuprumTurbo Scheduler V13 (%ld) by chenzyadb.", GetCompileDateCode(compileDate));

    if (!IsFileExist(configPath)) {
        WriteLog("E", "File \"%s\" doesn't exist.", configPath);
        exit(0);
    }
    if (!IsFileExist(modePath)) {
        WriteLog("E", "File \"%s\" doesn't exist.", modePath);
        exit(0);
    }

    WriteLog("I", "Scheduler Initizalizing.");

    InitConfigReader();
    InitCoCpuGovernor();
    InitTasksetHelper();
    GetModeDynamicData();
    RunInputListener();
    KernelGovernorOpt();

    int i;
    for (i = 0; i < cpuClusterNum; i++) {
        WriteCpuFreqViaMSM(govData[i].firstCpu, govData[i].freqTable[0], govData[i].freqTable[govData[i].freqTableItemNum - 1]);
        WriteCpuFreqViaPPM(govData[i].policy, govData[i].freqTable[0], govData[i].freqTable[govData[i].freqTableItemNum - 1]);
        WriteCpuFreqViaEpic(govData[i].policy, govData[i].freqTable[0], govData[i].freqTable[govData[i].freqTableItemNum - 1]);
        WriteCpuFreqViaGovernor(govData[i].firstCpu, govData[i].freqTable[0], govData[i].freqTable[govData[i].freqTableItemNum - 1]);
    }

    usleep(500000);

    pthread_create(&threadsTid, NULL, (void*)CgroupWatcher, NULL);
    pthread_create(&threadsTid, NULL, (void*)CoCpuGovernor, NULL);

    WriteLog("I", "Scheduler Running (daemon_pid = %d).", getpid());

    prctl(PR_SET_NAME, "ModeWatcher");

    char newMode[16];
    sscanf(ReadFile(NULL, modePath), "%s", newMode);
    if (strcmp(newMode, "powersave") == 0 || strcmp(newMode, "balance") == 0 || strcmp(newMode, "performance") == 0 || strcmp(newMode, "fast") == 0) {
        WriteLog("I", "Mode switch detected: null -> \"%s\".", newMode);
        sprintf(curMode, "%s", newMode);
    } else {
        WriteLog("I", "Mode switch detected: null -> \"balance\".");
        sprintf(curMode, "balance");
    }
    GetModeDynamicData();

    int fd = inotify_init();
    if (fd <= 0) {
        WriteLog("E", "Failed to init inotify.");
        exit(0);
    }
    int wd = inotify_add_watch(fd, modePath, IN_MODIFY);
    if (!wd) {
        WriteLog("E", "Failed to watch \"%s\"", modePath);
        exit(0);
    }

    char inotify_buf[16];
    struct inotify_event* watchEvent;

    while (1) {
        read(fd, inotify_buf, sizeof(inotify_buf));
        watchEvent = (struct inotify_event*)inotify_buf;
        if (watchEvent->mask == IN_MODIFY) {
            sscanf(ReadFile(NULL, modePath), "%s", newMode);
            if (strcmp(newMode, curMode) != 0) {
                if (strcmp(newMode, "powersave") == 0 || strcmp(newMode, "balance") == 0 || strcmp(newMode, "performance") == 0 || strcmp(newMode, "fast") == 0) {
                    WriteLog("I", "Mode switch detected: \"%s\" -> \"%s\".", curMode, newMode);
                    sprintf(curMode, "%s", newMode);
                }
                GetModeDynamicData();
            }
        }
        usleep(500000);
    }

    inotify_rm_watch(fd, wd);
    close(fd);

    return 0;
}
