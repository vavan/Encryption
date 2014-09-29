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


class SecureImpl {
private:
	static SSL_CTX* ctx;
	static void init();

public:
	SSL* connection;
	BIO* bio;
	SecureImpl();
	~SecureImpl();
	void checkErrors(const std::string& tag);
	std::string getError(int ret_code);
};



#endif /* SECURE_LAYER_H_ */
