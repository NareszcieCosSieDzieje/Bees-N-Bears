
#include "parameters.h"
#include "ansi_structs.h"


struct bear{
    int hungerMeter;//restart after attack
    int getFriendMeter; //reset after getting another bear
    int honeySharedMemoryID;
    int bearSharedMemoryID;
} angry_bear;

struct semBufs{
    struct sembuf upSemBuf;
    struct sembuf downSemBuf;
    struct sembuf upBearSemBuf; //o 10
    struct sembuf downBearSemBuf; //o 10
} sembufs;

struct semaphoreSet{
    int bearSemId;
    int sectionSemId;
    int memorySemId;
    int friendSemId;
    int gameSemId;
} semaphores;

struct messageIDs{
    int killMessage;
} msg_q;

struct msgbuf{   
    long mtype;
    int killType;
} msg_kill;


int busy=FALSE;
int stop_flag=FALSE;

float floatRandomGenerator( float min, float max );

void handle_sigterm(int sig);

void bearAttack();

void getFriend();

void cleanUP();

int main(int argc, char* argv[]){
    //signal(SIGTERM,handle_sigterm);
    srand(getpid());
    float random=-1;
    angry_bear.hungerMeter = 0;
    angry_bear.getFriendMeter = 0;
    sembufs.upSemBuf= (struct sembuf){0,1,0};
    sembufs.downSemBuf= (struct sembuf){0,-1,0};
    sembufs.upBearSemBuf= (struct sembuf){0,10,0};
    sembufs.downBearSemBuf= (struct sembuf){0,-10,0};
  
    angry_bear.honeySharedMemoryID = shmget(HONEY_SHARED_MEMORY_KEY, HONEY_SHARED_MEMORY_SIZE, IPC_CREAT);
    if (angry_bear.honeySharedMemoryID<0){
        perror("bear - honey shmget error");
        exit(2);
    }
    angry_bear.bearSharedMemoryID = shmget(BEAR_SHARED_MEMORY_KEY, BEAR_SHARED_MEMORY_SIZE, IPC_CREAT);
    if (angry_bear.bearSharedMemoryID < 0){
        perror("bear - bear shmget error");
        exit(2);
    }
    semaphores.bearSemId =  semget(SEMAPHORE_BEAR_KEY, 1, IPC_CREAT); 
    if (semaphores.bearSemId ==-1){
        perror("bear - bearsem get error");
        exit(3); 
    }
    semaphores.sectionSemId = semget(SEMAPHORE_SECTION_KEY, 1, IPC_CREAT);
    if (semaphores.sectionSemId ==-1){
        perror("bear - sectionsem get error");
        exit(3);
    }
    semaphores.memorySemId= semget(SEMAPHORE_MEMORY_KEY, 1, IPC_CREAT);
    if (semaphores.memorySemId ==-1){
        perror("bear - memorysem get error");
        exit(3);
    }
    semaphores.friendSemId= semget(SEMAPHORE_FRIEND_KEY, 1, IPC_CREAT);
    if (semaphores.friendSemId == -1){
        perror("bear - friendsem get error");
        exit(3);
    }
    semaphores.gameSemId =  semget(SEMAPHORE_GAME_KEY, 1, IPC_CREAT); 
    if (semaphores.gameSemId ==-1){
        perror("bear - gamesem get error");
        exit(3); 
    }
    msg_q.killMessage = msgget( BEAR_QUEUE_TERMINAL_KEY, IPC_CREAT); 
    struct timespec ts_a = {BEAR_ATTACK_SLEEP,0};
    struct timespec ts_f = {BEAR_GET_FRIEND_SLEEP,0};
    pid_t bearProcess = fork(); 
    if (bearProcess == -1){
        perror("bear - bearProcess fork error");   
        exit(4);
    }
    if ( bearProcess == 0 ) {
        pid_t division = fork();
        if (bearProcess == -1){
        perror("bear - bearProcess fork error"); 
        exit(4);
        }
        if (division == 0){
            signal(SIGTERM,handle_sigterm);//FIXME: eksperymetn
            while(TRUE){ 
                nanosleep(&ts_a, NULL);
                random=floatRandomGenerator(0.0,1.0);
                float chance1 = ((float)(BEAR_ATTACK_PROPABILITY + angry_bear.hungerMeter ))/100;
                if( random <= chance1 ) {
                    bearAttack();
                    angry_bear.hungerMeter=0;
                }
                else if(angry_bear.hungerMeter<100){
                    angry_bear.hungerMeter += BEAR_ATTACK_PROPABILITY_INCREASE;
                } //jak pelen to nic
            }
        }
        else {
            signal(SIGTERM,handle_sigterm);
            while(TRUE){
                nanosleep(&ts_f, NULL);
                floatRandomGenerator(0.0,1.0);
                float chance2 = ((float)(BEAR_GET_FRIEND_PROPABILITY + angry_bear.getFriendMeter))/100;
                if ( random <= chance2 ){
                    getFriend(); //TODO: wyskalowac to moze
                    angry_bear.getFriendMeter=0;
                }
                else if(angry_bear.getFriendMeter<100){
                    angry_bear.getFriendMeter += BEAR_GET_FRIEND_PROPABILITY_INCREASE;
                }
            }
        }
    } else {
        while(TRUE){
            msgrcv( msg_q.killMessage , &msg_kill, sizeof(msg_kill), 3, 0); //TODO: DEFINE MSG TYPES
            cleanUP();
        }
    } 
    return 0;
}

