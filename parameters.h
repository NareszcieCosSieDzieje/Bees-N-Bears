#ifndef parameters_H
#define parameters_H


#define TRUE 1
#define FALSE 0
#define NULL 0 //TODO: is it good?

//===========================MESSAGE---QUEUE========================\\

#define DEBUG_QUEUE_TERMINAL_KEY 88 //FIXME: usun
#define MESSAGE_QUEUE_TERMINAL_KEY 73
#define BEAR_QUEUE_TERMINAL_KEY 59
//================================MEMORY=============================\\

#define STATS_SHARED_MEMORY_KEY 199
#define QUEEN_SHARED_MEMORY_KEY 423
#define QUEEN_SHARED_MEMORY_SIZE 8 
#define FIGHTER_SHARED_MEMORY_KEY 555
#define FIGHTER_SHARED_MEMORY_SIZE 8 
#define WORKER_SHARED_MEMORY_KEY 771
#define WORKER_SHARED_MEMORY_SIZE 8 
#define HONEY_SHARED_MEMORY_KEY 929
#define HONEY_SHARED_MEMORY_SIZE 8 //TODO:
#define BEAR_SHARED_MEMORY_KEY 653
#define BEAR_SHARED_MEMORY_SIZE 8

//================================SEMAPHORES==========================\\

#define SEMAPHORE_FRIEND_KEY 1999 //TODO:
#define SEMAPHORE_GAME_KEY 2314 //REDEFINE DAMN
#define SEMAPHORE_MEMORY_KEY 3239
#define SEMAPHORE_SECTION_KEY 4876
#define SEMAPHORE_BEAR_KEY 5235

//==================================BEEHIVE============================\\

#define BEEHIVE_SIZE 100; //do ustawiania semaforow!//TODO:
#define BEE_SIZE 1
#define INITIAL_HONEY 200 //TODO: strojenie 
#define QUEENS_TO_VICTORY 3

#define BEE_WORKER 2
#define BEE_FIGHTER 3
#define QUEEN 4

//=================================WORKER_BEE===========================\\
//produces resources

#define WORKER_BEE_COST 7 //honey
#define WORKER_BEE_SPAWN_TIME 3
#define WORKER_BEE_PRODUCED_HONEY 2 //odkladany do magazynu
#define WORKER_BEE_PRODUCTION_TIME 3 //seconds
#define WORKER_BEE_CONSUMED_HONEY 1
#define WORKER_BEE_CONSUMPTION_TIME 14   //co 14 sek

//=================================FIGHTER_BEE===========================\\
//protects resources

#define FIGHTER_BEE_COST 10 
#define FIGHTER_BEE_SPAWN_TIME 5
#define FIGHTER_BEE_CONSUMED_HONEY 1
#define FIGHTER_BEE_CONSUMPTION_TIME 14     //co 14 sek

//==================================QUEEN================================\\
//generates victory points

#define QUEEN_COST 500
#define QUEEN_SPAWN_TIME 9
#define QUEEN_SPAWN_AS_QUEEN_PROPABILITY 50    //procenty czyli 0.5
#define QUEEN_SPAWN_AS_WORKER_PROPABILITY 25    //procenty czyli 0.25
#define QUEEN_SPAWN_AS_FIGHTER_PROPABILITY 25    //procenty czyli 0.25

//====================================FORREST===========================\\

#define INITIAL_NUMBER_OF_BEARS 1 

//=====================================BEAR==============================\\

#define BEAR_SIZE 10
#define BEAR_ATTACK_SLEEP 10   //co 10 sekund
#define BEAR_ATTACK_PROPABILITY 10     //10% 0.1
#define BEAR_ATTACK_PROPABILITY_INCREASE 10    //o 10%
#define BEAR_GET_FRIEND_PROPABILITY 5    //5%  DAC MNIEJSZA?
#define BEAR_GET_FRIEND_SLEEP 10    //co 10 sek
#define BEAR_GET_FRIEND_PROPABILITY_INCREASE 5   //5%
#define MAX_KILLED_BEES_PER_BEAR 10
#define BEAR_HONEY_CONSUMPTION 50
// #define BEAR_HUNGER_METER 10 //init values //TODO: usun
// #define BEAR_FRIEND_METER 5 //init values

#endif// parameters_H 