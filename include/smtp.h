#include <sys/sem.h>
void do_smtp(int client_sockfd);

int max(int a, int b);

void sig_chld(int signo);

int get_whitemail(char *ptr[], int *n);

int check_mail(char *mail, char *maillist[], int n);

int parase_mail(char *buf, char *mail);

int do_client(char *buf, int buflen, int serverfd, struct ctrl_flag *ctrlflag);

int do_server(int clientfd, int serverfd, struct ctrl_flag *ctrlflag);

int get_wanip(char *wanip);

int do_iptables_dnat(char *ip, int port);

void get_mail_detail(char *buf, char *detail);

void get_detail_buf(struct smtp_buf *smtpbuf, char *buf, int buflen);

int  get_mail_body(char *buf, int bufsize, unsigned char *body);
