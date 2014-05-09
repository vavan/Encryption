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
void Pipe::on_recv(Buffer& buffer) {
	if (!this->closing) {
		this->other->send(buffer);
	}
}
void Pipe::on_close() {
	LOG.debug("Pipe::on_close: %08X", this);
	this->closing = true;
	if (this->other) this->other->closing = true;
	if (queue.empty()) {
		LOG.debug("Pipe::closed: %08X", this);
		this->closed = true;
		if (this->other) this->other->closed = true;
	}
}
void Pipe::on_send() {
	Point::on_send();
	if (this->closing) {
		LOG.debug("Pipe::on_send, closing: %08X", this);
		this->closed = true;
		this->other->closed = true;
	}
}

void ServerPipe::init() {
//	this-->socket->listen();
}

void ClientPipe::init() {
	this->socket->connect();
}


