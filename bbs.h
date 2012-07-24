#ifndef _H_BBS_
#define _H_BBS_

#define LINE_PER_PAGE 24
#define INTERNAL_BUF_LEN 10000
#define CHAR_PER_LINE 80
#define MAX_MI_COUNT 10

class BBS
{
	public:
		BBS();
		~BBS();
		const unsigned char * getCurrentPage();
        int readPage();
        int connect(const char *host, int port = 23);
        int send(unsigned char *data, int len);
	private:
		unsigned char m_szBuffer[INTERNAL_BUF_LEN];
        unsigned char m_page[LINE_PER_PAGE][CHAR_PER_LINE];
        int m_fdSock;

		

};
#endif
