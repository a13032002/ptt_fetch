#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "bbs.h"
#include "bbs_err.h"
int main()
{
    BBS bbs;
    if (bbs.connect("ptt.cc") != BBS_ERROR_SUCCESS)
    {
        printf("bbs cannot connect\n");
        return 0;
    }

    while (1)
    {

        int ret = bbs.readPage();
        char s[1000];
        char *ptr = (char *)bbs.getCurrentPage();
        printf("bbs readPage return %d\n", ret);
        for (int i = 0; i < ret; i++)
            putchar(*ptr++);
        fgets(s, 1000, stdin);
        int len = strlen(s) - 1;
        if (len == 0)
            bbs.send((unsigned char *)"\r", 1);
        else
            bbs.send((unsigned char*) s, len);

        
        fflush(NULL);
    }



}
