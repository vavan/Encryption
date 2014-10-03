/*
 * secure_layer.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */
#include <iostream>

#include "secure_impl.h"
#include "socket.h"
#include "config.h"

SSL_CTX* SecureImpl::ctx = NULL;

std::string SecureImpl::getError(int ret_code) {
	char buf[1024];
	int err = SSL_get_error(this->connection, ret_code);
	ERR_error_string_n(err, buf, sizeof(buf));
	std::string out = buf;
	return buf;
}


void SecureImpl::checkErrors(const std::string& tag) {
	int i;
	LOG.errorStream() << tag << ':' << '\n';
	while ((i = ERR_get_error())) {
		char buf[1024];
		ERR_error_string_n(i, buf, sizeof(buf));
		LOG.errorStream() << "\tError " << i << ": " << buf << '\n';
	}
}

void SecureImpl::init() {
	if (ctx == NULL) {
		SSL_load_error_strings();
		SSL_library_init();
		ctx = SSL_CTX_new(SSLv3_method());
		SSL_CTX_set_options(ctx, SSL_OP_ALL);
	}
}

SecureImpl::SecureImpl() {
	init();
	connection = SSL_new(ctx);
}

SecureImpl::~SecureImpl() {
	SSL_free(connection);
}


ssize_t SecureImpl::connect() {
	int ret;
	if ((ret = SSL_connect(this->connection)) <= 0) {
		ret = SSL_get_error(this->connection, ret);
		if (ret == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE) {
			return Socket::INPROGRESS;
		} else {
			LOG.errorStream() << "SSL_connect=" << ret;
			return Socket::ERROR;
		}
	}
	return Socket::DONE;
}

ssize_t SecureImpl::accept() {
	int ret;
	if ((ret = SSL_accept(this->connection)) <= 0) {
		ret = SSL_get_error(this->connection, ret);
		if (ret == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE) {
			return Socket::INPROGRESS;
		} else {
			LOG.errorStream() << "SSL_accept=" << ret;
			this->checkErrors("SSL_check_private_key");
			return Socket::ERROR;
		}
	}
	return Socket::DONE;
}

ssize_t SecureImpl::send(char* buf, size_t size) {
	//TODO ->beautiful
	ssize_t ret = SSL_write(this->connection, buf, size);
	if (ret >= 0) {
		LOG.debugStream() << "QQQQ SSL[]. send:" << ret;
		return ret;
	} else {
		LOG.errorStream() << "SSL. Send failed:" << errno;
		return Socket::ERROR;
	}
}
ssize_t SecureImpl::recv(char* buf, const size_t size) {
	//TODO ->beautiful
	ssize_t ret = SSL_read(this->connection, buf, size);
	if (ret >= 0) {
		LOG.debugStream() << "ZZZZ SSL. Recv:" << ret;
		return ret;
	} else {
		LOG.errorStream() << "SSL. Recv failed:" << errno;
		return Socket::ERROR;
	}
}

