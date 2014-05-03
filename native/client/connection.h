#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

using namespace std;

class Socket
{
	int s;
public:
		Socket(int s = -1) {
			if (s == -1) {
				this->s = socket(AF_INET, SOCK_STREAM, 0);
			} else {
				this->s = s;
			}
		}
		~Socket() {
			close(s);
		}

		void connect(string addr, int port) {
		    struct sockaddr_in serveraddr;
		    char *servername;
		    socklen_t addr_size;

			bzero(&serveraddr,sizeof(serveraddr));
			serveraddr.sin_family=AF_INET;
			serveraddr.sin_port=htons(port);
			servername = (char*)gethostbyname(addr.c_str());
			inet_pton(AF_INET, servername, &serveraddr.sin_addr);

			addr_size=sizeof(serveraddr);
			if(::connect(s, (struct sockaddr *)&serveraddr, addr_size) == -1)
			{
				perror("connect");
				exit(1);
			}
			printf("DONE!!!!");
	    }

		void listen(int port) {
			struct sockaddr_in serveraddr;
			bzero((char *)&serveraddr, sizeof(serveraddr));
			serveraddr.sin_family = AF_INET;
			serveraddr.sin_port = htons(port);
			serveraddr.sin_addr.s_addr = INADDR_ANY;

			if(bind(s, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
			{
				perror("bind");
			}
			::listen(s, 5);
		}

		Socket accept() {
			struct sockaddr_in clientaddr;
			unsigned int addr_size = sizeof(clientaddr);
			int newsocket = ::accept(s, (struct sockaddr *)&clientaddr, &addr_size);
			return Socket(newsocket);
		}

		size_t send(char* buf, size_t size) {
			return ::send(s, buf, size, 0);
		}
		size_t recv(char* buf, size_t size) {
			return ::recv(s, buf, size, 0);
		}
};

