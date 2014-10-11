/*
 * secure_socket.h
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */

#ifndef SECURE_SOCKET_H_
#define SECURE_SOCKET_H_

#include "socket.h"
#include "normal_socket.h"
#include "secure_state.h"
#include "secure_impl.h"


class SecureSocket: public NormalSocket, public SecureLayer
{
private:
	BaseState* state;

protected:
	virtual Queue* get_send_queue();
	virtual Queue* get_recv_queue();

public:
	SecureSocket(const Addr& addr);
	SecureSocket(const Addr& addr, int accepted);
	virtual ~SecureSocket();

	void change_state(BaseState* new_state);

	virtual Socket* copy(const Addr& addr, int s);
	virtual Socket::RetCode send();
	virtual Socket::RetCode recv();
};



#endif /* SECURE_SOCKET_H_ */
