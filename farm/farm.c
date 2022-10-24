#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <util.h>
#include <utilities.h>
#include <boundedqueue.h>

#define N 100
#define UNIX_PATH_MAX 108
#define SOCKNAME "./farm.sck"
#define END "qwertyuiopasdfghjklzxcvbnm"
volatile sig_atomic_t flag = 0;
volatile sig_atomic_t flagP = 0;

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void gestore(int sig) {
    flag = 1;
}

void gestoreP(int sig) {
    flagP = 1;
}

void* Worker(void* arg) {
    BQueue_t* q = (BQueue_t*)arg;
    int fd_client, count, nread, check;
    long tot, num;
    char buff[N];
    struct sockaddr_un sa;

    strncpy(sa.sun_path, SOCKNAME, UNIX_PATH_MAX);
    sa.sun_family = AF_UNIX;

    while(1) {
        char* temp = (char*)pop(q);
        strcpy(buff, " ");

        // connect to server
        fd_client = socket(AF_UNIX, SOCK_STREAM, 0);

        while(connect(fd_client, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
            if(errno == ENOENT)
                sleep(1);
            else
                exit(EXIT_FAILURE);
        }

        // calculation on file
        if(strcmp(temp, END) != 0) {
            count = 0;
            tot = 0;
            FILE* file;
            file = fopen(temp, "r");
            if(file == NULL) {
                perror("fopen");
                exit(EXIT_FAILURE);
            }
            while(!feof(file)) {
                nread = fread(&num, sizeof(long), 1, file);
                if(nread != 0) {
                    tot += (count*num);
                    count++;
                }
            }
            sprintf(buff, "%ld", tot);
            strcat(buff, " ");
            strcat(buff, temp);
            fclose(file);
        }
        else {
            strcpy(buff, END);
            push(q, END);
        }

        //  write on socket, communication to server
        LOCK(&mtx);
        check = write(fd_client, buff, strlen(buff));
        // collector kill -> write fails
        if(check == -1 && flagP == 1) {
            close(fd_client);
            UNLOCK(&mtx);
            break;
        }
        close(fd_client);
        UNLOCK(&mtx);

        // check end elaboration string (END)
        if(strcmp(buff, END) == 0)
            break;
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    int nthread, qlen, ind;
    useconds_t delay;

    // initialization parameters
    ind = parCheck(&nthread, &qlen, &delay, argv);
    int nfile = argc - ind; //number of files
    int c = 0;  //number of END files received by server

    // signals handling
    struct sigaction s;
    sigset_t set;
    // masking of all signals while installing handlers
    EC(sigfillset(&set), "sigfillset1\n");
    EC(pthread_sigmask(SIG_SETMASK, &set, NULL),"sigmask\n");
    EC(sigemptyset(&set),"emptyset1\n");
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGQUIT);
	sigaddset(&set, SIGTERM);
    s.sa_mask = set;
    s.sa_handler = gestore;
    s.sa_flags = SA_RESTART;
    EC(sigaction(SIGINT, &s, NULL),"sigaction-SIGINT\n");
    EC(sigaction(SIGHUP, &s, NULL),"sigaction-SIGHUP\n");
    EC(sigaction(SIGQUIT, &s, NULL),"sigaction-SIGQUIT\n");
    EC(sigaction(SIGTERM, &s, NULL),"sigaction-SIGTERM\n");
    s.sa_handler = gestoreP;
    EC(sigaction(SIGPIPE, &s, NULL), "sigaction-SIGPIPE\n");
    // handlers installed, mask cleaning
    EC(sigemptyset(&set),"emptyset2\n");
    EC(pthread_sigmask(SIG_SETMASK, &set, NULL),"sigmask2\n");
    
    // MasterWorker and Collector creation
    pid_t pid = fork();
    // fork error
    if(pid == -1) {
        perror("fork");
        return -1;
    }
    // MasterWorker - client - son
    else if(pid == 0) {
        // initialization queue
        BQueue_t* q =  initBQueue(qlen);

        // thread client creation
        pthread_t tr[nthread];
        for(int i=0; i<nthread; i++) {
            if(pthread_create(&tr[i], NULL, &Worker, (void*)q) != 0) {
                fprintf(stderr, "pthread_create %d error\n", i);
                return -1;
            }
        }

        // filling queue with file names
        for(int i=ind; i<argc; i++) {
            if(isRegular(argv[i], NULL) == 1)
                push(q, (void*)argv[i]);
            if(i+1 != argc)
                usleep(delay*1000);
            if(flag == 1)
                break;
        }
        // insertion of END (end of file list string)
        push(q, END);

        // waiting for threads
        for(int i=0; i<nthread; i++) {
            if(pthread_join(tr[i], NULL) != 0) {
                fprintf(stderr, "pthread_join %d error\n", i);
                return -1;
            }
        }

        // deallocation
        pthread_mutex_destroy(&mtx);
        deleteBQueue(q, NULL);
        // deleting file.sck
        EC(unlink(SOCKNAME), "unlink\n");
    }
    // Collector - server - father
    else {
        char buff[N];
        int fd_server, maxS, temp, n, check;
        fd_set sockets, readSockets;
        struct sockaddr_un sa;

        strcpy(sa.sun_path, SOCKNAME);
        sa.sun_family = AF_UNIX;

        // socket creation and initialization
        fd_server = socket(AF_UNIX, SOCK_STREAM, 0);
        EC(bind(fd_server, (struct sockaddr*)&sa, sizeof(sa)), "bind\n");
        EC(listen(fd_server, SOMAXCONN), "listen\n");

        // initializing set
        FD_ZERO(&sockets);
        FD_SET(fd_server, &sockets);
        maxS = fd_server;

        // select
        while(1) {
		    readSockets=sockets;

            // error/interruption handling with reboot 
            while((check=select(maxS+1, &readSockets, NULL, NULL, NULL))==-1 && errno==EINTR);
            if(check == -1) {
                perror("select");
                exit(EXIT_FAILURE);
            }

            for (int i=0; i<=maxS; i++) {
                if(FD_ISSET(i, &readSockets)) {
                    if(i == fd_server) {
                        temp = accept(fd_server, NULL, 0);
                        EC(temp, "accept\n");
                        FD_SET(temp, &sockets);
                        if(maxS < temp)
                            maxS = temp;	
                    }
                    else {
                        n = read(i, buff, N);
                        EC(n, "read\n");
                        if(n == 0) {
                            FD_CLR(i, &sockets);
                            while(!FD_ISSET(maxS,&sockets))
                                maxS--;
                            close(i);
                        }
                        if(n > 0) {
                            buff[n]='\0';
                            if(strncmp(buff, END, strlen(END)) != 0)
                                printf("%s\n", buff);

                            // END (end of file list string) reception
                            else
                                c++;

                            // exit when all threads receive END
                            // and all of them end correctly
                            if(nthread == c)
                                break;
                        }
                    }
                }	
            }
            if(nthread == c)
                break;
        }
        // socket closure and waiting for MasterWorker
        close(fd_server);
        waitpid(0, NULL, 0);
    }

    return 0;
}