/*
 * secure_layer.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */
#include <iostream>

#include "secure_impl.h"
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

