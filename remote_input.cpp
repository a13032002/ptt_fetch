#include <cstdio>
#include "remote.h"
#include <termios.h>
#include <unistd.h>

int main()
{

    struct termios term;

    if (tcgetattr(STDIN_FILENO, &term) < 0)
    {
        puts("tcgetattr failed");
        return -1;
    }

    term.c_lflag &= ~(ECHO | ICANON);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) < 0)
    {
        puts("tcsetattr failed");
        return -1;
    }
    FILE *fptr;
    if ((fptr = fopen(REMOTE_INPUT, "w")) == NULL){
        puts("Cannot open file");
        return -2;
    }
    setvbuf(fptr, NULL, _IONBF, 0);
    puts("ok");
    char keyboard[1000];
    while(1)
    {
        int charRead = read(STDIN_FILENO, keyboard, 1000);
        for (int i = 0; i < charRead; i++)
        {
            if (keyboard[i] == 0x0A)
                keyboard[i] = 0x0D;
            fputc(keyboard[i], fptr);
            printf("get %02X\n", keyboard[i]);
        }
        
    }


}
