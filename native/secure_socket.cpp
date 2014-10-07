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
bool SecureSocket::listen() {
	NormalSocket::listen();
	return false;
}

Socket* SecureSocket::accept() {
	//TODO delete <accepted>
	NormalSocket* accepted = (NormalSocket*)NormalSocket::accept();
	if (accepted) {
		SecureSocket* secured = new SecureSocket(accepted);
		secured->state->accept(secured);
		secured->set_security("cert.pem","key.pem");
		return secured;
	} else {
		return NULL;
	}
}

//int SecureSocket::is_sending() {
//	return this->state->is_sending(this);
//}

ssize_t SecureSocket::send() {
	return this->state->send(this);
}

ssize_t SecureSocket::recv() {
	return this->state->recv(this);
}

void SecureSocket::change_state(BaseState* state) {
	this->state = state;
}

void SecureSocket::set_security(string cert_file, string key_file) {
	if (!cert_file.empty()) {
		if (SSL_use_certificate_file(this->impl->connection, cert_file.c_str(),
				SSL_FILETYPE_PEM) <= 0) {
			this->impl->checkErrors("SSL_use_certificate_file");
			return;
		}
	}
	if (!key_file.empty()) {
		if (SSL_use_RSAPrivateKey_file(this->impl->connection, key_file.c_str(),
				SSL_FILETYPE_PEM) <= 0) {
			this->impl->checkErrors("SSL_use_RSAPrivateKey_file");
			return;
		}
	}
	if (!key_file.empty() && !cert_file.empty()) {
		if (SSL_check_private_key(this->impl->connection) <= 0) {
			this->impl->checkErrors("SSL_check_private_key");
			return;
		}
	}
	if (SSL_set_cipher_list(this->impl->connection, "ALL") <= 0) {
		this->impl->checkErrors("SSL_set_cipher_list");
	}
}

