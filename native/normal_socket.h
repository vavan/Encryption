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
	void nonblock();
public:

	NormalSocket(const Addr& addr);
	NormalSocket(const Addr& addr, int accepted);
	virtual ~NormalSocket();

	virtual Socket* copy(const Addr& addr, int s);
	virtual Socket::RetCode connect();
	virtual Socket::RetCode listen();
	virtual Socket* accept();
	virtual Socket::RetCode send();
	virtual Socket::RetCode recv();
};




#endif /* NORMAL_SOCKT_H_ */
