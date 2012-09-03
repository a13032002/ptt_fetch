#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <cstring>
#include <cstdio>
#include <cassert>

#include <unistd.h>
#include "bbs.h"
#include "bbs_err.h"

#ifdef REMOTE_OUTPUT
extern FILE *ofptr;
#define DEBUG_PRINT(format, ...) fprintf(ofptr, format, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(format, ...) 
#endif
char mybuf[1000];

    BBS::BBS()
: m_fdSock(-1), m_nBufLen(0)
{
    m_nStartRow = BUFFER_RATIO * LINE_PER_PAGE / 2;
    m_i = m_j = 0;
    for (int i = 0; i < LINE_PER_PAGE * BUFFER_RATIO; i++)
        for (int j = 0; j < CHAR_PER_LINE; j++)
            m_page[i][j] = 0x20;

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

    unsigned char aytCmd[2] = {0xFF, 246};
    int idx = 0;
    fd_set fdRead;
    timespec waitTime;
    waitTime.tv_sec = 0;
    waitTime.tv_nsec = 10e7;

    DEBUG_PRINT("enter readPage\n", "");
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
            DEBUG_PRINT("Testing End miss count : %d\n", miss_count);
            if (miss_count == MAX_MISS_COUNT)
            {
                ::send(m_fdSock, aytCmd, sizeof(aytCmd), 0);
            }
            else
                miss_count++;

            if (idx >= 14 && !strncmp( (char *)m_szBuffer + idx - 14, ",ack:0(-0)  ", 12))
            {
                DEBUG_PRINT(mybuf, "idx a = %d\n", idx);
                
                idx -= 13;

                while (strncmp((char *)m_szBuffer + idx, "  (#0)fd:", 9))
                    idx--;

                m_szBuffer[idx] = 0;

                DEBUG_PRINT("idx b = %d\n", idx);

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
            miss_count = 0;
            len = recv(m_fdSock, m_szBuffer + idx, INTERNAL_BUF_LEN - idx, 0);
            idx += len;
            DEBUG_PRINT( "recv %d bytes\n", len);
        }
    }

    m_nBufLen = idx;
    parse();

    return idx;

}

const unsigned char * BBS::getCurrentPage()
{
#ifdef OUTPUT_ORIGIN
    return m_szBuffer;
#else
    return m_page[m_nStartRow];
#endif
}


