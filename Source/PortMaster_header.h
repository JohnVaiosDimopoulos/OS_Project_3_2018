#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
#include "Structs.h"

//The following pointers are global because we need access to them from the signal handlers
SharedMem* SharedMemory;
Queue RequestQueue;
int* shmptr;

void InitializeQueue(Queue*);
int IsEmpty(Queue*);
void Print(Queue*);
void InsertRequest(Request,Queue*);
void Remove(int,Queue*);
void PutVessel(VesselInfo*,int,int,int*,int*,int*,struct timeval*);
void GetInHandler(int);
void RequestHandler(int);
int CheckIfEmpty(VesselInfo*,int);
int ClosePort();
void WriteInLedger(VesselInfo*,int,char*);
void GetOutHandler(int);
int EmptyInSmall(int*);
int EmptyInMedium(int*);
int EmptyInLarge(int*);
void FullfilAny(int (*Fptr)(int*),int*,sem_t*,QueueNode*,VesselInfo*,int);
void InitializeSharedMem(int);
void FullfilRequest(int,int,int);