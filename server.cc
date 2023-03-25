// Copright 2023 William Novak-Condy
#include <iostream>
#include <sys/ipc.h>
#include <stdio.h>
#include <sys/shm.h>
#include <cstring>

using namespace std;

#define SIZE 4096

int main (int argc, char *argv[]) {
  key_t key = ftok("test.txt", 0);
  int shmid = shmget(key, SIZE, 0666|IPC_CREAT);
  char *mem_block = (char*) shmat(shmid,  (void*)0,0);

  cout << "Read this: " << mem_block << endl;

  char **keywords = (char**) malloc(sizeof(char*)* strlen(mem_block)+1);
  char *arg = (char*) malloc(sizeof(char)* strlen(mem_block)+1);
  char *op = (char*) malloc(sizeof(char) * 2);
  FILE* file;

  arg = strtok(mem_block, "_");
  int count = 0;
  while (arg != NULL) {
    keywords[count] = arg;
    arg = strtok(NULL, "_");
    count++;
  }
  if (count > 3) {
    op = keywords[2];
  }

  // Testing
  for (int i = 0; i < count; i++) {
    cout << keywords[i] << endl;
  }

  file = fopen(keywords[0], "r");
  if (file != NULL) {
    size_t len = 0;
    char *line = NULL;
    
    while ((getline(&line, &len, file)) != -1) {

    }
  }

  shmdt(mem_block);
}
