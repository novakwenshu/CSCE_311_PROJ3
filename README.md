# CSCE_311_PROJ3

# server.h:
This is the header file for server.cc which are linked together later in the make file. Provides the method prototype for the server function that is implemented in client.cc.

# server.cc
First, it prints out that the server has started. Then it enters a while true loop. It checks it a namesem with the hard coded name exists and then opens one. Then it waits on the client to initialize the shared memory. Once initialized, it opens and maps the memory. Then it gets the name of the file and opens it. It then tells the client that it has read the name. Then it sends lines of the file by placing them into shared memory. It make sure there is an offset and that it keeps track of where it is. Every time 4 lines have been put into the memory it signals the client to read them and waits until it recvieves a signal that its finished. After each round it clears the memory to make sure there are no conflicts. Then after it is done, it closes the file. If the file could not be opened, it sends "INVALID FILE" to the client. Once the file is closed, it sends "STOP" to the client. Finally it destroys the memory.

# client.h
This is the header file for client.cc which are later linked together in the make file. Provides the prototype for the client function which is implemented in client.cc. Additionally, it provides the protoype for the checkLine function which is used by the threads to check if it should print a line.

# client.cc
This file contains the definition and implementation of the client function and checkLine functions. The client function contains code to create shared memory and read lines from that memory to see if it fits the parametes given in the command line. It takes in an integer and a double character pointer as parameters. First, it defines a struct called thread_info which will hold information that the threads will need for the code they run. Then it unlinks the hard coded name for the shared memory just incase to avoid errors. Then is opens the shared memory, truncates it, and then maps its to a char* called store. Then, it initializes the 2 semaphores which are in shared mem from the shared memory struct. Then is opens the named semaphore with a hard coded name from ther server function. Then it posts the named semaphore which tells the server that the shared memory has been initialized. After this, it copies the file name from the command line and stores it in the shared memory. Then it waits until the server posts the 1st semaphore which means that it has recieved the file name. Then it creates an array of 4 threads and a corresponding array of thread infos. Then it enters a while true loop and waits on the server to signal that lines have been sent. Then it stores those lines inside a vector of strings. For each line, a thread is called and checks if it is a valid line based on the parameters from the command line arguments. To summarize that process, it just runs through each line and depending on the operator, it checks id the line inclusively or exculsivle contains the keywords. During this, it also checks if the server has sent the signal to stop or that the file given was invalid. Moreover, it also makes sure the previous thread is done runing before finishing the next one to prevent a race condition. Finally, after all of that, it signals the server using the second semaphore that it is ready for more lines. Then once the server signals the client to stop, it destroys the memory and returns 0.

# bankloan1.csv
Contains the data that is to be searched and parsed by the program. Theis contain information about bank loans.

# bankloan2.csv
Contains the data that is to be searched and parsed by the program. This is a larger file. This contain information about bank loans.

# cmain.cc
This file is linked with client.h and client.cc through the make file. This allows client.cc to interact with command line arguments.

# smain.cc
This file is linked with server.h and server.cc through the make file. This allows server..c to interact with command line arguments. Additionally keeps the main pristine.

# makefile
This creates object files and links server.h, smain.cc, and server.cc to make an executable csv-server. This can be done by running the command "make csv-server". It also does the same with client.h, cmain.cc, and client.cc to make an executable csv-client. This can be made by running the command "make csv-client". Using "make clean" it deletes any intermediary files and executables.

# shmstruct.h
Contains the definition and implementation of a struct called "shmbuf". It contains two pointers to semaphores, a size, an int, and a character array. These attributes are used to keep track of data related to the shared memory.