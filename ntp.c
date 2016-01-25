#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "sys/errno.h"
#include "lib_ntp.h"
#define ISWHITESPACE(x) ((x) == ' ' || (x) == '\n' || (x) == '\t' || (x) == '\r' || (x) == '\v')


static int ntp_check_open_log(char *logfile,int msg_len)
{
	int fd,status,filesize = 0;
	fd=open(logfile,O_WRONLY |O_CREAT|O_APPEND);
	if(fd <= 0)
	{
		status = mkdir(HY_NTP_LOG_DIR, 0755); 
		printf("set %s logfile   status:%d\n", logfile,status);
		fd=open(logfile,O_WRONLY |O_CREAT|O_APPEND);
		if(fd <=0)
		{
			printf("creat log error\n");
			return -1;
		}
	}
	filesize = lseek(fd,0,SEEK_END);
	//printf("filesize:%d ---> %d\n",filesize,NTP_DEFAULT_LOG_SIZE);
	if(filesize + msg_len > NTP_DEFAULT_LOG_SIZE)
	{
		close(fd);
		printf("trunc log file\n");
		fd=open(logfile,O_WRONLY |O_TRUNC|O_APPEND);
	}
    return fd;
}

int write_ntp_log(char *ifname)
{
	int fd;
    time_t tt;
    struct tm *now;

    char buf[512];
    time(&tt);
    now = localtime(&tt);

    sprintf(buf,"[%04d-%02d-%02d %02d:%02d:%02d]:%s",
        now->tm_year+1900,now->tm_mon+1,now->tm_mday,now->tm_hour,
        now->tm_min,now->tm_sec,ifname);
    
	printf("%s",buf);
	
	fd = ntp_check_open_log(HY_NTP_LOG_DIR HY_NTP_LOG_FILE,strlen(buf)+1);
	if(fd>=0)
	{
		//write log
		write(fd,buf,strlen(buf)+1);
		close(fd);
	}
	return 0;
}

static int read_string(char *ret_string, char *file_dir, char *tag)
{
	FILE *fp;
	char Str[512]={0};
	char *pStr, *pLf, *pRt;
	int ret = FAIL;
	int hasTag = FALSE, hasVlaue = FALSE;
	
	fp = fopen(file_dir, "r");
	if(!fp)
	{
		printf("fopen %s fail\n", file_dir);
		return FAIL;
	}

	while (fgets (Str, 512, fp) != NULL)
	{
		hasTag = FALSE;
		hasVlaue = FALSE;
		
		//printf("(%s:%d) fgets: %s\n",__FUNCTION__,__LINE__, Str);
		if(Str[0] == '\r' || Str[0] == '\n' || Str[0] == '#' || Str[0] == '=')
		{
			continue;
		}
			
		pStr = strchr(Str, '=');
		if(!pStr)
		{
			continue;
		}
		pLf = Str;
		pRt = pStr-1;
		pStr++;

		/*** find tag ***/
		while(ISWHITESPACE(*pRt) && pLf != pRt)
		{
			pRt--;
		}

		if(/*pLf == pRt &&*/ ISWHITESPACE(*pRt))
		{
			continue;
		}
		*(pRt+1) = 0;

		while(ISWHITESPACE(*pLf))
		{
			pLf++;
		}
		
		if (strcmp(tag, pLf))
		{
			continue;
		}

		/*** find value ***/
		pLf = pStr;
		pRt = pStr+strlen(pStr)-1;
		
		while(ISWHITESPACE(*pRt) && pLf != pRt)
		{
			pRt--;
		}

		if(/*pLf == pRt &&*/ ISWHITESPACE(*pRt))
		{
			break;
		}
		*(pRt+1) = 0;
		
		while(ISWHITESPACE(*pLf))
		{
			pLf++;
		}
		
		strcpy(ret_string, pLf);
		ret = SUCCESS;
		break;
	}
	
	fclose(fp);
	return ret;
}

int net_init_config()
{
	FILE *IniFile = NULL;
	int ret = SUCCESS,status;
	IniFile = fopen(PARAM_CFG_DIR PARAM_CFG_NAME, "r");
	if(!IniFile)
	{
		printf("set %s default config\n", PARAM_CFG_DIR PARAM_CFG_NAME);
		status = mkdir(PARAM_CFG_DIR, 0755); 
		printf("set %s default config   status:%d\n", PARAM_CFG_DIR PARAM_CFG_NAME,status);
		IniFile = fopen(PARAM_CFG_DIR PARAM_CFG_NAME, "w");
		if(!IniFile)
		{
			write_ntp_log("init ntp config error\n"); 
			return FAIL;
		}
		//set default net config
		fprintf (IniFile, "%s = %d\n",PARAM_NTP_SERVER_ENABLE,1);
		fprintf (IniFile, "%s = %s\n",PARAM_NTP_SERVER_TAG,NTP_DEFAULT_SERVER);
		fprintf (IniFile, "%s = %d\n",PARAM_NTP_SERVER_PORT,NTP_DEFAULT_PORT);
		fprintf (IniFile, "%s = %d\n",PARAM_NTP_SERVER_TIMEOUT,NTP_DEFAULT_TIMEOUT);
	}
	fclose (IniFile);
	return ret;
}

