#ifndef MYHEADER_H
#define MYHEADER_H

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<curses.h>
#include<netinet/in.h>
#include<errno.h>
#include<signal.h>
#include<arpa/inet.h>
#include<sys/select.h>
#include<sys/time.h>
#include<sys/wait.h>
#include<sys/ioctl.h>
#include<net/if.h>

#define VALID -1
#define NOT_SEND 0
#define HAS_SEND 1
#define SEND_TYPE_UPPER 0
#define SENF_TYPE_LOWER 1
#define BUF_SIZE 8192

#define debug_mail 0
#define debug_trans 0
#define debug_getdetail 0
#define debug_log 1

#define AGENTPORT 9807
#define BACKLOG 32

#ifndef uint_8 
typedef unsigned char  uint_8;
#endif

#ifndef uint_16 
typedef unsigned short uint_16;
#endif

#ifndef wchar
typedef uint_16 wchar;
#endif

struct smtp_buf{
	int start_label;
	int end_label;
	int has_attachment;
	int end_attachment;
	char buf[BUF_SIZE];

};

struct ctrl_flag{
	int sendflag;
	int sendtype;
	int whiteflag;
};

#endif
