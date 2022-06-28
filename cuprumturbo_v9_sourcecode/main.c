#define __USE_GNU
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <linux/input.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <time.h>
#define author chenzyadb
pthread_t tasksethelper_tid;
pthread_t usage_monitor_tid;
pthread_t cpu_governor_tid;
pthread_t thermal_monitor_tid;
pthread_t event_reader_tid[30];
char path[256];
char app_event[64];
char mode[32];
int foreground_pid=-1;
int core_num=0;
int touch_level=0;
int SCREEN_OFF=0;
int sc_max_usage=0;
int bc_max_usage=0;
int xc_max_usage=0;
int thermal_level=0;
int cpu_perf_mask[8]={0};
int sample_time=40000;
int sc_basic_freq_idx=0;
int bc_basic_freq_idx=0;
int xc_basic_freq_idx=0;
int sc_burst_freq_idx=0;
int bc_burst_freq_idx=0;
int xc_burst_freq_idx=0;
int sc_unattractive_freq_idx=0;
int bc_unattractive_freq_idx=0;
int xc_unattractive_freq_idx=0;
long int cluster0_freq_table[30]={0};
long int cluster1_freq_table[30]={0};
long int cluster2_freq_table[30]={0};
int sc_pwr_mask[30]={0};
int bc_pwr_mask[30]={0};
int xc_pwr_mask[30]={0};
int cluster0_cpu=-1;
int cluster1_cpu=-1;
int cluster2_cpu=-1;
int multi_perf_group[2]={0};
int single_perf_group[2]={0};
int efficiency_group[2]={0};
int comm_group[2]={0};
void write_log (const char *format, ...) {
	FILE *fp;
	fp = fopen("/storage/emulated/0/Android/data/xyz.chenzyadb.cu_toolbox/files/Cuprum_Log.txt","a");
    va_list arg;
    va_start(arg, format);
    time_t time_log = time(NULL);
    struct tm* tm_log = localtime(&time_log);
    fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d ", tm_log->tm_year + 1900, tm_log->tm_mon + 1, tm_log->tm_mday, tm_log->tm_hour, tm_log->tm_min, tm_log->tm_sec);
    vfprintf(fp, format, arg);
    va_end(arg);
    fprintf(fp,"\n");
    fflush(fp);
    fclose(fp);
}
void run_libcuprum(char command[64]) {
	char shell_buf[256];
	sprintf(shell_buf,"%s/libcuprum.sh \"%s\" ",path,command);
	system(shell_buf);
}
int get_config(void) {
	FILE *fp;
	int config_exist=0;
	char buf[64];
	char config_url[128];
	char platform_name[16];
	sprintf(config_url,"%s/config.txt",path);
	fp=popen("getprop ro.board.platform","r");
	fgets(buf,sizeof(buf),fp);
	sscanf(buf,"%s",platform_name);
	pclose(fp);
	fp=fopen(config_url,"r");
	while(fgets(buf,sizeof(buf),fp) != NULL) {
		if (strstr(buf,platform_name) && config_exist == 0) {
			config_exist=1;
			write_log("[I] platform %s supported, loading custom config.",platform_name);
		} else if (config_exist == 1) {
			sscanf(buf,"sc_basic_freq_idx=%d",&sc_basic_freq_idx);
			sscanf(buf,"bc_basic_freq_idx=%d",&bc_basic_freq_idx);
			sscanf(buf,"xc_basic_freq_idx=%d",&xc_basic_freq_idx);
			sscanf(buf,"sc_burst_freq_idx=%d",&sc_burst_freq_idx);
			sscanf(buf,"bc_burst_freq_idx=%d",&bc_burst_freq_idx);
			sscanf(buf,"xc_burst_freq_idx=%d",&xc_burst_freq_idx);
			sscanf(buf,"sc_unattractive_freq_idx=%d",&sc_unattractive_freq_idx);
			sscanf(buf,"bc_unattractive_freq_idx=%d",&bc_unattractive_freq_idx);
			sscanf(buf,"xc_unattractive_freq_idx=%d",&xc_unattractive_freq_idx);
			if (strstr(buf,"[") && strstr(buf,"]")) {
				return 1;
			}
		}
	}
	fclose(fp);
	if (config_exist == 0) {
		write_log("[I] platform %s not supported, loading generic config.",platform_name);
		sc_basic_freq_idx=8;
        bc_basic_freq_idx=8;
        xc_basic_freq_idx=8;
        sc_burst_freq_idx=12;
        bc_burst_freq_idx=12;
        xc_burst_freq_idx=12;
        sc_unattractive_freq_idx=16;
        bc_unattractive_freq_idx=16;
        xc_unattractive_freq_idx=16;
	}
	return 0;
}
void get_cpu_mask(void) {
	FILE *fp;
	char buf[64];
	char URL[256];
	long int cpu_max_freq;
	int i;
	for (i=0;i<=7;i++) {
    	sprintf(URL,"/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq",i);
        fp = fopen(URL,"r");
        if (fp != NULL) {
            fgets(buf,sizeof(buf),fp);
            fclose(fp);
            sscanf(buf,"%ld",&cpu_max_freq);
            cpu_perf_mask[i]=5000000000/cpu_max_freq;
        }
    }
}
void get_cpu_clusters(void) {
	if ((access("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq",0))!=-1) {
		cluster0_cpu=0;
	}
	if ((access("/sys/devices/system/cpu/cpufreq/policy4/scaling_min_freq",0))!=-1) {
		cluster1_cpu=4;
	    if ((access("/sys/devices/system/cpu/cpufreq/policy6/scaling_min_freq",0))!=-1) {
		    cluster2_cpu=6;
		} else if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq",0))!=-1) {
			cluster2_cpu=7;
		}
	} else if ((access("/sys/devices/system/cpu/cpufreq/policy2/scaling_min_freq",0))!=-1) {
		cluster1_cpu=2;
	} else if ((access("/sys/devices/system/cpu/cpufreq/policy6/scaling_min_freq",0))!=-1) {
		cluster1_cpu=6;
		if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq",0))!=-1) {
			cluster2_cpu=7;
		}
	} else if (cpu_perf_mask[3] != cpu_perf_mask[4] && core_num > 3) {
		cluster1_cpu=4;
	} else if (cpu_perf_mask[1] != cpu_perf_mask[2] && core_num == 3) {
		cluster1_cpu=2;
	}
}
void get_cpu_group(void) {
	if (cluster1_cpu == 4 && cluster2_cpu == 7) {
		efficiency_group[0]=0;
		efficiency_group[1]=3;
		single_perf_group[0]=6;
		single_perf_group[1]=7;
		multi_perf_group[0]=4;
		multi_perf_group[1]=6;
		comm_group[0]=0;
		comm_group[1]=5;
	} else if (cluster1_cpu == 4 && cluster2_cpu == 6) {
		efficiency_group[0]=0;
		efficiency_group[1]=3;
		single_perf_group[0]=6;
		single_perf_group[1]=7;
		multi_perf_group[0]=4;
		multi_perf_group[1]=7;
		comm_group[0]=0;
		comm_group[1]=5;
	} else if (cluster1_cpu == 6 && cluster2_cpu == 7) {
		efficiency_group[0]=0;
		efficiency_group[1]=3;
		single_perf_group[0]=6;
		single_perf_group[1]=7;
		multi_perf_group[0]=0;
		multi_perf_group[1]=6;
		comm_group[0]=0;
		comm_group[1]=6;
	} else if (cluster1_cpu == 6 && cluster2_cpu == -1) {
		efficiency_group[0]=0;
		efficiency_group[1]=3;
		single_perf_group[0]=6;
		single_perf_group[1]=7;
		multi_perf_group[0]=0;
		multi_perf_group[1]=6;
		comm_group[0]=0;
		comm_group[1]=7;
	} else if (cluster1_cpu == 4 && cluster2_cpu == -1 && core_num == 7) {
		efficiency_group[0]=0;
		efficiency_group[1]=3;
		single_perf_group[0]=6;
		single_perf_group[1]=7;
		multi_perf_group[0]=4;
		multi_perf_group[1]=6;
		comm_group[0]=0;
		comm_group[1]=5;
	} else if (cluster1_cpu == 4 && cluster2_cpu == -1 && core_num == 5) {
		efficiency_group[0]=0;
		efficiency_group[1]=3;
		single_perf_group[0]=4;
		single_perf_group[1]=5;
		multi_perf_group[0]=0;
		multi_perf_group[1]=5;
		comm_group[0]=0;
		comm_group[1]=5;
	} else if (cluster1_cpu == 2 && cluster2_cpu == -1) {
		efficiency_group[0]=0;
		efficiency_group[1]=1;
		single_perf_group[0]=2;
		single_perf_group[1]=3;
		multi_perf_group[0]=0;
		multi_perf_group[1]=3;
		comm_group[0]=0;
		comm_group[1]=3;
	} else {
		efficiency_group[0]=0;
		efficiency_group[1]=core_num;
		single_perf_group[0]=0;
		single_perf_group[1]=core_num;
		multi_perf_group[0]=0;
		multi_perf_group[1]=core_num;
		comm_group[0]=0;
		comm_group[1]=core_num;
	}
}
void get_cpu_tables(void) {
	FILE *fp;
	char url[256];
	char shell_buf[256];
	char buf[64];
	int i,j,select_freq_table;
	long int cluster0_max_freq,cluster1_max_freq,cluster2_max_freq;
	long int tables_step_freq,now_table_freq,get_cpu_freq,now_table_num;
	long int minus_freq,min_minus_freq;
	long int cpuinfo_available_freqs[50]={0};
	if (cluster0_cpu != -1) {
		sprintf(url,"/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq",cluster0_cpu);
		fp = fopen(url,"r");
		if (fp != NULL) {
			fgets(buf,sizeof(buf),fp);
			fclose(fp);
			sscanf(buf,"%ld",&cluster0_max_freq);
		}
		cluster0_freq_table[20]=cluster0_max_freq;
		tables_step_freq=cluster0_max_freq/20;
		sprintf(shell_buf,"cat /sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies | tr \" \" \"\n\"",cluster0_cpu);
		fp = popen (shell_buf,"r");
		if (fp != NULL) {
			now_table_num=0;
			while(fgets(buf,sizeof(buf),fp) != NULL) {
				sscanf(buf,"%ld",&cpuinfo_available_freqs[now_table_num]);
				now_table_num++;
			}
			fclose(fp);
		}
		for (i=0;i<20;i++) {
			now_table_freq=i*tables_step_freq;
			min_minus_freq=9999999;
			select_freq_table=0;
			for (j=0;j<50;j++) {
				minus_freq=cpuinfo_available_freqs[j]-now_table_freq;
				if (minus_freq < 0) {
					minus_freq=0-minus_freq;
				}
				if (minus_freq < min_minus_freq && cpuinfo_available_freqs[j] != 0) {
					min_minus_freq=minus_freq;
					select_freq_table=j;
				}
			}
			cluster0_freq_table[i]=cpuinfo_available_freqs[select_freq_table];
		}
	}
	if (cluster1_cpu != -1) {
		sprintf(url,"/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq",cluster1_cpu);
		fp = fopen(url,"r");
		if (fp != NULL) {
			fgets(buf,sizeof(buf),fp);
			fclose(fp);
			sscanf(buf,"%ld",&cluster1_max_freq);
		}
		tables_step_freq=cluster1_max_freq/20;
		cluster1_freq_table[20]=cluster1_max_freq;
		sprintf(shell_buf,"cat /sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies | tr \" \" \"\n\"",cluster1_cpu);
		fp = popen (shell_buf,"r");
		if (fp != NULL) {
			now_table_num=0;
			while(fgets(buf,sizeof(buf),fp) != NULL) {
				sscanf(buf,"%ld",&cpuinfo_available_freqs[now_table_num]);
				now_table_num++;
			}
			fclose(fp);
		}
		for (i=0;i<20;i++) {
			now_table_freq=i*tables_step_freq;
			min_minus_freq=9999999;
			select_freq_table=0;
			for (j=0;j<50;j++) {
				minus_freq=cpuinfo_available_freqs[j]-now_table_freq;
				if (minus_freq < 0) {
					minus_freq=0-minus_freq;
				}
				if (minus_freq < min_minus_freq && cpuinfo_available_freqs[j] != 0) {
					min_minus_freq=minus_freq;
					select_freq_table=j;
				}
			}
			cluster1_freq_table[i]=cpuinfo_available_freqs[select_freq_table];
		}
	}
	if (cluster2_cpu != -1) {
		sprintf(url,"/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq",cluster2_cpu);
		fp = fopen(url,"r");
		if (fp != NULL) {
			fgets(buf,sizeof(buf),fp);
			fclose(fp);
			sscanf(buf,"%ld",&cluster2_max_freq);
		}
		tables_step_freq=cluster2_max_freq/20;
		cluster2_freq_table[20]=cluster2_max_freq;
		sprintf(shell_buf,"cat /sys/devices/system/cpu/cpu%d/cpufreq/scaling_available_frequencies | tr \" \" \"\n\"",cluster2_cpu);
		fp = popen (shell_buf,"r");
		if (fp != NULL) {
			now_table_num=0;
			while(fgets(buf,sizeof(buf),fp) != NULL) {
				sscanf(buf,"%ld",&cpuinfo_available_freqs[now_table_num]);
				now_table_num++;
			}
			fclose(fp);
		}
		for (i=0;i<20;i++) {
			now_table_freq=i*tables_step_freq;
			min_minus_freq=9999999;
			select_freq_table=0;
			for (j=0;j<50;j++) {
				minus_freq=cpuinfo_available_freqs[j]-now_table_freq;
				if (minus_freq < 0) {
					minus_freq=0-minus_freq;
				}
				if (minus_freq < min_minus_freq && cpuinfo_available_freqs[j] != 0) {
					min_minus_freq=minus_freq;
					select_freq_table=j;
				}
			}
			cluster2_freq_table[i]=cpuinfo_available_freqs[select_freq_table];
		}
	}
}
void get_pwr_mask(void) {
	int i;
	for (i=0;i<sc_unattractive_freq_idx;i++) {
		sc_pwr_mask[i]=10*cluster0_freq_table[i]/1000*(60+i*2)*(60+i*2)/100000;
	}
	for (i=sc_unattractive_freq_idx;i<=20;i++) {
		sc_pwr_mask[i]=12*cluster0_freq_table[i]/1000*(60+i*2)*(60+i*2)/100000;
	}
	for (i=0;i<bc_unattractive_freq_idx;i++) {
		bc_pwr_mask[i]=10*cluster1_freq_table[i]/1000*(60+i*2)*(60+i*2)/100000;
	}
	for (i=bc_unattractive_freq_idx;i<=20;i++) {
		bc_pwr_mask[i]=12*cluster1_freq_table[i]/1000*(60+i*2)*(60+i*2)/100000;
	}
	for (i=0;i<xc_unattractive_freq_idx;i++) {
		xc_pwr_mask[i]=10*cluster2_freq_table[i]/1000*(60+i*2)*(60+i*2)/100000;
	}
	for (i=xc_unattractive_freq_idx;i<=20;i++) {
		xc_pwr_mask[i]=12*cluster2_freq_table[i]/1000*(60+i*2)*(60+i*2)/100000;
	}
}
void write_xc_freq(long int freq_level) {
	pthread_detach(pthread_self());
	int nfp;
    char buf[128];
    char file_url[128];
	if ((access("/sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq",0))!=-1) {
        sprintf(file_url,"/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq",cluster2_cpu);
        if ((access(file_url,0))!=-1) {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf,"%ld\n",cluster2_freq_table[freq_level]);
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
        sprintf(file_url,"/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq",cluster2_cpu);
        if ((access(file_url,0))!=-1) {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf,"%ld\n",cluster2_freq_table[freq_level]);
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
    } else {
        sprintf(file_url,"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq",cluster2_cpu);
        if ((access(file_url,0))!=-1) {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf,"%ld\n",cluster2_freq_table[freq_level]);
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
        sprintf(file_url,"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq",cluster2_cpu);
        if ((access(file_url,0))!=-1) {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf,"%ld\n",cluster2_freq_table[freq_level]);
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
    }
    pthread_exit(0);
}
void write_bc_freq(long int freq_level) {
	pthread_detach(pthread_self());
	int nfp;
    char buf[128];
    char file_url[128];
	if ((access("/sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq",0))!=-1) {
        sprintf(file_url,"/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq",cluster1_cpu);
        if ((access(file_url,0))!=-1) {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf,"%ld\n",cluster1_freq_table[freq_level]);
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
        sprintf(file_url,"/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq",cluster1_cpu);
        if ((access(file_url,0))!=-1) {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf,"%ld\n",cluster1_freq_table[freq_level]);
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
    } else {
        sprintf(file_url,"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq",cluster1_cpu);
        if ((access(file_url,0))!=-1) {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf,"%ld\n",cluster1_freq_table[freq_level]);
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
        sprintf(file_url,"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq",cluster1_cpu);
        if ((access(file_url,0))!=-1) {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf,"%ld\n",cluster1_freq_table[freq_level]);
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
    }
    pthread_exit(0);
}
void write_sc_freq(long int freq_level) {
	pthread_detach(pthread_self());
	int nfp;
    char buf[128];
    char file_url[128];
	if ((access("/sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq",0))!=-1) {
        sprintf(file_url,"/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq",cluster0_cpu);
        if ((access(file_url,0))!=-1) {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf,"%ld\n",cluster0_freq_table[freq_level]);
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
        sprintf(file_url,"/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq",cluster0_cpu);
        if ((access(file_url,0))!=-1) {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf,"%ld\n",cluster0_freq_table[freq_level]);
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
    } else {
        sprintf(file_url,"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq",cluster0_cpu);
        if ((access(file_url,0))!=-1) {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf,"%ld\n",cluster0_freq_table[freq_level]);
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
        sprintf(file_url,"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq",cluster0_cpu);
        if ((access(file_url,0))!=-1) {
            nfp = open(file_url, O_WRONLY);
            sprintf(buf,"%ld\n",cluster0_freq_table[freq_level]);
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
    }
    pthread_exit(0);
}
void write_freq(int sc_min,int sc_max,int bc_min,int bc_max,int xc_min,int xc_max) {
	long int SC_MIN_FREQ=cluster0_freq_table[sc_min];
	long int SC_MAX_FREQ=cluster0_freq_table[sc_max];
	long int MC_MIN_FREQ=cluster1_freq_table[bc_min];
	long int MC_MAX_FREQ=cluster1_freq_table[bc_max];
	long int BC_MIN_FREQ=cluster2_freq_table[xc_min];
	long int BC_MAX_FREQ=cluster2_freq_table[xc_max];
    int nfp ;
    char buf[128];
    char file_url[128];
    sprintf(file_url,"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq",cluster0_cpu);
    if ((access(file_url,0))!=-1) {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf,"%ld\n",SC_MIN_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    sprintf(file_url,"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq",cluster0_cpu);
    if ((access(file_url,0))!=-1) {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf,"%ld\n",SC_MAX_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    sprintf(file_url,"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq",cluster1_cpu);
    if ((access(file_url,0))!=-1) {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf,"%ld\n",MC_MIN_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    sprintf(file_url,"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq",cluster1_cpu);
    if ((access(file_url,0))!=-1) {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf,"%ld\n",MC_MAX_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    sprintf(file_url,"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_min_freq",cluster2_cpu);
    if ((access(file_url,0))!=-1) {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf,"%ld\n",BC_MIN_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    sprintf(file_url,"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_max_freq",cluster2_cpu);
    if ((access(file_url,0))!=-1) {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf,"%ld\n",BC_MAX_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    sprintf(file_url,"/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq",cluster0_cpu);
    if ((access(file_url,0))!=-1) {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf,"%ld\n",SC_MIN_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    sprintf(file_url,"/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq",cluster0_cpu);
    if ((access(file_url,0))!=-1) {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf,"%ld\n",SC_MAX_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    sprintf(file_url,"/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq",cluster1_cpu);
    if ((access(file_url,0))!=-1) {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf,"%ld\n",MC_MIN_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    sprintf(file_url,"/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq",cluster1_cpu);
    if ((access(file_url,0))!=-1) {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf,"%ld\n",MC_MAX_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    sprintf(file_url,"/sys/devices/system/cpu/cpufreq/policy%d/scaling_min_freq",cluster2_cpu);
    if ((access(file_url,0))!=-1) {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf,"%ld\n",BC_MIN_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    sprintf(file_url,"/sys/devices/system/cpu/cpufreq/policy%d/scaling_max_freq",cluster2_cpu);
    if ((access(file_url,0))!=-1) {
        nfp = open(file_url, O_WRONLY);
        sprintf(buf,"%ld\n",BC_MAX_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    if ((access("/proc/ppm/policy/hard_userlimit_cpu_freq",0))!=-1) {
        if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq",0))!=-1) {
            nfp = open("/proc/ppm/policy/hard_userlimit_cpu_freq", O_WRONLY);
            sprintf(buf,"%ld\40%ld\40%ld\40%ld\40%ld\40%ld\n",SC_MIN_FREQ,SC_MAX_FREQ,MC_MIN_FREQ,MC_MAX_FREQ,BC_MIN_FREQ,BC_MAX_FREQ);
            write(nfp,buf,strlen(buf));
            close(nfp);
        } else {
            nfp = open("/proc/ppm/policy/hard_userlimit_cpu_freq", O_WRONLY);
            sprintf(buf,"%ld\40%ld\40%ld\40%ld\n",SC_MIN_FREQ,SC_MAX_FREQ,MC_MIN_FREQ,MC_MAX_FREQ);
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
    }
    if ((access("/dev/cluster0_freq_min",0))!=-1) {
        nfp = open("/dev/cluster0_freq_min", O_WRONLY);
        sprintf(buf,"%ld\n",SC_MIN_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    if ((access("/dev/cluster0_freq_max",0))!=-1) {
        nfp = open("/dev/cluster0_freq_max", O_WRONLY);
        sprintf(buf,"%ld\n",SC_MAX_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    if ((access("/dev/cluster1_freq_min",0))!=-1) {
        nfp = open("/dev/cluster1_freq_min", O_WRONLY);
        sprintf(buf,"%ld\n",MC_MIN_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    if ((access("/dev/cluster1_freq_max",0))!=-1) {
        nfp = open("/dev/cluster1_freq_max", O_WRONLY);
        sprintf(buf,"%ld\n",MC_MAX_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    if ((access("/dev/cluster2_freq_min",0))!=-1) {
        nfp = open("/dev/cluster2_freq_min", O_WRONLY);
        sprintf(buf,"%ld\n",BC_MIN_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    if ((access("/dev/cluster2_freq_max",0))!=-1) {
        nfp = open("/dev/cluster2_freq_max", O_WRONLY);
        sprintf(buf,"%ld\n",BC_MAX_FREQ);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
}
void set_sched_boost(int boost_value) {
	int nfp ;
    char buf[128];
    if (boost_value == 0) {
        if (access("/dev/stune/top-app/schedtune.prefer_idle",0)!=-1) {
            nfp = open("/dev/stune/top-app/schedtune.prefer_idle", O_WRONLY);
            sprintf(buf,"0\n");
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
        if (access("/dev/cpuctl/top-app/cpu.uclamp.latency_sensitive",0)!=-1) {
            nfp = open("/dev/cpuctl/top-app/cpu.uclamp.latency_sensitive", O_WRONLY);
            sprintf(buf,"0\n");
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
        if (access("/dev/stune/rt/schedtune.prefer_idle",0)!=-1) {
            nfp = open("/dev/stune/rt/schedtune.prefer_idle", O_WRONLY);
            sprintf(buf,"0\n");
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
        if (access("/dev/cpuctl/rt/cpu.uclamp.latency_sensitive",0)!=-1) {
            nfp = open("/dev/cpuctl/rt/cpu.uclamp.latency_sensitive", O_WRONLY);
            sprintf(buf,"0\n");
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
    } else {
        if (access("/dev/stune/top-app/schedtune.prefer_idle",0)!=-1) {
            nfp = open("/dev/stune/top-app/schedtune.prefer_idle", O_WRONLY);
            sprintf(buf,"1\n");
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
        if (access("/dev/cpuctl/top-app/cpu.uclamp.latency_sensitive",0)!=-1) {
            nfp = open("/dev/cpuctl/top-app/cpu.uclamp.latency_sensitive", O_WRONLY);
            sprintf(buf,"1\n");
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
        if (access("/dev/stune/rt/schedtune.prefer_idle",0)!=-1) {
            nfp = open("/dev/stune/rt/schedtune.prefer_idle", O_WRONLY);
            sprintf(buf,"1\n");
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
        if (access("/dev/cpuctl/rt/cpu.uclamp.latency_sensitive",0)!=-1) {
            nfp = open("/dev/cpuctl/rt/cpu.uclamp.latency_sensitive", O_WRONLY);
            sprintf(buf,"1\n");
            write(nfp,buf,strlen(buf));
            close(nfp);
        }
    }
    if (access("/dev/stune/top-app/schedtune.boost",0)!=-1) {
        nfp = open("/dev/stune/top-app/schedtune.boost", O_WRONLY);
        sprintf(buf,"%d\n",boost_value);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    if (access("/dev/cpuctl/top-app/cpu.uclamp.min",0)!=-1) {
        nfp = open("/dev/cpuctl/top-app/cpu.uclamp.min", O_WRONLY);
        sprintf(buf,"%d\n",boost_value);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    if (access("/dev/stune/rt/schedtune.boost",0)!=-1) {
        nfp = open("/dev/stune/rt/schedtune.boost", O_WRONLY);
        sprintf(buf,"%d\n",boost_value);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
    if (access("/dev/cpuctl/rt/cpu.uclamp.min",0)!=-1) {
        nfp = open("/dev/cpuctl/rt/cpu.uclamp.min", O_WRONLY);
        sprintf(buf,"%d\n",boost_value);
        write(nfp,buf,strlen(buf));
        close(nfp);
    }
}
void usage_monitor(void) {
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME,"UsageMonitor");
    FILE *load_fp;
    char buf[512];
    char URL[512];
    int i;
    int tmp_usage=0;
    int cur_cpu_core=0;
    int tmp_sc_usage=0;
    int tmp_bc_usage=0;
    int tmp_xc_usage=0;
    long int cur_freq;
    long int user, nice, sys, idle, iowait, irq, softirq;
    long int all1[10]={0};
    long int all2=0;
    long int used1[10]={0};
    long int used2=0;
	while(1) {
        load_fp = fopen("/proc/stat", "r");
        if (load_fp != NULL) {
            fgets(buf, sizeof(buf), load_fp); 
            for (i=0;i<=core_num;i++) {
                fgets(buf, sizeof(buf), load_fp); 
                if (strstr(buf,"cpu")) {
                    sscanf(buf, "cpu%d %ld %ld %ld %ld %ld %ld %ld", &cur_cpu_core, &user, &nice, &sys, &idle, &iowait, &irq, &softirq);
                    all1[cur_cpu_core] = user + nice + sys + idle + iowait + irq + softirq;
                    used1[cur_cpu_core] = all1[cur_cpu_core] - idle;  
                } 
            }
            fclose(load_fp);               
            usleep(sample_time);
            tmp_sc_usage=0;
            tmp_bc_usage=0;
            tmp_xc_usage=0;
            load_fp = fopen("/proc/stat", "r");
            if (load_fp != NULL) {
                fgets(buf, sizeof(buf), load_fp); 
                for (i=0;i<=core_num;i++) {
                    fgets(buf, sizeof(buf), load_fp); 
                    if (strstr(buf,"cpu")) {
                        sscanf(buf, "cpu%d %ld %ld %ld %ld %ld %ld %ld",&cur_cpu_core, &user, &nice, &sys, &idle, &iowait, &irq, &softirq);
                        all2 = user + nice + sys + idle + iowait + irq + softirq;
                        used2 = all2 - idle ;  
                        tmp_usage = (used2-used1[cur_cpu_core])*100/(all2-all1[cur_cpu_core]);
                        if (cur_cpu_core < cluster1_cpu || cluster1_cpu == -1) {
                            if (tmp_usage > tmp_sc_usage) {
                                tmp_sc_usage=tmp_usage;
                            }
                        } else if (cur_cpu_core < cluster2_cpu || cluster2_cpu == -1) {
                            if (tmp_usage > tmp_bc_usage) {
                                tmp_bc_usage=tmp_usage;
                            }
                        } else {
                            if (tmp_usage > tmp_xc_usage) {
                                tmp_xc_usage=tmp_usage;
                            }
                        }
                    }
                }
                fclose(load_fp);
                sc_max_usage=tmp_sc_usage;
                bc_max_usage=tmp_bc_usage;
                xc_max_usage=tmp_xc_usage;
            }
        }
        if (SCREEN_OFF == 1) {
        	pthread_exit(0);
        }
    }
}
void cpu_governor(void) {
	pthread_detach(pthread_self());
    prctl(PR_SET_NAME,"CPUGovernor");
    pthread_t writer_tid;
    int limit_sc_level_min=0;
    int limit_bc_level_min=0;
    int limit_xc_level_min=0;
    int limit_sc_level_max[6]={0};
    int limit_bc_level_max[6]={0};
    int limit_xc_level_max[6]={0};
    int sc_target_loads[30]={0};
    int bc_target_loads[30]={0};
    int xc_target_loads[30]={0};
    int sched_rt_boost[6]={0};
    int target_sc_level=20;
    int target_bc_level=20;
    int target_xc_level=20;
    int target_pwr_mask=0;
    int now_sc_level=1;
    int now_bc_level=1;
    int now_xc_level=1;
    int sc_stable_ms=0;
    int bc_stable_ms=0;
    int xc_stable_ms=0;
    int sc_now_usage=0;
    int bc_now_usage=0;
    int xc_now_usage=0;
    int event_delay_ms=0;
    int app_event_level=0;
    int boost_delay_ms=0;
    int boost_level=0;
    int i;
    char res_app_event[64];
    char now_mode[32];
    while(1) {
        if (strcmp(mode,now_mode)!=0) {
            if (strcmp(mode,"powersave")==0) {
                limit_sc_level_max[0]=16;
                limit_bc_level_max[0]=16;
                limit_xc_level_max[0]=16;
                limit_sc_level_min=0;
                limit_bc_level_min=0;
                limit_xc_level_min=0;
            } else if (strcmp(mode,"balance")==0) {
                limit_sc_level_max[0]=18;
                limit_bc_level_max[0]=18;
                limit_xc_level_max[0]=16;
                limit_sc_level_min=0;
                limit_bc_level_min=0;
                limit_xc_level_min=0;
            } else if (strcmp(mode,"performance")==0) {
                limit_sc_level_max[0]=20;
                limit_bc_level_max[0]=20;
                limit_xc_level_max[0]=20;
                limit_sc_level_min=0;
                limit_bc_level_min=0;
                limit_xc_level_min=0;
            } else {
                limit_sc_level_max[0]=20;
                limit_bc_level_max[0]=20;
                limit_xc_level_max[0]=20;
                limit_sc_level_min=10;
                limit_bc_level_min=10;
                limit_xc_level_min=10;
            }
            for (i=1;i<=5;i++) {
                limit_sc_level_max[i]=limit_sc_level_max[i-1]-1;
                limit_bc_level_max[i]=limit_bc_level_max[i-1]-1;
                limit_xc_level_max[i]=limit_xc_level_max[i-1]-1;
            }
            if (strcmp(mode,"powersave")==0) {
                sched_rt_boost[0]=0;
                sched_rt_boost[1]=0;
                sched_rt_boost[2]=0;
            } else if (strcmp(mode,"balance")==0) {
                sched_rt_boost[0]=0;
                sched_rt_boost[1]=20;
                sched_rt_boost[2]=40;
            } else if (strcmp(mode,"performance")==0) {
                sched_rt_boost[0]=20;
                sched_rt_boost[1]=40;
                sched_rt_boost[2]=60;
            } else {
                sched_rt_boost[0]=80;
                sched_rt_boost[1]=80;
                sched_rt_boost[2]=80;
            }
            if (strcmp(mode,"powersave")==0) {
                sc_target_loads[0]=70;
                bc_target_loads[0]=70;
                xc_target_loads[0]=90;
            } else if (strcmp(mode,"balance")==0) {
                sc_target_loads[0]=60;
                bc_target_loads[0]=60;
                xc_target_loads[0]=80;
            } else if (strcmp(mode,"performance")==0) {
                sc_target_loads[0]=50;
                bc_target_loads[0]=50;
                xc_target_loads[0]=60;
            } else {
                sc_target_loads[0]=40;
                bc_target_loads[0]=40;
                xc_target_loads[0]=40;
            } 
            for (i=1;i<=20;i++) {
                if (sc_target_loads[i-1] < 95) {
                    sc_target_loads[i]=sc_target_loads[i-1]+1;
                }
                if (bc_target_loads[i-1] < 95) {
                    bc_target_loads[i]=bc_target_loads[i-1]+1;
                }
                if (xc_target_loads[i-1] < 95) {
                    xc_target_loads[i]=xc_target_loads[i-1]+1;
                }
            }
            target_sc_level=20;
            target_bc_level=20;
            target_xc_level=20;
            now_sc_level=1;
            now_bc_level=1;
            now_xc_level=1;
            sc_stable_ms=0;
            bc_stable_ms=0;
            xc_stable_ms=0;
            sc_now_usage=0;
            bc_now_usage=0;
            xc_now_usage=0;
            event_delay_ms=0;
            app_event_level=0;
            boost_level=0;
            boost_delay_ms=0;
            sprintf(now_mode,"%s",mode);
        }
        if (strcmp(app_event,res_app_event)!=0) {
            sprintf(res_app_event,"%s",app_event);
            if (strstr(res_app_event,"window_changed")) {
                app_event_level=1;
            }
            event_delay_ms=500;
        } else if (event_delay_ms > 0) {
            event_delay_ms--;
        } else {
            app_event_level=0;
        }
        if (app_event_level == 1) {
            if (boost_level != 2) {
                set_sched_boost(sched_rt_boost[2]);
                sample_time=10000;
                boost_level=2;
            }
        } else if (touch_level == 1) {
            if (boost_level != 1) {
                set_sched_boost(sched_rt_boost[1]);
                sample_time=20000;
                boost_level=1;
            }
        } else {
            if (boost_level != 0) {
                set_sched_boost(sched_rt_boost[0]);
                sample_time=40000;
                boost_level=0;
            }
        }
        if (xc_max_usage != xc_now_usage) {
            target_pwr_mask=xc_pwr_mask[now_xc_level]*xc_max_usage/xc_target_loads[now_xc_level];
            for (i=1;i<=20;i++) {
                if (target_pwr_mask < xc_pwr_mask[i]) {
                    target_xc_level=i;
                    break;
                }
            }
            xc_now_usage=xc_max_usage;
        }
        if (boost_level == 1 && target_xc_level < xc_basic_freq_idx) {
            target_xc_level = xc_basic_freq_idx;
        } else if (boost_level == 2 && target_xc_level < xc_burst_freq_idx) {
            target_xc_level = xc_burst_freq_idx;
        }
        if (target_xc_level > limit_xc_level_max[thermal_level]) {
            target_xc_level=limit_xc_level_max[thermal_level];
        } else if (target_xc_level < limit_xc_level_min) {
            target_xc_level=limit_xc_level_min;
        } else if (target_xc_level == 0) {
            target_xc_level=1;
        }
        if (bc_max_usage != bc_now_usage) {
            target_pwr_mask=bc_pwr_mask[now_bc_level]*bc_max_usage/bc_target_loads[now_bc_level];
            for (i=1;i<=20;i++) {
                if (target_pwr_mask < bc_pwr_mask[i]) {
                    target_bc_level=i;
                    break;
                }
            }
            bc_now_usage=bc_max_usage;
        }
        if (boost_level == 1 && target_bc_level < bc_basic_freq_idx) {
            target_bc_level = bc_basic_freq_idx;
        } else if (boost_level == 2 && target_bc_level < bc_burst_freq_idx) {
            target_bc_level = bc_burst_freq_idx;
        }
        if (target_bc_level > limit_bc_level_max[thermal_level]) {
            target_bc_level=limit_bc_level_max[thermal_level];
        } else if (target_bc_level < limit_bc_level_min) {
            target_bc_level=limit_bc_level_min;
        } else if (target_bc_level == 0) {
            target_bc_level=1;
        }
        if (sc_max_usage != sc_now_usage) {
            target_pwr_mask=sc_pwr_mask[now_sc_level]*sc_max_usage/sc_target_loads[now_sc_level];
            for (i=1;i<=20;i++) {
                if (target_pwr_mask < sc_pwr_mask[i]) {
                    target_sc_level=i;
                    break;
                }
            }
            sc_now_usage=sc_max_usage;
        }
        if (boost_level == 1 && target_sc_level < sc_basic_freq_idx) {
            target_sc_level = sc_basic_freq_idx;
        } else if (boost_level == 2 && target_sc_level < sc_burst_freq_idx) {
            target_sc_level = sc_burst_freq_idx;
        }
        if (target_sc_level > limit_sc_level_max[thermal_level]) {
            target_sc_level=limit_sc_level_max[thermal_level];
        } else if (target_sc_level < limit_sc_level_min) {
            target_sc_level=limit_sc_level_min;
        } else if (target_sc_level == 0) {
            target_sc_level=1;
        }
        if (target_xc_level < now_xc_level || target_xc_level < now_xc_level) {
		    if (xc_stable_ms == 200) {
			    pthread_create(&writer_tid,NULL,(void *)write_xc_freq,(void *)(long)target_xc_level);
                now_xc_level=target_xc_level;
                xc_stable_ms = 0;
			} else if (xc_stable_ms < 200) {
				xc_stable_ms++;
		    }
		} else if (target_xc_level > now_xc_level || target_xc_level > now_xc_level) {
	        pthread_create(&writer_tid,NULL,(void *)write_xc_freq,(void *)(long)target_xc_level);
            now_xc_level=target_xc_level;
            if (xc_stable_ms < 200) {
				xc_stable_ms++;
		    }
	    }
        if (target_bc_level < now_bc_level || target_bc_level < now_bc_level) {
		    if (bc_stable_ms == 200) {
			    pthread_create(&writer_tid,NULL,(void *)write_bc_freq,(void *)(long)target_bc_level);
                now_bc_level=target_bc_level;
                bc_stable_ms = 0;
			} else if (bc_stable_ms < 200) {
				bc_stable_ms++;
		    }
		} else if (target_bc_level > now_bc_level || target_bc_level > now_bc_level) {
			pthread_create(&writer_tid,NULL,(void *)write_bc_freq,(void *)(long)target_bc_level);
            now_bc_level=target_bc_level;
            if (bc_stable_ms < 200) {
				bc_stable_ms++;
		    }
	    }
	    if (target_sc_level < now_sc_level || target_sc_level < now_sc_level) {
		    if (sc_stable_ms == 200) {
			    pthread_create(&writer_tid,NULL,(void *)write_sc_freq,(void *)(long)target_sc_level);
                now_sc_level=target_sc_level;
                sc_stable_ms = 0;
			} else if (sc_stable_ms < 200) {
				sc_stable_ms++;
		    }
		} else if (target_sc_level > now_sc_level || target_sc_level > now_sc_level) {
	        pthread_create(&writer_tid,NULL,(void *)write_sc_freq,(void *)(long)target_sc_level);
            now_sc_level=target_sc_level;
            if (sc_stable_ms < 200) {
				sc_stable_ms++;
		    }
	    }
		usleep(1000);
	    if (SCREEN_OFF == 1) {
        	pthread_exit(0);
        }
    }
}
void TasksetHelper(void) {
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME,"TasksetHelper");
    FILE *fp;
    FILE *tid_fp;
    DIR *dir = NULL;  
    struct dirent *entry;  
    struct sched_param sched_p;
    char res_app_event[64];
    char seted_tasks[64*1024];
    char setting_tasks[64*1024];
    char task_info[32];
    char buf[256];
    char tid_dir[256];
    char tid_url[256];
    char tid_name[64];
    int seted_pid=0;
    int i,tid;
    int last_fg_tid_num=0;
    int fg_tid_num=0;
    int fg_tids[10000];
	cpu_set_t efficiency_mask;  
	cpu_set_t multi_perf_mask;  
	cpu_set_t single_perf_mask;  
	cpu_set_t comm_mask;  
    CPU_ZERO(&efficiency_mask);   
    for (i=efficiency_group[0];i<=efficiency_group[1];i++) {
	    CPU_SET(i,&efficiency_mask);  
    }
    CPU_ZERO(&multi_perf_mask);   
    for (i=multi_perf_group[0];i<=multi_perf_group[1];i++) {
	    CPU_SET(i,&multi_perf_mask);  
    }
    CPU_ZERO(&single_perf_mask);   
    for (i=single_perf_group[0];i<=single_perf_group[1];i++) {
	    CPU_SET(i,&single_perf_mask);  
    }
    CPU_ZERO(&comm_mask);   
    for (i=comm_group[0];i<=comm_group[1];i++) {
	    CPU_SET(i,&comm_mask);  
    }
    while(1) {
        last_fg_tid_num=fg_tid_num;
        fg_tid_num=0;
        sprintf(tid_dir,"/proc/%d/task/",foreground_pid);
        dir = opendir(tid_dir);
        if (dir != NULL) {
            memset(fg_tids,0,sizeof(fg_tids));
            while ((entry=readdir(dir)) != NULL) {  
                sscanf((*entry).d_name,"%d",&tid);
                fg_tid_num++;
                fg_tids[fg_tid_num]=tid;
            }
            closedir(dir);
        }
        if (last_fg_tid_num != fg_tid_num || seted_pid != foreground_pid || strcmp(app_event,res_app_event)!=0) {
            if (seted_pid != foreground_pid) {
                seted_pid=foreground_pid;
                memset(seted_tasks,0,sizeof(seted_tasks));
            } else if (strcmp(app_event,res_app_event)!=0) {
                sprintf(res_app_event,"%s",app_event);
                memset(seted_tasks,0,sizeof(seted_tasks));
            }
            memset(setting_tasks,0,sizeof(setting_tasks));
            for (i=1;i<=fg_tid_num;i++) {  
                sprintf(tid_url,"/proc/%d/task/%d/comm",foreground_pid,fg_tids[i]);
                tid_fp=fopen(tid_url,"r");
                if (tid_fp != NULL) {
                    fgets(tid_name,sizeof(tid_name),tid_fp);
                    fclose(tid_fp);
                    sprintf(task_info,"[%d_%s]",fg_tids[i],tid_name);
                    if (strstr(seted_tasks,task_info) == NULL) {
                        if (strstr(tid_name,"RenderThread")) {
                            sched_setaffinity(fg_tids[i],sizeof(single_perf_mask),&single_perf_mask);
                        } else if (strstr(tid_name,"GLThread")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        } else if (strstr(tid_name,"JNISurfaceText")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        } else if (strstr(tid_name,"blur")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        } else if (strstr(tid_name,"hwui")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        } else if (strstr(tid_name,"android.ui")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        } else if (strstr(tid_name,"anim")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);   
                        } else if (strstr(tid_name,"android.display")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);  
                        } else if (strstr(tid_name,"UnityMain")) {
                            sched_setaffinity(fg_tids[i],sizeof(single_perf_mask),&single_perf_mask);  
                        } else if (strstr(tid_name,"MainThread-UE4")) {
                            sched_setaffinity(fg_tids[i],sizeof(single_perf_mask),&single_perf_mask);  
                        } else if (strstr(tid_name,"UnityGfxDevice")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask); 
                        } else if (strstr(tid_name,"UnityMulti")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask); 
                        } else if (strstr(tid_name,"UnityPreload")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);  
                        } else if (strstr(tid_name,"UnityChoreograp")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        } else if (strstr(tid_name,"Worker Thread")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        } else if (strstr(tid_name,"Job.Worker")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);  
                        } else if (strstr(tid_name,"GameThread")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&single_perf_mask); 
                        } else if (strstr(tid_name,"RHIThread")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        } else if (strstr(tid_name,"NativeThread")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        } else if (strstr(tid_name,"TaskGraphNP")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        } else if (strstr(tid_name,"glp")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        } else if (strstr(tid_name,"MINECRAFT")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&single_perf_mask);
                        } else if (strstr(tid_name,"Gesture")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        } else if (strstr(tid_name,".gifmaker")) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        } else if (strstr(tid_name,"Chrome")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"Chromium")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"WebView")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"webView")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"Webview")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"android.fg")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"android.io")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"miui.fg")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"IJK")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"Ijk")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"ijk")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"Binder")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"Async")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"async")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"Vsync")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"vsync")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"CrGpuMain")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"Compositor")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"CrRendererMain")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"HeapTaskDaemon")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"FinalizerDaemon")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"ReferenceQueue")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"Jit thread pool")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"OkHttp")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"SearchDaemon")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"Profile")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"ThreadPool")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"PoolThread")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"Download")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"download")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"Audio")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"audio")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"Video")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"video")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"Mixer")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"mixer")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"mali-")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"ged-swd")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"GPU completion")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"FramePolicy")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"ScrollPolicy")) {
                            sched_setaffinity(fg_tids[i],sizeof(comm_mask),&comm_mask);
                        } else if (strstr(tid_name,"xcrash")) {
                            sched_setaffinity(fg_tids[i],sizeof(efficiency_mask),&efficiency_mask);
                        } else if (strstr(tid_name,"bugly")) {
                            sched_setaffinity(fg_tids[i],sizeof(efficiency_mask),&efficiency_mask);
                        } else if (strstr(tid_name,"android.bg")) {
                            sched_setaffinity(fg_tids[i],sizeof(efficiency_mask),&efficiency_mask);
                        } else if (strstr(tid_name,"miui.bg")) {
                            sched_setaffinity(fg_tids[i],sizeof(efficiency_mask),&efficiency_mask);
                        } else if (strstr(tid_name,"SensorService")) {
                            sched_setaffinity(fg_tids[i],sizeof(efficiency_mask),&efficiency_mask);
                        } else if (strstr(tid_name,"HealthService")) {
                            sched_setaffinity(fg_tids[i],sizeof(efficiency_mask),&efficiency_mask);
                        } else if (strstr(tid_name,"background")) {
                            sched_setaffinity(fg_tids[i],sizeof(efficiency_mask),&efficiency_mask);
                        } else if (strstr(tid_name,"report")) {
                            sched_setaffinity(fg_tids[i],sizeof(efficiency_mask),&efficiency_mask);
                        } else if (strstr(tid_name,"tt_pangle")) {
                            sched_setaffinity(fg_tids[i],sizeof(efficiency_mask),&efficiency_mask);
                        } else if (fg_tids[i] == foreground_pid) {
                            sched_setaffinity(fg_tids[i],sizeof(multi_perf_mask),&multi_perf_mask);
                        }
                        sprintf(setting_tasks,"%s %s",setting_tasks,task_info);
                    } else {
                        sprintf(setting_tasks,"%s %s",setting_tasks,task_info);
                    }
                }
                sprintf(seted_tasks,"%s",setting_tasks);
            }
        }
        usleep(500000);
        if (SCREEN_OFF == 1) {
        	pthread_exit(0);
        }
    }
}
void event_reader(long int ts_event) {
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME,"EventReader");
	struct input_event input_state;
	char shell_buf[256];
	char touch_screen_url[128];
    sprintf(touch_screen_url,"/dev/input/event%ld",ts_event);
    write_log("[I] EventReader: listening %s. ",touch_screen_url);
    int event_fd;
    event_fd=open(touch_screen_url, O_RDONLY);
    while(1) {
        input_state.type=0;
        input_state.value=0;
        read (event_fd, &input_state, sizeof(input_state));
        if (input_state.type == 3 || input_state.value == 1) {
            touch_level=1;
        } else {
            usleep(5000);
            touch_level=0;
        }
        usleep(1000);
    }
    close(event_fd);
}
void Run_EventReader(void) {
      char buf[64];
      char input_url[64];
      FILE *input_fp;
      long int i;    
      int ABS_DEV=0;
      for (i = 0; i < 30; i++) {  
      	ABS_DEV=0;
          sprintf(input_url,"/sys/class/input/input%ld/uevent",i);
          input_fp = fopen(input_url, "r");
          if (input_fp != NULL) {
              while (fgets(buf,sizeof(buf),input_fp) != NULL) {
                  if (strstr(buf,"ABS=")) {
              	    ABS_DEV=1;
                  }
              }
              fclose(input_fp);
          } 
          if (ABS_DEV == 1) {
              pthread_create(&event_reader_tid[i],NULL,(void *)event_reader,(void *)i);
          }
      }
}
void thermal_monitor(void) {
	DIR *dir;
	FILE *fp;
	struct dirent *entry;  
	char thermal_type_url[128];
	char cpu_temp_url[128];
	char buf[32];
	long int cpu_temp=0;
	pthread_detach(pthread_self());
	prctl(PR_SET_NAME,"ThermalMonitor");
	sprintf(cpu_temp_url,"null");
	dir = opendir("/sys/class/thermal/");
    if (dir != NULL) {
        while ((entry=readdir(dir)) != NULL) {  
            if (strstr((*entry).d_name,"thermal_zone")) {
                sprintf(thermal_type_url,"/sys/class/thermal/%s/type",(*entry).d_name);
                fp=fopen(thermal_type_url,"r");
                if (fp) {
                    fgets(buf,sizeof(buf),fp);
                    fclose(fp);
                    if (strstr(buf,"cpu") || strstr(buf,"msm_therm")) {
                        sprintf(cpu_temp_url,"/sys/class/thermal/%s/temp",(*entry).d_name);
                    }
                }
            }
        }
        closedir(dir);
    }
    if (strcmp(cpu_temp_url,"null")!=0) {
        write_log("[I] ThermalMonitor: Watching %s.",cpu_temp_url);
    } else {
        write_log("[E] ThermalMonitor: Unable to get CPU temperature.");
        pthread_exit(0);
    }
    while(1) {
        fp=fopen(cpu_temp_url,"r");
        if (fp) {
            fgets(buf,sizeof(buf),fp);
            fclose(fp);
            sscanf(buf,"%ld",&cpu_temp);
            if (cpu_temp > 10000) {
                cpu_temp=cpu_temp/1000;
            } else if (cpu_temp > 1000) {
                cpu_temp=cpu_temp/100;
            } else if (cpu_temp > 100) {
                cpu_temp=cpu_temp/10;
            }
            if (strcmp(mode,"fast")!=0) {
                if (cpu_temp >= 90) {
                    thermal_level=5;
                } else if (cpu_temp >= 85) {
                    thermal_level=4;
                } else if (cpu_temp >= 80) {
                    thermal_level=3;
                } else if (cpu_temp >= 75) {
                    thermal_level=2;
                } else if (cpu_temp >= 70) {
                    thermal_level=1;
                } else {
                    thermal_level=0;
                }
            } else {
                thermal_level=0;
            }
        }
        usleep(500000);
    }
}
int main(int argc, char * argv[]) {
	FILE *fp;
	char recv_buf[128];
    char buf[128];
	char shell[256];
	char foreground_pkg[256];
	char last_foreground_pkg[256];
	char get_mode[32];
	int screen_on_val=1;
	int nfp;
	int i;
	int sockfd,newfd;
	int recv_num=0;
    sprintf(path,"%s",argv[0]);
    for (i=strlen(path); i>0;i--) {
        if (path[i] == '/') {
           path[i]='\0';
           break;
        }
    }
    sprintf(mode,"null");
    sprintf(get_mode,"balance");
	run_libcuprum("init");
	system("echo $(date +%F) $(date +%T) [I] CuprumTurbo V9 by chenzyadb@coolapk.com > '/sdcard/Android/data/xyz.chenzyadb.cu_toolbox/files/Cuprum_Log.txt'");
	write_log("[I] Initizalizing.");
	if (access("/sys/devices/system/cpu/cpu7/cpufreq/scaling_cur_freq",0)!=-1) {
        core_num=7;
    } else if (access("/sys/devices/system/cpu/cpu5/cpufreq/scaling_cur_freq",0)!=-1) {
        core_num=5;
    } else if (access("/sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq",0)!=-1) {
        core_num=3;
    }
    get_config();
	get_cpu_mask();
	get_cpu_clusters();
	get_cpu_tables();
	get_cpu_group();
	get_pwr_mask();
	write_log("[I] cluster0=cpu%d, cluster1=cpu%d, cluster2=cpu%d. ",cluster0_cpu,cluster1_cpu,cluster2_cpu);
	write_log("[I] cpu_group: efficiency=%d-%d, multi_perf=%d-%d, single_perf=%d-%d, comm=%d-%d. ",efficiency_group[0],efficiency_group[1],multi_perf_group[0],multi_perf_group[1],single_perf_group[0],single_perf_group[1],comm_group[0],comm_group[1]);
	write_log("[I] Cluster0 CPUFreq Table:");
	for (i=0;i<=20;i++) {
		write_log("[I] idx=%d, freq=%ld KHz, pwr_mask=%d.",i,cluster0_freq_table[i],sc_pwr_mask[i]);
	}
	write_log("[I] basic_freq=%ld KHz, burst_freq=%ld KHz, unattractive_freq=%ld KHz.",cluster0_freq_table[sc_basic_freq_idx],cluster0_freq_table[sc_burst_freq_idx],cluster0_freq_table[sc_unattractive_freq_idx]);
	if (cluster1_cpu != -1) {
	    write_log("[I] Cluster1 CPUFreq Table:");
	    for (i=0;i<=20;i++) {
		    write_log("[I] idx=%d, freq=%ld KHz, pwr_mask=%d.",i,cluster1_freq_table[i],bc_pwr_mask[i]);
	    }
	    write_log("[I] basic_freq=%ld KHz, burst_freq=%ld KHz, unattractive_freq=%ld KHz.",cluster1_freq_table[bc_basic_freq_idx],cluster1_freq_table[bc_burst_freq_idx],cluster1_freq_table[bc_unattractive_freq_idx]);
	}
	if (cluster2_cpu != -1) {
	    write_log("[I] Cluster2 CPUFreq Table:");
	    for (i=0;i<=20;i++) {
		    write_log("[I] idx=%d, freq=%ld KHz, pwr_mask=%d.",i,cluster2_freq_table[i],xc_pwr_mask[i]);
	    }
	    write_log("[I] basic_freq=%ld KHz, burst_freq=%ld KHz, unattractive_freq=%ld KHz.",cluster2_freq_table[xc_basic_freq_idx],cluster2_freq_table[xc_burst_freq_idx],cluster2_freq_table[xc_unattractive_freq_idx]);
	}
	Run_EventReader();
    struct sockaddr_in server_addr;
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr=inet_addr("127.0.0.188"); 
    server_addr.sin_port=htons(23333);
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(server_addr));
    write_log("[I] Waiting for Socket Client Connection. ");
    listen(sockfd,1);
    newfd=accept(sockfd,NULL,NULL);			
    if (newfd < 0) {
        write_log("[E] Socket Client Connection Failed.");
    } else {
        write_log("[I] Socket Client Connected.");
    }
    pthread_create(&tasksethelper_tid,NULL,(void *)TasksetHelper,NULL);
    pthread_create(&usage_monitor_tid,NULL,(void *)usage_monitor,NULL);
    pthread_create(&cpu_governor_tid,NULL,(void *)cpu_governor,NULL);
    pthread_create(&thermal_monitor_tid,NULL,(void *)thermal_monitor,NULL);
    sleep(1);
	write_log("[I] Service Running. ");
	prctl(PR_SET_NAME,"SocketServer");
	while(1){
		memset(recv_buf,0,sizeof(recv_buf));
        recv_num=recv(newfd,recv_buf,sizeof(recv_buf),0);
        if (recv_num > 0) {
            sscanf(recv_buf,"mode=%s",get_mode);
            sscanf(recv_buf,"app_event=%s",app_event);
            sscanf(recv_buf,"foreground_pkg=%s",foreground_pkg);
            sscanf(recv_buf,"screen_on=%d",&screen_on_val);
        } else {
            close(newfd);
            newfd=accept(sockfd,NULL,NULL);
            if (newfd < 0) {
                write_log("[E] Socket Client Connection Failed. ");
            }
        }
		if (screen_on_val == 0) {
			if (SCREEN_OFF == 0) {
				SCREEN_OFF=1;
				run_libcuprum("powersave");
				run_libcuprum("standby");
			    write_freq(0,20,0,20,0,20);
			    set_sched_boost(0);
			}
		} else {
			if (SCREEN_OFF == 1) {
				write_freq(20,20,20,20,20,20);
				run_libcuprum("init");
                run_libcuprum(mode);
                thermal_level=0;
                sample_time=40000;
                foreground_pid=-1;
                SCREEN_OFF=0;
                sleep(2);
                write_freq(0,20,0,20,0,20);
			    pthread_create(&tasksethelper_tid,NULL,(void *)TasksetHelper,NULL);
			    pthread_create(&usage_monitor_tid,NULL,(void *)usage_monitor,NULL);
			    pthread_create(&cpu_governor_tid,NULL,(void *)cpu_governor,NULL);
		    }
            if (strcmp(get_mode,mode)!=0){
                write_log("[I] mode switching %s -> %s.",mode,get_mode);
                run_libcuprum(get_mode);
                sprintf(mode,"%s",get_mode);
            }
            if (strcmp(last_foreground_pkg,foreground_pkg)!=0) {
                sprintf(shell,"ps -eo pid,name | grep \"%s\" | awk '{print $1}'",foreground_pkg);
                fp = popen(shell,"r");
                if (fp != NULL) {
                    fgets(buf,sizeof(buf),fp);
                    sscanf(buf,"%d",&foreground_pid);
                    pclose(fp);
                    sprintf(last_foreground_pkg,"%s",foreground_pkg);
                } else {
                    foreground_pid=-1;
                }
            }
        }
	}
}
