/*
 * proxy.cpp
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#include "normal_socket.h"
#include "secure_socket.h"
#include "proxy.h"
#include "worker.h"
#include "config.h"


Listener::Listener(Worker* parent, Socket* socket): WorkItem(parent, socket) {}

void Listener::init() {
	this->socket->listen();
}

void Listener::recv() {
	Socket* accepted = this->socket->accept();
	if (accepted) {
		Socket *s;
		if (Config::get().outbound_secure) {
			s = new SecureSocket(Config::get().outbound);
		} else {
			s = new NormalSocket(Config::get().outbound);
		}

		ServerPipe* sp = new ServerPipe(this->parent, accepted);
		ClientPipe* cp = new ClientPipe(this->parent, s);
		sp->join(cp);
		sp->init();
		cp->init();
	}
}

void Listener::send() {}

void Pipe::join(Pipe* other) {
	this->other = other;
	this->other->other = this;

	this->socket->recv_queue = &this->other->send_queue;
	this->other->socket->recv_queue = &this->send_queue;
	this->socket->send_queue = &this->send_queue;
	this->other->socket->send_queue = &this->other->send_queue;
}


void Pipe::on_close() {
	this->closing = true;
	this->other->closing = true;
	if (this->other->send_queue.empty()) {
		this->parent->remove(this);
		this->parent->remove(this->other);
	}
}

void Pipe::on_send() {
	if (this->closing) {
//		if (this->send_queue.empty()) {
			this->parent->remove(this);
			this->parent->remove(this->other);
//		}
	}
}

