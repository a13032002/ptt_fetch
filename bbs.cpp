#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <cstring>
#include <cstdio>

#include <unistd.h>
#include "bbs.h"
#include "bbs_err.h"

    BBS::BBS()
: m_fdSock(-1)
{
}

BBS::~BBS()
{
    if (m_fdSock != -1)
        close(m_fdSock);
}

int BBS::connect(const char *host, int port /* = 23 */)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
        return BBS_ERROR_FUNCTION_SOCKET_FAILED;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    struct hostent *ph;
    if (!(ph = gethostbyname(host)))
        return BBS_ERROR_FUNCTION_GETHOSTENT_FAILED;

    memcpy(&addr.sin_addr, ph->h_addr, ph->h_length);
    if (::connect(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0)
        return BBS_ERROR_FUNCITON_CONNECT_FAILED;

    m_fdSock = sock;
    ::send(sock, "a13032002\r", 10, 0);
    return BBS_ERROR_SUCCESS;
}

int BBS::readPage()
{
    if (m_fdSock == -1)
        return BBS_ERROR_NOT_CONNECTED;

    unsigned char aytCmd[2] = {0xFF, 246};
    int idx = 0;
    fd_set fdRead;
    timespec waitTime;
    waitTime.tv_sec = 0;
    waitTime.tv_nsec = 10e7;

    //unsigned char buf[4096];
    //printf("m_sock = %d\n", m_fdSock);
    if (::send(m_fdSock, aytCmd, sizeof(aytCmd), 0) != 2)
        return BBS_ERROR_FUNCTION_SEND_FAILED;

    int miss_count = 0;

    while (1)
    {
        FD_ZERO(&fdRead);
        FD_SET(m_fdSock, &fdRead);

        int fdAvailable = pselect(m_fdSock + 1, &fdRead, NULL, NULL, &waitTime, NULL);
        if (fdAvailable < 0)
            return BBS_ERROR_INTERNAL_ERROR;

        int len;
        if (fdAvailable == 0)
        {
            printf("enter fail testing\n");
            if (miss_count == MAX_MISS_COUNT
            miss_count++;
            if (idx >= 14 && !strncmp( (char *)m_szBuffer + idx - 14, ",ack:0(-0)  ", 12))
            {
                printf("idx a = %d\n", idx);
                idx -= 13;
                while (strncmp((char *)m_szBuffer + idx, "  (#0)fd:", 9))
                    idx--;
                m_szBuffer[idx] = 0;
                printf("idx b = %d\n", idx);
                break;

            }
            else if(idx >= 18 && !strncmp( (char *)m_szBuffer + idx - 18, "I'm still alive.", 16))
            {
                idx -= 18;
                break;
            }
            else
                continue;
        }
        else if(FD_ISSET(m_fdSock, &fdRead)){

            len = recv(m_fdSock, m_szBuffer + idx, INTERNAL_BUF_LEN - idx, 0);
            idx += len;
            printf("recv %d bytes\n", len);
        }
    }
    return idx;

}

const unsigned char * BBS::getCurrentPage()
{
    return m_szBuffer;
}


int BBS::send(unsigned char *data, int len)
{
    if (m_fdSock == -1)
        return BBS_ERROR_NOT_CONNECTED;
    if (::send(m_fdSock, (char *)data, len, 0) != len)
        return BBS_ERROR_FUNCTION_SEND_FAILED;
    return BBS_ERROR_SUCCESS;
}
