/*
 * normal_sockt.h
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */

#ifndef NORMAL_SOCKT_H_
#define NORMAL_SOCKT_H_

#include "socket.h"

class NormalSocket: public Socket
{
protected:
	int s;
public:
	NormalSocket(Addr addr);
	NormalSocket(Addr addr, int s);
	NormalSocket(const NormalSocket& socket);
	virtual ~NormalSocket();

	virtual int get();
	virtual bool connect();
	virtual bool listen();
	virtual Socket* accept();
	virtual size_t send(char* buf, size_t size);
	virtual size_t recv(char* buf, const size_t size);
	virtual void nonblock();
};




#endif /* NORMAL_SOCKT_H_ */
