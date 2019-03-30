#ifndef ANSI_STRUCTS_H
#define ANSI_STRUCTS_H

#define _POSIX_C_SOURCE 199309L //TODO:
#define NANO_PER_SEC 1000000000.0

#include <time.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h> 
#include <sys/ipc.h> 
#include <sys/shm.h>
#include <sys/types.h>   
#include <sys/stat.h>
#include <sys/ipc.h> 
#include <sys/sem.h>
#include <stdio.h>
#include <string.h>
#include<sys/types.h>
#include <sys/ipc.h> 
#include <sys/msg.h>
#include <errno.h>

//  struct sembuf {    
//           short sem_num;
//           short sem_op;
//           short sem_flg;
//  }; 

 union semun {         
     int val;          //setval
     struct semid_ds *buf; /* for IPC_STAT and IPC_SET */       
     short *array;        /* used for GETALL and SETALL */   
     };


// struct semBufs{
//     struct sembuf upSemBuf;
//     struct sembuf downSemBuf;
// };

// struct semaphoreSet{
//     int bearSemId;
//     int sectionSemId;
//     int memorySemId;
// };

#endif //ANSI_STRUCTS_H