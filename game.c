#include "parameters.h"
#include "ansi_structs.h"

// Do pamieci wspoldzielonej, segmentow
#define fighterKillPos 0
#define workerKillPos 1 
#define fighterAddPos 2
#define workerAddPos 3
#define fighterPIDs 4
#define workerPIDs 1028
#define TERMINAL_PID 2052

//===================================GLOBAL----STRUCTS============================\\

struct msgbufTerminal
{
    long mtype;
    int honey;
    int workers;
    int fighters;
    int queens;
    int bears;
} msg_to_terminal;

struct msgbufBear
{
    long mtype;
    int killType;
} bear_msg;


struct semBufs{
    struct sembuf upSemBuf;
    struct sembuf downSemBuf;
    struct sembuf downHiveSemBuf;
} sembufs;


struct forest{
    int bears;
} forest;


struct sharedMemoryIDs{
    int honeySharedMemoryID; 
    int bearSharedMemoryID;
    int queenSharedMemoryID;
    int fighterSharedMemoryID;
    int workerSharedMemoryID;
    int statsSharedMemoryID;
} shared_memory;

struct semaphoreIDs{
    int bearSemID;
    int beeHiveSemID;
    int memorySemID;
    int friendSemID;
    int gameSemID;
} semaphores;


struct msgqIDs{
    int toTerminalID;
    int bearMessageID;
    int debugMessageID;// FIXME: USUN
} messages;


 int* honeyAmount;
 int* bearAmount;
 int* workerAmount;
 int* fighterAmount;
 int* queenAmount;

int WORK=1;

int terminalMSG;
 int RECEIVER=TRUE;

//================================FUNCTION----PROTOTYPES==========================\\

void sigint_handler(int sigint);

void sigterm_handler(int sigterm);

void produceBee();

void produceWorker();

void produceFighter();

void produceQueen();

void createBear();

char killBees();

void cleanUP();

float floatRandomGenerator( float min, float max );

int min(int a, int b);

int inputScan();

int inputScan2();
 
//TODO: czasami przekraczana jest kasa na -1, zeby misie zabijaly i sie robily jeszcze
//czyszczenie dobre

//dac waity do parentow forka

//ogarnac niedzwiedzie zmniejszyc ich czestotliwosc albo jeszcze dubbig costam

