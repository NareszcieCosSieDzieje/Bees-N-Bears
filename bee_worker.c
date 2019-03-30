
#include "parameters.h"
#include "ansi_structs.h"


struct worker_bee{
    int honeySharedMemoryID;
} humble_bee;

struct semBufs{
    struct sembuf upSemBuf;
    struct sembuf downSemBuf;
} sembufs;

struct semaphoreSet{
    int bearSemId;
    int sectionSemId;
    int memorySemId;
} semaphores;

int busy=FALSE;
int stop_flag=FALSE;


struct msgbuf //FIXME: USUN !
{
    long mtype;
    int zjadacz;
    int producent;
} msg_to_show;

struct messageQIDs{  //FIXME: USUN
    int toTerminal;
} messages;

void handle_sigterm(int sig);

void produceHoney();

void consumeHoney();

int main(int argc, char* argv[]){
    //signal(SIGTERM, handle_sigterm);
    messages.toTerminal = msgget( DEBUG_QUEUE_TERMINAL_KEY, IPC_CREAT); //FIXME: USUN!!!!
    sembufs.upSemBuf= (struct sembuf){0,1,0};   
    sembufs.downSemBuf= (struct sembuf){0,-1,0};
    semaphores.bearSemId =  semget(SEMAPHORE_BEAR_KEY, 1, IPC_CREAT); 
    semaphores.sectionSemId = semget(SEMAPHORE_SECTION_KEY, 1, IPC_CREAT);
    semaphores.memorySemId= semget(SEMAPHORE_MEMORY_KEY, 1, IPC_CREAT);
    humble_bee.honeySharedMemoryID = shmget(HONEY_SHARED_MEMORY_KEY, HONEY_SHARED_MEMORY_SIZE, IPC_CREAT);
    pid_t beeFork = fork();
    if (humble_bee.honeySharedMemoryID<0){
        perror("worker - shmget bear error");
        exit(1);
    }
    if(beeFork == -1){
        perror("worker - beeFork error");
        exit(2);
    }
    if(beeFork==0){
        msg_to_show.mtype=1; //FIXME: USUN
        msg_to_show.producent=0;
        msg_to_show.zjadacz=1;
        signal(SIGTERM, handle_sigterm); //FIXME: ekspperyment
        while(TRUE){
            consumeHoney();
            sleep(WORKER_BEE_CONSUMPTION_TIME);
        }
    }
    else if (beeFork>1){
        msg_to_show.mtype=1; //FIXME: USUN
        msg_to_show.producent=1;
        msg_to_show.zjadacz=0;
        signal(SIGTERM, handle_sigterm); //FIXME: eksperyment
        while(TRUE){
            produceHoney(); //TODO: is this okay
            sleep(WORKER_BEE_PRODUCTION_TIME);
        }
    }
    return 0;
}      

          
void handle_sigterm(int sig){ //TODO: Fix in bear too
    stop_flag=TRUE;
    while( busy == TRUE){
        sleep(1);
    }
    msgsnd(messages.toTerminal,&msg_to_show,sizeof(msg_to_show),0);
    exit(sig);
}
    
void consumeHoney(){
    if(stop_flag==FALSE){
    busy=TRUE;
    semop(semaphores.bearSemId,&sembufs.downSemBuf, 1); //downSem
    semop(semaphores.bearSemId, &sembufs.upSemBuf, 1); //upSem
    semop(semaphores.sectionSemId ,&sembufs.downSemBuf, 1); //dowm
    semop(semaphores.memorySemId ,&sembufs.downSemBuf, 1); //down
    int *honeyAddr;
    honeyAddr = shmat(humble_bee.honeySharedMemoryID, NULL, 0);
    if ( *honeyAddr <= 0 ){
        //koniec gry
        shmdt(honeyAddr);
        semop(semaphores.memorySemId ,&sembufs.upSemBuf, 1); // up
        semop(semaphores.sectionSemId ,&sembufs.upSemBuf, 1); // up
        busy=FALSE;
        return;
    }
    *honeyAddr -= WORKER_BEE_CONSUMED_HONEY;
    shmdt(honeyAddr);
    semop(semaphores.memorySemId ,&sembufs.upSemBuf, 1); // up
    semop(semaphores.sectionSemId ,&sembufs.upSemBuf, 1); // up
    busy=FALSE;
    }
}

void produceHoney(){
    if(stop_flag==FALSE){
    busy=TRUE;
    semop(semaphores.bearSemId,&sembufs.downSemBuf, 1); 
    semop(semaphores.bearSemId, &sembufs.upSemBuf, 1); 
    semop(semaphores.sectionSemId ,&sembufs.downSemBuf, 1);
    semop(semaphores.memorySemId ,&sembufs.downSemBuf, 1); 
    int *honeyAddr;
    honeyAddr = shmat(humble_bee.honeySharedMemoryID, NULL, 0);
    if ( *honeyAddr <= 0 ){
        shmdt(honeyAddr); 
        semop(semaphores.memorySemId ,&sembufs.upSemBuf, 1); 
        semop(semaphores.sectionSemId ,&sembufs.upSemBuf, 1); 
        busy=FALSE;
        return;
    }
    *honeyAddr += FIGHTER_BEE_CONSUMED_HONEY;
    shmdt(honeyAddr); 
    semop(semaphores.memorySemId ,&sembufs.upSemBuf, 1); 
    semop(semaphores.sectionSemId ,&sembufs.upSemBuf, 1); 
    busy=FALSE;
    }
}

