#include "PortMaster_header.h"

void InitializeQueue(Queue* Q){//Set both pointers to null
    Q->front=NULL;
    Q->rear=NULL;
}

int IsEmpty(Queue* Q){
    return (Q->front==NULL);
}//check if the Queue is empty

void Print(Queue*Q){
    if(IsEmpty(Q)){
        printf("Empty");
    }
    QueueNode* temp;
    temp=Q->front;
    while (temp!=NULL){
        printf("ID:%d-REQ:%d->",temp->VesselRequest.Vessel_id,temp->VesselRequest.RequestType);
        temp=temp->next;
    }
    printf("\n");
}//Print the Queue

void InsertRequest(Request Req,Queue* Q){
    QueueNode* temp;
    //creation of the new node
    temp=(QueueNode*)malloc(sizeof(QueueNode));
    temp->VesselRequest.RequestType=Req.RequestType;
    temp->VesselRequest.Vessel_id=Req.Vessel_id;
    temp->next=NULL;
    if(Q->rear==NULL){ //if the Queue is empty
        Q->front=temp;
        Q->rear=temp;
    }
    else{
        Q->rear->next=temp;
        Q->rear=temp;
    }
}

void Remove(int Vessel_id,Queue* Q){
    QueueNode* Current;
    QueueNode* Previus;

    //if we have only one node in the Queue
    if(Q->rear==Q->front){
        Current=Q->front;
        Q->front=NULL;
        Q->rear=NULL;
        free(Current);
        return;
    }
    //if the node to remove is the first one
    if(Q->front->VesselRequest.Vessel_id==Vessel_id){
        Current=Q->front;
        Q->front=Q->front->next;
        free(Current);
        return;
    }
    Current=Q->front->next;
    Previus=Q->front;
    //start searchin and keep track of the previus node
    while (Current!=NULL&&Previus!=NULL){
        if(Current->VesselRequest.Vessel_id==Vessel_id){
            //got the node
            //if it i the last one
            if(Current==Q->rear){
                Q->rear=Previus;
            }
            //if it is in the middle
            QueueNode* temp =Current;
            Previus->next=Current->next;
            free(temp);
            return;
        }
        //continue the search
        Previus=Current;
        Current=Current->next;
    }
    return;
}

void PutVessel(VesselInfo* PossitionArray,int ArraySize,int PossitionType,int* GotIn,int* IncomeType,int* TotalPossType,struct timeval*WaitTimeType){
    for(int i=0;i<ArraySize;i++){
        if(PossitionArray[i].state==1&&PossitionArray[i].Vessel_id==SharedMemory->CurrentVessel.Vessel_id){ //search for reserved spot
            PossitionArray[i].state=2;//mark tha possition as unavailable
            //Write the Vessels info in the shared memory
            //the vessel writes the info in the Current Vessel Part of the Shared Memory
            PossitionArray[i].type=PossitionType;
            PossitionArray[i].Waitingtime.tv_sec=SharedMemory->CurrentVessel.Waitingtime.tv_sec;
            PossitionArray[i].Waitingtime.tv_usec=SharedMemory->CurrentVessel.Waitingtime.tv_usec;
            PossitionArray[i].ArrivalTime.tv_sec=SharedMemory->CurrentVessel.ArrivalTime.tv_sec;
            PossitionArray[i].ArrivalTime.tv_usec=SharedMemory->CurrentVessel.ArrivalTime.tv_usec;
            PossitionArray[i].ParkingPeriod=SharedMemory->CurrentVessel.ParkingPeriod;
            strcpy(PossitionArray[i].VesselName,SharedMemory->CurrentVessel.VesselName);
            PossitionArray[i].MyPosition.type=PossitionType;
            PossitionArray[i].MyPosition.index=i;
            //let the Vessel know its possition an the cost
            SharedMemory->CurrentVessel.MyPosition.type=PossitionType;
            SharedMemory->CurrentVessel.MyPosition.index=i;
            SharedMemory->CurrentVessel.PossionCost=PossitionArray[i].PossionCost;
            //Setting Up the Stats
            timeradd(&(SharedMemory->CurrentVessel.Waitingtime),WaitTimeType,WaitTimeType);
            timeradd(&(SharedMemory->CurrentVessel.Waitingtime),&(SharedMemory->PortStats.TotalWaitTime),&(SharedMemory->PortStats.TotalWaitTime));
            SharedMemory->PortStats.TotalVesselsParked++;
            (*TotalPossType)++;
            (*IncomeType)+=(SharedMemory->CurrentVessel.PossionCost)*(SharedMemory->CurrentVessel.ParkingPeriod);
            SharedMemory->PortStats.TotalIncome+=(SharedMemory->CurrentVessel.PossionCost)*(SharedMemory->CurrentVessel.ParkingPeriod);
            //Signal the vessel to go to sleep
            kill(SharedMemory->CurrentVessel.Vessel_id,SIGUSR2);
            *GotIn=1;
            break;
        }
    }
}