//======================================MAIN=========================================\\

    int main(int argc, char* argv[]){
        srand((unsigned int)time(NULL));
        sembufs.upSemBuf= (struct sembuf){0,1,0};  
        sembufs.downSemBuf= (struct sembuf){0,-1,0}; 
        sembufs.downHiveSemBuf= (struct sembuf){0,-100,0};
        //TODO: moze ustawic na 10 i na 100 zeby zablokowac s
        //signal(SIGTERM,sigterm_handler);
        semaphores.bearSemID = semget(SEMAPHORE_BEAR_KEY, 1, 0666 | IPC_CREAT);
        if(semaphores.bearSemID == -1){
            perror("main - bear sem error\n");
            exit(2);
        }
        semaphores.beeHiveSemID = semget(SEMAPHORE_SECTION_KEY, 1, 0666 | IPC_CREAT);
        if(semaphores.beeHiveSemID == -1){
            perror("main - beeHive sem error\n");
            exit(2);
        }
        semaphores.memorySemID = semget(SEMAPHORE_MEMORY_KEY, 1, 0666 | IPC_CREAT);
        if(semaphores.memorySemID == -1){
            perror("main - memory sem error\n");
            exit(2);
        }
        semaphores.gameSemID  = semget(SEMAPHORE_GAME_KEY, 1, 0666 | IPC_CREAT); 
        if(semaphores.gameSemID == -1){
            perror("main - game sem get error\n");
            exit(2);
        }
        semaphores.friendSemID = semget(SEMAPHORE_FRIEND_KEY, 1, 0666 | IPC_CREAT); 
        if(semaphores.friendSemID == -1){
            perror("main - friend sem error\n");
            exit(2);
        }
        shared_memory.honeySharedMemoryID = shmget( HONEY_SHARED_MEMORY_KEY, HONEY_SHARED_MEMORY_SIZE, IPC_CREAT | 0660); 
        if(shared_memory.honeySharedMemoryID == -1){
            perror("main - honey memory error\n");
            exit(3);
        }
        shared_memory.bearSharedMemoryID = shmget( BEAR_SHARED_MEMORY_KEY, BEAR_SHARED_MEMORY_SIZE, IPC_CREAT | 0660); 
        if(shared_memory.bearSharedMemoryID == -1){
            perror("main - bear memory error\n");
            exit(3);
        }
        shared_memory.workerSharedMemoryID = shmget( WORKER_SHARED_MEMORY_KEY, WORKER_SHARED_MEMORY_SIZE, IPC_CREAT | 0660); 
        if(shared_memory.workerSharedMemoryID == -1){
            perror("main - worker memory error\n");
            exit(3);
        }
        shared_memory.fighterSharedMemoryID = shmget( FIGHTER_SHARED_MEMORY_KEY, FIGHTER_SHARED_MEMORY_SIZE, IPC_CREAT | 0660); 
        if(shared_memory.fighterSharedMemoryID == -1){
            perror("main - fighter memory error\n");
            exit(3);
        }
        shared_memory.queenSharedMemoryID = shmget( QUEEN_SHARED_MEMORY_KEY, QUEEN_SHARED_MEMORY_SIZE, IPC_CREAT | 0660); 
        if(shared_memory.queenSharedMemoryID == -1){
            perror("main - queens memory error\n");
            exit(3);
        } 
        shared_memory.statsSharedMemoryID = shmget( STATS_SHARED_MEMORY_KEY, 2053*sizeof(int), IPC_CREAT | 0660); 
        if(shared_memory.statsSharedMemoryID == -1){
            perror("main - stats shmget error\n");
            exit(3);
        }
        int *myMemory = shmat (shared_memory.statsSharedMemoryID, NULL, 0);
        myMemory[workerAddPos] = workerPIDs;//0;    
        myMemory[fighterAddPos] = fighterPIDs;//0;
        myMemory[workerKillPos] = workerPIDs;//0;
        myMemory[fighterKillPos] = fighterPIDs;//0;
        for(int gh=4;gh<2052;gh++){ //2052-1 TODO: #define
            myMemory[gh]=0;
        }
        shmdt(myMemory); 
        messages.toTerminalID = msgget(MESSAGE_QUEUE_TERMINAL_KEY, IPC_CREAT | 0666);
        if (messages.toTerminalID < 0){
            perror("main - terminal msgget error");
            exit(19);
        }
        messages.bearMessageID = msgget(BEAR_QUEUE_TERMINAL_KEY, IPC_CREAT | 0666);
        if (messages.bearMessageID < 0){
            perror("main - bear msget error");
            exit(19);
        }
        messages.debugMessageID = msgget(DEBUG_QUEUE_TERMINAL_KEY, IPC_CREAT | 0666);
        if (messages.debugMessageID < 0){ //TODO:FIXME:FIXME:
            perror("main - debug msget error");
            exit(19);
        }
        union semun arg;
        arg.val = 1; 
        int bearCMD = semctl(semaphores.bearSemID, 0, SETVAL, arg);
        if(bearCMD == -1){
            perror("main - bear sem setval error\n");
            exit(4);
        } 
        int memoryCMD = semctl(semaphores.memorySemID, 0, SETVAL, arg);
        if(memoryCMD == -1){
            perror("main - memory sem setval error\n");
            exit(4);
        }
        int gameCMD = semctl(semaphores.gameSemID, 0, SETVAL, arg);
        if(gameCMD == -1){
            perror("main - game sem setval error\n");
            exit(4);
        }
        int friendCMD = semctl(semaphores.friendSemID, 0, SETVAL, arg);
        if( friendCMD == -1){
            perror("main - friend sem setval error\n");
            exit(4);
        }
        arg.val = BEEHIVE_SIZE; //last one for bee_hive
        int hiveCMD = semctl(semaphores.beeHiveSemID, 0, SETVAL, arg);
        if( hiveCMD == -1){
            perror("main - beehive sem setval error\n");
            exit(4);
        }  
        pid_t hiveFork = fork();
        if (hiveFork == -1){
            perror("main - hiveFork error\n");
            exit(5);
        }
        if(hiveFork == 0){
            pid_t inputOutput = fork(); 
            if(inputOutput == -1){
                perror("main - fork inOut error");
                exit(6);
            }
            if(inputOutput == 0){
                signal(SIGTERM,sigterm_handler);
                honeyAmount = shmat(shared_memory.honeySharedMemoryID, NULL, 0);
                if ( *honeyAmount == -1){ 
                    perror("main - honey attach memory in inOut error\n");
                    exit(7);
                }
                bearAmount = shmat(shared_memory.bearSharedMemoryID, NULL, 0);
                if ( *bearAmount == -1){
                    perror("main - bear attach memory in inOut error\n");
                    exit(7);
                }
                workerAmount = shmat(shared_memory.workerSharedMemoryID, NULL, 0);
                if ( *workerAmount == -1){
                    perror("main - worker attach memory in inOut error\n");
                    exit(7);
                }
                fighterAmount = shmat(shared_memory.fighterSharedMemoryID, NULL, 0);
                if ( *fighterAmount == -1){
                    perror("main - fighter attach memory in inOut error\n");
                    exit(7);
                }
                queenAmount = shmat(shared_memory.queenSharedMemoryID, NULL, 0);
                if ( *queenAmount == -1){
                    perror("main - queen attach memory in inOut error\n");
                    exit(7);
                }
                *honeyAmount = INITIAL_HONEY;
                *bearAmount=0;
                *workerAmount=0;
                *fighterAmount=0;
                *queenAmount=0;
                while(RECEIVER==TRUE){  
                    msg_to_terminal.mtype=1;
                    msg_to_terminal.workers = *workerAmount;
                    msg_to_terminal.fighters = *fighterAmount;
                    msg_to_terminal.queens = *queenAmount;
                    msg_to_terminal.honey = *honeyAmount;
                    msg_to_terminal.bears = *bearAmount;
                    msgsnd(messages.toTerminalID, &msg_to_terminal, sizeof(msg_to_terminal), 0);   
                    sleep(1);//TODO: ten sleep obczaic
                }
                shmdt(honeyAmount);
                shmdt(bearAmount);
                shmdt(workerAmount);  //TODO: FIXME:
                shmdt(fighterAmount);
                shmdt(queenAmount);
                exit(45);
            }
            else { 
                while(TRUE){
                    msgrcv( messages.bearMessageID , &bear_msg, sizeof(bear_msg), 2, 0); 
                    bear_msg.mtype=1;
                    bear_msg.killType = killBees(); 
                    msgsnd(messages.bearMessageID, &bear_msg, sizeof(bear_msg), 0);   
                }                   
            }
        }
        else {
            signal(SIGINT,sigint_handler);
            while(TRUE){
                printf("Start gry - 1\nKoniec gry - 2\n");
                terminalMSG = inputScan(1,2);
                if(terminalMSG == 2){
                    printf("Koniec gry!\n");
                    cleanUP(); 
                }
                else if(terminalMSG == 1){
                    printf("Start gry!\n");
                    break;   
                }
            }
            int delay = FALSE;
            //INICJALIZaCJA PSZCZOL 
            for(int gg=0;gg<80;gg++){
                produceWorker(delay);
            }
            for(int dd=0;dd<20;dd++){
               produceFighter(delay);   
            }
            for(int iterator=0;iterator<INITIAL_NUMBER_OF_BEARS;iterator++){
                createBear(); 
            }
            int* honey_addr = shmat(shared_memory.honeySharedMemoryID, NULL, 0);
            if ( *honey_addr == -1){ 
                perror("main - honey attach memory in sim error\n");
                exit(17);
            }
            int* queen_addr = shmat(shared_memory.queenSharedMemoryID, NULL, 0);
            if ( *queen_addr == -1){ 
                perror("main - honey attach memory in sim error\n");
                exit(17);
            }
            system("clear");
            printf("Wyprodukuj robotnice - 1, koszt %d\n",WORKER_BEE_COST);
            printf("Wyprodukuj wojownika - 2, koszt %d\n",FIGHTER_BEE_COST);
            printf("Wyprodukuj krolowa - 3, koszt %d\n",QUEEN_COST);
            printf("Koniec gry - 4\n"); 
            struct timespec start, end;
            double start_sec, end_sec, elapsed_sec;
            elapsed_sec=0;
            while( ( *queen_addr < 3) && ( *honey_addr > 0) ){
                clock_gettime(CLOCK_REALTIME, &start);
                if( elapsed_sec>5 ){
                    elapsed_sec=0;
                    system("clear");
                    printf("Wyprodukuj robotnice - 1, koszt %d\n",WORKER_BEE_COST);
                    printf("Wyprodukuj wojownika - 2, koszt %d\n",FIGHTER_BEE_COST);
                    printf("Wyprodukuj krolowa - 3, koszt %d\n",QUEEN_COST);
                    printf("Koniec gry - 4\n"); 
                }
                terminalMSG = inputScan(1,4);
                if (terminalMSG == 1){
                    produceBee(BEE_WORKER);
                }
                else if (terminalMSG == 2){
                    produceBee(BEE_FIGHTER);
                }
                else if (terminalMSG == 3){
                   produceBee(QUEEN);
                }
                else if (terminalMSG == 4){
                    break;
                }
                else {
                    printf("\nZLE DANE WEJSCIOWE, SPROBUJ PONOWNIE.\n");
                }
                clock_gettime(CLOCK_REALTIME, &end);
                start_sec = start.tv_sec + start.tv_nsec/NANO_PER_SEC;
                end_sec = end.tv_sec + end.tv_nsec/NANO_PER_SEC;
                elapsed_sec = elapsed_sec + end_sec - start_sec;
            }
            system("clear");
            printf("KONIEC GRY!\n");
            if (*honey_addr <= 0){
                printf("\nPRZEGRANA :(\n");
            }
            else {
                printf("\nWYGRANA, ZDOBYTO WSZYSTKIE KROLOWE :)\n");
            }
            printf("Zamykam...\n");
            shmdt(honey_addr);
            shmdt(queen_addr);
            sleep(5);
            cleanUP();
        }       
        return 0;
    }

