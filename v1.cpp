#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "bbs.h"
#include "bbs_err.h"
#include "remote.h"
#include <unistd.h>
BBS bbs;
FILE *dump;
void printPage()
{
    int ret = bbs.readPage();
#ifdef OUTPUT_ORIGIN
    char *ptr = (char *)bbs.getCurrentPage();
    fwrite(ptr, ret, 1, dump);
    for (int i = 0; i < ret; i++)
    {
        if (*ptr == 0x0A)
            putchar('\v');
        else
            putchar(*ptr);
        ptr++;
    }
    
#else

    char *ptr = (char *)bbs.getCurrentPage();
    for(int i = 0; i < LINE_PER_PAGE;i++)
    {
        for (int j = 0; j < CHAR_PER_LINE; j++)
        {
            putchar(*ptr);
            ptr++;
        }
        if( i != LINE_PER_PAGE - 1) 
            putchar('\n');
    }
#endif

}
int main()
{
    dump = fopen("dump", "w");
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
