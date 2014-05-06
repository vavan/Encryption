/*
 * socket.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

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
#include <string>
#include <stdio.h>
#include <stdlib.h>

#ifndef SOCKET_H_
#define SOCKET_H_


class Addr {
public:
	std::string ip;
	int port;
	Addr(std::string ip, int port) {
		this->ip=ip;
		this->port=port;
	}
	Addr(const Addr& addr) {
		this->ip=addr.ip;
		this->port=addr.port;
	}
	Addr() {
		this->ip="0.0.0.0";
		this->port=0;
	}
};

class Socket
{
	Addr addr;
public:
	int s;
	Socket(Addr addr) : addr(addr) {
		this->s = socket(AF_INET, SOCK_STREAM, 0);
	}
	Socket(Addr addr, int s) : addr(addr) {
		this->s = s;
	}
	Socket(const Socket& socket) : addr(socket.addr), s(socket.s) {
	}
	virtual ~Socket() {
		close(s);
	}

	void connect() {
		struct sockaddr_in serveraddr;
		char *servername;
		socklen_t addr_size;

		bzero(&serveraddr,sizeof(serveraddr));
		serveraddr.sin_family=AF_INET;
		serveraddr.sin_port=htons(addr.port);
		servername = (char*)gethostbyname(addr.ip.c_str());
		inet_pton(AF_INET, servername, &serveraddr.sin_addr);

		addr_size=sizeof(serveraddr);
		if(::connect(s, (struct sockaddr *)&serveraddr, addr_size) == -1)
		{
			perror("connect");
			exit(1);
		}
//		printf("DONE!!!!");
	}

	void listen() {
		struct sockaddr_in serveraddr;
		bzero((char *)&serveraddr, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(addr.port);
		serveraddr.sin_addr.s_addr = INADDR_ANY; //TODO

		if(bind(s, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
		{
			perror("bind");
		}
		::listen(s, 5);
	}

	Socket* accept() {
		struct sockaddr_in clientaddr;
		unsigned int addr_size = sizeof(clientaddr);
		int newsocket = ::accept(s, (struct sockaddr *)&clientaddr, &addr_size);
		return new Socket(this->addr, newsocket);
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




#endif /* SOCKET_H_ */
