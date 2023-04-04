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

struct thread_info {
  char **str;
  string line;
  int argNum;
  int *lineNum;
};

int client (int argc, char *argv[]) {

  shm_unlink("test.txt");
  int shmid = shm_open("test.txt", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  if (shmid == -1) {
    cout << "shm_open fail" << endl;
  }

  if (ftruncate(shmid, sizeof(struct shmbuf)) == -1) {
    cout << "Mem not truncated" << endl;
  }

  struct shmbuf *store = static_cast<shmbuf*>(mmap(NULL, sizeof(*store), PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0));
  if (store == MAP_FAILED) {
    cout << "MAP FAILED" << endl;
  }

  if (sem_init(&store->sem1, 1, 0) == -1) {
    cout << "Error with sem_init on 1" << endl;
  }
  if (sem_init(&store->sem2, 1, 0) == -1) {
    cout << "Error with sem_init on 2" << endl;
  }
  

  sem_t *sem = sem_open("/namesem", 0, 0, 0);

  store->count = strlen(argv[1]);

  if (sem_post(sem) == -1) {
    cout << "Named sem post error" << endl;
  }
  // Sends the important info from agrv to the shared memory
  memcpy(&store->buffer, argv[1], strlen(argv[1]));
  if (sem_wait(&store->sem1) == -1) {
    cout << "sem_wait error" << endl;
  }

  pthread_t threads[NUMTHREADS];
  struct thread_info td[NUMTHREADS];
  void *res;
  int lineCount = 1;

  while (true) {
    if (sem_wait(&store->sem1) == -1) {
      cout << "Error with sem_wait when reading mem" << endl;
    }
    
    vector<string> lines;
    for (int i = 0; i < 4; i++) {
      char line[BUF_SIZE];
      strncpy(line,&store->buffer[i*OFFSET], OFFSET);
      lines.push_back(line);
    }

    int badFile = lines.at(0).compare("INVALID FILE");
    if (badFile == 0) {
      cerr << "INVALID FILE" << endl;
      shmctl(shmid, IPC_RMID, NULL);
      munmap(store->buffer,SIZE);
      return 0;
    }

    int stop = lines.at(0).compare("STOP");
    if (stop == 0) {
      shmctl(shmid, IPC_RMID, NULL);
      munmap(store->buffer,SIZE);
      return 0;
    }
    
    for (int i = 0; i < 4; i++) {
      td[i].argNum = argc;
      td[i].str = argv;
      td[i].line = lines.at(i);
      td[i].lineNum = &lineCount;

      if (i != 0) {
        if (pthread_join (threads[i-1], &res) != 0) {
          cout << "Pthread join error" << endl;
        }
      }

      int check = pthread_create(&threads[i], NULL, &checkLine, (void *)&td[i]);
      if (check) {
        cout << strerror(errno) << endl;
        cout << "Failed to create thread" << endl;
      }
      if (store->toBreak) {
        break;
      }
    }
    
    if (sem_post(&store->sem2) == -1) {
      cout << "Error in sem_post while reading lines from shared memory" << endl;
    }
  }
  
  shmctl(shmid, IPC_RMID, NULL);
  munmap(store->buffer,SIZE);
  return 0;
}


void* checkLine (void *threadarg) {
  struct thread_info *data;
  data = (struct thread_info *) threadarg;
  if (data->argNum > 3) {
    if (data->str[3][0] == '+') {
      for (int i = 2; i < data->argNum; i++) {
        if(i%2 == 0) {
          // For each keyword it checks if it is contained in the string.
          // If found, it breaks the loop to avoid duplicates
          if (data->line.find(data->str[i]) != string::npos) {
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
        cout << *(data->lineNum) << "\t";
        cout << data->line << endl;
        *(data->lineNum) = *(data->lineNum) + 1;
      }
    }
    // If there is only one argument given
  } else if (data->argNum == 3) {
    if ((data->line.find(data->str[2])) != string::npos) {
      cout << *(data->lineNum) << "\t";
      cout << data->line << endl;
      *(data->lineNum) = *(data->lineNum) + 1;
    }
  }
  return NULL;
}