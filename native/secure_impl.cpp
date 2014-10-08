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
#include "queue.h"
#include "secure_socket.h"



//TODO LOG output with socket <fd>, like NormalSocket
//TODO Clean up

SSL_CTX* SecureImpl::ctx = NULL;

void SecureImpl::set_security(string cert_file, string key_file) {
	if (SSL_CTX_use_certificate_file(SecureImpl::ctx, cert_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
		checkErrors("SSL_use_certificate_file");
		return;
	}
	if (SSL_CTX_use_RSAPrivateKey_file(SecureImpl::ctx, key_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
		checkErrors("SSL_use_RSAPrivateKey_file");
		return;
	}
	if (SSL_CTX_check_private_key(SecureImpl::ctx) <= 0) {
		checkErrors("SSL_check_private_key");
		return;
	}
}

std::string SecureImpl::getError(int ret_code) {
	char buf[1024];
	int err = SSL_get_error(this->connection, ret_code);
	ERR_error_string_n(err, buf, sizeof(buf));
	std::string out = buf;
	return buf;
}


void SecureImpl::checkErrors(const std::string& tag) {
	int i;
	LOG.errorStream() << tag << ':';
	while ((i = ERR_get_error())) {
		char buf[1024];
		ERR_error_string_n(i, buf, sizeof(buf));
		LOG.errorStream() << "\tError " << i << ": " << buf;
	}
}

void SecureImpl::init() {
	if (ctx == NULL) {
		SSL_load_error_strings();
		SSL_library_init();
		ctx = SSL_CTX_new(SSLv23_method());
		set_security("cert.pem","key.pem");
		if (SSL_CTX_set_cipher_list(SecureImpl::ctx, "RC4, AES128") <= 0) {
			checkErrors("SSL_CTX_set_cipher_list");
		}
//		SSL_CTX_set_options(ctx, SSL_OP_ALL);
	}
}

SecureImpl::SecureImpl(SecureSocket* parent) : parent(parent) {
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

ssize_t SecureImpl::send() {
	Buffer * buffer = parent->send_queue->get_back();
	ssize_t ret = SSL_write(this->connection, &(*buffer)[0], buffer->size());
	if (ret >= 0) {
		LOG.debugStream() << "QQQQ SSL[]. send:" << ret;
		parent->send_queue->compleate(ret);
		return ret;
	} else {
		LOG.errorStream() << "SSL. Send failed:" << SSL_get_error(this->connection, ret);
		checkErrors("ssl send");
		return Socket::ERROR;
	}
	return 0;
}
ssize_t SecureImpl::recv() {
	Buffer * buffer = parent->recv_queue->get_front();
	ssize_t ret = SSL_read(this->connection, &(*buffer)[0], buffer->size());
	if (ret >= 0) {
		LOG.debugStream() << "ZZZZ SSL. Recv:" << ret;
		if (ret > 0) parent->recv_queue->compleate(ret);
		return ret;
	} else {
		LOG.errorStream() << "SSL. Recv failed:" << SSL_get_error(this->connection, ret);
		checkErrors("ssl recv");
		return Socket::ERROR;
	}
}

