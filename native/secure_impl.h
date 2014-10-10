/*
 * secure_layer.h
 *
 *  Created on: Sep 25, 2014
 *      Author: vova
 */

#ifndef SECURE_LAYER_H_
#define SECURE_LAYER_H_

#include <string>
#include <openssl/ssl.h>

class Queue;

class SecureLayer
{
private:
	static SSL_CTX* ctx;
	static void init();
	static void log_ssl_error(const std::string& tag, int id = -1);
	static void set_security(std::string cert_file, std::string key_file);
	void log_error(const std::string& tag, ssize_t ret_code);

private:
	SSL* connection;
	int derived_s;

protected:
	virtual Queue* get_send_queue() = 0;
	virtual Queue* get_recv_queue() = 0;

public:
	SecureLayer();
	virtual ~SecureLayer();

	void set(int derived_socket);
	ssize_t do_connect();
	ssize_t do_accept();
	ssize_t do_recv();
	ssize_t do_send();
};



#endif /* SECURE_LAYER_H_ */
