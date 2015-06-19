#ifndef SEMOPT_H
#define SEMOPT_H

#ifndef SEM_KEY
#define SEM_KEY 456
#endif

#ifndef semun

union semun {

	int val;
	struct sem_ds *buf;
	unsigned short *array;
};

#endif

int semaphore_p(int sem_id);

int semaphore_v(int sem_id);

int set_semvalue(int sem_id);

void del_semvalue(int sem_id);

int do_log(char *buf, int buflen, char *log_dir);

#endif
