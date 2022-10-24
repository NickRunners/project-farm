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

#include <utilities.h>
#include <util.h>


int parCheck(int* n, int* q, useconds_t* t, char* s[]) {
  if(strncmp(s[1], "-n", strlen("-n")) == 0) {
    *n = atoi(s[2]);
    if(strncmp(s[3], "-q", strlen("-q")) == 0) {
      *q = atoi(s[4]);
      if(strncmp(s[5], "-t", strlen("-t")) == 0) {
        *t = atoi(s[6]);
        return 7;
      }
      else {
        *t = 0;
        return 5;
      }
    }
    else {
      *q = 8;
      *t = 0;
      return 3;
    }
  }
  else if(strncmp(s[1], "-q", strlen("-q")) == 0) {
    *n = 4;
    *q = atoi(s[2]);
    if(strncmp(s[3], "-t", strlen("-t")) == 0) {
      *t = atoi(s[4]);
      return 5;
    }
    else {
      *t = 0;
      return 3;
    }
  }
  else if(strncmp(s[1], "-t", strlen("-t")) == 0) {
    *n = 4;
    *q = 8;
    *t = atoi(s[2]);
    return 3;
  }
  else {
    *n = 4;
    *q = 8;
    *t = 0;
    return 1;
  }
}
