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

  shm_unlink("test.txt");
  int shmid = shm_open("test.txt", O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  if (shmid == -1) {
    cerr << "shm_open " << strerror(errno) << endl;
  } else {
    clog << "SERVER STARTED" << endl;
  }

  if (ftruncate(shmid, sizeof(struct shmbuf)) == -1) {
    cout << "Mem not truncated" << endl;
  } else {
    cout << "Mem truncated" << endl;
  }

  // May not need to be static casted
  struct shmbuf *store = static_cast<shmbuf*>(mmap(nullptr, sizeof(*store), PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0));
  if (store == MAP_FAILED) {
      cout << "MAP FAILED" << endl;
      cerr << "mmap " << strerror(errno) << endl;
  } else {
    cout << "Map casted" << endl;
  }
  
  if (sem_init(&store->sem1, 1, 0) == -1) {
    cout << "Error with sem_init on 1" << endl;
  }
  if (sem_init(&store->sem2, 1, 0) == -1) {
    cout << "Error with sem_init on 2" << endl;
  }

  if (sem_wait(&store->sem1) == -1) {
    cout << "sem_wait error";
  }
  clog << "CLIENT REQUEST RECIEVED" << endl;
  cout << "Read this: " << store->buffer << endl;

  char **keywords = (char**) malloc(sizeof(char*)* strlen(store->buffer)+1);
  char *arg = (char*) malloc(sizeof(char)* strlen(store->buffer)+1);
  char *op = (char*) malloc(sizeof(char) * 2);
  FILE* file;

  arg = strtok(store->buffer, "_");
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
  while (true) {
    if (file != NULL) {
      size_t len = 0;
      char *line = NULL;
      while ((getline(&line, &len, file)) != -1) {
        
      }
    }
  }

  shm_unlink("test.txt");
}