void cleanUP(){ //TODO: killflag -> busy
    int* bear_amount = shmat(angry_bear.bearSharedMemoryID, NULL, 0);
    semop(semaphores.friendSemId, &sembufs.downSemBuf, 1); //mozliwe ze nie potrzeba? check
    *bear_amount-=1;
    semop(semaphores.friendSemId, &sembufs.upSemBuf, 1); 
    shmdt(bear_amount);
    kill(0,SIGTERM);
    exit(22);
}

void handle_sigterm(int sig){
    stop_flag=TRUE;
    while( busy == TRUE){
       sleep(1);
    }
    exit(sig);
    //czy niedzwiedz ma jakos zamykac kolejke?
}

void getFriend(){
    semop(semaphores.gameSemId, &sembufs.downSemBuf, 1);
    semop(semaphores.gameSemId, &sembufs.upSemBuf, 1);
    if(stop_flag == FALSE){
    busy = TRUE;
    int* bear_amount = shmat(angry_bear.bearSharedMemoryID, NULL, 0); //FIXME:
    if (*bear_amount>100){ 
        shmdt(bear_amount);
        return;
    } //FIXME: check
    pid_t spawnBear = fork();
    if(spawnBear == -1 ){
        shmdt(bear_amount);
        perror("bear - spawnBear error");
        exit(3);
    }
    if (spawnBear == 0){
        setpgid(0, 0);
        execlp("./bear.out", "bear.out", NULL);
    }
    else { 
        int* bear_amount = shmat(angry_bear.bearSharedMemoryID, NULL, 0);
        semop(semaphores.friendSemId, &sembufs.downSemBuf, 1); 
        *bear_amount+=1;
        semop(semaphores.friendSemId, &sembufs.upSemBuf, 1); 
        shmdt(bear_amount);
        busy=FALSE;
    }
    }
}


void bearAttack(){
    int killCase=0;
    if(stop_flag == FALSE){
    busy = TRUE;
    semop(semaphores.bearSemId,&sembufs.downSemBuf,1); 
    semop(semaphores.sectionSemId,&sembufs.downBearSemBuf,1); //down 10
    semop(semaphores.bearSemId,&sembufs.upSemBuf,1); 
    semop(semaphores.memorySemId,&sembufs.downSemBuf,1);
    msg_kill.mtype = 2;
    msg_kill.killType = 0;
    msgsnd(msg_q.killMessage, &msg_kill, sizeof(msg_kill), 0);  
    msgrcv( msg_q.killMessage , &msg_kill, sizeof(msg_kill), 1, 0); 
    if ( msg_kill.killType == 1 ){
        angry_bear.hungerMeter = 0;   
        //nie bierz miodu
    }
    else if ( msg_kill.killType == 2 ) {
        int *honeyAddr = shmat(angry_bear.honeySharedMemoryID, NULL, 0);
        if ( *honeyAddr <= 0 ){
            shmdt(honeyAddr);
            semop(semaphores.memorySemId ,&sembufs.upSemBuf, 1); 
            semop(semaphores.sectionSemId ,&sembufs.upBearSemBuf, 1); //up 10
            busy=FALSE;
            return;
         }
        int honey = *honeyAddr;
        int consumption = BEAR_HONEY_CONSUMPTION;
        if(honey<consumption){
            *honeyAddr = 0;
        }
        else {
        *honeyAddr -= consumption;
        } 
        shmdt(honeyAddr);
        angry_bear.hungerMeter= 0;
    }
    else {
        int *honeyAddr = shmat(angry_bear.honeySharedMemoryID, NULL, 0);
        if ( *honeyAddr <= 0 ){
            shmdt(honeyAddr);
            semop(semaphores.memorySemId ,&sembufs.upSemBuf, 1); 
            semop(semaphores.sectionSemId ,&sembufs.upBearSemBuf, 1); //up 10
            busy=FALSE;
            return;
            // perror("bear - attack honeyAddr error ");
            // exit(5);
        }
        int honey = *honeyAddr;
        int consumption = BEAR_HONEY_CONSUMPTION;
        if(honey<consumption){
            *honeyAddr = 0;
        }
        else {
        *honeyAddr -= consumption;
        } 
        shmdt(honeyAddr);
    }
    semop(semaphores.memorySemId ,&sembufs.upSemBuf, 1); 
    semop(semaphores.sectionSemId ,&sembufs.upBearSemBuf, 1); //up 10
    busy=FALSE;
    }
}

float floatRandomGenerator( float min, float max )
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}

