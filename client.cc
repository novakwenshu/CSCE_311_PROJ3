// Copright 2023 William Novak-Condy
#include <iostream>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <cstring>
#include <string>
#include <vector>
#include "shmstruct.h"
#include <client.h>

#define SIZE 4096
#define SEMNAME "SHMSEM"

using namespace std;

// SHM = Shared Memory
// Struct to hold info for the method called by the threads
struct thread_info {
  char **str;
  string line;
  int argNum;
  int *lineNum;
};

int client (int argc, char *argv[]) {

  // Makes sure there is no existing memory
  shm_unlink("test.txt");
  // STEP 1 Creates the shared memory and truncates it
  int shmid = shm_open("test.txt", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  if (shmid == -1) {
    cout << "shm_open fail" << endl;
  }

  if (ftruncate(shmid, sizeof(struct shmbuf)) == -1) {
    cout << "Mem not truncated" << endl;
  }
  // Maps the memory
  struct shmbuf *store = static_cast<shmbuf*>(mmap(NULL, sizeof(*store), PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0));
  if (store == MAP_FAILED) {
    cout << "MAP FAILED" << endl;
  }
  // Initializes the semaphores inside the struct from shmstruct.h
  if (sem_init(&store->sem1, 1, 0) == -1) {
    cout << "Error with sem_init on 1" << endl;
  }
  if (sem_init(&store->sem2, 1, 0) == -1) {
    cout << "Error with sem_init on 2" << endl;
  }

  // Opens the named semaphore created by the server
  sem_t *sem = sem_open("/namesem", 0, 0, 0);

  store->count = strlen(argv[1]);
  // tell the client that it initialized the SHM
  if (sem_post(sem) == -1) {
    cout << "Named sem post error" << endl;
  }
  // STEP 2 copies the file name into the SHM
  memcpy(&store->buffer, argv[1], strlen(argv[1]));
  // Waits until the server has opened the file to proceed
  if (sem_wait(&store->sem1) == -1) {
    cout << "sem_wait error" << endl;
  }

  pthread_t threads[NUMTHREADS];
  struct thread_info td[NUMTHREADS];
  void *res;
  int lineCount = 1;

  while (true) {
    // Waits for the server to copy lines from the file into the SHM
    if (sem_wait(&store->sem1) == -1) {
      cout << "Error with sem_wait when reading mem" << endl;
    }
    // STEP 3 Obtains 4 or less lines from SHM and puts them in a vector
    vector<string> lines;
    for (int i = 0; i < 4; i++) {
      char line[BUF_SIZE];
      strncpy(line,&store->buffer[i*OFFSET], OFFSET);
      lines.push_back(line);
    }

    // Checks if the server sent the message that it could not open the file
    int badFile = lines.at(0).compare("INVALID FILE");
    if (badFile == 0) {
      cerr << "INVALID FILE" << endl;
      shmctl(shmid, IPC_RMID, NULL);
      munmap(store->buffer,SIZE);
      return 0;
    }
    // Checks if the server has signaled that there are no more lines left in the file to send
    int stop = lines.at(0).compare("STOP");
    if (stop == 0) {
      // STEP 6 Destroys mem
      shmctl(shmid, IPC_RMID, NULL);
      munmap(store->buffer,SIZE);
      // STEP 7 returns 0
      return 0;
    }
    
    // This loop ensures that the threads run in order
    for (int i = 0; i < 4; i++) {
      td[i].argNum = argc;
      td[i].str = argv;
      td[i].line = lines.at(i);
      td[i].lineNum = &lineCount;
      // if threads have already been created
      if (i != 0) {
        if (pthread_join (threads[i-1], &res) != 0) {
          cout << "Pthread join error" << endl;
        }
      }
      // STEP 4 Creating a thread for each line sent.
      int check = pthread_create(&threads[i], NULL, &checkLine, (void *)&td[i]);
      if (check) {
        cout << strerror(errno) << endl;
        cout << "Failed to create thread" << endl;
      }
      if (store->toBreak) {
        break;
      }
    }
    // Signals the server that it is ready for more lines
    if (sem_post(&store->sem2) == -1) {
      cout << "Error in sem_post while reading lines from shared memory" << endl;
    }
  }
  // If some how it breaks the while loop it destoys the memory
  shmctl(shmid, IPC_RMID, NULL);
  munmap(store->buffer,SIZE);
  return 0;
}

// Method that the threads call to check if the line should be print out
void* checkLine (void *threadarg) {
  struct thread_info *data;
  data = (struct thread_info *) threadarg;
  if (data->argNum > 3) {
    // If the operator is OR
    if (data->str[3][0] == '+') {
      for (int i = 2; i < data->argNum; i++) {
        // The key words will only be in the even indices
        if(i%2 == 0) {
          // For each keyword it checks if it is contained in the string.
          // If found, it breaks the loop to avoid duplicates
          if (data->line.find(data->str[i]) != string::npos) {
            // STEP 5 Printing lines to cout if meets the criteria based on operator
            cout << *(data->lineNum) << "\t";
            cout << data->line << endl;
            *(data->lineNum) = *(data->lineNum) + 1;
            break;
          }
        }
      }
        // If the operator is AND
    } else if (data->str[3][0] == 'x') {
      bool toSend = true;
      for (int i = 2; i < data->argNum; i++) {
        if (i%2 == 0) {
          // If even one of the keywords in not in the line,
          // it tells the program to not send anything
          if (data->line.find(data->str[i]) == string::npos) {
            // Tracker to see if it should send
            toSend = false;
            break;
          }
        }
      }
      if (toSend) {
        // STEP 5 Printing lines to cout if meets the criteria based on operator
        cout << *(data->lineNum) << "\t";
        cout << data->line << endl;
        *(data->lineNum) = *(data->lineNum) + 1;
      }
    }
    // If there is only one argument given
  } else if (data->argNum == 3) {
    if ((data->line.find(data->str[2])) != string::npos) {
      // STEP 5 Printing lines to cout if meets the criteria based on operator
      cout << *(data->lineNum) << "\t";
      cout << data->line << endl;
      *(data->lineNum) = *(data->lineNum) + 1;
    }
  }
  return NULL;
}