void GetInHandler(int signum) { //signal handler in order to actually put the vessel inside an let the vessel know its possition
    signal(SIGCHLD, GetInHandler);
    int GotIn=0;
    //similar procedure with the request handler but this time we inform the shared memory
    switch (SharedMemory->CurrentVessel.type){
        case 1:{
            PutVessel(SharedMemory->SmallPositions,SharedMemory->SmallCap,1,&GotIn,&(SharedMemory->PortStats.SmallIncome),&(SharedMemory->PortStats.TotalSmallParked),&(SharedMemory->PortStats.TotalSmallWaitTime));
            break;
        }
        case 2:{
            PutVessel(SharedMemory->SmallPositions,SharedMemory->SmallCap,1,&GotIn,&(SharedMemory->PortStats.SmallIncome),&(SharedMemory->PortStats.TotalSmallParked),&(SharedMemory->PortStats.TotalSmallWaitTime));
            if(!GotIn)
                PutVessel(SharedMemory->MediumPositions,SharedMemory->MediumCap,2,&GotIn,&(SharedMemory->PortStats.MediumIncome),&(SharedMemory->PortStats.TotalMediumParked),&(SharedMemory->PortStats.TotalMediumWaitTIme));
            break;
        }
        case 3:{
            PutVessel(SharedMemory->SmallPositions,SharedMemory->SmallCap,1,&GotIn,&(SharedMemory->PortStats.SmallIncome),&(SharedMemory->PortStats.TotalSmallParked),&(SharedMemory->PortStats.TotalSmallWaitTime));
            if(!GotIn)
                PutVessel(SharedMemory->MediumPositions,SharedMemory->MediumCap,2,&GotIn,&(SharedMemory->PortStats.MediumIncome),&(SharedMemory->PortStats.TotalMediumParked),&(SharedMemory->PortStats.TotalMediumWaitTIme));
            if(!GotIn)
                PutVessel(SharedMemory->LargePositions,SharedMemory->LargeCap,3,&GotIn,&(SharedMemory->PortStats.LargeIncome),&(SharedMemory->PortStats.TotalLargeParked),&(SharedMemory->PortStats.TotalLargeWaitTime));
            break;
        }
        case 4:{
            PutVessel(SharedMemory->MediumPositions,SharedMemory->MediumCap,2,&GotIn,&(SharedMemory->PortStats.MediumIncome),&(SharedMemory->PortStats.TotalMediumParked),&(SharedMemory->PortStats.TotalMediumWaitTIme));
            break;
        }
        case 5:{
            PutVessel(SharedMemory->MediumPositions,SharedMemory->MediumCap,2,&GotIn,&(SharedMemory->PortStats.MediumIncome),&(SharedMemory->PortStats.TotalMediumParked),&(SharedMemory->PortStats.TotalMediumWaitTIme));
            if(!GotIn)
                PutVessel(SharedMemory->LargePositions,SharedMemory->LargeCap,3,&GotIn,&(SharedMemory->PortStats.LargeIncome),&(SharedMemory->PortStats.TotalLargeParked),&(SharedMemory->PortStats.TotalLargeWaitTime));
        }
        case 6:{
            PutVessel(SharedMemory->LargePositions,SharedMemory->LargeCap,3,&GotIn,&(SharedMemory->PortStats.LargeIncome),&(SharedMemory->PortStats.TotalLargeParked),&(SharedMemory->PortStats.TotalLargeWaitTime));
            break;
        }
    }
}

