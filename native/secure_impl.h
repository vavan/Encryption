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
#include "socket.h"

class Queue;

class SecureLayer
{
private:
	static SSL_CTX* ctx;
	static void init();
	static void set_security(std::string cert_file, std::string key_file);
	static void log_ssl_error(const std::string& function);
	void log_error(const std::string& tag, ssize_t ret_code = -1);

private:
	SSL* connection;
	int derived_s;

protected:
	virtual Queue* get_send_queue() = 0;
	virtual Queue* get_recv_queue() = 0;

public:
	SecureLayer(int derived_socket);
	virtual ~SecureLayer();

	Socket::RetCode do_connect();
	Socket::RetCode do_accept();
	Socket::RetCode do_recv();
	Socket::RetCode do_send();
};



#endif /* SECURE_LAYER_H_ */
