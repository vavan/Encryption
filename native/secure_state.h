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
class IdleState;
extern IdleState idleState;

class BaseState {
public:
	virtual ~BaseState(){};
//	virtual bool is_sending(SecureSocket* ctx) = 0;
	virtual size_t send(SecureSocket* ctx) = 0;
	virtual size_t recv(SecureSocket* ctx) = 0;
	virtual void connect(SecureSocket* ctx){};
	virtual void accept(SecureSocket* ctx){};
};
class IdleState: public BaseState  {
public:
//	virtual ~BaseState(){};
//	bool is_sending(SecureSocket* ctx);
	size_t send(SecureSocket* ctx);
	size_t recv(SecureSocket* ctx);
	void connect(SecureSocket* ctx);
	void accept(SecureSocket* ctx);
};
class ConnectingState : public BaseState  {
	size_t try_connect(SecureSocket* ctx);
public:
	virtual ~ConnectingState(){};
//	virtual bool is_sending(SecureSocket* ctx);
	virtual size_t send(SecureSocket* ctx);
	virtual size_t recv(SecureSocket* ctx);
};
class AcceptingState : public BaseState  {
	size_t try_accept(SecureSocket* ctx);
public:
	virtual ~AcceptingState(){};
//	virtual bool is_sending(SecureSocket* ctx);
	virtual size_t send(SecureSocket* ctx);
	virtual size_t recv(SecureSocket* ctx);
};
class EstablishedState : public BaseState  {
public:
	virtual ~EstablishedState(){};
//	virtual bool is_sending(SecureSocket* ctx);
	virtual size_t send(SecureSocket* ctx);
	virtual size_t recv(SecureSocket* ctx);
};


#endif /* SECURE_STATE_H_ */
