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

#define SIZE 4096
#define SEMNAME "SHMSEM"

using namespace std;

int main (int argc, char *argv[]) {

  shm_unlink("test.txt");
  int shmid = shm_open("test.txt", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  if (shmid == -1) {
    cout << "shm_open fail" << endl;
  } else {
    cout << "Shared Memory Created" << endl;
  }

  if (ftruncate(shmid, sizeof(struct shmbuf)) == -1) {
    cout << "Mem not truncated" << endl;
  } else {
    cout << "Mem truncated" << endl;
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

  sem_t *sem = sem_open("Test_sem", O_CREAT | O_EXCL, 0644, 0);
  char *op = (char*) malloc(sizeof(char) * 2);
  
  // Potential error maybe +1 to count
  store->count = strlen(argv[1]);
  cout << "Args complete" << endl;

  cout << "test" << endl;
  if (sem_post(sem) == -1) {
    cout << "Named sem post error" << endl;
  }
  // Sends the important info from agrv to the shared memory
  // Same thing here. May need to add +1 maybe not
  cout << "test" << endl;
  memcpy(&store->buffer, argv[1], strlen(argv[1]));
  cout << store->buffer << endl;
  if (sem_wait(&store->sem1) == -1) {
    cout << "sem_wait error" << endl;
  }
  cout << "Wrote args to mem" << endl;

  while (true) {
    cout << "test" << endl;
    if (sem_wait(&store->sem2) == -1) {
      cout << "Error with sem_wait when reading mem" << endl;
    }
    vector<string> lines;
    for (int i = 0; i < 4; i++) {
      char line[BUF_SIZE];
      strncpy(line,&store->buffer[i*OFFSET], OFFSET);
      lines.push_back(line);
    }
    for (int i = 0; i < 4; i++) {
      cout << lines.at(i) << endl;
    }

  }
  
  shmctl(shmid, IPC_RMID, NULL);
  munmap(store->buffer,SIZE);
  return 0;
}


void checkLine (char *str[], char *line, int argNum) {
  if (str[3][0] == '+') {
    for (int i = 2; i < argNum; i++) {
      // For each keyword it checks if it is contained in the string.
      // If found, it breaks the loop to avoid duplicates
      if (strstr(line, str[i]) != NULL) {
         cout << line << endl;
      }
    }
        // If the operator is AND
  } else if (str[3][0] == 'x') {
    bool toSend = true;
    for (int i = 2; i < argNum; i++) {
      // If even one of the keywords in not in the line,
      // it tells the program to not send anything
      if (strstr(line, str[i]) == NULL) {
        // Tracker to see if it should send
        toSend = false;
      }
    }
    if (toSend) {
      cout << line << endl;
    }
  // If there is only one argument given
  } else if (argNum <= 3) {
    if (strstr(line, str[1])) {
      cout << line << endl;
    }
  }
}