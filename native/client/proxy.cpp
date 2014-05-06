/*
 * proxy.cpp
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#include "config.h"
#include "connection.h"
#include "proxy.h"

void Listener::init() {
	this->socket->listen();
}
void Listener::do_recv() {
	Socket* accepted = this->socket->accept();
	ServerPipe* sp = new ServerPipe(this->parent, accepted);
	ClientPipe* cp = new ClientPipe(this->parent, new Socket(Config::get().client));
	sp->join(cp);
}


void Pipe::join(Pipe* other) {
	this->other = other;
	this->other->other = this;
}
void Pipe::do_recv() {
	if (!this->closing) {
		Buffer& buffer = Point::recv();
		if (buffer.size() > 0) {
			this->other->send(buffer);
		} else {
			this->closing = true;
			this->other->closing = true;
		}
	}
}
void Pipe::do_send() {
	if (!this->queue.empty()) {
		Point::do_send();
	} else {
		if (this->closing) {
			this->closed = true;
			this->other->closed = true;
		}
	}
}

void ServerPipe::init() {
	//do nothing
}

void ClientPipe::init() {
	this->socket->connect();
}


