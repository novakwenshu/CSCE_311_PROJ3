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

// TODO DESTROY NAMED SEMPAHORE

int server (int argc, char *argv[]) {
  
  // STEP 1
  cout << "SERVER STARTED" << endl;
  while (true) {
    // Unlinks
    sem_unlink("/namesem");
    sem_t *sem = sem_open("/namesem", O_CREAT, S_IRGRP | S_IRUSR | S_IWGRP | S_IWUSR, 0);

    if (sem == NULL) {
      cout << "Named sem not instantiated" << endl;
      cout << strerror(errno) << endl;
    }
    // Waits for the client to initialize the mem
    if (sem_wait(sem) == -1) {
      cout << "Named sem wait error" << endl;
    }

    // STEP 3 opens shared memory in order to obtain path
    int shmid = shm_open("test.txt", O_RDWR, 0);
    if (shmid == -1) {
      cerr << "shm_open " << strerror(errno) << endl;
    }
    
    // MAPS THE MEMORY
    struct shmbuf *store = static_cast<shmbuf*>(mmap(nullptr, sizeof(*store), PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0));
    if (store == MAP_FAILED) {
        cout << "MAP FAILED" << endl;
        cerr << "mmap " << strerror(errno) << endl;
    }
    
    // STEP 2 Gets the file name from the client and uses it to open the file
    char *path = (char*) malloc(sizeof(char)* strlen(store->buffer)+1);
    FILE* file;
    path = store->buffer;

    clog << "CLIENT REQUEST RECIEVED" << endl;
    clog << "\tMEMORY OPEN" << endl;

    // STEP 4 Opening the file
    file = fopen(path, "r");
    int count = 0;
    // Tells client that it has read the file name
    if (sem_post(&store->sem1) == -1) {
    cout << "sem_wait error" << endl;
    }
    
    if (file != NULL) {
      // If the file is valid
      clog << "\tOPENING: " << path << endl;
      size_t len = 0;
      char *line = NULL;
      while ((getline(&line, &len, file)) != -1) {
        if ((count != 0 && count%4 == 0)) {
          // Once 4 lines have been put into shared memory, tell client that it can read it
          if (sem_post(&store->sem1) == -1) {
            cout << "Error in sem_wait while putting lines in shared memory" << endl;
          }
          // Wait until client is done processing the lines
          if (sem_wait(&store->sem2) == -1) {
            cout << "Error in sem_post while putting lines in shared memory" << endl;
          }
          // Clears the buffer
          memset(store->buffer, '\0', sizeof(store->buffer));
        }
        // Depending on the line offsets where it is in SHM
        memcpy(&store->buffer[(count%4)*1024], line, strlen(line)+1);
        count++;
      }
      // One last post to get the leftover lines over
      if (sem_post(&store->sem1) == -1) {
        cout << "Error in sem_wait while putting lines in shared memory" << endl;
      }
      if (sem_wait(&store->sem2) == -1) {
        cout << "Error in sem_post while putting lines in shared memory" << endl;
      }
      // Clears
      memset(store->buffer, '\0', sizeof(store->buffer));
      // Closes the file since all lines have been sent
      fclose(file);
      clog << "\tFILE CLOSED" << endl;
      // This means the file was invalid
    } else if (file == NULL) {
        // Sends thats the file was invalid to server
        memcpy(&store->buffer[0], "INVALID FILE", strlen("INVALID FILE")+1);
        // Tells the server that it can see what in the SHM
        if (sem_post(&store->sem1) == -1) {
          cout << "Error in sem_wait while putting lines in shared memory" << endl;
        }
    }

    // After everything is done, this tells the client to stop
    memcpy(&store->buffer[0], "STOP", strlen("STOP")+1);
    if (sem_post(&store->sem1) == -1) {
      cout << "Error with sem_wait when reading mem" << endl;
    }

    // Closes memory
    shmctl(shmid, IPC_RMID, NULL);
    munmap(store->buffer,4096);
    clog << "\tMEMORY CLOSED" << endl;
  }
}
