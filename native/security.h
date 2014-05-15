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

#define KEY_LENGTH  4096
#define PUB_EXP     3
#define PRINT_KEYS


class Security
{
public:
	Security() {
	    encrypted = NULL;    // Encrypted message
	    decrypted = NULL;    // Decrypted message
	}
	~Security() {
		_free();
	}

	char   *pri_key;           // Private key
    char   *pub_key;           // Public key
    char   *err;               // Buffer for any error messages
    char   *encrypted;    // Encrypted message
    char   *decrypted;    // Decrypted message
    RSA    *keypair;
    BIO    *pri;
    BIO    *pub;

	void build() {
		err = (char*)malloc(512);

	    size_t pri_len;            // Length of private key
	    size_t pub_len;            // Length of public key

	    // Generate key pair
//	    printf("Generating RSA (%d bits) keypair...", KEY_LENGTH);
//	    fflush(stdout);
	    keypair = RSA_generate_key(KEY_LENGTH, PUB_EXP, NULL, NULL);

	    // To get the C-string PEM form:
	    pri = BIO_new(BIO_s_mem());
	    pub = BIO_new(BIO_s_mem());

	    PEM_write_bio_RSAPrivateKey(pri, keypair, NULL, NULL, 0, NULL, NULL);
	    PEM_write_bio_RSA_PUBKEY(pub, keypair);

	    pri_len = BIO_pending(pri);
	    pub_len = BIO_pending(pub);

	    pri_key = (char *)malloc(pri_len + 1);
	    pub_key = (char *)malloc(pub_len + 1);

	    BIO_read(pri, pri_key, pri_len);
	    BIO_read(pub, pub_key, pub_len);

	    pri_key[pri_len] = '\0';
	    pub_key[pub_len] = '\0';

	    #ifdef PRINT_KEYS
	        //printf("\n%s\n%s\n", pri_key, pub_key);
			ofstream out("pri.pem");
			out << pri_key;
			ofstream out1("pub.pem");
			out1 << pub_key;

	    #endif
//	    printf("done.\n");
	}

	int encrypt(char *msg, int size) {
	    // Encrypt the message
	    encrypted = (char *)malloc(RSA_size(keypair));
	    int encrypt_len;
	    err = (char *)malloc(130);
	    if((encrypt_len = RSA_public_encrypt(size, (unsigned char*)msg, (unsigned char*)encrypted,
	                                         keypair, RSA_PKCS1_OAEP_PADDING)) == -1) {
	        ERR_load_crypto_strings();
	        ERR_error_string(ERR_get_error(), err);
	        fprintf(stderr, "Error encrypting message: %s\n", err);
//	        goto free_stuff;
	    }
	    return encrypt_len;
	}

	char* decrypt(char *encrypt, int encrypt_len) {
	    // Decrypt it
	    decrypted = (char *)malloc(encrypt_len);
	    if(RSA_private_decrypt(encrypt_len, (unsigned char*)encrypt, (unsigned char*)decrypted,
	                           keypair, RSA_PKCS1_OAEP_PADDING) == -1) {
	        ERR_load_crypto_strings();
	        ERR_error_string(ERR_get_error(), err);
	        fprintf(stderr, "Error decrypting message: %s\n", err);
	        //goto free_stuff;
	    }
//	    printf("Decrypted message: %s\n", decrypted);
	    return decrypted;
	}

	void _free() {
//	    free_stuff:
	    RSA_free(keypair);
	    BIO_free_all(pub);
	    BIO_free_all(pri);
	    free(pri_key);
	    free(pub_key);
	    free(encrypted);
	    free(decrypted);
	    free(err);
	}
};




#endif /* SECURITY_H_ */
