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


Listener::Listener(Worker* parent, Socket* socket): WorkItem(parent, socket) {}

bool Listener::is_sending() {
	return false;
}

void Listener::init() {
	this->socket->listen();
}

void Listener::recv() {
	Socket* accepted = this->socket->accept();
	ServerPipe* sp = new ServerPipe(this->parent, accepted);
	ClientPipe* cp = new ClientPipe(this->parent, new NormalSocket(Config::get().client));
	sp->join(cp);
	sp->init();
	cp->init();
}

void Listener::send() {
};


Buffer* Pipe::get_buffer() {
	return this->other->send_queue.get_front();
}

void Pipe::join(Pipe* other) {
	this->other = other;
	this->other->other = this;
}

void Pipe::on_recv(Buffer* buffer) {
}

void Pipe::on_close() {
	this->closing = true;
	if (this->other)
		this->other->closing = true;
	if (send_queue.empty()) {
		this->closed = true;
		if (this->other) this->other->closed = true;
	}
}

void Pipe::on_send(Buffer* buffer) {
	LOG.debugStream() << "Sent bytes:" << buffer->size();
	if (this->closing) {
		this->closed = true;
		this->other->closed = true;
	}
}

