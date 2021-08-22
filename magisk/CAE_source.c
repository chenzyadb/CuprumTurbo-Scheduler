//CuprumTurbo Adjustment V3 
//Cuprum Aware Engine (CuAware) Source
//Author: Chenzyadb @ Coolapk.com
//Copyright © Chenzyadb 2021 All Rights Reserved.
//Mail: chenzyadb@qq.com
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/input.h>
int GetTouchEvent(void) {
      char buf[64];
      char input_url[64];
      char LOG_INFO[128];
      FILE *input_fd;
      int i;    
      for (i = 0; i < 30;i++) {  
          sprintf(input_url,"cat /sys/class/input/input%d/uevent | grep \"PROP=\"",i);
          input_fd = popen(input_url, "r");
          fgets(buf, sizeof(buf), input_fd); 
          pclose(input_fd);
          if(strstr(buf,"PROP=2")) {
              sprintf(LOG_INFO,"echo $(date +%%F) $(date +%%T) [I] CuAware: TouchScreen Input: /dev/input/event%d . >> '/data/Cuprum_Log.txt'",i);
              system(LOG_INFO);
              return i;
          } 
       }
    system("echo $(date +%F) $(date +%T) [W] CuAware: Can't find TouchScreen on your device. >> '/data/Cuprum_Log.txt'");
    return 0;
}
int write_freq(int sc_min,int sc_max,int bc_min,int bc_max) {
	long int SC_MIN_FREQ,SC_MAX_FREQ,MC_MIN_FREQ,MC_MAX_FREQ,BC_MIN_FREQ,BC_MAX_FREQ;
    FILE *fp ;
    int nfp ;
    char buf[128];
    char URL[128];
    sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu0_freq",sc_min);
    fp = fopen(URL, "r");
    fgets(buf,sizeof(buf),fp); 
    sscanf(buf,"%ld",&SC_MIN_FREQ);
    fclose(fp);
    nfp = open("/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq", O_WRONLY);
    sprintf(buf,"%ld\n",SC_MIN_FREQ);
    write(nfp,buf,sizeof(buf));
    close(nfp);
    sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu0_freq",sc_max);
    fp = fopen(URL, "r");
    fgets(buf,sizeof(buf),fp); 
    sscanf(buf,"%ld",&SC_MAX_FREQ);
    fclose(fp);
    nfp = open("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq", O_WRONLY);
    sprintf(buf,"%ld\n",SC_MAX_FREQ);
    write(nfp,buf,sizeof(buf));
    close(nfp);
    if ((access("/sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq",0))!=-1) {
        sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu0_freq",sc_min);
        fp = fopen(URL, "r");
        fgets(buf,sizeof(buf),fp); 
        sscanf(buf,"%ld",&SC_MIN_FREQ);
        fclose(fp);
        nfp = open("/sys/devices/system/cpu/cpufreq/policy0/scaling_min_freq", O_WRONLY);
        sprintf(buf,"%ld\n",SC_MIN_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
    }
    if ((access("/sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq",0))!=-1) {
        sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu0_freq",sc_max);
        fp = fopen(URL, "r");
        fgets(buf,sizeof(buf),fp); 
        sscanf(buf,"%ld",&SC_MAX_FREQ);
        fclose(fp);
        nfp = open("/sys/devices/system/cpu/cpufreq/policy0/scaling_max_freq", O_WRONLY);
        sprintf(buf,"%ld\n",SC_MAX_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
    }
    if ((access("/sys/devices/system/cpu/cpufreq/policy2/scaling_min_freq",0))!=-1) {
        sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu2_freq",bc_min);
        fp = fopen(URL, "r");
        fgets(buf,sizeof(buf),fp); 
        sscanf(buf,"%ld",&MC_MIN_FREQ);
        fclose(fp);
        nfp = open("/sys/devices/system/cpu/cpufreq/policy2/scaling_min_freq", O_WRONLY);
        sprintf(buf,"%ld\n",MC_MIN_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
    }
    if ((access("/sys/devices/system/cpu/cpufreq/policy2/scaling_max_freq",0))!=-1) {
        sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu2_freq",bc_max);
        fp = fopen(URL, "r");
        fgets(buf,sizeof(buf),fp); 
        sscanf(buf,"%ld",&MC_MAX_FREQ);
        fclose(fp);
        nfp = open("/sys/devices/system/cpu/cpufreq/policy2/scaling_max_freq", O_WRONLY);
        sprintf(buf,"%ld\n",MC_MAX_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
    }
    if ((access("/sys/devices/system/cpu/cpufreq/policy4/scaling_min_freq",0))!=-1) {
        sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu4_freq",bc_min);
        fp = fopen(URL, "r");
        fgets(buf,sizeof(buf),fp); 
        sscanf(buf,"%ld",&MC_MIN_FREQ);
        fclose(fp);
        nfp = open("/sys/devices/system/cpu/cpufreq/policy4/scaling_min_freq", O_WRONLY);
        sprintf(buf,"%ld\n",MC_MIN_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
        if ((access("/sys/devices/system/cpu/cpufreq/policy6/scaling_min_freq",0))!=-1) {
            sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu6_freq",bc_min);
            fp = fopen(URL, "r");
            fgets(buf,sizeof(buf),fp); 
            sscanf(buf,"%ld",&BC_MIN_FREQ);
            fclose(fp);
            nfp = open("/sys/devices/system/cpu/cpufreq/policy6/scaling_min_freq", O_WRONLY);
            sprintf(buf,"%ld\n",BC_MIN_FREQ);
            write(nfp,buf,sizeof(buf));
            close(nfp);
        } else if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq",0))!=-1) {
            sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu7_freq",bc_min);
            fp = fopen(URL, "r");
            fgets(buf,sizeof(buf),fp); 
            sscanf(buf,"%ld",&BC_MIN_FREQ);
            fclose(fp);
            nfp = open("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", O_WRONLY);
            sprintf(buf,"%ld\n",BC_MIN_FREQ);
            write(nfp,buf,sizeof(buf));
            close(nfp);
        } 
    } else if ((access("/sys/devices/system/cpu/cpufreq/policy6/scaling_min_freq",0))!=-1) {
        sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu6_freq",bc_min);
        fp = fopen(URL, "r");
        fgets(buf,sizeof(buf),fp); 
        sscanf(buf,"%ld",&MC_MIN_FREQ);
        fclose(fp);
        nfp = open("/sys/devices/system/cpu/cpufreq/policy6/scaling_min_freq", O_WRONLY);
        sprintf(buf,"%ld\n",MC_MIN_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
        if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq",0))!=-1) {
            sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu7_freq",bc_min);
            fp = fopen(URL, "r");
            fgets(buf,sizeof(buf),fp); 
            sscanf(buf,"%ld",&BC_MIN_FREQ);
            fclose(fp);
            nfp = open("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq", O_WRONLY);
            sprintf(buf,"%ld\n",BC_MIN_FREQ);
            write(nfp,buf,sizeof(buf));
            close(nfp);
        } 
    } else if ((access("/sys/devices/system/cpu/cpu4/cpufreq/scaling_min_freq",0))!=-1) {
        sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu4_freq",bc_min);
        fp = fopen(URL, "r");
        fgets(buf,sizeof(buf),fp); 
        sscanf(buf,"%ld",&MC_MIN_FREQ);
        fclose(fp);
        nfp = open("/sys/devices/system/cpu/cpu4/cpufreq/scaling_min_freq", O_WRONLY);
        sprintf(buf,"%ld\n",MC_MIN_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
    }
    if ((access("/sys/devices/system/cpu/cpufreq/policy4/scaling_max_freq",0))!=-1) {
        sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu4_freq",bc_max);
        fp = fopen(URL, "r");
        fgets(buf,sizeof(buf),fp); 
        sscanf(buf,"%ld",&MC_MAX_FREQ);
        fclose(fp);
        nfp = open("/sys/devices/system/cpu/cpufreq/policy4/scaling_max_freq", O_WRONLY);
        sprintf(buf,"%ld\n",MC_MAX_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
        if ((access("/sys/devices/system/cpu/cpufreq/policy6/scaling_max_freq",0))!=-1) {
            sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu6_freq",bc_max);
            fp = fopen(URL, "r");
            fgets(buf,sizeof(buf),fp); 
            sscanf(buf,"%ld",&BC_MAX_FREQ);
            fclose(fp);
            nfp = open("/sys/devices/system/cpu/cpufreq/policy6/scaling_max_freq", O_WRONLY);
            sprintf(buf,"%ld\n",BC_MAX_FREQ);
            write(nfp,buf,sizeof(buf));
            close(nfp);
        } else if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_max_freq",0))!=-1) {
            sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu7_freq",bc_max);
            fp = fopen(URL, "r");
            fgets(buf,sizeof(buf),fp); 
            sscanf(buf,"%ld",&BC_MAX_FREQ);
            fclose(fp);
            nfp = open("/sys/devices/system/cpu/cpufreq/policy7/scaling_max_freq", O_WRONLY);
            sprintf(buf,"%ld\n",BC_MAX_FREQ);
            write(nfp,buf,sizeof(buf));
            close(nfp);
        }
    } else if ((access("/sys/devices/system/cpu/cpufreq/policy6/scaling_max_freq",0))!=-1) {
        sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu6_freq",bc_max);
        fp = fopen(URL, "r");
        fgets(buf,sizeof(buf),fp); 
        sscanf(buf,"%ld",&MC_MAX_FREQ);
        fclose(fp);
        nfp = open("/sys/devices/system/cpu/cpufreq/policy6/scaling_max_freq", O_WRONLY);
        sprintf(buf,"%ld\n",MC_MAX_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
        if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_max_freq",0))!=-1) {
            sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu7_freq",bc_max);
            fp = fopen(URL, "r");
            fgets(buf,sizeof(buf),fp); 
            sscanf(buf,"%ld",&BC_MAX_FREQ);
            fclose(fp);
            nfp = open("/sys/devices/system/cpu/cpufreq/policy7/scaling_max_freq", O_WRONLY);
            sprintf(buf,"%ld\n",BC_MAX_FREQ);
            write(nfp,buf,sizeof(buf));
            close(nfp);
        }
    } else if ((access("/sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq",0))!=-1) {
        sprintf(URL,"/data/Cuprum_Custom/cpufreq_tables/level%d/cpu4_freq",bc_max);
        fp = fopen(URL, "r");
        fgets(buf,sizeof(buf),fp); 
        sscanf(buf,"%ld",&MC_MAX_FREQ);
        fclose(fp);
        nfp = open("/sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq", O_WRONLY);
        sprintf(buf,"%ld\n",MC_MAX_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
    }
    //MTK UnderClock
    if ((access("/proc/ppm/policy/hard_userlimit_cpu_freq",0))!=-1) {
        if ((access("/sys/devices/system/cpu/cpufreq/policy7/scaling_min_freq",0))!=-1) {
            nfp = open("/proc/ppm/policy/hard_userlimit_cpu_freq", O_WRONLY);
            sprintf(buf,"%ld\40%ld\40%ld\40%ld\40%ld\40%ld\n",SC_MIN_FREQ,SC_MAX_FREQ,MC_MIN_FREQ,MC_MAX_FREQ,BC_MIN_FREQ,BC_MAX_FREQ);
            write(nfp,buf,sizeof(buf));
            close(nfp);
        } else {
            nfp = open("/proc/ppm/policy/hard_userlimit_cpu_freq", O_WRONLY);
            sprintf(buf,"%ld\40%ld\40%ld\40%ld\n",SC_MIN_FREQ,SC_MAX_FREQ,MC_MIN_FREQ,MC_MAX_FREQ);
            write(nfp,buf,sizeof(buf));
            close(nfp);
        }
    }
    //Exynos UnderClock
    if ((access("/sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster0_min_freq",0))!=-1) {
        nfp = open("/sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster0_min_freq", O_WRONLY);
        sprintf(buf,"%ld\n",SC_MIN_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
    }
    if ((access("/sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster0_max_freq",0))!=-1) {
        nfp = open("/sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster0_max_freq", O_WRONLY);
        sprintf(buf,"%ld\n",SC_MAX_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
    }
    if ((access("/sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster1_min_freq",0))!=-1) {
        nfp = open("/sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster1_min_freq", O_WRONLY);
        sprintf(buf,"%ld\n",MC_MIN_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
    }
    if ((access("/sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster1_max_freq",0))!=-1) {
        nfp = open("/sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster1_max_freq", O_WRONLY);
        sprintf(buf,"%ld\n",MC_MAX_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
    }
    if ((access("/sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster2_min_freq",0))!=-1) {
        nfp = open("/sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster2_min_freq", O_WRONLY);
        sprintf(buf,"%ld\n",BC_MIN_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
    }
    if ((access("/sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster2_max_freq",0))!=-1) {
        nfp = open("/sys/devices/system/cpu/cpufreq/mp-cpufreq/cluster2_max_freq", O_WRONLY);
        sprintf(buf,"%ld\n",BC_MAX_FREQ);
        write(nfp,buf,sizeof(buf));
        close(nfp);
    }
    return 0;
}
int main(void){
	// Get CPU Core Number.
	FILE *core_file ;
    char core_buf[8];
    int core_num;
    core_file = fopen("/sys/devices/system/cpu/kernel_max", "r");
    fgets(core_buf, sizeof(core_buf), core_file); 
    sscanf(core_buf, "%d", &core_num);
    fclose(core_file);
    // Initialize Value
    FILE *fp;
    char get_mode[32];
    char buf[128];
    char cpu[8];
    char mode[16];
    int i;
    int fa_delay_time=0;
    int fasw_state=0;
    int boost_level[7]={0};
	int BOOST_STATE=0;
	int SCREEN_OFF=0;
    int long_time=200;
    int single_time=40;
    int get_mode_time=0;
    int get_dp_time=0;
    int get_jank_time=0;
    int long_usage=0;
    int single_usage=0;
    long int avg_all1,avg_all2,avg_used1,avg_used2;
    long int user, nice, sys, idle, iowait, irq, softirq;
    long int all1[8]={0};
    long int all2[8]={0};
    long int used1[8]={0};
    long int used2[8]={0};
    int cpu_perf_mask[8]={0};
    char URL[64];
	system("echo $(date +%F) $(date +%T) [I] CuAware: Service Initialize Success. >> '/data/Cuprum_Log.txt'");
	//Get SurfaceFlinger Data
    long long int frame_time;
    int jank_num;
    if (system("dumpsys SurfaceFlinger --latency > /dev/null") != -1) {
        system("echo $(date +%F) $(date +%T) [I] CuAware: Get SurfaceFlinger Data Success. >> '/data/Cuprum_Log.txt'");
	} else {
		system("echo $(date +%F) $(date +%T) [W] CuAware: Failed to get SurfaceFlinger data. >> '/data/Cuprum_Log.txt'");
    }
    //Get Display Power data
    fp = popen("dumpsys power | grep \"Display Power: state=\" ", "r");
    fgets(buf, sizeof(buf),fp); 
    if (strstr(buf,"state=")){
        system("echo $(date +%F) $(date +%T) [I] CuAware: Get PowerManager Data Success. >> '/data/Cuprum_Log.txt'");
    } else {
        system("echo $(date +%F) $(date +%T) [W] CuAware: Failed to get PowerManager data. >> '/data/Cuprum_Log.txt'");
    }
    pclose(fp);
	// Connect to /dev/input/event
    struct input_event event_value;
    char EVENT_URL[64];
    int event_fd;
    int touch_event=GetTouchEvent();
    sprintf(EVENT_URL,"/dev/input/event%d",touch_event);
    event_fd = open (EVENT_URL, O_RDONLY| O_NONBLOCK);
    //Get Per CPU Perf Info
    fp=fopen("/data/Cuprum_Custom/cpu_perf_mask","r");
    fgets(buf,sizeof(buf),fp);
    sscanf(buf,"%d%d%d%d%d%d%d%d",&cpu_perf_mask[0],&cpu_perf_mask[1],&cpu_perf_mask[2],&cpu_perf_mask[3],&cpu_perf_mask[4],&cpu_perf_mask[5],&cpu_perf_mask[6],&cpu_perf_mask[7]);
    fclose(fp);
    system("echo $(date +%F) $(date +%T) [I] CuAware: Service Running.  >> '/data/Cuprum_Log.txt'");
	while(1){
		//Get Display Power State
		if (get_dp_time == 0) {
            if (SCREEN_OFF == 0 && BOOST_STATE < 2) {
                fp = popen("dumpsys power | grep \"Display Power: state=\" ", "r");
                fgets(buf, sizeof(buf),fp); 
                pclose(fp);
                if (strstr(buf,"state=DOZE") || strstr(buf,"state=OFF")) {
                    SCREEN_OFF=1;
                    write_freq(0,10,0,10);
                    if (fasw_state != 0) {
                        system("kill -STOP $(pgrep frame_analysis)");
                        fasw_state=0;
                    }
                    system("kill -STOP $(ps -eo pid,name | grep \"taskclassify\" | awk '{print $1}')");
                    system("$(pwd)/libcuprum powersave");
                    BOOST_STATE=9;
                    long_time=200;
                    single_time=40;
                    get_mode_time=0;
                    get_dp_time=0;
                    get_jank_time=0;
                    long_usage=0;
                    single_usage=0;
                    boost_level[0]=0;
                    boost_level[1]=0;
                    boost_level[2]=0;
                    boost_level[3]=0;
                    boost_level[4]=0;
                    boost_level[5]=0;
                    boost_level[6]=0;
                }
            }
            get_dp_time=2000;
        } else {
            get_dp_time=get_dp_time-1;
        }
        if (SCREEN_OFF == 1) {
            event_value.type=0;
            read (event_fd, &event_value, sizeof (event_value));
            if (event_value.type == 3) {
                fp = popen("dumpsys power | grep \"Display Power: state=\" ", "r");
                fgets(buf, sizeof(buf),fp); 
                pclose(fp);
                if (strstr(buf,"state=ON")) {
                    SCREEN_OFF=0;
                    write_freq(10,10,10,10);
                    system("kill -CONT $(ps -eo pid,name | grep \"taskclassify\" | awk '{print $1}')");
                    if ( strcmp(mode,"powersave") == 0 ) {
                        system("$(pwd)/libcuprum powersave");
                    } else if ( strcmp(mode,"balance") == 0 ) {
                        system("$(pwd)/libcuprum balance");
                    } else {
                        system("$(pwd)/libcuprum performance");
                    }
                }
            }
        }
        if(SCREEN_OFF == 0) {
		    //Get Adjust Mode
		    if (get_mode_time == 0){
                fp = fopen("/data/Cuprum_Custom/mode", "r");
                fgets(buf, sizeof(buf),fp); 
                sscanf(buf, "%s", get_mode);
                fclose(fp);
                //Run main&gpu Adjust
                if ( strcmp(mode,get_mode) != 0 ) {
                    if ( strcmp(get_mode,"powersave") == 0 ) {
                        system("$(pwd)/libcuprum powersave");
                        sscanf(get_mode,"%s",mode);
                        system("echo $(date +%F) $(date +%T) [I] CuAware: AdjustMode switch to powersave success. >> '/data/Cuprum_Log.txt'");
                    } else if ( strcmp(get_mode,"balance") == 0 ) {
                        system("$(pwd)/libcuprum balance");
                        sscanf(get_mode,"%s",mode);
                        system("echo $(date +%F) $(date +%T) [I] CuAware: AdjustMode switch to balance success. >> '/data/Cuprum_Log.txt'");
                    } else {
                        system("$(pwd)/libcuprum performance");
                        sscanf(get_mode,"%s",mode);
                        system("echo $(date +%F) $(date +%T) [I] CuAware: AdjustMode switch to performance success. >> '/data/Cuprum_Log.txt'");
                    }
                    write_freq(0,10,0,10);
                    BOOST_STATE=0;
                }
                get_mode_time=500;
            } else {
                get_mode_time=get_mode_time-1;
            }
            if (BOOST_STATE < 2) {
                single_time=40;
                single_usage=0;
		        //Get Long CPU Load Value
                if (long_time == 200) {
                 	fp = fopen("/proc/stat", "r");
                     fgets(buf, sizeof(buf), fp); 
                     sscanf(buf, "%s%ld%ld%ld%ld%ld%ld%ld", cpu, &user, &nice, &sys, &idle, &iowait, &irq, &softirq);
                     fclose(fp);
                     avg_all1 = user + nice + sys + idle + iowait + irq + softirq; 
                     avg_used1=user+nice+sys;                             
                     long_time=long_time-1;
                }else if (long_time > 0) {
                 	long_time=long_time-1;
                }else{
                 	fp = fopen("/proc/stat", "r");
                     fgets(buf, sizeof(buf), fp); 
                     sscanf(buf, "%s%ld%ld%ld%ld%ld%ld%ld", cpu, &user, &nice, &sys, &idle, &iowait, &irq, &softirq);
                     fclose(fp);
                     avg_all2 = user + nice + sys + idle + iowait + irq + softirq; 
                     avg_used2=user+nice+sys;                                         
                     long_usage=(avg_used2-avg_used1)*100/(avg_all2-avg_all1);
                     long_time=200;
                }
            } else {
                long_time=200;
                long_usage=0;
                //Get Single CPU Load Value
                if (single_time == 40) {
                    fp = fopen("/proc/stat", "r");
                    fgets(buf, sizeof(buf), fp); 
                    for (i=0;i<core_num;i++) {
                        fgets(buf, sizeof(buf), fp); 
                        sscanf(buf, "%s%ld%ld%ld%ld%ld%ld%ld", cpu, &user, &nice, &sys, &idle, &iowait, &irq, &softirq);
                        all1[i] = user + nice + sys + idle + iowait + irq + softirq;
                        used1[i] = user + nice + sys ;  
                    }
                    fclose(fp);               
                    single_time=single_time-1;
                 } else if (single_time > 0) {
                    single_time=single_time-1;
                 } else {
		            fp = fopen("/proc/stat", "r");
                    fgets(buf, sizeof(buf), fp); 
                    for (i=0;i<core_num;i++) {
                        fgets(buf, sizeof(buf), fp); 
                        sscanf(buf, "%s%ld%ld%ld%ld%ld%ld%ld", cpu, &user, &nice, &sys, &idle, &iowait, &irq, &softirq);
                        all2[i] = user + nice + sys + idle + iowait + irq + softirq;
                        used2[i] = user + nice + sys ;  
                    }
                    fclose(fp);
                    int tmp_usage=0;
                    long int cur_freq;
                    single_usage=0;
                    for (i=0;i<core_num;i++) {
                        sprintf(URL,"/sys/devices/system/cpu/cpu%d/cpufreq/scaling_cur_freq",i);
                        fp=fopen(URL,"r");
                        fgets(buf, sizeof(buf),fp); 
                        sscanf(buf,"%ld",&cur_freq);
                        fclose(fp);
                        int usage_info = (used2[i]-used1[i])*100/(all2[i]-all1[i]);
                        if (usage_info == 100) {
                            tmp_usage=0;
                        } else {
                            tmp_usage=(used2[i]-used1[i])*100/(all2[i]-all1[i])*cur_freq*cpu_perf_mask[i]/1000000000;
                        }
                        if (tmp_usage > single_usage) {
                            single_usage=tmp_usage;
                        }
                    }
                    single_time=40;
                 }
             }
            // Get JANK Value
            if (fasw_state != 0) {
                if (get_jank_time > 0) {
                    get_jank_time=get_jank_time-1;
                } else {
                    if ((access("/data/Cuprum_Custom/jank_num",0))!=-1) {
                        fp = fopen("/data/Cuprum_Custom/jank_num", "r");
                        fgets(buf, sizeof(buf), fp); 
                        sscanf(buf,"%d",&jank_num);
                        fclose(fp);
                    } else {
                        jank_num=0;
                    }
                    get_jank_time=50;
                }
            } else {
                jank_num=0;
                get_jank_time=50;
            }
            // Read /dev/input/event
            event_value.type=0;
            read (event_fd, &event_value, sizeof (event_value));
            // Frame_Analysis Switch
            if (event_value.type == 3) {
                fa_delay_time=2000;
                if (fasw_state != 1) {
                    system("kill -CONT $(pgrep frame_analysis)");
                    fasw_state=1;
                }
            } else {
                jank_num=0;
                if (fa_delay_time > 0) {
                    fa_delay_time=fa_delay_time-1;
                } else {
                    if (fasw_state != 0) {
                        system("kill -STOP $(pgrep frame_analysis)");
                        fasw_state=0;
                    }
                }
            }
           // Boost Selecter
           if (single_usage > 360 || jank_num > 4) {
               boost_level[6]=200;
           } else if (single_usage > 320 || jank_num > 2) {
               boost_level[5]=200;
           } else if (single_usage > 280 || jank_num > 0) {
               boost_level[4]=200;
           } else if (long_usage > 60 || single_usage > 240) {
               boost_level[3]=500;
           } else if (long_usage > 40 || single_usage > 200) {
               boost_level[2]=500;
           } else if (event_value.type == 3) {
               boost_level[1]=200;
           } else if (long_usage > 20) {
               boost_level[0]=2000;
           }
           if (boost_level[6] > 0) {
               boost_level[6]=boost_level[6]-1;
               boost_level[5]=0;
               boost_level[4]=0;
               boost_level[3]=0;
               boost_level[2]=0;
               boost_level[1]=0;
               boost_level[0]=0;
           }
           if (boost_level[5] > 0) {
               boost_level[5]=boost_level[5]-1;
               boost_level[4]=0;
               boost_level[3]=0;
               boost_level[2]=0;
               boost_level[1]=0;
               boost_level[0]=0;
           }
           if (boost_level[4] > 0) {
               boost_level[4]=boost_level[4]-1;
               boost_level[3]=0;
               boost_level[2]=0;
               boost_level[1]=0;
               boost_level[0]=0;
           }
           if (boost_level[3] > 0) {
               boost_level[3]=boost_level[3]-1;
               boost_level[2]=0;
               boost_level[1]=0;
               boost_level[0]=0;
           }
           if (boost_level[2] > 0) {
               boost_level[2]=boost_level[2]-1;
               boost_level[1]=0;
               boost_level[0]=0;
            }
           if (boost_level[1] > 0) {
               boost_level[1]=boost_level[1]-1;
               boost_level[0]=0;
           }
           if (boost_level[0] > 0) {
               boost_level[0]=boost_level[0]-1;
           }
           if ( strcmp(mode,"powersave") == 0 ) {
               if (boost_level[6] > 0) {
                  if (BOOST_STATE != 7) {
                       write_freq(8,8,6,6);
                       BOOST_STATE=7;
                   }
               } else if (boost_level[5] > 0) {
                   if (BOOST_STATE != 6) {
                       write_freq(7,8,6,6);
                       BOOST_STATE=6;
                   }
               } else if (boost_level[4] > 0) {
                   if (BOOST_STATE != 5) {
                       write_freq(6,8,6,6);
                       BOOST_STATE=5;
                   }
               } else if (boost_level[3] > 0) {
                   if (BOOST_STATE != 4) {
                       write_freq(5,8,5,6);
                       BOOST_STATE=4;
                   }
               } else if (boost_level[2] > 0) {
                   if (BOOST_STATE != 3) {
                       write_freq(4,8,4,5);
                       BOOST_STATE=3;
                   }
               } else if (boost_level[1] > 0) {
                   if (BOOST_STATE != 2) {
                       write_freq(3,7,3,4);
                       BOOST_STATE=2;
                   }
               } else if (boost_level[0] > 0) {
                   if (BOOST_STATE != 1) {
                       write_freq(2,6,2,3);
                       BOOST_STATE=1;
                   }
               } else if (BOOST_STATE != 0) {
                   write_freq(0,5,0,3);
                   BOOST_STATE=0;
               }
           } else if ( strcmp(mode,"balance") == 0 ) {
               if (boost_level[6] > 0) {
                   if (BOOST_STATE != 7) {
                       write_freq(8,8,8,8);
                       BOOST_STATE=7;
                   }
               } else if (boost_level[5] > 0) {
                  if (BOOST_STATE != 6) {
                       write_freq(8,8,7,8);
                       BOOST_STATE=6;
                   }
               } else if (boost_level[4] > 0) {
                   if (BOOST_STATE != 5) {
                       write_freq(8,8,6,7);
                       BOOST_STATE=5;
                   }
               } else if (boost_level[3] > 0) {
                   if (BOOST_STATE != 4) {
                       write_freq(7,8,5,6);
                       BOOST_STATE=4;
                   }
               } else if (boost_level[2] > 0) {
                   if (BOOST_STATE != 3) {
                       write_freq(6,8,4,5);
                       BOOST_STATE=3;
                   }
               } else if (boost_level[1] > 0) {
                   if (BOOST_STATE != 2) {
                       write_freq(5,7,3,4);
                       BOOST_STATE=2;
                   }
               } else if (boost_level[0] > 0) {
                   if (BOOST_STATE != 1) {
                       write_freq(4,6,2,4);
                       BOOST_STATE=1;
                   }
               } else if (BOOST_STATE != 0) {
                   write_freq(0,6,0,4);
                   BOOST_STATE=0;
               }
           } else {
               if (boost_level[6] > 0) {
                   if (BOOST_STATE != 7) {
                       write_freq(10,10,10,10);
                       BOOST_STATE=7;
                   }
               } else if (boost_level[5] > 0) {
                   if (BOOST_STATE != 6) {
                       write_freq(10,10,9,10);
                       BOOST_STATE=6;
                   }
               } else if (boost_level[4] > 0) {
                   if (BOOST_STATE != 5) {
                       write_freq(10,10,8,9);
                       BOOST_STATE=5;
                   }
               } else if (boost_level[3] > 0) {
                   if (BOOST_STATE != 4) {
                       write_freq(9,10,7,8);
                       BOOST_STATE=4;
                   }
               } else if (boost_level[2] > 0) {
                   if (BOOST_STATE != 3) {
                       write_freq(8,9,6,7);
                       BOOST_STATE=3;
                   }
               } else if (boost_level[1] > 0) {
                   if (BOOST_STATE != 2) {
                       write_freq(7,8,6,6);
                       BOOST_STATE=2;
                   }
               } else if (boost_level[0] > 0) {
                   if (BOOST_STATE != 1) {
                       write_freq(6,7,5,5);
                       BOOST_STATE=1;
                   }
               } else if (BOOST_STATE != 0) {
                   write_freq(5,7,4,5);
                   BOOST_STATE=0;
               }
           } 
          usleep(1000);
       } else {
          usleep(10000);
       }
	}
	close(event_fd);
	return 0;
}
    
   