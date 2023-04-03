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

using namespace std;



int main (int argc, char *argv[]) {

  int shmid = shm_open("test.txt", O_RDWR, 0);
  if (shmid == -1) {
    cerr << "shm_open " << strerror(errno) << endl;
  } else {
    clog << "SERVER STARTED" << endl;
  }

  sem_t *sem = sem_open("Test_sem", O_CREAT | O_EXCL, 0664, 0);

  if (sem == NULL) {
    cout << "Named sen not instantiated" << endl;
  }

  // May not need to be static casted
  struct shmbuf *store = static_cast<shmbuf*>(mmap(nullptr, sizeof(*store), PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0));
  if (store == MAP_FAILED) {
      cout << "MAP FAILED" << endl;
      cerr << "mmap " << strerror(errno) << endl;
  } else {
    cout << "Map casted" << endl;
  }
  
  cout << "test" << endl;
  if (sem_wait(sem) == -1) {
    cout << "Named sem wait error" << endl;
  }

  while (true) {
    if (sem_post(&store->sem1) == -1) {
      cout << "sem_post error";
    }
    clog << "CLIENT REQUEST RECIEVED" << endl;
    cout << "Read this: " << store->buffer << endl;

    char *path = (char*) malloc(sizeof(char)* strlen(store->buffer)+1);
    FILE* file;
    path = store->buffer;

    file = fopen(path, "r");
    int count = 0;
    if (file != NULL) {
      size_t len = 0;
      char *line = NULL;
      while ((getline(&line, &len, file)) != -1) {
        if (count != 0 && count%4 == 0) {
          if (sem_wait(&store->sem2) == -1) {
            cout << "Error in sem_wait while putting lines in shared memory" << endl;
          }
        }
        // If error maybe check if sending null terminator since no + 1
        memcpy(&store->buffer[(count%4)*1024], line, strlen(line)+1);
        char test[BUF_SIZE];
        strncpy(test,&store->buffer[count%4*OFFSET], OFFSET);
        cout << test << endl;
        count++;
      }
      sem_post(&store->sem2);
    }
  }
}
