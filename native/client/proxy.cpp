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
	this->closing = true;
	this->other->closing = true;
	if (queue.empty()) {
		this->closed = true;
		this->other->closed = true;
	}
}
void Pipe::on_send() {
	Point::on_send();
	if (this->closing) {
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


