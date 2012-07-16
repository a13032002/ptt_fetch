#define LINE_PER_PAGE 24
#define INTERNAL_BUF_LEN 5000

class BBS
{
	public:
		BBS(const char *ip, int port = 23);
		~BBS();
		const char * getCurrentPage();
	private:
		bool readPage();
		unsigned char m_szBuffer[INTERNAL_BUF_LEN];

		

}
