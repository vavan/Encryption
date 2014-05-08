/*
 * socket.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */
#ifndef SOCKET_H_
#define SOCKET_H_

#include "addr.h"



class Socket
{
public:
	Addr addr;
	int s;
	Socket(Addr addr);
	Socket(Addr addr, int s);
	Socket(const Socket& socket);
	virtual ~Socket();

	void connect();
	void listen();
	Socket* accept();
	size_t send(char* buf, size_t size);
	size_t recv(char* buf, const size_t size);
	void nonblock();
};




#endif /* SOCKET_H_ */
