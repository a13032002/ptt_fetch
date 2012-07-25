#include "remote.h"
#include "bbs.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <pthread.h>
#include <unistd.h>
FILE *fptr = NULL;
FILE *ofptr = NULL;
BBS* _bbs;
extern void printPage();

void *fetch(void *arg)
{
    char c;
    while(1)
    {
        c = fgetc(fptr);
        if (c == 0x0A)
            c = 0x0D;
        _bbs->send((unsigned char*)&c, 1);
        printPage();
    }

}

int remote_input(BBS& b)
{
    _bbs = &b;

    unlink(REMOTE_INPUT);
    if (mkfifo(REMOTE_INPUT, 0777) != 0)
        return -1;

    if ((fptr = fopen(REMOTE_INPUT, "r")) == NULL)
        return -2;
    setvbuf(fptr, NULL, _IONBF, 0);


    pthread_t thid;
    if (pthread_create(&thid, NULL, fetch, NULL) != 0)
        return -3;

    return 0;
}


int remote_output_init()
{
    unlink(REMOTE_OUTPUT);
    if (mkfifo(REMOTE_OUTPUT, 0777) != 0)
        return -1;

    if ((ofptr = fopen(REMOTE_OUTPUT, "w")) == NULL)
        return -2;

    setvbuf(ofptr, NULL, _IONBF, 0);
    return 0;
}
