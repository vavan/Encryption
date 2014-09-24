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

Buffer* Pipe::recv() {
	Buffer* proxy_buffer = this->other->send_queue.push();
	LOG << "proxy_buffer push: " << proxy_buffer;


	proxy_buffer->resize(Queue::DEPTH);
	int recved = this->socket->recv(&(*proxy_buffer)[0], proxy_buffer->size());
	if (recved >= 0) {
		proxy_buffer->resize(recved);
	} else {
		proxy_buffer->resize(0);
		LOG << "Recv ERROR. Drop connection";
	}
	return proxy_buffer;
}

void Pipe::join(Pipe* other) {
	this->other = other;
	this->other->other = this;
}
void Pipe::on_recv(Buffer* buffer) {
//	if (!this->closing) {
//		this->other->send(buffer);
//	}
}
void Pipe::on_close() {
	LOG << "Pipe::on_close: " << this;
	this->closing = true;
	if (this->other) this->other->closing = true;
	if (send_queue.empty()) {
		LOG << "Pipe::closed: " << this;
		this->closed = true;
		if (this->other) this->other->closed = true;
	}
}
void Pipe::on_send() {
	Point::on_send();
	if (this->closing) {
		LOG << "Pipe::on_send, closing: " << this;
		this->closed = true;
		this->other->closed = true;
	}
}

void ServerPipe::init() {
	this->socket->listen();
}

void ClientPipe::init() {
	this->socket->connect();
}