void RequestHandler(int signum){//this handler is called when a vessel sends a signal to get in
    signal(SIGINT,RequestHandler);
    Request NewRequest;
    //getting the request
    NewRequest.Vessel_id=SharedMemory->LastRequest.Vessel_id;
    NewRequest.RequestType=SharedMemory->LastRequest.RequestType;
    //insert the request in the queue
    InsertRequest(NewRequest,&RequestQueue);
    //let the vesell know that we received the request
    kill(NewRequest.Vessel_id,SIGUSR1);
    //post the semaphores in order to try and fullfil the request
    sem_post(&(SharedMemory->Fullfil));
    sem_post(&(SharedMemory->RequestWaiting));
}

int CheckIfEmpty(VesselInfo* PossitionArray,int Arraysize){ //checks if all the possitions of a certain type are empty
    for(int i =0;i<Arraysize;i++){
        if(PossitionArray[i].state==1||PossitionArray[i].state==2)
            return 0;
    }
    return 1;
}

int ClosePort(){//if all possitions are empty close the pory

    if(!CheckIfEmpty(SharedMemory->SmallPositions,SharedMemory->SmallCap))
        return 0;
    if(!CheckIfEmpty(SharedMemory->MediumPositions,SharedMemory->MediumCap))
        return 0;
    if(!CheckIfEmpty(SharedMemory->LargePositions,SharedMemory->LargeCap))
        return 0;
    else
        return 1;

}

void WriteInLedger(VesselInfo* PossitionArray,int index,char* PossitionType){
    //opening the public ledger file and writing in it
    FILE* PublicLedger=fopen("PublicLedger","a");
    if(PublicLedger==NULL){
        perror("Public Ledger Opening");
        exit(-1);
    }
    fprintf(PublicLedger,"%s","=====================\n");
    fflush(PublicLedger);

    fprintf(PublicLedger,"%s %s %s","Vessel Name:",PossitionArray[index].VesselName,"\n");
    fflush(PublicLedger);

    fprintf(PublicLedger,"%s %d %s","Veesel Id:",PossitionArray[index].Vessel_id,"\n");
    fflush(PublicLedger);

    fprintf(PublicLedger,"%s %d %s","Request Type:",PossitionArray[index].type,"\n");

    fprintf(PublicLedger,"%s %s %s","Possition Type:",PossitionType,"\n");
    fflush(PublicLedger);

    fprintf(PublicLedger,"%s %d %s","Possition Index:",index,"\n");
    fflush(PublicLedger);

    fprintf(PublicLedger,"%s %d %s","Parking Period:",PossitionArray[index].ParkingPeriod,"\n");
    fflush(PublicLedger);

    fprintf(PublicLedger,"%s %ld%s%ld %s","Arrival Time:",PossitionArray[index].ArrivalTime.tv_sec,",",PossitionArray[index].ArrivalTime.tv_usec,"\n");
    fflush(PublicLedger);

    fprintf(PublicLedger,"%s %ld%s%ld %s","Waiting Time:",PossitionArray[index].Waitingtime.tv_sec,",",PossitionArray[index].Waitingtime.tv_usec,"\n");
    fflush(PublicLedger);


    fprintf(PublicLedger,"%s %d %s","Total Cost:",((PossitionArray[index].ParkingPeriod)*(PossitionArray[index].PossionCost)),"\n");
    fflush(PublicLedger);

    fprintf(PublicLedger,"%s","=====================\n");
    fflush(PublicLedger);

    fclose(PublicLedger);
}

