#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "Structs.h"

void WaitInQueue(char*,char *,sem_t*,sem_t*,int,FILE*,struct timeval*);
