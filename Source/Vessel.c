#include "Vessel_header.h"


int main(int argc,char** argv){
    //argv[1]->Name argv[2]->ReqType argv[3]->ParkPeriod argv[4]->ManTime argv[5]->shmid argv6->PortMasterId
    int shmid,RequestType,ParkedPeriod,ManTime,PortMasterId,SigRetured,PossitionCost, FirstHalf, SecondHalf,CurrentCost;
    int* shmptr;
    char Name[BUFFSIZE];
    Possition MyPoss;
    sigset_t GetingInRespond,WritingMyInfoRespond;
    struct timeval CurrentTime,WaitingTime,ArrivalTime;
    FILE* logging;

    //open the logging file
    logging=fopen("Logging.txt","a");
    if(logging==NULL){
        perror("Logging Opening");
        exit(-1);
    }
    //Set up the Sigsets in order to sigwait later
    sigemptyset(&GetingInRespond);
    sigemptyset(&WritingMyInfoRespond);
    sigaddset(&GetingInRespond,SIGUSR1);
    sigaddset(&WritingMyInfoRespond,SIGUSR2);
    sigprocmask(SIG_BLOCK,&WritingMyInfoRespond,NULL);
    sigprocmask(SIG_BLOCK,&GetingInRespond,NULL);

    //getting the argumets
    strcpy(Name,argv[1]);
    RequestType = atoi(argv[2]);
    ParkedPeriod= atoi(argv[3]);
    ManTime=atoi(argv[4]);
    shmid=atoi(argv[5]);
    PortMasterId=atoi(argv[6]);
    //first and second half determine the time when the vessel will wake up and informs about its current cost
    FirstHalf=ParkedPeriod/2;
    SecondHalf=ParkedPeriod-FirstHalf;

    //attach to the shared memory
    shmptr=shmat(shmid,0,0);
    if(shmptr==(void*)-1){
        perror("shmat");
        exit(-1);
    }
    SharedMem* SharedMemory =(SharedMem*)((char*)shmptr);
    //wait untin the shared memory is unblocked
    sem_wait(&(SharedMemory->Busy_Shm));
    //print stuff in standar output and logging file
    gettimeofday(&CurrentTime,NULL);
    gettimeofday(&ArrivalTime,NULL);//keep my arrival time
    fprintf(logging,"%s %s %d %s %ld%s%ld %s ",Name,"Sending RequestSignal with request type:",RequestType, "to portMaster at:",CurrentTime.tv_sec,",",CurrentTime.tv_usec,"\n");
    printf("%d %s %d %s %ld%s%ld %s ",getpid(),"Sending RequestSignal with request type:",RequestType, "to portMaster at:",CurrentTime.tv_sec,",",CurrentTime.tv_usec,"\n");
    fflush(logging);
    //write my request in the share memory
    SharedMemory->LastRequest.Vessel_id=getpid();
    SharedMemory->LastRequest.RequestType=RequestType;
    //inform port master about my request
    kill(PortMasterId,SIGINT);
    //wait for port masters answer
    sigwait(&GetingInRespond,&SigRetured);

    //wait untill the request is fullfiled
    switch(RequestType){
        case 1:{
            WaitInQueue("Starts waiting in Small Queue at:",Name,&(SharedMemory->Small),&(SharedMemory->Busy_Shm),ManTime,logging,&WaitingTime);
            break;
        }
        case 2:{
            WaitInQueue("Starts waiting in Small-Medium Queue at:",Name,&(SharedMemory->Small_Medium),&(SharedMemory->Busy_Shm),ManTime,logging,&WaitingTime);
            break;

        }
        case 3:{
            WaitInQueue("Starts waiting in Small-Medium-Large Queue at:",Name,&(SharedMemory->Small_Medium_Large),&(SharedMemory->Busy_Shm),ManTime,logging,&WaitingTime);
            break;
        }
        case 4:{
            WaitInQueue("Starts waiting in Medium Queue at:",Name,&(SharedMemory->Medium),&(SharedMemory->Busy_Shm),ManTime,logging,&WaitingTime);
            break;
        }
        case 5:{
            WaitInQueue("Starts waiting in Medium-Large Queue at:",Name,&(SharedMemory->Medium_Large),&(SharedMemory->Busy_Shm),ManTime,logging,&WaitingTime);
            break;
        }
        case 6:{
            WaitInQueue("Starts waiting in Small Large at:",Name,&(SharedMemory->Large),&(SharedMemory->Busy_Shm),ManTime,logging,&WaitingTime);
            break;
        }
    }



    //wait until there is no movement
    sem_wait(&(SharedMemory->Move));
    //wait for the sharedmemory to be available
    sem_wait(&(SharedMemory->Busy_Shm));

    //write stuff
    gettimeofday(&CurrentTime,NULL);
    fprintf(logging,"%s %s %ld%s%ld %s ",Name,"Is My turn to go in at:",CurrentTime.tv_sec,",",CurrentTime.tv_usec,"\n");
    printf("%d %s %ld%s%ld %s ",getpid(),"Is My turn to go in at:",CurrentTime.tv_sec,",",CurrentTime.tv_usec,"\n");
    fflush(logging);

    //writing my information in the shared memory in order for the port master to read them
    SharedMemory->CurrentVessel.Vessel_id=getpid();
    SharedMemory->CurrentVessel.ArrivalTime.tv_sec=ArrivalTime.tv_sec;
    SharedMemory->CurrentVessel.ArrivalTime.tv_usec=ArrivalTime.tv_usec;
    SharedMemory->CurrentVessel.Waitingtime.tv_sec=WaitingTime.tv_sec;
    SharedMemory->CurrentVessel.Waitingtime.tv_usec=WaitingTime.tv_usec;
    SharedMemory->CurrentVessel.ParkingPeriod=ParkedPeriod;
    strcpy(SharedMemory->CurrentVessel.VesselName,Name);
    SharedMemory->CurrentVessel.type=RequestType;
    SharedMemory->CurrentVessel.state=1;

    //send signal to the port master to read my info
    kill(PortMasterId,SIGCHLD);
    //wait until the port master is done
    sigwait(&WritingMyInfoRespond,&SigRetured);

    //read my possition and the cost
    MyPoss.type=SharedMemory->CurrentVessel.MyPosition.type;
    MyPoss.index=SharedMemory->CurrentVessel.MyPosition.index;
    PossitionCost=SharedMemory->CurrentVessel.PossionCost;
    sem_post(&(SharedMemory->Busy_Shm));

    printf("vessel %d starting manuver to park in spot type %d with index %d\n",getpid(),MyPoss.type,MyPoss.index);
    fprintf(logging,"%s %s %s %d %s %d %s","vessel",Name,"starting manuver to  park in spot type:",MyPoss.type,"with index",MyPoss.index,"\n");
    fflush(logging);
    //start manuvering to get in
    sleep(ManTime);

    printf("vessel %d parked\n",getpid());
    fprintf(logging,"%s %s %s","vessel",Name,"parked:\n");
    fflush(logging);

    //let other vessels move
    sem_post(&(SharedMemory->Move));


    gettimeofday(&CurrentTime,NULL);
    fprintf(logging,"%s %s %ld%s%ld %s ",Name,"Is inside and starts sleeping at:",CurrentTime.tv_sec,",",CurrentTime.tv_usec,"\n");
    printf("%d %s %ld%s%ld %s ",getpid(),"Is inside and starts sleeping at:",CurrentTime.tv_sec,",",CurrentTime.tv_usec,"\n");
    fflush(logging);

    //start sleeping and at the half of my sleeptime inform about the cost
    sleep(FirstHalf);
    CurrentCost=FirstHalf*PossitionCost;
    fprintf(logging,"%s %s %d %s",Name,": Current cost is",CurrentCost,"Going Back To Sleep\n");
    sleep(SecondHalf);

    printf("%d woke up\n",getpid());

    //wait until there is no movement
    sem_wait(&(SharedMemory->Move));
    //wait until the port master is done
    sem_wait(&(SharedMemory->Busy_Shm));
    //write my possition in order for the port master to know that im leaving
    SharedMemory->LeavingVessel.index=MyPoss.index;
    SharedMemory->LeavingVessel.type=MyPoss.type;

    gettimeofday(&CurrentTime,NULL);
    fprintf(logging,"%s %s %ld%s%ld %s ",Name,"Started Manuvering in order to leave:",CurrentTime.tv_sec,",",CurrentTime.tv_usec,"\n");
    printf("%d %s %ld%s%ld %s ",getpid(),"Started Manuvering in order to leave:",CurrentTime.tv_sec,",",CurrentTime.tv_usec,"\n");
    fflush(logging);
    //start manuvering in order to leave
    sleep(ManTime);

    gettimeofday(&CurrentTime,NULL);
    fprintf(logging,"%s %s %ld%s%ld %s ",Name,"Left at:",CurrentTime.tv_sec,",",CurrentTime.tv_usec,"\n");
    printf("%d %s %ld%s%ld %s ",getpid(),"Left at:",CurrentTime.tv_sec,",",CurrentTime.tv_usec,"\n");
    fflush(logging);
    //let the portmaster know that i have left
    kill(PortMasterId,SIGTRAP);
    //free the movement
    sem_post(&(SharedMemory->Move));
    //detach from the shared memory and close logging file
    shmdt((void*)shmptr);
    fclose(logging);
    exit(0);
}
