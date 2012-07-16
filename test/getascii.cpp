#include <cstdio>
#include <termios.h>
#include <unistd.h>
int main(){
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


	int c;
	while(c = getchar())
	{

		printf("0x%X\n", (int) c);
	}
}
