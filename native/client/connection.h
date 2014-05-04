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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <list>
using namespace std;

class Socket
{
public:
	int s;
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
		size_t recv(char* buf, const size_t size) {
			return ::recv(s, buf, size, 0);
		}
		void nonblock() {
			int rc = fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);
			if (rc < 0)
			{
				perror("nonblock");
			}
		}
};

class Pipe: public Socket {
	Pipe* other;
	static const size_t size = 8192;
	char buffer[size];
public:
	void join(Pipe* other) {
		this->other = other;
	}
	void recv() {
		int recved = 0;//this->recv((char*)buffer, size);
		this->other->queue(buffer, recved);
	}
	void queue(char* buffer, int size){
		//TODO
	}
	void unqueue(){
		//TODO
	}
};

class PipeList {
	list<int> l;
	list<Pipe> pipes;
	fd_set rfds;
public:
	int build() {
		int max = 0;
		FD_ZERO(&rfds);
		for (list<Pipe>::iterator i = pipes.begin(); i != pipes.end(); i++) {
			int s = (*i).s;
			FD_SET(s, &rfds);
			if (s > max) max = s;
		}
		return max+1;
	}
	void add(Pipe& pipe) {

	}
	void wait() {
		int max = this->build();
		int retval = select(max, &rfds, NULL, NULL, NULL);
		if (retval == -1)
			perror("select()");
		else if (retval)
			if FD_ISSET(0, &rfds) {

			}
	}

};