void GetOutHandler(int signum){ //signal handler is calles when a vessel is leaving
    signal(SIGTRAP,GetOutHandler);
    int type = SharedMemory->LeavingVessel.type;
    int index= SharedMemory->LeavingVessel.index;

    switch(type){
        case 1:{ //type1=small array
            SharedMemory->SmallPositions[index].state=0; //mark the spot as empty
            WriteInLedger(SharedMemory->SmallPositions,index,"Small"); //write tha vessels info in the ledger

            break;
        }
        case 2:{//type2 = medium array
            SharedMemory->MediumPositions[index].state=0;
            WriteInLedger(SharedMemory->MediumPositions,index,"Medium");
            break;
        }
        case 3:{//type3=large array
            SharedMemory->LargePositions[index].state=0;
            WriteInLedger(SharedMemory->LargePositions,index,"Large");
            break;
        }
    }
    sem_post(&(SharedMemory->Fullfil)); //check for a vessel to put in the possition
    sem_post(&(SharedMemory->Busy_Shm));//unblock the shared memory (it was block by the vessel process)
    //if this was the last vessel and the request queue is empty then end the proccess
    if(ClosePort()){
        //unblock Myport and let it terminate
        sem_post(&(SharedMemory->PortClosed));
        //detach from the shared Memory
        shmdt((void*)shmptr);
        exit(0);
    }

}

int EmptyInSmall(int* index){
    for(int i=0;i<SharedMemory->SmallCap;i++){
        if(SharedMemory->SmallPositions[i].state==0){
            *index=i;
            return 1;
        }
    }
    return 0;
} //check if a possition in empty in Small array

int EmptyInMedium(int* index){
    for(int i=0;i<SharedMemory->MediumCap;i++){
        if(SharedMemory->MediumPositions[i].state==0) {
            *index=i;
            return 1;
        }
    }
    return 0;
} //check if a possition in empty in Medium array

int EmptyInLarge(int* index){ //check if a possition in empty in Large array
    for(int i =0;i<SharedMemory->LargeCap;i++){
        if(SharedMemory->LargePositions[i].state==0){
            *index=i;
            return 1;
        }
    }
    return 0;

}

void FullfilAny(int (*Fptr)(int*),int* RequestDone,sem_t* Semaphore,QueueNode* temp,VesselInfo* PossitionArray,int Cost){
    int index=-1;
    int RequestId;
    if(Fptr(&index)){ // if there is an empty spot in the arrat
        RequestId=temp->VesselRequest.Vessel_id;
        Remove(RequestId,&RequestQueue); // remove the request from the queue
        PossitionArray[index].Vessel_id=RequestId;
        PossitionArray[index].state=1;//reserve the possition
        PossitionArray[index].PossionCost=Cost;//inform the vessel about the cost
        *RequestDone=1;
        sem_post(Semaphore);//post the semaphore at witch the vessel is waiting for the request to be fullfilled
    }

}

