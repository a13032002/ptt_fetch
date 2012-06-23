#include <iostream>
#include <cstdio>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>


using namespace std;

int main(int argc, char* argv [])
{
	int fdSock = socket(AF_INET, SOCK_STREAM, 0);

	if (fdSock < 0)
	{
		perror("socket () failed");
		return -1;
	}

	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(23);
	if (inet_pton(AF_INET, "140.112.172.12", &addr.sin_addr) != 1)
	{
		perror("inet_pton failed");
		return -1;
	}
	if (connect(fdSock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
	{
		perror("connect failed");
		return -1;
	}
	
	unsigned char buffer[4096];
	int len;
	/*
	len = recv(fdSock, buffer, sizeof(buffer), 0);
	for (int i = 0;i < len; i++)
	{
		if(buffer[i] == 0xFF)
			cout << "IAC recv" << endl;

		//cout << ((int)buffer[i]) << endl;
	}
	*/
	
	while ((len = recv(fdSock, buffer, sizeof(buffer), 0)))
	{
		for (int i = 0;i < len; i++)
			if(buffer[i] == 0xFF)
				cout << "IAC recv" 
				<< " I = " << (int)buffer[i+1] 
				<< " O = " << (int)buffer[i+2]
				<< endl;

		buffer[len] = 0;
//		cout << buffer;
	}

	return 0;
}
