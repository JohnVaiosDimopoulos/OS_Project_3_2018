#include "MyPort_header.h"

int main(int argc ,char** argv){

    char* ConfigFileName,*WorkLoadName,*MonitorConfigFileName;
    int BlockConfig=0,BlockWorkLoad=0,BlockMonitor=0;
    struct timeval CurrTime;
    //Getting the arguments
    if((argc==7||argc==5)){
        for(int i = 1;i<(argc-1);i+=2){
            if(strcmp(argv[i],"-l")==0&&BlockConfig==0){
                ConfigFileName = malloc(sizeof(char)*strlen(argv[i+1]+1));
                strcpy(ConfigFileName,argv[i+1]);
                BlockConfig=1;
            }
            else if(strcmp(argv[i],"-w")==0&&BlockWorkLoad==0){
                WorkLoadName = malloc(sizeof(char)*strlen(argv[i+1])+1);
                strcpy(WorkLoadName,argv[i+1]);
                BlockWorkLoad=1;
            }
            else if(strcmp(argv[i],"-m")==0&&BlockMonitor==0){
                MonitorConfigFileName=malloc(sizeof(char)*strlen(argv[i+1])+1);
                strcpy(MonitorConfigFileName,argv[i+1]);
                BlockMonitor=1;

            }
        }
    }
    else{
        printf("Wrong number of arguments!Correct format:-l configfile -w WorkLoadFile -m MonitorConfig \n");
        exit(-1);
    }

    int SmallCapacity,MediumCapacity,LargeCapacity,SmallCost,MediumCost,LargeCost,shmid,PortMasterID,MonitorID,pid,SigRetVal;
    int *shmprt;
    sigset_t sig1,sig2;
    FILE* ConfigFilePtr,*WorkloadFilePtr,*logging;

    //Config File has the configuration of the port.The capacity of each possition and the cost per second
    ConfigFilePtr = fopen(ConfigFileName,"r");
    if(ConfigFilePtr==NULL){
        perror("ConfigFile opening");
        exit(0);
    }
    //In the logging file the Proccess Write their actions so we can keep track of the Port
    //They Also write stuff in the standar output
    logging = fopen("Logging.txt","a");
    if(logging==NULL){
        perror("Logging openig");
        exit(0);
    }

    gettimeofday(&CurrTime,NULL);
    fprintf(logging,"%s %ld%s%ld %s","Myport started at:",CurrTime.tv_sec,",",CurrTime.tv_usec,"\n");
    fflush(logging);

    //getting the configuration options
    fscanf(ConfigFilePtr,"%d",&SmallCapacity);
    fscanf(ConfigFilePtr,"%d",&MediumCapacity);
    fscanf(ConfigFilePtr,"%d",&LargeCapacity);
    fscanf(ConfigFilePtr,"%d",&SmallCost);
    fscanf(ConfigFilePtr,"%d",&MediumCost);
    fscanf(ConfigFilePtr,"%d",&LargeCost);
    fclose(ConfigFilePtr);


    shmprt = SharedMemSetup(&shmid,SmallCapacity,MediumCapacity,LargeCapacity);
    SharedMem* SharedMemory=InitSharedMem(shmprt, SmallCapacity, MediumCapacity, LargeCapacity);

    //converting the id of the shared memory into a string to give it as an argument to the other possesses
    char SharedMemId_buf[BUFFSIZE];
    sprintf(SharedMemId_buf,"%d",shmid);

    //forking and calling the port master
    if((PortMasterID=fork())==-1){
        perror("fork");
        exit(-1);
    }
    if(PortMasterID==0){
        //converting the costs to pass them to the port master
        char SmallCost_buf[BUFFSIZE],MediumCost_buf[BUFFSIZE],LargeCost_buf[BUFFSIZE];
        sprintf(SmallCost_buf,"%d",SmallCost);
        sprintf(MediumCost_buf,"%d",MediumCost);
        sprintf(LargeCost_buf,"%d",LargeCost);

        gettimeofday(&CurrTime,NULL);
        fprintf(logging,"%s %ld%s%ld %s","Launching Port Master at:",CurrTime.tv_sec,",",CurrTime.tv_usec,"\n");
        fflush(logging);

        if((execl("./PortMaster","PortMaster",SmallCost_buf,MediumCost_buf,LargeCost_buf,SharedMemId_buf,NULL)==-1)){
            perror("execl");
            exit(-1);
        }
    }
    //block in the Semaphore as long as the port master is initialising
    sem_wait(&(SharedMemory->ProccesInitialization));

    //forking and calling the monitor process
    if((MonitorID=fork())==-1){
        perror("fork");
        exit(-1);
    }
    if(MonitorID==0){
        if((execl("./Monitor","Monitor",MonitorConfigFileName,SharedMemId_buf,NULL))==-1){
         perror("execl");
         exit(-1);
        }
    }
    //block until the monitor finsh its initialization
    sem_wait(&(SharedMemory->ProccesInitialization));

    //We can use the a Workload file to call the Vessels or manualy via terminal
    if(BlockWorkLoad) {
        //opening the workload file
        WorkloadFilePtr = fopen(WorkLoadName, "r");
        if (WorkloadFilePtr == NULL) {
            perror("WorkloadFile opening");
            exit(-1);
        }

        int NumberOfVessels;
        char VesselName[BUFFSIZE];
        char RequestType[BUFFSIZE];
        char ParkPeriod[BUFFSIZE];
        char Mantime[BUFFSIZE];
        char PortMasterId_buf[BUFFSIZE];
        sprintf(PortMasterId_buf, "%d", PortMasterID);

        //the first line in the file is the number of vessels to call
        fscanf(WorkloadFilePtr, "%d", &NumberOfVessels);
        //loop and start calling the vessels
        for (int num = 0; num < NumberOfVessels; num++) {
            fscanf(WorkloadFilePtr, "%s %s %s %s ", VesselName, RequestType, ParkPeriod, Mantime);

            if ((pid = fork()) == -1) {
                perror("fork");
                exit(-1);
            }
            if (pid == 0) {
                if((execl("./Vessel", "Vessel", VesselName, RequestType, ParkPeriod, Mantime, SharedMemId_buf,
                      PortMasterId_buf, NULL)==-1)){
                    perror("execl");
                    exit(-1);
                }
            }
            //wait one second and then give the next vessel
            sleep(1);
        }
        fclose(WorkloadFilePtr);
    }
    //block here until the port master closes the port
    sem_wait(&(SharedMemory->PortClosed));
    printf("PORT CLOSED\n");
    //stop the monitor proccess
    SharedMemory->CloseMonitor=0;
    //wait for monitor end
    sem_wait(&(SharedMemory->Busy_Monitor));
    free(WorkLoadName);
    free(ConfigFileName);
    free(MonitorConfigFileName);
    fclose(logging);
    //Destroy all the semaphores
    DestorySemaphores(SharedMemory);
    //Delete the Shared Memory
    if((shmctl(shmid,IPC_RMID,NULL))==-1){
        perror("Shmctl");
        exit(-1);
    }
    exit(0);



}