void InitializeSharedMem(int shmid){

    //attach to the shared memory segment
    shmptr =(int*)shmat(shmid,0,0);
    if(shmptr==(void*)-1){
        perror("shmat");
        exit(-1);
    }

    //set up the pointers depending on the size of each array
    SharedMemory = (SharedMem*)((char*)shmptr);
    SharedMemory->SmallPositions=(VesselInfo*)((char*)shmptr+ sizeof(SharedMem));
    SharedMemory->MediumPositions=(VesselInfo*)(((char*)SharedMemory->SmallPositions)+(SharedMemory->SmallCap* sizeof(VesselInfo)));
    SharedMemory->LargePositions=(VesselInfo*)(((char*)SharedMemory->MediumPositions)+(SharedMemory->MediumCap* sizeof(VesselInfo)));

    //Set all the possitons to be empty
    for(int i =0;i<SharedMemory->SmallCap;i++)
        SharedMemory->SmallPositions[i].state=0;
    for(int j=0;j<SharedMemory->MediumCap;j++)
        SharedMemory->MediumPositions[j].state = 0;
    for(int k=0;k<SharedMemory->LargeCap;k++)
        SharedMemory->LargePositions[k].state=0;

    //Initialize all the stats variables
    SharedMemory->PortStats.TotalIncome=0;
    SharedMemory->PortStats.SmallIncome=0;
    SharedMemory->PortStats.MediumIncome=0;
    SharedMemory->PortStats.LargeIncome=0;
    SharedMemory->PortStats.TotalVesselsParked=0;
    SharedMemory->PortStats.TotalSmallParked=0;
    SharedMemory->PortStats.TotalMediumParked=0;
    SharedMemory->PortStats.TotalLargeParked=0;

}

void FullfilRequest(int SmallCharge,int MediumCharge,int LargeCharge) {
    //Fullfils one request per call
    QueueNode *temp;
    int RequestDone = 0;
    temp = RequestQueue.front;
    while (temp != NULL) {
        switch (temp->VesselRequest.RequestType) {
            case 1: {//request type1 ==only Small
                FullfilAny(EmptyInSmall, &RequestDone, &(SharedMemory->Small), temp, SharedMemory->SmallPositions,SmallCharge);
                break;
            }
            case 2: {//request type2 ==only Small or Medium
                FullfilAny(EmptyInSmall, &RequestDone, &(SharedMemory->Small_Medium), temp,SharedMemory->SmallPositions,SmallCharge);
                if(!RequestDone)//if the request wasnt Fullfilled try to put it in medium
                    FullfilAny(EmptyInMedium, &RequestDone, &(SharedMemory->Small_Medium), temp,SharedMemory->MediumPositions,MediumCharge);
                break;
            }
            case 3: {//request type3 ==only Small or Medium or Large
                FullfilAny(EmptyInSmall, &RequestDone, &(SharedMemory->Small_Medium_Large), temp,SharedMemory->SmallPositions,SmallCharge);
                if(!RequestDone)//if the request wasnt Fullfilled try to put it in medium
                    FullfilAny(EmptyInMedium, &RequestDone, &(SharedMemory->Small_Medium_Large), temp,SharedMemory->MediumPositions,MediumCharge);
                if(!RequestDone)//if the request wasnt Fullfilled try to put it in Large
                    FullfilAny(EmptyInLarge, &RequestDone, &(SharedMemory->Small_Medium_Large), temp,SharedMemory->LargePositions,LargeCharge);
                break;
            }
            case 4: {//request type4 ==only Medium
                FullfilAny(EmptyInMedium, &RequestDone, &(SharedMemory->Medium), temp,SharedMemory->MediumPositions,MediumCharge);
                break;
            }

            case 5: {//request type5 ==only Medium or Large
                FullfilAny(EmptyInMedium, &RequestDone, &(SharedMemory->Medium_Large), temp,SharedMemory->MediumPositions,MediumCharge);
                if(!RequestDone)//if the request wasnt Fullfilled try to put it in Large
                    FullfilAny(EmptyInLarge, &RequestDone, &(SharedMemory->Medium_Large), temp,SharedMemory->LargePositions,LargeCharge);
                break;
            }
            case 6: {//request type1 ==only Large
                FullfilAny(EmptyInLarge,&(RequestDone),&(SharedMemory->Large),temp,SharedMemory->LargePositions,LargeCharge);
                break;
            }
        }
        if (!RequestDone)//try the next request if this wasnt fullfiled
            temp = temp->next;
        else
            break;
    }
    if(!RequestDone)
        // reset the sem if noo request was fullfiled
        sem_post(&(SharedMemory->RequestWaiting));
}
