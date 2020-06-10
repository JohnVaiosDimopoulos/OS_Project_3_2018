#include "PortMaster_header.h"

int main(int argc,char** argv){
    //argv[1]->SmallCost argv[2]->MediumCost argv[3]->LargeCost argv[4]->shared memory id
    //Initialization Phase///
    //setting up signal handlers
    signal(SIGCHLD,GetInHandler);
    signal(SIGINT,RequestHandler);
    signal(SIGTRAP,GetOutHandler);

    int SmallCharge,MediumCharge,LargeCharge,MyPortPid,shmid;
    //getting the arguments
    SmallCharge = atoi(argv[1]);
    MediumCharge = atoi(argv[2]);
    LargeCharge = atoi(argv[3]);
    shmid=atoi(argv[4]);
    //Initialize the Shared Memory Segment and the Request Queue
    InitializeSharedMem(shmid);
    InitializeQueue(&RequestQueue);
    //Let Mypost Know that initialization is done
    sem_post(&(SharedMemory->ProccesInitialization));
    while(1){
        //if no request is on the queue block here
        sem_wait(&(SharedMemory->RequestWaiting));
        //if no request can be fullfiled block here(each time a vessel leaves or a new request comes this semaphore is posted)
        sem_wait(&(SharedMemory->Fullfil));
        //block until you can access the shared Memory
        sem_wait(&(SharedMemory->Busy_Shm));
        //For Checking purposes print the queue before and after
        printf("CALLING FULLFIL REQUEST\n");
        printf("BEFORE:");
        Print(&RequestQueue);
        FullfilRequest(SmallCharge,MediumCharge,LargeCharge);
        printf("AFTER:");
        Print(&RequestQueue);
        //free the shared memory segment
        sem_post(&(SharedMemory->Busy_Shm));
    }
}