int get_ntp_enable(char *pwd)
{
	return read_string(pwd, PARAM_CFG_DIR PARAM_CFG_NAME, PARAM_NTP_SERVER_ENABLE);
}

int get_ntp_server(char *pwd)
{
	return read_string(pwd, PARAM_CFG_DIR PARAM_CFG_NAME, PARAM_NTP_SERVER_TAG);
}

int get_ntp_port(char *pwd)
{
	return read_string(pwd, PARAM_CFG_DIR PARAM_CFG_NAME, PARAM_NTP_SERVER_PORT);
}

int get_ntp_timeout(char *pwd)
{
	return read_string(pwd, PARAM_CFG_DIR PARAM_CFG_NAME, PARAM_NTP_SERVER_TIMEOUT);
}

int send_packet_ntpserver(int fd)
{
    unsigned int data[12];
    struct timeval now;
    int ret = 0;
#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4
#define PREC -6

    if (sizeof(data) != 48)
    {
        fprintf(stderr,"size error\n");
        return -1;
    }

    memset((char*)data, 0, sizeof(data));
    data[0] = htonl((LI << 30) | (VN << 27) | (MODE << 24)
                  | (STRATUM << 16) | (POLL << 8) | (PREC & 0xff));
    data[1] = htonl(1<<16);  /* Root Delay (seconds) */
    data[2] = htonl(1<<16);  /* Root Dispersion (seconds) */
    gettimeofday(&now, NULL);
    data[10] = htonl(now.tv_sec + JAN_1970); /* Transmit Timestamp coarse */
    data[11] = htonl(NTPFRAC(now.tv_usec));  /* Transmit Timestamp fine   */
    ret = send(fd, data, 48, 0);
    printf("send_packet........\n");
    return ret;
}

void get_packet_timestamp(struct ntptime *udp_arrival_ntp)
{
    struct timeval udp_arrival;

    gettimeofday(&udp_arrival, NULL);
    udp_arrival_ntp->coarse = udp_arrival.tv_sec + JAN_1970;
    udp_arrival_ntp->fine   = NTPFRAC(udp_arrival.tv_usec);
//    printf("get_packet_timestamp..........\n");
}

void rfc1305print(unsigned int *data, struct ntptime *arrival, struct timeval* tv)
{
    int li, vn, mode, stratum, poll, prec;
    int delay, disp, refid;
    struct ntptime reftime, orgtime, rectime, xmttime;
    

#define Data(i) ntohl(((unsigned int *)data)[i])
    li      = Data(0) >> 30 & 0x03;
    vn      = Data(0) >> 27 & 0x07;
    mode    = Data(0) >> 24 & 0x07;
    stratum = Data(0) >> 16 & 0xff;
    poll    = Data(0) >>  8 & 0xff;
    prec    = Data(0)       & 0xff;
    if (prec & 0x80) prec|=0xffffff00;
    delay   = Data(1);
    disp    = Data(2);
    refid   = Data(3);
    reftime.coarse = Data(4);
    reftime.fine   = Data(5);
    orgtime.coarse = Data(6);
    orgtime.fine   = Data(7);
    rectime.coarse = Data(8);
    rectime.fine   = Data(9);
    xmttime.coarse = Data(10);
    xmttime.fine   = Data(11);
#undef Data

    tv->tv_sec = xmttime.coarse - JAN_1970;
    tv->tv_usec = USEC(xmttime.fine);
}
int get_time_from_ntpserver(unsigned int *data,struct timeval* tv)
{
    struct ntptime arrival_ntp;
    
    get_packet_timestamp(&arrival_ntp);
    rfc1305print(data,&arrival_ntp,tv);
    return 0;
}
void set_local_time(struct timeval tv)
{
    /* need root user. */
//    if (0 != getuid() && 0 != geteuid())
//        return;
    tv.tv_sec=tv.tv_sec+8*3600;
    settimeofday(&tv, NULL);
}


