
#include "parameters.h"
#include "ansi_structs.h"

struct fighter_bee{
    int honeySharedMemoryID;
} strong_bee;


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


void consumeHoney();

void handle_sigterm(int sig); 


int main(int argc, char* argv[]){
    signal(SIGTERM, handle_sigterm);
    sembufs.upSemBuf= (struct sembuf){0,1,0};
    sembufs.downSemBuf= (struct sembuf){0,-1,0};
    semaphores.bearSemId =  semget(SEMAPHORE_BEAR_KEY, 1, IPC_CREAT); 
    strong_bee.honeySharedMemoryID = shmget(HONEY_SHARED_MEMORY_KEY, HONEY_SHARED_MEMORY_SIZE, IPC_CREAT);
    if (strong_bee.honeySharedMemoryID<0){
        perror("fighter - shmget error");
        exit(1);
    }
    if (semaphores.bearSemId == -1){
        perror("fighter - bearSEMID error");
        exit(2);
    }
    semaphores.sectionSemId = semget(SEMAPHORE_SECTION_KEY, 1, IPC_CREAT);
    if (semaphores.sectionSemId == -1){
        perror("fighter - sectionSEMID error");
        exit(2);
    }
    semaphores.memorySemId= semget(SEMAPHORE_MEMORY_KEY, 1, IPC_CREAT);
    if (semaphores.memorySemId == -1){
        perror("fighter - memorySEMID error");
        exit(7);
    }
    while(TRUE){
        consumeHoney();
        sleep(WORKER_BEE_CONSUMPTION_TIME);
    }
    return 0;
}
    
          
void handle_sigterm(int sig){
    stop_flag=TRUE;
    while( busy==TRUE){ 
        sleep(1); //FIXME: check
    }
    exit(sig);
}


void consumeHoney(){
        if(stop_flag==FALSE) {
        busy=TRUE;
        semop(semaphores.bearSemId,&sembufs.downSemBuf, 1);
        semop(semaphores.bearSemId, &sembufs.upSemBuf, 1);
        semop(semaphores.sectionSemId ,&sembufs.downSemBuf, 1); 
        semop(semaphores.memorySemId ,&sembufs.downSemBuf, 1);
        int *honeyAddr;
        honeyAddr = shmat(strong_bee.honeySharedMemoryID, NULL, 0);
        if ( *honeyAddr <= 0){
            //koniec gry
            shmdt(honeyAddr);
            semop(semaphores.memorySemId ,&sembufs.upSemBuf, 1); 
            semop(semaphores.sectionSemId ,&sembufs.upSemBuf, 1);
            return;
        }
        *honeyAddr -= FIGHTER_BEE_CONSUMED_HONEY; //zje i koniec gry 
        shmdt(honeyAddr);
        semop(semaphores.memorySemId ,&sembufs.upSemBuf, 1); 
        semop(semaphores.sectionSemId ,&sembufs.upSemBuf, 1);
        busy=FALSE; 
        }
}

