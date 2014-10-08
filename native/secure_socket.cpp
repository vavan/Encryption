/*
 * secure_socket.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */

#include <sys/types.h>
#include "secure_socket.h"
#include "config.h"

using namespace std;


SecureSocket::SecureSocket(const Addr& addr) :
		NormalSocket(addr), SecureLayer() {
	this->set(this->s);
	this->state = &connecting;
	LOG.infoStream() << "SSL["<< this->s << "]. Create/New";
}

SecureSocket::SecureSocket(const Addr& addr, int accepted) :
		NormalSocket(addr, accepted), SecureLayer() {
	this->set(this->s);
	this->state = &accepting;
	LOG.infoStream() << "SSL["<< this->s << "]. Create/Accepted";
}

SecureSocket::~SecureSocket() {
	LOG.infoStream() << "SSL["<< this->s << "]. Delete/Close";
}


Socket* SecureSocket::copy(const Addr& addr, int newsocket) {
	return new SecureSocket(this->addr, newsocket);
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

Queue* SecureSocket::get_send_queue() {
	return this->send_queue;
}

Queue* SecureSocket::get_recv_queue() {
	return this->recv_queue;
}

