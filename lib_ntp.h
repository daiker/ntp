#ifndef __LIB_UPDATE_H_
#define __LIB_UPDATE_H_
#include <sys/time.h>
#include <time.h>
//#define NTP_SERVER    "ntp.sjtu.edu.cn"
//#define NTP_SERVER    "time.nist.gov"
#define NTP_SERVER_DEFAULT_IP    "128.138.141.172"

#define NTP_DEFAULT_SERVER 		"52.84.1.115" //"time.nist.gov" //"192.168.2.166" //"24.56.178.140" //
#define NTP_DEFAULT_PORT      	123
#define NTP_DEFAULT_TIMEOUT     3600*5
#define NTP_DEFAULT_LOG_SIZE	1024		//limit log file size
#define JAN_1970      0x83aa7e80      /* 2208988800 1970 - 1900 in seconds */

#define NTPFRAC(x) (4294 * (x) + ((1981 * (x))>>11))
#define USEC(x) (((x) >> 12) - 759 * ((((x) >> 10) + 32768) >> 16))

#ifndef SUCCESS
#define SUCCESS 0  //返回结果成功
#define FAIL -1 //返回错误
#define FALSE 0
#endif

#define HY_NTP_LOG_DIR				"/usr/hip_ipcam/"
#define HY_NTP_LOG_FILE				"ntp_log"	//"/home/daiker/hy_ntp_log"
#define PARAM_CFG_DIR 				"/usr/hip_ipcam/"				//"/home/daiker/"
#define PARAM_CFG_NAME 				"ntpconfig.ini"
#define PARAM_NTP_SERVER_TAG 		"ntp_server"
#define PARAM_NTP_SERVER_PORT 		"ntp_port"
#define PARAM_NTP_SERVER_ENABLE 	"ntp_enable"
#define PARAM_NTP_SERVER_TIMEOUT 	"ntp_timeout"

struct ntptime
{
    unsigned int coarse;//粗略时间
    unsigned int fine;//精细时间
};

/*
    发送本地信息到ntp服务器
*/
int send_packet_ntpserver(int fd);
/*
    取得从ntp服务器的数据data后，分析得到同步时间tv
*/
int get_time_from_ntpserver(unsigned int *data,struct timeval* tv);

/*
    设置rtc的时间
*/

int set_rtc_time(struct tm* settime);
/*
    从rtc取得实时时间
*/
int get_rtc_time(struct tm* gettime);
void * ntp_init(void *param);
#endif

