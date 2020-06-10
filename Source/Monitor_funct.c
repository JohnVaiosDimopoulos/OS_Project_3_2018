#include "Monitor_header.h"


int* InitializeSharedMemory(int shmid,VesselInfo** Small,VesselInfo** Medium,VesselInfo** Large,SharedMem** SharedMemptr){
    //attach to the shared memory segment
    int *shmptr=(int*)shmat(shmid,0,0);
    if(shmptr==(void*)-1){
        perror("shmat");
        exit(-1);
    }

    SharedMem* SharedMemory=(SharedMem*)((char*)shmptr);
    //we use local pointers for the arrays because the attached address is random and we cant rely on the offsets that port master setted
    *Small=(VesselInfo*)((char*)shmptr+ sizeof(SharedMem));
    *Medium=(VesselInfo*)(((char*)*Small)+(SharedMemory->SmallCap)* sizeof(VesselInfo));
    *Large=(VesselInfo*)(((char*)*Medium)+(SharedMemory->MediumCap)* sizeof(VesselInfo));
    *SharedMemptr=SharedMemory;
    return shmptr;
}

void PritCategoryStats(int Income,struct timeval WaitTime,int TotalVessels,FILE* PrintFile){

    fprintf(PrintFile,"%s %d %s","=>Total Income:",Income,"\n");
    fflush(PrintFile);

    double AvgIncome=(double)Income/TotalVessels;
    fprintf(PrintFile,"%s %f %s","=> Average Income:",AvgIncome,"\n");
    fflush(PrintFile);

    fprintf(PrintFile,"%s %ld%s%ld %s","=>Total Wait Time:",WaitTime.tv_sec,",",WaitTime.tv_usec,"\n");
    fflush(PrintFile);

    double AvgWaitTime =(WaitTime.tv_sec+(WaitTime.tv_usec/1000.0))/TotalVessels;
    fprintf(PrintFile,"%s %f %s","=>Average Wait Time:",AvgWaitTime,"\n");
    fflush(PrintFile);

}

void PrintStats(SharedMem* SharedMemory){
    FILE* PrintFile=fopen("MonitorStatsReport","a");
    if (PrintFile==NULL){
        perror("MonitorStatsReport Opening");
        exit(-1);
    }

    struct timeval CurrTime;
    gettimeofday(&CurrTime,NULL);
    fprintf(PrintFile,"%s %ld%s%ld %s","====At:",CurrTime.tv_sec,",",CurrTime.tv_usec,"====\n");
    fflush(PrintFile);

    fprintf(PrintFile,"%s","=====Port Stats=====\n");
    fflush(PrintFile);

    fprintf(PrintFile,"%s","===Small===\n");
    fflush(PrintFile);
    PritCategoryStats(SharedMemory->PortStats.SmallIncome,SharedMemory->PortStats.TotalSmallWaitTime,SharedMemory->PortStats.TotalSmallParked,PrintFile);


    fprintf(PrintFile,"%s","===Medium===\n");
    fflush(PrintFile);
    PritCategoryStats(SharedMemory->PortStats.MediumIncome,SharedMemory->PortStats.TotalMediumWaitTIme,SharedMemory->PortStats.TotalMediumParked,PrintFile);

    fprintf(PrintFile,"%s","===Large===\n");
    fflush(PrintFile);
    PritCategoryStats(SharedMemory->PortStats.LargeIncome,SharedMemory->PortStats.TotalLargeWaitTime,SharedMemory->PortStats.TotalLargeParked,PrintFile);

    fprintf(PrintFile,"%s","===Total===\n");
    fflush(PrintFile);
    PritCategoryStats(SharedMemory->PortStats.TotalIncome,SharedMemory->PortStats.TotalWaitTime,SharedMemory->PortStats.TotalVesselsParked,PrintFile);


    fprintf(PrintFile,"%s","===================\n");
    fflush(PrintFile);

    fclose(PrintFile);

}

void PritCategoryStatus(VesselInfo* PossitionsArray,int ArratSize,FILE* PrintFile){
    for(int i=0;i<ArratSize;i++){

        fprintf(PrintFile,"%s %d %s","==index",i,"==\n");
        if(PossitionsArray[i].state==0){
            fprintf(PrintFile,"%s","=>Possition Is Empty\n");
            fflush(PrintFile);
        }
        else if(PossitionsArray[i].state==1){
            fprintf(PrintFile,"%s %d %s","Position Reserved for Vessel with id:",PossitionsArray[i].Vessel_id,"\n");
            fflush(PrintFile);
        }
        else{

            fprintf(PrintFile,"%s %s %s","=>Vessel Name:",PossitionsArray[i].VesselName,"\n");
            fflush(PrintFile);

            fprintf(PrintFile,"%s %d %s","=>Request Type:",PossitionsArray[i].type,"\n");
            fflush(PrintFile);

            fprintf(PrintFile,"%s %d %s","=>Parking Period:",PossitionsArray[i].ParkingPeriod,"\n");
            fflush(PrintFile);

            fprintf(PrintFile,"%s %ld%s%ld %s","=>Arrival Time:",PossitionsArray[i].ArrivalTime.tv_sec,",",PossitionsArray[i].ArrivalTime.tv_usec,"\n");
            fflush(PrintFile);

            fprintf(PrintFile,"%s %ld%s%ld %s","=>Waiting Time:",PossitionsArray[i].Waitingtime.tv_sec,",",PossitionsArray[i].Waitingtime.tv_usec,"\n");
            fflush(PrintFile);

            fprintf(PrintFile,"%s %d %s","=>Position Cost:",PossitionsArray[i].PossionCost,"\n");
            fflush(PrintFile);
        }
    }
}

void PrintCurrentPortStatus(SharedMem* SharedMemory,VesselInfo* SmallPossitions,VesselInfo* MediumPossitions,VesselInfo* LargePossitions){

    FILE* PrintFile=fopen("MonitorStatusReport","a");
    if(PrintFile==NULL){
        perror("MonitorStatusReport Opening");
        exit(-1);
    }
    struct timeval CurrTime;
    gettimeofday(&CurrTime,NULL);
    fprintf(PrintFile,"%s %ld%s%ld %s","====At:",CurrTime.tv_sec,",",CurrTime.tv_usec,"====\n");
    fflush(PrintFile);

    fprintf(PrintFile,"%s","======Port_Status======\n");
    fflush(PrintFile);

    fprintf(PrintFile,"%s","===Small===\n");
    fflush(PrintFile);
    PritCategoryStatus(SmallPossitions,SharedMemory->SmallCap,PrintFile);

    fprintf(PrintFile,"%s","===Medium===\n");
    fflush(PrintFile);
    PritCategoryStatus(MediumPossitions,SharedMemory->MediumCap,PrintFile);

    fprintf(PrintFile,"%s","===Large===\n");
    fflush(PrintFile);
    PritCategoryStatus(LargePossitions,SharedMemory->MediumCap,PrintFile);

    fprintf(PrintFile,"%s","-------------\n");
    fflush(PrintFile);

    fclose(PrintFile);
}
