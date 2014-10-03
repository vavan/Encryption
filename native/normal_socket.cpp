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
	LOG.debugStream() << "SOCKET["<< this->s << "]. New";
}

NormalSocket::NormalSocket(Addr addr, int s) :
		Socket(addr), s(s) {
	LOG.debugStream() << "SOCKET["<< this->s << "]. Copy";
}

NormalSocket::NormalSocket(const NormalSocket* socket) :
		Socket(socket->addr), s(socket->s) {
	LOG.debugStream() << "SOCKET["<< this->s << "]. Copy";
}

NormalSocket::~NormalSocket() {
	LOG.debugStream() << "SOCKET["<< this->s << "]. Delete/Close";
	::close(this->s);
}

int NormalSocket::get() {
	return this->s;
}

bool NormalSocket::connect() {
	LOG.debugStream() << "SOCKET["<< this->s << "]. Connect";
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
			LOG.errorStream() << "SOCKET["<< this->s << "]. Connect failed:" << errno;
			return false;
		}
	}
	return true;
}

bool NormalSocket::listen() {
	LOG.debugStream() << "SOCKET["<< this->s << "]. Listen";
	struct sockaddr_in serveraddr;
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(addr.port);
	//TODO Replace with input parameter
	serveraddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(s, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Bind failed:" << errno;
		return false;
	}
	if (::listen(s, LISTEN_BACKLOG) < 0) {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Listen failed:" << errno;
		return false;
	}
	return true;
}

Socket* NormalSocket::accept() {
	LOG.debugStream() << "SOCKET["<< this->s << "]. Accept";
	struct sockaddr_in clientaddr;
	unsigned int addr_size = sizeof(clientaddr);
	int newsocket = ::accept(s, (struct sockaddr *) &clientaddr, &addr_size);
	if (newsocket > 0) {
		return new NormalSocket(this->addr, newsocket);
	} else {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Accept failed:" << errno;
		return NULL;
	}
}

void NormalSocket::nonblock() {
	int rc = fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);
	if (rc < 0) {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Couldn't set non blocking mode";
	}
}

ssize_t NormalSocket::send() {
	if (!send_queue->empty()) {
		Buffer* buffer = send_queue->get_back();
		ssize_t ret = ::send(s, &(*buffer)[0], buffer->size(), 0);
		if (ret >= 0) {
			send_queue->compleate(ret);
			LOG.debugStream() << "SOCKET["<< this->s << "]. Send:" << ret;
			return ret;
		} else {
			LOG.errorStream() << "SOCKET["<< this->s << "]. Send failed:" << errno;
			return Socket::ERROR;
		}
	}
	return 0;
}

ssize_t NormalSocket::recv() {
	Buffer* buffer = recv_queue->get_front();
	ssize_t ret = ::recv(s, &(*buffer)[0], buffer->size(), 0);
	if (ret >= 0) {
		recv_queue->compleate(ret);
		LOG.debugStream() << "SOCKET["<< this->s << "]. Recv:" << ret;
		return ret;
	} else {
		LOG.errorStream() << "SOCKET["<< this->s << "]. Recv failed:" << errno;
		return Socket::ERROR;
	}
}