//===========================FUNCTION-----DEFINITIONS===============================\\


void sigterm_handler(int sigterm){
    RECEIVER=FALSE;
    sleep(10);
    exit(sigterm);
}

void cleanUP(){
    
    int* deleteBees = shmat(shared_memory.statsSharedMemoryID,NULL,0); 
    int* deleteBears = shmat(shared_memory.bearSharedMemoryID,NULL,0);
    int i = 0;
    bear_msg.mtype = 3;
    bear_msg.killType=0;
    
    semop(semaphores.gameSemID, &sembufs.downSemBuf, 1);
    for(i; i< *deleteBears;i++ ){
        msgsnd(messages.toTerminalID, &bear_msg, sizeof(bear_msg), 0); 
    } 
    
    
    i = fighterPIDs;
    for(i; i<2052;i++){ 
        if(deleteBees[i] != 0){
        kill(-deleteBees[i], SIGTERM);     
        }
    }

    int terminal_pid = deleteBees[2052]; printf("terminal pid: %d\n",terminal_pid);
    if(terminal_pid!=0){
    kill(terminal_pid,SIGTERM);
    }
    shmdt(deleteBees);
    shmdt(deleteBears);

    msgctl(messages.bearMessageID, IPC_RMID, NULL);
    msgctl(messages.debugMessageID, IPC_RMID, NULL);
    semctl(semaphores.bearSemID, 0, IPC_RMID, 0);        //usuniecie semafor
    semctl(semaphores.memorySemID, 0, IPC_RMID, 0);
    semctl(semaphores.beeHiveSemID, 0, IPC_RMID, 0);
    semctl(semaphores.gameSemID,0,IPC_RMID,0); //TODO: check
    semctl(semaphores.friendSemID, 0, IPC_RMID,0);
    shmctl(shared_memory.honeySharedMemoryID,IPC_RMID,NULL);  //usuniecie pamieci
    shmctl(shared_memory.bearSharedMemoryID,IPC_RMID,NULL);
    shmctl(shared_memory.workerSharedMemoryID,IPC_RMID,NULL);
    shmctl(shared_memory.fighterSharedMemoryID,IPC_RMID,NULL);
    shmctl(shared_memory.queenSharedMemoryID,IPC_RMID,NULL);
    shmctl(shared_memory.statsSharedMemoryID,IPC_RMID,NULL);
    msgctl(messages.toTerminalID , IPC_RMID, NULL);
    kill(0, SIGTERM); //make a user defined singal
   // sleep(5); 
   // kill(0, SIGKILL); 
}

 
void sigint_handler(int sigint){
    cleanUP();
    exit(sigint);
}


