/*
 * security.cpp
 *
 *  Created on: May 15, 2014
 *      Author: vova
 */

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string.h>
#include <fstream>
#include <iostream>
#include "config.h"
#include "security.h"


#define KEY_LENGTH  4096
#define PUB_EXP     3
#define SSL_ERRORMSG_BUFFER_LENGTH 120

const char* Security::pri_file = "pri.pem";
const char* Security::pub_file = "pub.pem";


Security::Security() {
	encrypted = NULL;    // Encrypted message
	decrypted = NULL;    // Decrypted message
	err = (char *) malloc(SSL_ERRORMSG_BUFFER_LENGTH);
}

Security::~Security() {
	RSA_free(keypair);
	BIO_free_all(pub);
	BIO_free_all(pri);
	if (pri_key) free(pri_key);
	free(pub_key);
	free(encrypted);
	free(decrypted);
	free(err);
}

bool Security::keypair_exists() {
	if (access("pri.pem", F_OK) != -1) {
		if (access("pub.pem", F_OK) != -1) {
			return true;
		}
	}
	return false;
}


void Security::generate_keypair() {
	LOG << "Start to build key pair";

	size_t pri_len;            // Length of private key
	size_t pub_len;// Length of public key

	// Generate key pair
	keypair = RSA_generate_key(KEY_LENGTH, PUB_EXP, NULL, NULL);

	// To get the C-string PEM form:
	pri = BIO_new(BIO_s_mem());
	pub = BIO_new(BIO_s_mem());

	PEM_write_bio_RSAPrivateKey(pri, keypair, NULL, NULL, 0, NULL,
			NULL);
	PEM_write_bio_RSA_PUBKEY(pub, keypair);

	pri_len = BIO_pending(pri);
	pub_len = BIO_pending(pub);

	pri_key = (char *) malloc(pri_len + 1);
	pub_key = (char *) malloc(pub_len + 1);

	BIO_read(pri, pri_key, pri_len);
	BIO_read(pub, pub_key, pub_len);

	pri_key[pri_len] = '\0';
	pub_key[pub_len] = '\0';

	std::ofstream out("pri.pem");
	out << pri_key;
	std::ofstream out1("pub.pem");
	out1 << pub_key;
}

void Security::load_keypair() {
	LOG<< "Use existing key pair";
	FILE* fpri = fopen(Security::pri_file, "rb");
	FILE* fpub = fopen(Security::pub_file, "rb");

	RSA* z;
	z = PEM_read_RSAPrivateKey(fpri, &keypair, NULL, NULL);
	if (!z) {
		ERR_load_crypto_strings();
		ERR_error_string(ERR_get_error(), err);
		LOG << "***SECURIY: " << err;
	}
	pri_key = NULL;

	fseek(fpub, 0, SEEK_END);
	long fsize = ftell(fpub);
	fseek(fpub, 0, SEEK_SET);

	pub_key = (char*)malloc(fsize + 1);
	fsize = fread(pub_key, 1, fsize, fpub);
	pub_key[fsize] = '\0';
	fclose(fpub);
	fclose(fpri);
}

void Security::build() {
	if (keypair_exists()) {
		load_keypair();
	} else {
		generate_keypair();
	}
}

int Security::encrypt(char *msg, int size) {
	encrypted = (char *) malloc(RSA_size(keypair));
	int encrypt_len;
	if ((encrypt_len = RSA_public_encrypt(size, (unsigned char*) msg,
			(unsigned char*) encrypted, keypair, RSA_PKCS1_PADDING)) == -1) {
		ERR_load_crypto_strings();
		ERR_error_string(ERR_get_error(), err);
		fprintf(stderr, "Error encrypting message: %s\n", err);
	}
	return encrypt_len;
}

char* Security::decrypt(char *encrypt, int encrypt_len) {
	decrypted = (char *) malloc(encrypt_len);
	if (RSA_private_decrypt(encrypt_len, (unsigned char*) encrypt,
			(unsigned char*) decrypted, keypair, RSA_PKCS1_PADDING) == -1) {
		ERR_load_crypto_strings();
		ERR_error_string(ERR_get_error(), err);
		fprintf(stderr, "Error decrypting message: %s\n", err);
	}
	return decrypted;
}

