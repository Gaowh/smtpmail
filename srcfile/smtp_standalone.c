#include "../include/myheader.h"
#include "../include/smtp.h"
#include "../include/semopt.h"

void do_smtp(int client_sockfd)
{
	struct	sockaddr_in realserver_addr;
	struct	sockaddr_in original_dst = {0};
	
	int	original_len = sizeof(original_dst);
	int	original_port;
	char	*original_ipaddr;
	
	int 	server_sockfd;
	int	res;
	
	char 	detail[BUF_SIZE];
	char 	buf[BUF_SIZE];
	char 	tm_buf[BUF_SIZE];
	char	*label;

	fd_set	rset;
	FD_ZERO(&rset);
 	
	/*usr getsockopt's option 80 to get the original destination of the smtpmail*/
	res = getsockopt(client_sockfd,SOL_IP,80,&original_dst,&original_len);
	
	/*get success*/
	if( res == 0 ) {
		original_port = ntohs(original_dst.sin_port);
		original_ipaddr = inet_ntoa(original_dst.sin_addr);
	}
	/* failed*/
	else {
		fprintf(stderr,"getsockopt error:%s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	/*creat a new socket*/
	if((server_sockfd  = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr,"sorry socket error:%s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	/*initialize the server addr struct*/
	bzero(&realserver_addr,sizeof(realserver_addr));
	
	realserver_addr.sin_family = AF_INET;
	
	realserver_addr.sin_port = htons(original_port);
	
	inet_pton(AF_INET, original_ipaddr,&realserver_addr.sin_addr.s_addr);
	
	/*try to connect agent and original server*/
	res = (connect(server_sockfd,(struct sockaddr *)&realserver_addr,sizeof(realserver_addr)));
	if(res == -1){
		fprintf(stderr,"connect error :%s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	/*creat and initialize struct smtp_buf , this struct is used to save the pre_detail of mail*/
	struct smtp_buf smtpbuf;
	smtpbuf.start_label = NOT_SEND;
	smtpbuf.end_label = NOT_SEND;
	smtpbuf.has_attachment = NOT_SEND;
	smtpbuf.end_attachment = NOT_SEND;

	/*creat and initialize the flag struct ,these flags are used to decide when and how to control mail trans*/
	struct ctrl_flag ctrlflag;
	ctrlflag.sendflag = NOT_SEND;
	ctrlflag.sendtype = VALID;
	ctrlflag.whiteflag = NOT_SEND;
	
	for( ; ;){

		/*use select to do the request*/
		FD_SET(client_sockfd,&rset);
		FD_SET(server_sockfd,&rset);

		int maxfd = max(client_sockfd,server_sockfd) + 1;
		select(maxfd,&rset,NULL,NULL,NULL);

		if( FD_ISSET(client_sockfd, &rset) ){
			
			/*read message from client*/
			 if((res = read(client_sockfd, buf, BUF_SIZE)) == -1){
			 	
				fprintf(stderr,"read form client error:%s\n",strerror(errno));
				
				close(client_sockfd);
				close(server_sockfd);
				exit(EXIT_FAILURE);
			 }
			
			/*if we don't get all the details we want,  do get_detail_buf*/
			 if(smtpbuf.start_label == NOT_SEND || smtpbuf.end_label == NOT_SEND){
			 	
				if(debug_getdetail) printf("now do get_detail_buf\n");
				get_detail_buf(&smtpbuf, buf, res);
				if(debug_getdetail) printf("get_detaiil_buf end\n");
			 }
			
			 /*after we do do_client*/
			 res = do_client(buf,res,server_sockfd,&ctrlflag);
			
			 if(res == -1){
				
				fprintf(stderr,"do_client error:%s\n",strerror(errno));
				close(client_sockfd);
				close(server_sockfd);
				exit(EXIT_FAILURE);
			 }
			 else if(res == 0){
			
				close(client_sockfd);
				close(server_sockfd);
				break;
			 }
		}

		if(FD_ISSET(server_sockfd,&rset)){
			
			/*read message from server and do do_server*/
			res = do_server(client_sockfd,server_sockfd,&ctrlflag);
			
			if(res == -1){
				fprintf(stderr,"do_server error:%s\n",strerror(errno));
				
				close(client_sockfd);
				close(server_sockfd);
				exit(EXIT_FAILURE);
			}
			else if(res == 0){
				close(client_sockfd);
				close(server_sockfd);
				break;
			}
		}
	}
	
	bzero(detail,BUF_SIZE);

	/*get detail from the pre_detail buf*/
	get_mail_detail(smtpbuf.buf, detail);
	
	/*get the sem for log*/
	int logsem = semget((key_t)SEM_KEY, 1, 0666 | IPC_CREAT);

	/*try to get key of doing log*/
	if(! semaphore_p(logsem) ){
		
		fprintf(stderr,"semaphore_p error: %s\n", strerror(errno));
		exit(EXIT_SUCCESS);
	}
	/*if get key then write the detail to file*/
	do_log(detail,strlen(detail),"/bh_manage/smtpmail/mail_info");
	
	/*free the key ,if failed set the sem's value of 1*/
	if(! semaphore_v(logsem)) set_semvalue(logsem);
	
	exit(EXIT_SUCCESS);

}
