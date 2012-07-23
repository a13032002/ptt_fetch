#include <cstdio>
#include <string>
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

    int ret = bbs.readPage();

    printf("bbs readPage return %d\n", ret);

    printf("%s", bbs.getCurrentPage());



}
