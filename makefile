client.o : client.cc client.h shmstruct.h
	g++ -std=c++17 -Wall -I . -c client.cc
server.o : server.cc server.h shmstruct.h
	g++ -std=c++17 -Wall -I . -c server.cc
smain.o : smain.cc
	g++ -std=c++17 -Wall -I . -c $<
cmain.o : cmain.cc
	g++ -std=c++17 -Wall -I . -c $<
server : server.o smain.o
	g++ -std=c++17 -Wall -I . -o server smain.o server.o
client : client.o cmain.o
	g++ -std=c++17 -Wall -I . -o client cmain.o client.o
clean :
	rm *.o