int main(void)
{
    /* create socket. */
	int sock,ret,i=0,j=0;
    /* bind local address. */
    struct sockaddr_in addr_src,addr_dst;
    int addr_len = sizeof(addr_src);
	char pwd[128]={0};
	struct hostent *answer;
	char ipstr[16];

    memset(&addr_src, 0, addr_len);
    addr_src.sin_family = AF_INET;
    addr_src.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_src.sin_port = htons(0);
	int ntp_port,ntp_timeout,set_time_flag = 0;
    struct timeval timeout;

	fd_set fds_read;
	unsigned int buf[12];
	struct sockaddr server;
	socklen_t svr_len;
	struct timeval newtime;

    char buf_log[256];
	
	ret = net_init_config();
	if(ret != SUCCESS)
	{
		write_ntp_log("start ntp_init error\n");
		return 0;
	}
	write_ntp_log("start ntp ok\n");
	while(1)
	{
		if(get_ntp_enable(pwd)!=0)
		{
			//write_ntp_log("check ntp_enable error \n\t ");
			printf("check ntp_enable error\n");
			sleep(30);
			continue;
		}

		if(atoi(pwd) != 1)
		{	
			printf("__%d ntp enable    :	%s\n", __LINE__,pwd);
			sleep(3);
			continue;
		}
		
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if(sock<0)
		{
			sleep(3);
			write_ntp_log("creat socket error\n");
			continue;
		}
#if 0	//不使用阻塞方式，方便在select之前第一次正确获取时间，并设置
		if((flags = fcntl(sock, F_GETFL, 0)) < 0)
		{
			perror("Netwrok test...\n");
			close(sock);
			return FAIL;
		}
		if(fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) 
		{
			perror("Network test...\n");
			close(sock);
			return FAIL;
		}
#endif
	    ret=bind(sock, (struct sockaddr*)&addr_src, addr_len);
		if(ret!=0)
		{
			sleep(3);
			close(sock);
			write_ntp_log("bind error\n");
			continue;
		}
	    /* connect to ntp server. */
	    memset(&addr_dst, 0, addr_len);
	    addr_dst.sin_family = AF_INET;	

		if(get_ntp_server(pwd)==0)
		{
			//printf("00 (%s:%d)ip    :	%s\n", __FUNCTION__,__LINE__,pwd);
			answer = gethostbyname(pwd);
			if (answer == NULL)
			{
				sleep(3);				 //这里出错的原因是dhcp线程还没有设置好ip，先睡一秒，再继续
				herror("gethostbyname"); //由gethostbyname自带的错误处理函数
				write_ntp_log("gethostname error\n");
				continue;
			}
			//printf("11 (%s:%d)ip    :	%s\n", __FUNCTION__,__LINE__,pwd);
			for (j = 0; (answer->h_addr_list)[j] != NULL; j++) //一个域名可能会有多个ip的情况
			{
				inet_ntop(AF_INET, (answer->h_addr_list)[j], ipstr, 16);
				printf("ip_%d:	%s\n", j,ipstr);
			}
			
			if((answer->h_addr_list)[i] != NULL) //i = 0,第一个ip失败，i++ ,依次遍历各个ip。
			{
				inet_ntop(AF_INET, (answer->h_addr_list)[i], ipstr, 16);
			}else
			{
				i = 0;
				continue;
			}
			sprintf(buf_log,"get_ntp_ip %s\n",ipstr);
			write_ntp_log(buf_log);
			addr_dst.sin_addr.s_addr=inet_addr(ipstr);
		}
		else
		{
			 addr_dst.sin_addr.s_addr=inet_addr(NTP_SERVER_DEFAULT_IP);
			 sprintf(buf_log,"get cfg ntp_server null set default %s  \n ",NTP_SERVER_DEFAULT_IP);
			 write_ntp_log(buf_log);
		}

		if(get_ntp_port(pwd)==0)
		{
			ntp_port = atoi(pwd);
			if(ntp_port == 0)
			{
				ntp_port = NTP_DEFAULT_PORT;
			}
		}else
		{
			ntp_port = NTP_DEFAULT_PORT;
		}
		
	    addr_dst.sin_port = htons(ntp_port);
		ret=connect(sock, (struct sockaddr*)&addr_dst, addr_len);
		if(ret!=0)
		{
			sleep(3);
			close(sock);
			write_ntp_log("connect ntp server ...\n");
			i++;
			continue;
		}
		if(get_ntp_timeout(pwd)==0)
		{
			ntp_timeout = atoi(pwd);
			if(ntp_timeout == 0)
			{
				ntp_timeout = NTP_DEFAULT_TIMEOUT;
			}
		}else
		{
			ntp_timeout = NTP_DEFAULT_TIMEOUT;
		}
		
		printf("connect success,timeout = %d\n",ntp_timeout);

		//then use select timeout get ntpserver time and set it
		while(1)
		{
			FD_ZERO(&fds_read);
			FD_SET(sock, &fds_read);
			
			timeout.tv_sec = ntp_timeout;
			if(set_time_flag == 0)
			{
				timeout.tv_sec = 1; //程序一启动就设置时间，之后由timeout超时后再设置
			}
			timeout.tv_usec = 0;
			ret = select(sock + 1, &fds_read, NULL, NULL, &timeout);
			if (0 == ret || !FD_ISSET(sock, &fds_read))
			{
				/* send ntp protocol packet. */
				send_packet_ntpserver(sock);
				continue;
			}
			else if(ret<0)
			{
				break;
			}
			
			/* recv ntp server's response. */
			recvfrom(sock, buf, sizeof(buf), 0, &server, &svr_len);

			/* get server's time and set it. */
			get_time_from_ntpserver(buf,&newtime);
			
			printf("ntp sec :newtime.sec:%ld  usec:%ld\n",newtime.tv_sec,newtime.tv_usec);
			
			set_local_time(newtime);
			set_time_flag = 1;
			sprintf(buf_log,"set time:%ld\n",newtime.tv_sec);
			write_ntp_log(buf_log);
	    }

		close(sock);
    }
    return SUCCESS;
}
//#endif

