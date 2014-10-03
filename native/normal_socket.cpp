/*
 * socket.cpp
 *
 *  Created on: May 7, 2014
 *      Author: vova
 */
/*
 * socket.h
 *
 *  Created on: May 5, 2014
 *      Author: vova
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include "normal_socket.h"
#include "config.h"
#include "queue.h"

#define LISTEN_BACKLOG 100

NormalSocket::NormalSocket(Addr addr) :
		Socket(addr) {
	this->s = socket(AF_INET, SOCK_STREAM, 0);
	this->nonblock();
	LOG.debugStream() << "SOCKET.New:" << this->s;
}

NormalSocket::NormalSocket(Addr addr, int s) :
		Socket(addr), s(s) {
	LOG.debugStream() << "SOCKET.Copy:" << this->s;
}

NormalSocket::NormalSocket(const NormalSocket* socket) :
		Socket(socket->addr), s(socket->s) {
	LOG.debugStream() << "SOCKET.Copy:" << this->s;
}

NormalSocket::~NormalSocket() {
	LOG.debugStream() << "SOCKET.Delete:" << this->s;
	::close(this->s);
}

int NormalSocket::get() {
	return this->s;
}

bool NormalSocket::connect() {
	LOG.debugStream() << "SOCKET.Connect:" << this->s;
	struct sockaddr_in serveraddr;
	socklen_t addr_size;

	bzero(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(addr.port);
	inet_pton(AF_INET, addr.ip.c_str(), &serveraddr.sin_addr);

	addr_size = sizeof(serveraddr);
	if (::connect(s, (struct sockaddr *) &serveraddr, addr_size) == -1) {
		int ret = errno;
		if (ret != EINPROGRESS) {
			LOG.errorStream() << "SOCKET. Connect failed:" << this->s << " errno:" << ret;
			return false;
		}
	}
	return true;
}

bool NormalSocket::listen() {
	LOG.debugStream() << "SOCKET.Listen:" << this->s;
	struct sockaddr_in serveraddr;
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(addr.port);
	//TODO Replace with input parameter
	serveraddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(s, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		LOG.errorStream() << "SOCKET. bind failed:" << this->s;
		return false;
	}
	if (::listen(s, LISTEN_BACKLOG) < 0) {
		LOG.errorStream() << "SOCKET. bind failed:" << this->s;
		return false;
	}
	return true;
}

Socket* NormalSocket::accept() {
	LOG.debugStream() << "SOCKET.Accept:" << this->s;
	struct sockaddr_in clientaddr;
	unsigned int addr_size = sizeof(clientaddr);
	int newsocket = ::accept(s, (struct sockaddr *) &clientaddr, &addr_size);
	if (newsocket > 0) {
		return new NormalSocket(this->addr, newsocket);
	} else {
		LOG.errorStream() << "SOCKET. accept failed:" << this->s;
		return NULL;
	}
}

void NormalSocket::nonblock() {
	int rc = fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);
	if (rc < 0) {
		LOG.errorStream() << "SOCKET.Couldn't set non blocking mode:" << this->s;
	}
}

ssize_t NormalSocket::send() {
	if (!send_queue->empty()) {
		Buffer * buffer = send_queue->get_back();
		ssize_t ret = ::send(s, &(*buffer)[0], buffer->size(), 0);
		if (ret >= 0) {
			send_queue->compleate(ret);
			LOG.debugStream() << "QQQQ SOCKET["<< this->s << "]. Send:" << ret;
			return ret;
		} else {
			LOG.errorStream() << "SOCKET["<< this->s << "]. Send failed:" << errno;
			return Socket::ERROR;
		}
	}
	return 0;
}

ssize_t NormalSocket::recv() {
	Buffer * buffer = recv_queue->get_front();
	ssize_t ret = ::recv(s, &(*buffer)[0], buffer->size(), 0);
	if (ret >= 0) {
		recv_queue->compleate(ret);
		LOG.debugStream() << "ZZZZ SOCKET["<< this->s << "]. Recv:" << ret;
		return ret;
	} else {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Recv failed:" << errno;
		return Socket::ERROR;
	}
}

