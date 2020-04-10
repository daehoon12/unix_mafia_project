#include <signal.h>
#include <unistd.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define MAX_PEOPLE 10
#define MEMORY_SIZE 10000
#define PORTNUM 9004

struct mymsgbuf {
	long mtype;
	char mtext[800];
};


enum Jobs{
	Host =0,
	Mafia,
	Citizen
};