int min(int a, int b){
    return a<b?a:b;
}
                                            
              
//FIXME: PROBLEM CZY MISIE MOGA ZABIJAC WSPOLBIERZNIE????!!! zabijane sa nie wszystkie pszczoly

char killBees(){
    semop(semaphores.gameSemID, &sembufs.downSemBuf, 1);
    int* fighter_addr = shmat(shared_memory.fighterSharedMemoryID, NULL, 0);
    int* worker_addr = shmat(shared_memory.workerSharedMemoryID, NULL, 0);
    int* stats = shmat(shared_memory.statsSharedMemoryID, NULL, 0);
    int fighters= *fighter_addr;
    int workers = *worker_addr; 
    int killType = 0;
    // int jk=0;
    // for(int k=4;k<2052;k++){
    //     if(stats[k]!=0){ //FIXME:
    //         jk++;
    //     }
    // } printf("jk przed: %d\n",jk);
    
    int maxFightersToKill = min(10, fighters);
    int it = 0;
    if (maxFightersToKill > 0){
    it = stats[fighterKillPos];             
    for(int i = 0; i < maxFightersToKill; i++){
        if(stats[it] !=0 ){
            kill(-stats[it],SIGTERM);
            stats[it]=0;
        }
        it += 1;
    }
    stats[fighterKillPos]=it;
    }
    int rest = 10-maxFightersToKill;
    int maxWorkersToKill = min(rest, workers);
    if (maxWorkersToKill > 0){  
    it=stats[workerKillPos];
    for(int p = 0; p < maxWorkersToKill; p++){
        if(stats[it] !=0 ){
            kill(-stats[it],SIGTERM);
            stats[it]=0;
        }
        it += 1;
    }
    stats[workerKillPos]=it;
    }

    // jk=0; //FIXME:
    // for(int b=4;b<2052;b++){
    //     if(stats[b]!=0){
    //         jk++;
    //     }
    // } printf("jk po: %d\n",jk);
    
    *fighter_addr -= maxFightersToKill;
    *worker_addr -= maxWorkersToKill;
    shmdt(worker_addr);
    shmdt(fighter_addr);
    shmdt(stats);
    semop(semaphores.gameSemID, &sembufs.upSemBuf, 1);
    if(maxFightersToKill == 10){
        killType = 1;
    } 
    else if (maxFightersToKill == 9){
        killType = 2;
    }
    else {
        killType= 3 ;
    }
    return killType;
}


