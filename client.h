// Copyright 2023 William Novak-Condy
#include <string>
using namespace std;

#ifndef CLIENT_H_
#define CLIENT_H_
int client (int argc, char * argv[]);

void* checkLine (void *threadarg);
#endif // CLIENT_H_