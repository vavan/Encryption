/*
 * socket.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */
#ifndef SOCKET_H_
#define SOCKET_H_
#include <string>

class Addr {
public:
	std::string ip;
	int port;
	Addr(std::string ip, int port) :
			ip(ip), port(port) {
	}
	Addr(const Addr& addr) :
			ip(addr.ip), port(addr.port) {
	}
	Addr() :
			ip(""), port(0) {
	}
	std::string str();
};

class Socket {
protected:
	Addr addr;
public:
	Socket(Addr addr);
	virtual ~Socket();

	virtual int get() = 0;
	virtual bool connect() = 0;
	virtual bool listen() = 0;
	virtual Socket* accept() = 0;
	virtual size_t send(char* buf, size_t size) = 0;
	virtual size_t recv(char* buf, const size_t size) = 0;
	virtual void nonblock() = 0;
};

#endif /* SOCKET_H_ */
