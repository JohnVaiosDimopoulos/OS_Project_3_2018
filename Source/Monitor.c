#include "Monitor_header.h"



int main(int argc,char** argv){
    char* ConfigFile;
    int Shmid,Time,StatTime,StatsPid,Status;
    FILE* ConfigFilePtr;
    //getting tha arguments
    ConfigFile=malloc(sizeof(char)*strlen(argv[1])+1);
    strcpy(ConfigFile,argv[1]);
    Shmid=atoi(argv[2]);
    //opening configuration file
    ConfigFilePtr=fopen(ConfigFile,"r");
    free(ConfigFile);

    if(ConfigFilePtr==NULL){
        perror("Opening Monitor Config File");
        exit(-1);
    }
    //get the times from configuration file
    fscanf(ConfigFilePtr,"%d",&Time);
    fscanf(ConfigFilePtr,"%d",&StatTime);
    fclose(ConfigFilePtr);
    //
    VesselInfo* SmallPossitions;
    VesselInfo* MediumPossitions;
    VesselInfo* LargePossitions;
    SharedMem* SharedMemory;
    //initialize shared memory
    int* shmptr=InitializeSharedMemory(Shmid,&SmallPossitions,&MediumPossitions,&LargePossitions,&SharedMemory);
    //let myport know that im done with the initializing phase
    sem_post(&(SharedMemory->ProccesInitialization));

    //fork in two processes one takes care of the Status and one of the Statistics
    if((StatsPid=fork())==-1){
        perror("forn in Monitor");
        exit(-1);
    }

    if(StatsPid==0){
        //the stats process
        while(SharedMemory->CloseMonitor){
            sleep(StatTime);
            //wait untill the shared memory is available
            sem_wait(&(SharedMemory->Busy_Shm));
            PrintStats(SharedMemory);
            sem_post(&(SharedMemory->Busy_Shm));
        }
        shmdt((void*)shmptr);
        exit(0);
    }
    while (SharedMemory->CloseMonitor){
        sleep(Time);
        sem_wait(&(SharedMemory->Busy_Shm));
        PrintCurrentPortStatus(SharedMemory,SmallPossitions,MediumPossitions,LargePossitions);
        sem_post(&(SharedMemory->Busy_Shm));
    }
    //when im done wait for the child to be done too before letting myport terminate
    wait(&Status);
    sem_post(&(SharedMemory->Busy_Monitor));
    shmdt((void*)shmptr);

}
