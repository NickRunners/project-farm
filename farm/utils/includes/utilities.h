#if !defined(_UTILITIES_H)
#define _UTILITIES_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/select.h>
#include <arpa/inet.h>

// check function return value
#define EC(fun, msg) {if(fun == -1) {fprintf(stderr, msg); exit(EXIT_FAILURE);}}

// assign optional arguents values 
// it returns index to start reading from in file list
int parCheck(int* n, int* q, useconds_t* t, char* s[]);



#endif
