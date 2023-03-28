// Copright 2023 William Novak-Condy
#include <iostream>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <cstring>
#include "shmstruct.h"

#define SIZE 4096
#define SEMNAME "SHMSEM"

using namespace std;

int main (int argc, char *argv[]) {

  int shmid = shm_open("test.txt", O_RDWR, 0);
  if (shmid == -1) {
    cout << "shm_open fail" << endl;
  } else {
    cout << "Shared Memory Created" << endl;
  }

  struct shmbuf *store = static_cast<shmbuf*>(mmap(NULL, sizeof(*store), PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0));
  if (store == MAP_FAILED) {
    cout << "MAP FAILED" << endl;
  }
  int arglen = 0;

  for (int i = 1; i < argc; i++) {
    arglen += strlen(argv[i] + 1);
  }

  char *args = (char*) malloc(sizeof(char)*arglen);
  strcpy(args, argv[1]);

  for (int i = 2; i < argc; i++) {
    strcat(args, "_");
    strcat(args, argv[i]);
  }
  strcat(args, "\0");
  // Potential error maybe +1 to count
  store->count = strlen(args);
  cout << "Args complete" << endl;
  // Sends the important info from agrv to the shared memory
  // Same thing here. May need to add +1 maybe not
  memcpy(&store->buffer, args, strlen(args));
  cout << store->buffer << endl;
  if (sem_post(&store->sem1) == -1) {
    cout << "sem_post error" << endl;
  }
  cout << "Wrote args to mem" << endl;
  
  

  shmctl(shmid, IPC_RMID, NULL);
  munmap(store->buffer,SIZE);
  return;
}