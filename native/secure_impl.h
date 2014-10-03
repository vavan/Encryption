/*
 * secure_layer.h
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */

#ifndef SECURE_LAYER_H_
#define SECURE_LAYER_H_

#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <string>
#include "queue.h"


class SecureSocket;

//TODO SecureImpl -> SecureSocket
class SecureImpl {
private:
	static SSL_CTX* ctx;
	static void init();

public:
	SSL* connection;
	BIO* bio;
	SecureSocket* parent;

	SecureImpl(SecureSocket* parent);
	virtual ~SecureImpl();
	void checkErrors(const std::string& tag);
	std::string getError(int ret_code);

	virtual ssize_t connect();
	virtual ssize_t accept();
	virtual ssize_t send();
	virtual ssize_t recv();
};



#endif /* SECURE_LAYER_H_ */
