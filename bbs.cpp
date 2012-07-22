#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <cstring>

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
    return BBS_ERROR_SUCCESS;
}

int BBS::readPage()
{
    if (m_fdSock == -1)
        return BBS_ERROR_NOT_CONNECTED;
    
    int idx = 0;
    fd_set fdRead;
    timespec waitTime;
    waitTime.tv_sec = 0;
    waitTime.tv_nsec = 10e7;

    //unsigned char buf[4096];
    while (1)
    {
        FD_ZERO(&fdRead);
        FD_SET(m_fdSock, &fdRead);
        
        int fdAvailable = pselect(m_fdSock + 1, &fdRead, NULL, NULL, &waitTime, NULL);
        if (fdAvailable < 0)
            return BBS_ERROR_INTERNAL_ERROR;
        if (fdAvailable == 0)
        {
            //TO DO: send SYT to check;
            //return BBS_ERROR_SUCCESS;
            break;
        }
        int len;
        len = recv(m_fdSock, m_szBuffer + idx, INTERNAL_BUF_LEN - idx, 0);
    }
    return BBS_ERROR_SUCCESS;

}

const unsigned char * BBS::getCurrentPage()
{
    return m_szBuffer;
}
