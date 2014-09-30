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


class SecureImpl {
private:
	static SSL_CTX* ctx;
	static void init();

public:
	SSL* connection;
	BIO* bio;
	SecureImpl();
	virtual ~SecureImpl();
	void checkErrors(const std::string& tag);
	std::string getError(int ret_code);

	virtual ssize_t connect();
	virtual ssize_t accept();
	virtual ssize_t send(char* buf, size_t size);
	virtual ssize_t recv(char* buf, const size_t size);



};



#endif /* SECURE_LAYER_H_ */
