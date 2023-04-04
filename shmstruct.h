// Copyright 2023 William Novak-Condy
#ifndef SHMSTRUCT_H_
#define SHMSTRUCT_H_

#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

#define BUF_SIZE 4096
#define OFFSET 1024
#define NUMTHREADS 4
struct shmbuf {
    sem_t sem1;
    sem_t sem2;
    size_t count;  // Number of bytes being used in the buffer
    char buffer[BUF_SIZE];
};


#endif // SHMSTRUCT_H_