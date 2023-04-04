// Copright 2023 William Novak-Condy
#include <iostream>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <cstring>
#include <ctype.h>
#include "shmstruct.h"
#include "server.h"

using namespace std;



int server (int argc, char *argv[]) {
  cout << "SERVER STARTED" << endl;
  while (true) {
    sem_unlink("/namesem");
    sem_t *sem = sem_open("/namesem", O_CREAT, S_IRGRP | S_IRUSR | S_IWGRP | S_IWUSR, 0);

    if (sem == NULL) {
      cout << "Named sem not instantiated" << endl;
      cout << strerror(errno) << endl;
    }
    if (sem_wait(sem) == -1) {
      cout << "Named sem wait error" << endl;
    }

    int shmid = shm_open("test.txt", O_RDWR, 0);
    if (shmid == -1) {
      cerr << "shm_open " << strerror(errno) << endl;
    }
    
    // May not need to be static casted
    struct shmbuf *store = static_cast<shmbuf*>(mmap(nullptr, sizeof(*store), PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0));
    if (store == MAP_FAILED) {
        cout << "MAP FAILED" << endl;
        cerr << "mmap " << strerror(errno) << endl;
    }
    

    char *path = (char*) malloc(sizeof(char)* strlen(store->buffer)+1);
    FILE* file;
    path = store->buffer;
    cout << store->buffer << endl;

    clog << "CLIENT REQUEST RECIEVED" << endl;
    clog << "\tMEMORY OPEN" << endl;

    file = fopen(path, "r");
    int count = 0;
    if (sem_post(&store->sem1) == -1) {
    cout << "sem_wait error" << endl;
    }
    
    if (file != NULL) {
      clog << "\tOPENING: " << path << endl;
      size_t len = 0;
      char *line = NULL;
      while ((getline(&line, &len, file)) != -1) {
        if ((count != 0 && count%4 == 0)) {
          if (sem_post(&store->sem1) == -1) {
            cout << "Error in sem_wait while putting lines in shared memory" << endl;
          }
          if (sem_wait(&store->sem2) == -1) {
            cout << "Error in sem_post while putting lines in shared memory" << endl;
          }
          memset(store->buffer, '\0', sizeof(store->buffer));
        }
        // If error maybe check if sending null terminator since no + 1
        memcpy(&store->buffer[(count%4)*1024], line, strlen(line)+1);
        char test[BUF_SIZE];
        strncpy(test,&store->buffer[count%4*OFFSET], OFFSET);
        count++;
      }
      // One last post to get the last lines over
      if (sem_post(&store->sem1) == -1) {
        cout << "Error in sem_wait while putting lines in shared memory" << endl;
      }
      if (sem_wait(&store->sem2) == -1) {
        cout << "Error in sem_post while putting lines in shared memory" << endl;
      }
      memset(store->buffer, '\0', sizeof(store->buffer));
      fclose(file);
      clog << "\tFILE CLOSED" << endl;
    } else if (file == NULL) {
        memcpy(&store->buffer[0], "INVALID FILE", strlen("INVALID FILE")+1);
        if (sem_post(&store->sem1) == -1) {
          cout << "Error in sem_wait while putting lines in shared memory" << endl;
        }
    }

    memcpy(&store->buffer[0], "STOP", strlen("STOP")+1);
    if (sem_post(&store->sem1) == -1) {
      cout << "Error with sem_wait when reading mem" << endl;
    }

    shmctl(shmid, IPC_RMID, NULL);
    munmap(store->buffer,4096);
    clog << "\tMEMORY CLOSED" << endl;
  }
}
