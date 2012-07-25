#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "bbs.h"
#include "bbs_err.h"
#include "remote.h"
#include <unistd.h>
BBS bbs;
void printPage()
{
    int ret = bbs.readPage();
    char *ptr = (char *)bbs.getCurrentPage();
    for (int i = 0; i < ret; i++)
    {
        if (*ptr == 0x0A)
            putchar('\v');
        else
            putchar(*ptr);
        ptr++;
    }

}
int main()
{
	setvbuf(stdout, NULL, _IONBF, 0);

    if (bbs.connect("ptt.cc") != BBS_ERROR_SUCCESS)
    {
        printf("bbs cannot connect\n");
        return 0;
    }
    int ret = remote_input(bbs);
    printf("remote_input return %d\n", ret);
    ret = remote_output_init();
    printf("remote_output_init return %d\n", ret);
    printPage();
    
    while(1)
    {
        pause();
    }
}
