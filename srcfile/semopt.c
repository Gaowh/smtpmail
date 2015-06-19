#include "../include/myheader.h"
#include "../include/semopt.h"
#include<sys/sem.h>


int do_log(char *buf, int buflen, char *log_dir){

	printf("do log....\n");
	FILE *fp;
	int nres;
	
	fp = fopen(log_dir, "a+");
	
	if(fp == NULL){
		fprintf(stderr,"open logfile error:%s\n",strerror(errno));
		return -1;
	}

	int fd = fileno(fp);

	nres = write(fd,"******************************************************\n",55);
	nres = write(fd,buf,buflen);
	
	if(nres != buflen){
	
		fprintf(stderr,"write error:%s\n",strerror(errno));
		return -1;
	}
	nres = write(fd,"******************************************************\n\n",56);
	
	fclose(fp);
	printf("do log end...\n");
	return 1;
}


int semaphore_p(int sem_id){
	
	struct sembuf sem_b;

	sem_b.sem_num = 0;
	sem_b.sem_op = -1;
	sem_b.sem_flg = SEM_UNDO;

	if(semop(sem_id, &sem_b, 1) == -1) {
		
		fprintf(stderr,"set semop error: %s\n", strerror(errno));
		return 0;
	}
	
	return 1;

}

int semaphore_v(int sem_id){

	struct sembuf sem_b;

	sem_b.sem_num = 0;
	sem_b.sem_op = 1;
	sem_b.sem_flg = SEM_UNDO;

	if(semop(sem_id, &sem_b, 1) == -1){
		
		fprintf(stderr,"set semop  error: %s\n",strerror(errno));
		return 0;
	}

	return 1;
}


int set_semvalue(int sem_id){

	union semun sem_un;

	sem_un.val = 1;
	
	if(semctl(sem_id, 0, SETVAL, sem_un) == -1){
		
		fprintf(stderr,"set semctl error: %s\n", strerror(errno));
		return 0;	
	}
	return 1;
}


void del_semvalue(int sem_id){

	union semun sem_un;

	if(semctl(sem_id, 0 , IPC_RMID, sem_un) == -1){
		
		fprintf(stderr,"failed to delete semaphore\n");

	}
}
