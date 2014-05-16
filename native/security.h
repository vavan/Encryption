/*
 * security.h
 *
 *  Created on: May 3, 2014
 *      Author: vova
 */

#ifndef SECURITY_H_
#define SECURITY_H_


#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <stdio.h>
#include <string.h>
#include "config.h"

class Security
{
public:
	Security();
	~Security();
	static const char* pri_file;
	static const char* pub_file;

	char   *pri_key;           // Private key
    char   *pub_key;           // Public key
    char   *err;               // Buffer for any error messages
    char   *encrypted;    // Encrypted message
    char   *decrypted;    // Decrypted message
    RSA    *keypair;
    BIO    *pri;
    BIO    *pub;

    bool keypair_exists();
    void generate_keypair();
    void load_keypair();

   	void build();
	int encrypt(char *msg, int size);
	char* decrypt(char *encrypt, int encrypt_len);

};




#endif /* SECURITY_H_ */
