#include "MyPort_header.h"

int* SharedMemSetup(int* shmid,int SmallCapacity,int MediumCapacity,int LargeCapacity){
    //Setting up the shared memory segment and attaching to it
    int* shmprt;
    //creating the segment
    *shmid = shmget(IPC_PRIVATE,sizeof(SharedMem)+(SmallCapacity+MediumCapacity+LargeCapacity)* sizeof(VesselInfo),IPC_CREAT|0666);
    if(*shmid<0){
        perror("Shmget");
        exit(-1);
    }
    //attaching at a random (page aligned) aaddress
    shmprt=(int*)shmat(*shmid,0,0);
    if(shmprt==(void*)-1){
        perror("Shmatt");
        exit(-1);
    }
    return shmprt;
}

void InializeSemaphore(sem_t* Semaphore,int value){
    if(sem_init(Semaphore,1,value)!=0){
        perror("Semaphore Initialization");
        exit(-1);
    }
}

SharedMem* InitSharedMem(int *shmptr, int SmallCapaity, int MediumCapacity, int LargeCapacity){
    //Initializing all the semaphores
    SharedMem* SharedMemory = (SharedMem*)((char*)shmptr);

    InializeSemaphore(&(SharedMemory->Busy_Shm),1);
    InializeSemaphore(&(SharedMemory->Move),1);
    InializeSemaphore(&(SharedMemory->Small),0);
    InializeSemaphore(&(SharedMemory->Small_Medium),0);
    InializeSemaphore(&(SharedMemory->Small_Medium_Large),0);
    InializeSemaphore(&(SharedMemory->Medium),0);
    InializeSemaphore(&(SharedMemory->Medium_Large),0);
    InializeSemaphore(&(SharedMemory->Large),0);
    InializeSemaphore(&(SharedMemory->RequestWaiting),0);
    InializeSemaphore(&(SharedMemory->Fullfil),0);
    InializeSemaphore(&(SharedMemory->PortClosed),0);
    InializeSemaphore(&(SharedMemory->Busy_Monitor),0);
    InializeSemaphore(&(SharedMemory->ProccesInitialization),0);

    //writing the Cappacity of each possition type in the shared memory
    SharedMemory->SmallCap=SmallCapaity;
    SharedMemory->MediumCap=MediumCapacity;
    SharedMemory->LargeCap=LargeCapacity;
    SharedMemory->CloseMonitor=1;

    return SharedMemory;

}

void SemDestroy(sem_t* Semaphore){
    if(sem_destroy(Semaphore)==-1){
        perror("Semaphore Destruction");
        exit(-1);
    }

}

void DestorySemaphores(SharedMem* SharedMemory){
    SemDestroy(&(SharedMemory->Busy_Shm));
    SemDestroy(&(SharedMemory->Busy_Monitor));
    SemDestroy(&(SharedMemory->Move));
    SemDestroy(&(SharedMemory->Small));
    SemDestroy(&(SharedMemory->Small_Medium));
    SemDestroy(&(SharedMemory->Small_Medium_Large));
    SemDestroy(&(SharedMemory->Medium));
    SemDestroy(&(SharedMemory->Medium_Large));
    SemDestroy(&(SharedMemory->Large));
    SemDestroy(&(SharedMemory->RequestWaiting));
    SemDestroy(&(SharedMemory->Fullfil));
    SemDestroy(&(SharedMemory->PortClosed));
    SemDestroy(&(SharedMemory->ProccesInitialization));


}
