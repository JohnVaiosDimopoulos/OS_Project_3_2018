#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "Structs.h"

int* SharedMemSetup(int*,int,int,int);
void InializeSemaphore(sem_t*,int);
SharedMem* InitSharedMem(int *,int,int,int);
void SemDestroy(sem_t*);
void DestorySemaphores(SharedMem*);