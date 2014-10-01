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

bool Listener::is_sending() {
	return false;
}

void Listener::init() {
	this->socket->listen();
}

void Listener::recv() {
	NormalSocket* accepted = (NormalSocket*)this->socket->accept();
	if (accepted) {
		ServerPipe* sp = new ServerPipe(this->parent, accepted);
//		ClientPipe* cp = new ClientPipe(this->parent,
//				new NormalSocket(Config::get().client));
//		sp->join(cp);
//		sp->init();
//		cp->init();
	}
}

void Listener::send() {

};

void Pipe::return_buffer() {
	//TODO FIX ME
//	this->other->send_queue.return_front();
	this->send_queue.return_front();
}

Buffer* Pipe::get_buffer() {
	//TODO FIX ME
//	return this->other->send_queue.get_front();
	return this->send_queue.get_front();
}

void Pipe::join(Pipe* other) {
	this->other = other;
	this->other->other = this;
}

void Pipe::on_recv(Buffer* buffer) {
	LOG.debugStream() << "PIPE, Recv bytes:" << buffer->size();
}

void Pipe::on_close() {
	this->closing = true;
	if (this->other)
		this->other->closing = true;
	if (send_queue.empty()) {
		this->closed = true;
		if (this->other)
			this->other->closed = true;
	}
}

void Pipe::on_send(Buffer* buffer) {
	LOG.debugStream() << "Sent bytes:" << buffer->size();
	if (this->closing) {
		this->closed = true;
		if (this->other)
			this->other->closed = true;
	}
}

