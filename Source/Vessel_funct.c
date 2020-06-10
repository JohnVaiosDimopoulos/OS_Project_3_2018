#include "Vessel_header.h"

void WaitInQueue(char* Messege,char * Name,sem_t* SemToWait,sem_t* SemToPost,int ManTime,FILE* logging,struct timeval* WaitingTime){
    struct timeval StartTime,EndTime;
    //write in logging file
    gettimeofday(&StartTime,NULL);
    fprintf(logging,"%s %s %ld%s%ld %s ",Name,Messege,StartTime.tv_sec,",",StartTime.tv_usec,"\n");
    printf("%d %s %ld%s%ld %s ",getpid(),Messege,StartTime.tv_sec,",",StartTime.tv_usec,"\n");
    fflush(logging);
    //free the shared memory
    sem_post(SemToPost);
    //wait in the proper shemaphore depending on the request
    sem_wait(SemToWait);
    //after i pass calculate the time i was waiting
    gettimeofday(&EndTime,NULL);
    timersub(&EndTime,&StartTime,WaitingTime);
}
