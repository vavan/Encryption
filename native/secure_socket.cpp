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

using namespace std;

SecureSocket::SecureSocket(Addr addr) :
		NormalSocket(addr) {
	this->impl = new SecureImpl();
	if (SSL_set_fd(this->impl->connection, this->s) == 0)
		this->impl->checkErrors("SSL_set_fd");
	is_connecting = false;
}

SecureSocket::SecureSocket(NormalSocket& socket) :
		NormalSocket(socket) {
	this->impl = new SecureImpl();
	if (SSL_set_fd(this->impl->connection, this->s) == 0)
		this->impl->checkErrors("SSL_set_fd");
	is_connecting = false;
}

SecureSocket::~SecureSocket() {
	delete this->impl;
}

void SecureSocket::setSecurity(string cert_file, string key_file) {
	if (!cert_file.empty())
		if (SSL_use_certificate_file(this->impl->connection, cert_file.c_str(),
		SSL_FILETYPE_PEM) <= 0) {
			this->impl->checkErrors("SSL_use_certificate_file");
			return;
		}
	if (!key_file.empty())
		if (SSL_use_RSAPrivateKey_file(this->impl->connection, key_file.c_str(),
		SSL_FILETYPE_PEM) <= 0) {
			this->impl->checkErrors("SSL_use_RSAPrivateKey_file");
			return;
		}
	if (!key_file.empty() && !cert_file.empty())
		if (SSL_check_private_key(this->impl->connection) <= 0) {
			this->impl->checkErrors("SSL_check_private_key");
			return;
		}
	if (SSL_set_cipher_list(this->impl->connection, "ALL") <= 0) {
		this->impl->checkErrors("SSL_set_cipher_list");
	}
}

bool SecureSocket::connect() {
//	string conn = "127.0.0.1:5689";
//	this->impl->bio = BIO_new_connect((char*) conn.c_str());
//	if (BIO_do_connect(this->impl->bio) <= 0) {
//		return false;
//	}
//	SSL_set_bio(this->impl->connection, this->impl->bio, this->impl->bio);

	NormalSocket::connect();
	SSL_set_connect_state(this->impl->connection);
//	if (SSL_connect(this->impl->connection) <= 0) {
//		int ret = ERR_get_error();
//		if ((ret == SSL_ERROR_WANT_READ) || (ret == SSL_ERROR_WANT_WRITE)) {
//			is_connecting = true;
//			return true;
//		} else {
//			this->impl->checkErrors("SSL_connect");
//			return false;
//		}
//	}
	is_connecting = true;
	return true;
}

bool SecureSocket::connect2() {
	int ret = 0;
	if ((ret = SSL_connect(this->impl->connection)) <= 0) {
		LOG.errorStream() << "SSL_connect=" << ret;
//		int ret = ERR_get_error();
//		if ((ret == SSL_ERROR_WANT_READ) || (ret == SSL_ERROR_WANT_WRITE)) {
//			is_connecting = true;
//			return true;
//		} else {
			this->impl->checkErrors("SSL_connect");
//			return false;
//		}
	}
	return false;
}

size_t SecureSocket::send(char* buf, size_t size) {
	int result;
	socklen_t result_len = sizeof(result);
	if (getsockopt(this->s, SOL_SOCKET, SO_ERROR, &result, &result_len) < 0) {
		LOG.errorStream() << "SOCKET. Cant getopt=" << result;
	}
	LOG.debugStream() << "SOCKET. getopt=" << result;

	if (is_connecting) {
		this->connect2();
		this->connect2();
		is_connecting = false;
		return 1;
	} else {
		return SSL_write(this->impl->connection, buf, size);
	}
}

size_t SecureSocket::recv(char* buf, const size_t size) {
	if (is_connecting)
		this->connect();
	return SSL_read(this->impl->connection, buf, size);
}

bool SecureSocket::listen() {
	return false;
}

Socket* SecureSocket::accept() {
	return NULL;
}

void SecureSocket::nonblock() {
}

int SecureSocket::get() {
	return 0;
}

