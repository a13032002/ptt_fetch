#include <cstdio>
#include "remote.h"

int main()
{
    FILE *fptr = fopen(REMOTE_OUTPUT, "r");
    setvbuf(fptr, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    int  c;
    while((c = fgetc(fptr)) != EOF)
    {
        putchar(c);
    }
}
