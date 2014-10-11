/*
 * secure_state.h
 *
 *  Created on: Sep 29, 2014
 *      Author: vova
 */

#ifndef SECURE_STATE_H_
#define SECURE_STATE_H_

#include "secure_impl.h"

class SecureSocket;
class ConnectingState;
class AcceptingState;
extern ConnectingState connecting;
extern AcceptingState accepting;

class BaseState {
public:
	virtual ~BaseState(){};
	virtual Socket::RetCode send(SecureSocket* ctx) = 0;
	virtual Socket::RetCode recv(SecureSocket* ctx) = 0;
};
class ConnectingState : public BaseState  {
	Socket::RetCode try_connect(SecureSocket* ctx);
public:
	virtual ~ConnectingState(){};
	virtual Socket::RetCode send(SecureSocket* ctx);
	virtual Socket::RetCode recv(SecureSocket* ctx);
};
class AcceptingState : public BaseState  {
	Socket::RetCode try_accept(SecureSocket* ctx);
public:
	virtual ~AcceptingState(){};
	virtual Socket::RetCode send(SecureSocket* ctx);
	virtual Socket::RetCode recv(SecureSocket* ctx);
};
class EstablishedState : public BaseState  {
public:
	virtual ~EstablishedState(){};
	virtual Socket::RetCode send(SecureSocket* ctx);
	virtual Socket::RetCode recv(SecureSocket* ctx);
};


#endif /* SECURE_STATE_H_ */
