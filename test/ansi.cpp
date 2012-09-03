#include <cstdio>

int main()
{
    FILE *ptr = fopen("ansi.code", "r");
    if (!ptr)
    {
        puts("cannot open file");
        return -1;
    }
    
    int c;
    while (fscanf(ptr, "%02X", &c) == 1)
        putchar(c);


}
