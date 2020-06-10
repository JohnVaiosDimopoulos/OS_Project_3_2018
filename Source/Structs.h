#include <semaphore.h>
#include <time.h>
#define BUFFSIZE 20


struct Vessel_Request{
    int Vessel_id;
    int RequestType;
}typedef Request;

struct QueueNode{
    Request VesselRequest;
    struct QueueNode* next;

}typedef QueueNode;

struct MyQueue{
    QueueNode* front;
    QueueNode* rear;

}typedef Queue;

struct RequestArrayElement{
    Request MyReq;
    int status;

}typedef RequestArrayElement;

struct Vessel_Possition{
    int type;
    int index;
}typedef Possition;

struct Port_Stats{
    int TotalIncome;
    int SmallIncome;
    int MediumIncome;
    int LargeIncome;
    int TotalVesselsParked;
    int TotalSmallParked;
    int TotalMediumParked;
    int TotalLargeParked;
    struct timeval TotalSmallWaitTime;
    struct timeval TotalLargeWaitTime;
    struct timeval TotalMediumWaitTIme;
    struct timeval TotalWaitTime;


}typedef Stats;

struct Vessel_Info {
    int state;
    int Vessel_id;
    char VesselName[BUFFSIZE];
    int type;
    int ParkingPeriod;
    int PossionCost;
    struct timeval ArrivalTime;
    struct timeval Waitingtime;
    Possition MyPosition;
}typedef VesselInfo;


struct Shared_Memory{
    Stats PortStats;
    int SmallCap;
    int MediumCap;
    int LargeCap;
    int CloseMonitor;
    sem_t Move;
    sem_t Busy_Shm;
    sem_t Busy_Monitor;
    sem_t Small;
    sem_t Small_Medium;
    sem_t Small_Medium_Large;
    sem_t Medium;
    sem_t Medium_Large;
    sem_t Large;
    sem_t RequestWaiting;
    sem_t Fullfil;
    sem_t PortClosed;
    sem_t ProccesInitialization;
    Request LastRequest;
    VesselInfo CurrentVessel;
    Possition LeavingVessel;
    VesselInfo* SmallPositions;
    VesselInfo* MediumPositions;
    VesselInfo* LargePositions;
}typedef SharedMem;