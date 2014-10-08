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

	NormalSocket(Addr& addr);
	NormalSocket(Addr& addr, int s);
	NormalSocket(const NormalSocket* socket);
	virtual ~NormalSocket();

	virtual int get();
	virtual bool connect();
	virtual bool listen();
	virtual Socket* accept();
	virtual ssize_t send();
	virtual ssize_t recv();
	virtual void nonblock();
};




#endif /* NORMAL_SOCKT_H_ */
