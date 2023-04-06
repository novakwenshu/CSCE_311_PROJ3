// Copyright 2023 William Novak-Condy
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <ctype.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#ifndef SERVER_H_
#define SERVER_H_
int server(int argc, char *argv[]);

#endif  // SERVER_H_
