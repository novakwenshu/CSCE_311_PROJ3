// Copyright 2023 William Novak-Condy
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <string>

#ifndef CLIENT_H_
#define CLIENT_H_
int client(int argc, char * argv[]);

void* checkLine(void *threadarg);
#endif  // CLIENT_H_
