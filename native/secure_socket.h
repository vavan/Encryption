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

class SecureImpl;

class SecureSocket: public NormalSocket
{
private:
	SecureImpl* impl;
	bool is_connecting;
public:
	SecureSocket(Addr addr);
	SecureSocket(NormalSocket& socket);
	virtual ~SecureSocket();

	virtual int get();
	virtual bool connect();
	virtual bool connect2();
	virtual bool listen();
	virtual Socket* accept();
	virtual size_t send(char* buf, size_t size);
	virtual size_t recv(char* buf, const size_t size);
	virtual void nonblock();

	void setSecurity(std::string cert_file, std::string key_file);
};



#endif /* SECURE_SOCKET_H_ */
