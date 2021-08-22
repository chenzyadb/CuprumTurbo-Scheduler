#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
int main(void) {
    long long int frame_table[128]={0};
    long long int frame_time;
    int jank_num,i;
    FILE *sfdata_file;
    int output_fp;
    char sf_buf[256];
    char output_buf[16];
    long long int a,b;
    long long int now_frame_time;
    while(1){
        sfdata_file = popen("dumpsys SurfaceFlinger --latency", "r");
        fgets(sf_buf, sizeof(sf_buf), sfdata_file); 
        sscanf(sf_buf, "%lld", &frame_time);
        for ( i=0 ; i<127 ; i++ ) {
            fgets(sf_buf, sizeof(sf_buf), sfdata_file); 
            sscanf(sf_buf, "%lld%lld%lld",&a,&frame_table[i],&b);
        }
        jank_num=0;
        for ( i=1 ; i<127 ; i++ ) {
            now_frame_time=frame_table[i]-frame_table[i-1];
            if (now_frame_time < 99999999999999 && now_frame_time > frame_time) {
                jank_num=jank_num+1;
            }
        }
        pclose(sfdata_file);
        if ((access("/data/Cuprum_Custom/jank_num",0))!=-1) {
            output_fp = open("/data/Cuprum_Custom/jank_num", O_WRONLY);
            sprintf(output_buf,"%d\n",jank_num);
            write(output_fp,output_buf,sizeof(output_buf));
            close(output_fp);
        } else {
            system("echo 0 > /data/Cuprum_Custom/jank_num");
        }
        if (jank_num > 0) {
            system("dumpsys SurfaceFlinger --latency-clear");
        }
        usleep(50000);
    }
    return 0;
}