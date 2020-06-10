#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <semaphore.h>
#include <sys/wait.h>
#include "Structs.h"

int* InitializeSharedMemory(int,VesselInfo**,VesselInfo**,VesselInfo**,SharedMem**);
void PritCategoryStats(int,struct timeval,int,FILE*);
void PrintStats(SharedMem*);
void PritCategoryStatus(VesselInfo*,int,FILE*);
void PrintCurrentPortStatus(SharedMem*,VesselInfo*,VesselInfo*,VesselInfo*);