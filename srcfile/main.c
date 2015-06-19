#include "../include/myheader.h"
#include "../include/smtp.h"
#include "../include/gb2312_to_unicode.h"
#include "../include/code.h"
#include "../include/semopt.h"

int main(void)
{
	int	smtp_listen_sockfd, smtp_client_sockfd,smtp_server_sockfd,smtp_tmp_sockfd;
	int	udp_sockfd;
	int	res,smtpport = AGENTPORT;
	int	listen_backlog = BACKLOG;
	char	wanip[16];

	pid_t	childpid;
	socklen_t	clilen;

	struct	sockaddr_in smtp_client_addr, smtp_server_addr;

	int 	sem4log;


	/*创建信号量 信号量的值设为1 在记录日志的时候使用该信号量*/
	sem4log = semget((key_t)SEM_KEY, 1, 0666 | IPC_CREAT);
	set_semvalue(sem4log);



	/*get coding table of gb2312<->unicode*/
	int 	i,j;
	unsigned int 	index1,index2;	
	unsigned short tmp;

	for(j=0;j<7445;j++){	
		tmp = gb2312_to_unicode[j][1]-0xa0a0;
	
		index1 = tmp>>8;
		index2 = tmp&0xff;

		if(!is_bigendian()){
	
			index1 = tmp&0xff;
			index2 = tmp>>8;
			
		}
		gb_2_uni[index1][index2] = gb2312_to_unicode[j][0];
	}/*end of coding table*/
	
	
	/*创建监听套接字*/	
	if( (smtp_listen_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){         
		fprintf(stderr,"socket error:%s\n\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	/* 得到wan口的ip地址*/
	res = get_wanip(wanip);         
	if(res == -1){
		fprintf(stderr,"get_wanip error: %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	/*初始化代理的地址结构*/
	bzero(&smtp_server_addr, sizeof(smtp_server_addr));                              
	smtp_server_addr.sin_family = AF_INET;
	inet_pton(AF_INET,wanip,&smtp_server_addr.sin_addr);
	smtp_server_addr.sin_port = htons(smtpport);

	/*调用bind函数，如果当前端口被占用则改变端口号继续调用 直到成功为止*/
	while(bind(smtp_listen_sockfd, (const struct sockaddr *)&smtp_server_addr, 
	sizeof(smtp_server_addr)) <0){    
		
		if(strncmp("Address already in use",strerror(errno),
			   strlen("Address already in use")) == 0){
			
			smtpport += 1;
			smtp_server_addr.sin_port = htons(smtpport);
		}

		else{
			fprintf(stderr,"bind error: %s\n",strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	
	/*设置iptables的DNAT,将smtp邮件转到代理端口*/
	if(res = do_iptables_dnat(wanip,smtpport) == -1){
		
		fprintf(stderr,"set iptables dnat error: %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	/*start lisent*/
	if(listen(smtp_listen_sockfd, listen_backlog) == -1){
		
		fprintf(stderr,"sorry listen error:%s\n",strerror(errno));	
		exit(EXIT_FAILURE);
	}

	if(debug_trans) printf("listening on %s:%d\n",wanip,smtpport);

	signal(SIGCHLD, SIG_IGN);	
	for ( ; ; ) {
		
		clilen = sizeof(smtp_client_addr);
		
		/*accept child*/
		if ( (smtp_client_sockfd = accept(smtp_listen_sockfd, (struct sockaddr *) &smtp_client_addr, 
			&clilen)) < 0) {
			
			if (errno == EINTR) continue;		
			else{
				fprintf(stderr,"sorry! accept error:%s,pid:%d\n",strerror(errno),getpid());
				exit(EXIT_FAILURE);
			}
		}

		/*for each child ,creat one process to deal with it */
		if ((childpid = fork()) == 0) {
			
			close(smtp_listen_sockfd);
			do_smtp(smtp_client_sockfd);
		}
	
	}
}