int BBS::send(unsigned char *data, int len)
{
    if (m_fdSock == -1)
        return BBS_ERROR_NOT_CONNECTED;
    if (::send(m_fdSock, (char *)data, len, 0) != len)
        return BBS_ERROR_FUNCTION_SEND_FAILED;
    return BBS_ERROR_SUCCESS;
}
int BBS::isAlphabet(char c)
{
    if ( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
        return 1;
    return 0;
}

int BBS::parse()
{
    DEBUG_PRINT("Parsing.....\n");
    for (unsigned char *ptr = m_szBuffer, *end = ptr + m_nBufLen; ptr < end;)
    {
        unsigned char c = *ptr;
        unsigned char nc = *(ptr + 1);
        if (c >= 0x81 && c <= 0xFE && ((nc >= 0x40 && c <= 0x7E) || (nc >= 0xA1 && nc <= 0xFE)))
        {
            assert(m_i >= 0 && m_i < LINE_PER_PAGE && m_j >= 0  && m_j < CHAR_PER_LINE);
            m_page[m_nStartRow + m_i][m_j++] = c;
            assert(*(ptr + 1) != 0x1B);
            m_page[m_nStartRow + m_i][m_j++] = *(ptr + 1);

            ptr += 2;
        }
        else if (c == 0x1B)
        {
            int delta = parseVTcommand(ptr, end);
            ptr += delta;
        }
        else if (c == 0x0D)
        {
            ptr++;
            m_j = 0;
        }
        else if (c == 0x0A)
        {
            if (m_i == LINE_PER_PAGE - 1)
                m_nStartRow++;
            else
                m_i++;
            ptr++;
        }
        else if (c == 0x07)
            ptr++;
        else if (c == 0x08)
        {
            m_j--;
            assert(m_i >= 0 && m_i < LINE_PER_PAGE && m_j >= 0  && m_j < CHAR_PER_LINE);
            ptr++;
        }
        else if (c == 0x0B || c == 0x0C)
        {
            assert(false);
        }
        else 
        {
            assert(m_i >= 0 && m_i < LINE_PER_PAGE && m_j >= 0  && m_j < CHAR_PER_LINE);
            m_page[m_nStartRow + m_i][m_j++] = c;
            ptr++;
        }
    }
    DEBUG_PRINT("End Parsing... Start Row : %d, i : %d, j : %d\n", m_nStartRow, m_i, m_j);
}

int BBS::parseVTcommand(unsigned char *start, unsigned char* bound)
{
    unsigned char next = *(start + 1);

    DEBUG_PRINT(mybuf, "VT CMD : %c %c %c %c %c %c \n", start[1], start[2], start[3], start[4], start[5], start[6]);
    if (next == '9')
    {
        puts("Restore current cursor position");
        return 2;
    }
    else if (next == '8')
    {
        puts("Save current cursor position");
        return 2;
    }
    else 
    {
        int delta = 0;
        unsigned char endChar = *start;
        while(start <= bound && !isAlphabet(endChar = *(start + delta)))
            delta++;
        int row, col;
        switch (endChar)
        {
            case 'H':
                if (!strncmp("[H", (const char *)start + 1, 2))
                {
                    //puts("Home-positioning to root (0-0)");
                    m_i = m_j = 0;
                    assert(m_i >= 0 && m_i < LINE_PER_PAGE && m_j >= 0  && m_j < CHAR_PER_LINE);
                }
                else if (sscanf((const char*) start + 1, "[%d;%dH", &row, &col) == 2)
                {
                    //printf("Home-positioning to %d and %d coordinates\n", row, col);
                    m_i = row - 1;
                    m_j = col - 1;
                    assert(m_i >= 0 && m_i < LINE_PER_PAGE && m_j >= 0  && m_j < CHAR_PER_LINE);
                }
                else 
                    puts("unknown H commad");
                break;
            case 'K':
                if (!strncmp("[K", (const char*)start + 1, 2) || !strncmp("[0K", (const char *)start + 1, 3))
                {
                    ;//puts("Clear line from current cursor position to end of line");
                    for (int j = m_j; j < CHAR_PER_LINE; j++)
                        m_page[m_nStartRow + m_i][j] = ' ';
                    assert(m_i >= 0 && m_i < LINE_PER_PAGE && m_j >= 0  && m_j < CHAR_PER_LINE);
                }
                else if (!strncmp("[1K", (const char *)start + 1, 3))
                {
                    ;//puts("Clear line from beginning to current cursor position");
                    for (int j = 0; j <= m_j; j++)
                        m_page[m_nStartRow + m_i][j] = ' ';
                    assert(m_i >= 0 && m_i < LINE_PER_PAGE && m_j >= 0  && m_j < CHAR_PER_LINE);
                }
                else if (!strncmp("[2K", (const char *)start + 1, 3))
                {
                    ;//puts("Clear whole line (cursor position unchanged)");
                    for (int j = 0; j < CHAR_PER_LINE; j++)
                        m_page[m_nStartRow + m_i][j] = ' ';
                    assert(m_i >= 0 && m_i < LINE_PER_PAGE && m_j >= 0  && m_j < CHAR_PER_LINE);
                }
                else 
                    puts("unknown K commad");
                break;
            case 'm':
                //puts("coloring");
                break;
            case 'M':
                if (!strncmp("M", (const char *)start + 1, 1))
                {
                    ;//puts("Move/scroll window down one line");
                    
                    m_nStartRow--;
                    assert(m_i >= 0 && m_i < LINE_PER_PAGE && m_j >= 0  && m_j < CHAR_PER_LINE);
                }
                break;
            case 'J':
                if (!strncmp("[J", (const char *)start + 1, 2) || !strncmp("[0J", (const char *)start + 1, 3))
                {
                    ;//puts("Clear screen from cursor down");
                    for (int j = m_j; j < CHAR_PER_LINE; j++)
                        m_page[m_nStartRow + m_i][j] = ' ';
                    for (int i = m_i + 1; i < LINE_PER_PAGE; i++)
                        for (int j = 0; j < CHAR_PER_LINE; j++)
                            m_page[m_nStartRow + i][j] = ' ';
                    assert(m_i >= 0 && m_i < LINE_PER_PAGE && m_j >= 0  && m_j < CHAR_PER_LINE);
                }
                else if (!strncmp("[1J", (const char *)start + 1, 3))
                {
                    ;//puts("Clear screen from cursor up");
                    for (int j = m_j; j < CHAR_PER_LINE; j++)
                        m_page[m_nStartRow + m_i][j] = ' ';
                    for (int i = m_i - 1; i >= 0; i--)
                        for (int j = 0; j < CHAR_PER_LINE; j++)
                            m_page[m_nStartRow + i][j] = ' ';
                    assert(m_i >= 0 && m_i < LINE_PER_PAGE && m_j >= 0  && m_j < CHAR_PER_LINE);
                }
                else if (!strncmp("[2J", (const char *)start + 1, 3))
                {
                    ;//puts("Clear entire screen");
                    for (int i  = 0; i < LINE_PER_PAGE; i++)
                        for (int j = 0; j < CHAR_PER_LINE; j++)
                            m_page[m_nStartRow + i][j] = ' ';
                    assert(m_i >= 0 && m_i < LINE_PER_PAGE && m_j >= 0  && m_j < CHAR_PER_LINE);
                }
                else
                    puts("unknown J command");
                break;
            default:
                char cmd[200] = {0};
                strncpy(cmd, (const char *)start, delta + 1);
                printf("unknown command:%s\n", cmd);
                assert(false);
        }

        return delta + 1;
    }
}
