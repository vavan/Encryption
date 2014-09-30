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

class SecureImpl;

class SecureSocket: public NormalSocket
{
private:
//	SecureImpl* impl;
	BaseState* state;
//	bool is_connected;
//	int _is_sending;
//	ConnectState ssl_connect();
public:
	SecureImpl* impl;
	SecureSocket(Addr addr);
	SecureSocket(NormalSocket* socket);
	virtual ~SecureSocket();

	void change_state(BaseState* new_state);

	virtual bool connect();
	virtual bool listen();
	virtual Socket* accept();
	virtual ssize_t send(char* buf, size_t size);
	virtual ssize_t recv(char* buf, const size_t size);
	virtual int is_sending();

	void set_security(std::string cert_file, std::string key_file);
};



#endif /* SECURE_SOCKET_H_ */
