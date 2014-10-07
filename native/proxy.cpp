/*
 * proxy.cpp
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#include "normal_socket.h"
#include "config.h"
#include "connection.h"
#include "proxy.h"
#include "secure_socket.h"


Listener::Listener(Worker* parent, Socket* socket): WorkItem(parent, socket) {}

//bool Listener::is_sending() {
//	return false;
//}

void Listener::init() {
	this->socket->listen();
}

void Listener::recv() {
	NormalSocket* accepted = (NormalSocket*)this->socket->accept();
	if (accepted) {
		ServerPipe* sp = new ServerPipe(this->parent, accepted);
		ClientPipe* cp = new ClientPipe(this->parent,
				new NormalSocket(Config::get().client));
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

//void Pipe::on_recv() {}

void Pipe::on_close() {

	LOG.debugStream() << "Pipe::on_close1:"<<this->closing<<"|"<<this->other->send_queue.empty()<<"|"<<this->closed;

	this->closing = true;
	if (this->other)
		this->other->closing = true;
	if (this->other->send_queue.empty()) {
		this->closed = true;
		this->parent->remove(this);
		if (this->other)
			this->other->closed = true;
			this->parent->remove(this->other);
	}
	LOG.debugStream() << "Pipe::on_close2:"<<this->closing<<"|"<<this->other->send_queue.empty()<<"|"<<this->closed<<"|"<<this->other->event->events;
}

void Pipe::on_send() {
	if (this->closing) {
		this->closed = true;
		this->parent->remove(this);
		if (this->other)
			this->other->closed = true;
			this->parent->remove(this->other);
	}
}

