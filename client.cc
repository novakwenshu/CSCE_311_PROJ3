// Copright 2023 William Novak-Condy
#include <iostream>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include <cstring>

#define SIZE 4096

using namespace std;
 
int main (int argc, char *argv[]) {

  key_t key = ftok("test.txt", 40);
  int shmid = shmget(key, SIZE, 0644 | IPC_CREAT); 
  char *mem_block = (char*) shmat(shmid, (void*)0,0);

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
  // Sends the important info from agrv to the shared memory
  strncpy(mem_block, args, SIZE);


  //shmctl(shmid, IPC_RMID, NULL);
}