void produceBee(int bee_type){
    semop(semaphores.memorySemID,&sembufs.downSemBuf, 1);
    int* honey_addr = shmat(shared_memory.honeySharedMemoryID, NULL, 0);
    if (*honey_addr<=0){
        return; // TODO: jakis return zeby dac info ze nie ma
    }
    if(bee_type==BEE_WORKER){
        if ( *honey_addr > WORKER_BEE_COST){
            *honey_addr -= WORKER_BEE_COST;
            shmdt(honey_addr);
            semop(semaphores.memorySemID,&sembufs.upSemBuf, 1); 
            printf("\nProdukcja pracownika\n");
            int delay = TRUE;
            produceWorker(delay);
        } 
        else {
            printf("\nNie wystarczajace srodki na zakup pracownika.\n");
            shmdt(honey_addr);
            semop(semaphores.memorySemID,&sembufs.upSemBuf, 1); 
            return;
        } 
    }
    else if (bee_type==BEE_FIGHTER){
        if ( *honey_addr > FIGHTER_BEE_COST){
            *honey_addr-=FIGHTER_BEE_COST;
            shmdt(honey_addr);
            semop(semaphores.memorySemID ,&sembufs.upSemBuf, 1);
            printf("\nProdukcja wojownika\n");
            int delay = TRUE; 
            produceFighter(delay);
        }
        else {
            printf("\nNie wystarczajace srodki na zakup wojownika.\n");
            shmdt(honey_addr);
            semop(semaphores.memorySemID,&sembufs.upSemBuf, 1); 
            return;
        }
    }
    else if (bee_type==QUEEN){
        float random = floatRandomGenerator(0.0,1.0);
        if(random<0.25){
            shmdt(honey_addr);
            semop(semaphores.memorySemID ,&sembufs.upSemBuf, 1); 
            produceBee(BEE_WORKER);
        }
        else if ( (random >= 0.25) && (random <= 0.5)){
            shmdt(honey_addr);
            semop(semaphores.memorySemID ,&sembufs.upSemBuf, 1); 
            produceBee(BEE_FIGHTER);
        }
        else { 
            if (*honey_addr > QUEEN_COST){
                *honey_addr -= QUEEN_COST;
                shmdt(honey_addr);
                semop(semaphores.memorySemID ,&sembufs.upSemBuf, 1); 
                printf("\nProdukcja królowej\n");
                produceQueen();
             }
             else {
                 printf("\nNie wystarczajace srodki na zakup krolowej.\n");
                 shmdt(honey_addr);
                 semop(semaphores.memorySemID,&sembufs.upSemBuf, 1); 
                 return;
             }
        }
    }
}

    
void produceWorker(int delay){
    pid_t newProcess = fork();
    if (newProcess<0){
        perror("main - produceWorker error");
        exit(8);
    }
    else if (newProcess == 0){
        if(delay == TRUE){
            sleep(WORKER_BEE_SPAWN_TIME); 
        }
        semop(semaphores.gameSemID, &sembufs.downSemBuf, 1);
        int* worker_addr = shmat(shared_memory.workerSharedMemoryID, NULL, 0);
        int* stats = shmat(shared_memory.statsSharedMemoryID, NULL, 0);
        setpgid(0, 0);
        // if(delay == TRUE){
        //     sleep(WORKER_BEE_SPAWN_TIME); 
        // }
        int position = stats[workerAddPos]; 
        stats[position]=getpgrp(); //same as gpid
        stats[workerAddPos]+=1;          
        *worker_addr += 1;
        shmdt(worker_addr);
        shmdt(stats);
        semop(semaphores.gameSemID, &sembufs.upSemBuf, 1); 
        execlp("./bee_worker.out", "bee_worker.out", NULL);
    } 
}



