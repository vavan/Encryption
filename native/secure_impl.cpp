/*
 * secure_layer.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */

/*
 * secure_socket.cpp
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */

#include "secure_impl.h"

#include <log4cpp/Category.hh>
#include <log4cpp/CategoryStream.hh>
#include <openssl/err.h>
#include <openssl/ossl_typ.h>
#include <sys/types.h>
#include <vector>

#include "config.h"
#include "socket.h"
#include "queue.h"

using namespace std;

SSL_CTX* SecureLayer::ctx = NULL;

SecureLayer::SecureLayer() {
	init();
	this->connection = SSL_new(ctx);
}

SecureLayer::~SecureLayer() {
	SSL_free(this->connection);
}

void SecureLayer::set(int derived_socket) {
	derived_s = derived_socket;
	if (SSL_set_fd(this->connection, derived_socket) == 0)
		log_error("SSL_set_fd");

}

void SecureLayer::set_security(string cert_file, string key_file) {
	if (SSL_CTX_use_certificate_file(SecureLayer::ctx, cert_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
		log_ssl_error("SSL_CTX_use_certificate_file");
		return;
	}
	if (SSL_CTX_use_RSAPrivateKey_file(SecureLayer::ctx, key_file.c_str(), SSL_FILETYPE_PEM) <= 0) {
		log_ssl_error("SSL_CTX_use_RSAPrivateKey_file");
		return;
	}
	if (SSL_CTX_check_private_key(SecureLayer::ctx) <= 0) {
		log_ssl_error("SSL_CTX_check_private_key");
		return;
	}
}



//TODO its not working!!!
void SecureLayer::log_ssl_error(const std::string& function) {
	int i;
	LOG.errorStream() << "SSL ERROR: " << function;
	while ((i = ERR_get_error())) {
		char buf[1024];
		ERR_error_string_n(i, buf, sizeof(buf));
		LOG.errorStream() << "*** " << i << ": " << buf;
	}
}

void SecureLayer::log_error(const std::string& tag, ssize_t ret_code) {
	LOG.errorStream() << "SSL[" << this->derived_s << "] Error in: " << tag << "|" << ret_code << "|" << errno ;
	if (ret_code != -1) {
		char buf[1024];
		ERR_error_string_n(ret_code, buf, sizeof(buf));
		LOG.errorStream() << "*** " << ret_code << ": " << buf;
	}
	int i;
	while ((i = ERR_get_error())) {
		char buf[1024];
		ERR_error_string_n(i, buf, sizeof(buf));
		LOG.errorStream() << "*** " << i << ": " << buf;
	}
}


void SecureLayer::init() {
	if (ctx == NULL) {
		SSL_load_error_strings();
		SSL_library_init();
		ctx = SSL_CTX_new(SSLv23_method());
		set_security("cert.pem","key.pem");
		if (SSL_CTX_set_cipher_list(SecureLayer::ctx, "ALL") <= 0) { //"RC4, AES128"
			log_ssl_error("SSL_CTX_set_cipher_list");
		}
		SSL_CTX_set_options(ctx, SSL_OP_ALL);
	}
}

ssize_t SecureLayer::do_connect() {
	int ret;
	if ((ret = SSL_connect(this->connection)) <= 0) {
		ret = SSL_get_error(this->connection, ret);
		if (ret == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE) {
			return Socket::INPROGRESS;
		} else {
			log_error("SSL_connect");
			return Socket::ERROR;
		}
	}
	return Socket::DONE;
}

ssize_t SecureLayer::do_accept() {
	int ret;
	if ((ret = SSL_accept(this->connection)) <= 0) {
		ret = SSL_get_error(this->connection, ret);
		if (ret == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE) {
			return Socket::INPROGRESS;
		} else {
			log_error("SSL_accept");
			return Socket::ERROR;
		}
	}
	return Socket::DONE;
}

ssize_t SecureLayer::do_send() {
	Buffer * buffer = get_send_queue()->get_back();
	ssize_t ret = SSL_write(this->connection, &(*buffer)[0], buffer->size());
	if (ret >= 0) {
		LOG.debugStream() << "SSL["<< this->derived_s << "]. Send:" << ret;
		get_send_queue()->compleate(ret);
		return ret;
	} else {
		ret = SSL_get_error(this->connection, ret);
		if (ret == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE) {
			return Socket::INPROGRESS;
		} else {
			log_error("SSL_write", ret);
			return Socket::ERROR;
		}
	}
	return 0;
}

ssize_t SecureLayer::do_recv() {
	Buffer * buffer = get_recv_queue()->get_front();
	ssize_t ret = SSL_read(this->connection, &(*buffer)[0], buffer->size());
	if (ret >= 0) {
		LOG.debugStream() << "SSL["<< this->derived_s << "]. Recv:" << ret;
		if (ret > 0)
			get_recv_queue()->compleate(ret);
		return ret;
	} else {
		ret = SSL_get_error(this->connection, ret);
		if (ret == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE) {
			return Socket::INPROGRESS;
		} else {
			log_error("SSL_read", ret);
			return Socket::ERROR;
		}
	}
}


