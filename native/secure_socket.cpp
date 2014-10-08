/*
 * secure_socket.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */

#include <sys/types.h>
#include <sys/socket.h>
#include "config.h"
#include "secure_socket.h"
#include "secure_impl.h"
#include "worker.h"

using namespace std;


SecureSocket::SecureSocket(Addr& addr) :
		NormalSocket(addr) {
	this->impl = new SecureImpl(this);
	this->state = &idleState;
	if (SSL_set_fd(this->impl->connection, this->s) == 0)
		this->impl->checkErrors("SSL_set_fd");
}

SecureSocket::SecureSocket(NormalSocket* socket) :
		NormalSocket(socket) {
	this->impl = new SecureImpl(this);
	this->state = &idleState;
	if (SSL_set_fd(this->impl->connection, this->s) == 0)
		this->impl->checkErrors("SSL_set_fd");
}

SecureSocket::~SecureSocket() {
	delete this->impl;
}

bool SecureSocket::connect() {
	NormalSocket::connect();
	this->state->connect(this);
	return true;
}

Socket* SecureSocket::accept() {
	//TODO delete <accepted>
	NormalSocket* accepted = (NormalSocket*)NormalSocket::accept();
	if (accepted) {
		SecureSocket* secured = new SecureSocket(accepted);
		secured->state->accept(secured);
		return secured;
	} else {
		return NULL;
	}
}

ssize_t SecureSocket::send() {
	return this->state->send(this);
}

ssize_t SecureSocket::recv() {
	return this->state->recv(this);
}

void SecureSocket::change_state(BaseState* state) {
	this->state = state;
}



