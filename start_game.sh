#!/bin/bash

./cleanup.sh 
sleep 2
gcc bee_fighter.c -o bee_fighter.out
gcc bee_worker.c -o bee_worker.out
gcc bear.c -o bear.out
gcc game.c -o game.out
gcc game_status.c -o game_status.out
gcc usuwanie_pszczol_terminal.c -o usuwanie.out

x-terminal-emulator -e ./game.out & x-terminal-emulator -e ./game_status.out & x-terminal-emulator -e ./usuwanie.out
