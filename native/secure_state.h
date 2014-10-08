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
	virtual size_t send(SecureSocket* ctx) = 0;
	virtual size_t recv(SecureSocket* ctx) = 0;
};
class ConnectingState : public BaseState  {
	size_t try_connect(SecureSocket* ctx);
public:
	virtual ~ConnectingState(){};
	virtual size_t send(SecureSocket* ctx);
	virtual size_t recv(SecureSocket* ctx);
};
class AcceptingState : public BaseState  {
	size_t try_accept(SecureSocket* ctx);
public:
	virtual ~AcceptingState(){};
	virtual size_t send(SecureSocket* ctx);
	virtual size_t recv(SecureSocket* ctx);
};
class EstablishedState : public BaseState  {
public:
	virtual ~EstablishedState(){};
	virtual size_t send(SecureSocket* ctx);
	virtual size_t recv(SecureSocket* ctx);
};


#endif /* SECURE_STATE_H_ */
