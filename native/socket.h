/*
 * socket.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */
#ifndef SOCKET_H_
#define SOCKET_H_

#include <sys/types.h>
#include <string>

class Queue;

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
	int s;
public:
	enum SocketReturns {
		ENDOFFILE = 0,
		DONE = 0,
		ERROR = -1,
		INPROGRESS = -2
	};
	Queue* recv_queue;
	Queue* send_queue;

	Socket(const Addr& addr);
	Socket(const Addr& addr, int accepted);
	virtual ~Socket();

	int get();
	virtual Socket* copy(const Addr& addr, int s) = 0;
	virtual bool connect() = 0;
	virtual bool listen() = 0;
	virtual Socket* accept() = 0;
	virtual ssize_t send() = 0;
	virtual ssize_t recv() = 0;

};

#endif /* SOCKET_H_ */