void produceFighter(int delay){
    pid_t newProcess = fork();
    if (newProcess<0){
        perror("main - produceFighter error");
        exit(9);
    }
    else if (newProcess == 0){
        if(delay == TRUE){
        sleep(FIGHTER_BEE_SPAWN_TIME);
        }
        semop(semaphores.gameSemID, &sembufs.downSemBuf, 1); 
        int* fighter_addr = shmat(shared_memory.fighterSharedMemoryID, NULL, 0);
        int* stats = shmat(shared_memory.statsSharedMemoryID, NULL, 0);
        setpgid(0, 0);
        // if(delay == TRUE){
        //     sleep(FIGHTER_BEE_SPAWN_TIME);
        // }
        int position = stats[fighterAddPos]; 
        stats[position]=getpgrp(); 
        stats[fighterAddPos]+=1;                
        *fighter_addr+=1; 
        shmdt(fighter_addr);
        shmdt(stats);
        semop(semaphores.gameSemID, &sembufs.upSemBuf, 1);    
        execlp("./bee_fighter.out", "bee_fighter.out", NULL);
    } 
}


void produceQueen(){
    pid_t queen = fork();
     if (queen < 0){
        perror("main - produceQueen error");
        exit(10);
    }
    else if(queen == 0){
        int* queen_addr = shmat(shared_memory.queenSharedMemoryID, NULL, 0);         
        sleep(QUEEN_SPAWN_TIME);
        *queen_addr +=1;  
        shmdt(queen_addr);
        exit(30);
    }
}

void createBear(){
    pid_t spawnBear = fork();
    if(spawnBear == -1 ){
        perror("main - createBear error");
        exit(18);
    }
    else if (spawnBear == 0){
        int* bear_addr = shmat(shared_memory.bearSharedMemoryID, NULL, 0);
        setpgid(0, 0);
        *bear_addr+=1;
        shmdt(bear_addr);
        execlp("./bear.out", "bear.out", NULL);
    }
}


float floatRandomGenerator( float min, float max )
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}


int inputScan(int min, int max){
  int n = 0;
  char buffer[128];
  fgets(buffer,sizeof(buffer),stdin);
  n = atoi(buffer); 
  return ( n > max || n < min ) ? 0 : n;
}


int inputScan2(int min, int max){
    int n = 0;
    char buffer[128];
    if(read(0,&buffer,sizeof(buffer))<0){
      if(errno==EINTR){
         return 4;//exit(40);
      }
      else{
          n = atoi(buffer); 
          return ( n > max || n < min ) ? 0 : n;
      }
    }
}


