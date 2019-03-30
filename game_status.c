#include "parameters.h"
#include "ansi_structs.h"  

struct msgbuf
{
    long mtype;
    int honey;
    int workers;
    int fighters;
    int queens;
    int bears;
} msg_to_show;

struct msgpid
{
    long mtype;
    int terminalPID;
} init_msg;


struct messageQIDs{
    int toTerminal;
} messages;

struct shmIDs{
    int statsSharedMemoryID;
}shared_memory;


void signal_handler(int sig);

int main(){
    msg_to_show.mtype=0;
    msg_to_show.honey=0;
    msg_to_show.workers=0;
    msg_to_show.fighters=0;
    msg_to_show.queens=0;
    msg_to_show.bears=0;
    init_msg.mtype= 2;
    init_msg.terminalPID=getpid();
    messages.toTerminal = msgget( MESSAGE_QUEUE_TERMINAL_KEY, IPC_CREAT);
    if(messages.toTerminal == -1){
        perror("game status - msgget error");
        exit(1);
    }
    shared_memory.statsSharedMemoryID = shmget( STATS_SHARED_MEMORY_KEY, 2053*sizeof(int), IPC_CREAT); 
    if(shared_memory.statsSharedMemoryID == -1){
        perror("terminal - stats shmget error\n");
        exit(2);
    }
    int* stats = shmat(shared_memory.statsSharedMemoryID,NULL,0); 
    stats[2052]=getpid();
    shmdt(stats);

    signal(SIGINT, signal_handler);
    signal(SIGTERM,signal_handler);
    while(TRUE){
        msgrcv( messages.toTerminal , &msg_to_show, sizeof(msg_to_show), 1, 0);
        system("clear"); 
        printf("Ilosc pracownikow: %d\n", msg_to_show.workers);
        printf("Ilosc wojownikow: %d\n", msg_to_show.fighters);
        printf("Ilosc krolowych: %d\n", msg_to_show.queens);
        printf("Ilosc plastrow miodu: %d\n", msg_to_show.honey);
        printf("Ilosc misiow: %d\n", msg_to_show.bears);
        if(msg_to_show.honey <= msg_to_show.workers+msg_to_show.fighters){
            printf("Uwaga, za malo miodu by wyzywic pszczoly\n");
        }
        sleep(1);
    }
    return 0;
}

void signal_handler(int sig){
       exit(sig);
}